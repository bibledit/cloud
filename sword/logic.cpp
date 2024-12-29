/*
 Copyright (©) 2003-2025 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <sword/logic.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <database/logs.h>
#include <database/books.h>
#include <database/cache.h>
#include <database/config/general.h>
#include <database/versifications.h>
#include <sync/resources.h>
#include <tasks/logic.h>
#include <demo/logic.h>
#ifdef HAVE_SWORD
#include <swmgr.h>
#include <installmgr.h>
#include <filemgr.h>
#include <swmodule.h>
#endif
#include <developer/logic.h>
#include <database/logic.h>


std::mutex sword_logic_installer_mutex {};
bool sword_logic_installing_module {false};
#ifdef HAVE_SWORD
std::mutex sword_logic_library_access_mutex {};
#endif
std::mutex sword_logic_diatheke_run_mutex {};


std::string sword_logic_get_path ()
{
  std::string sword_path {"."};
  char * home = getenv ("HOME");
  if (home) sword_path = home;
  sword_path.append ("/.sword/InstallMgr");
  return sword_path;
}


void sword_logic_refresh_module_list ()
{
  Database_Logs::log ("Refreshing list of SWORD modules");
  
  std::string out_err {};
  
  // Initialize SWORD directory structure and configuration.
  std::string sword_path = sword_logic_get_path ();
  filter_url_mkdir (sword_path);
  std::string swordconf = "[Install]\n"
                     "DataPath=" + sword_path + "/\n";
  filter_url_file_put_contents (filter_url_create_path ({sword_path, "sword.conf"}), swordconf);
  std::string config_files_path = filter_url_create_root_path ({"sword"});
  filter::shell::run ("cp -r " + config_files_path + "/locales.d " + sword_path, out_err);
  sword_logic_log (out_err);
  filter::shell::run ("cp -r " + config_files_path + "/mods.d " + sword_path, out_err);
  sword_logic_log (out_err);
  
  // Initialize basic user configuration.
#ifdef HAVE_SWORD
  sword_logic_installmgr_initialize ();
#else
  filter::shell::run (std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " --allow-internet-access-and-risk-tracing-and-jail-or-martyrdom --allow-unverified-tls-peer -init", out_err);
  sword_logic_log (out_err);
#endif
  
  // Sync the configuration with the online known remote repository list.
#ifdef HAVE_SWORD
  if (!sword_logic_installmgr_synchronize_configuration_with_master ()) {
    Database_Logs::log ("Failed to synchronize SWORD configuration with the master remote source list");
    // Since this could be a network failure, exit from the entire update routine.
    // The advantage of existing already at this stage is that the list of known SWORD resources
    // will be left untouched in case of a network error.
    return;
  }
#else
  filter::shell::run (std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " --allow-internet-access-and-risk-tracing-and-jail-or-martyrdom --allow-unverified-tls-peer -sc", out_err);
  filter::strings::replace_between (out_err, "WARNING", "enable? [no]", "");
  sword_logic_log (out_err);
#endif
  
  // List the remote sources.
  std::vector <std::string> remote_sources {};
#ifdef HAVE_SWORD
  sword_logic_installmgr_list_remote_sources (remote_sources);
#else
  filter::shell::run (std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " -s", out_err);
  sword_logic_log (out_err);
  std::vector <std::string> lines = filter::strings::explode (out_err, '\n');
  for (auto line : lines) {
    line = filter::strings::trim (line);
    if (line.find ("[") != std::string::npos) {
      line.erase (0, 1);
      if (line.find ("]") != std::string::npos) {
        line.erase (line.length () - 1, 1);
        remote_sources.push_back (line);
        Database_Logs::log (line);
      }
    }
  }
#endif
  
  std::vector <std::string> sword_modules {};
  
  for (const auto& remote_source : remote_sources) {
    
#ifdef HAVE_SWORD
    if (!sword_logic_installmgr_refresh_remote_source (remote_source)) {
      Database_Logs::log ("Error refreshing remote source " + remote_source);
    }
#else
    filter::shell::run (std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " --allow-internet-access-and-risk-tracing-and-jail-or-martyrdom --allow-unverified-tls-peer -r \"" + remote_source + "\"", out_err);
    filter::strings::replace_between (out_err, "WARNING", "type yes at the prompt", "");
    sword_logic_log (out_err);
#endif

    std::vector <std::string> modules {};
#ifdef HAVE_SWORD
    sword_logic_installmgr_list_remote_modules (remote_source, modules);
    for (const auto& module : modules) {
      sword_modules.push_back ("[" + remote_source + "]" + " " + module);
    }
#else
    filter::shell::run (std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " -rl \"" + remote_source + "\"", out_err);
    lines = filter::strings::explode (out_err, '\n');
    for (auto line : lines) {
      line = filter::strings::trim (line);
      if (line.empty ()) continue;
      if (line.find ("[") == std::string::npos) continue;
      if (line.find ("]") == std::string::npos) continue;
      modules.push_back ("[" + remote_source + "]" + " " + line);
    }
    for (const auto& module : modules) {
      sword_modules.push_back (module);
    }
#endif
    Database_Logs::log (remote_source + ": " + std::to_string (modules.size ()) + " modules");
  }
  
  // Store the list of remote sources and their modules.
  // It is stored in the client files area.
  // Clients can access it from there too.
  std::string path = sword_logic_module_list_path ();
  filter_url_file_put_contents (path, filter::strings::implode (sword_modules, "\n"));
  
  Database_Logs::log ("Ready refreshing SWORD module list");
}


std::string sword_logic_module_list_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "sword_modules.txt"});
}


// Gets the name of the remote source of the $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_source (std::string line)
{
  if (line.length () < 10) return std::string();
  line.erase (0, 1);
  size_t pos = line.find ("]");
  if (pos == std::string::npos) return std::string();
  line.erase (pos);
  return line;
}


// Gets the module name of the $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_remote_module (std::string line)
{
  if (line.length () < 10) return std::string();
  line.erase (0, 2);
  if (line.length () < 10) return std::string();
  size_t pos = line.find ("[");
  if (pos == std::string::npos) return std::string();
  line.erase (0, pos + 1);
  pos = line.find ("]");
  if (pos == std::string::npos) return std::string();
  line.erase (pos);
  return line;
}


// Gets the module name of the $line like this:
// [Shona]  (1.1)  - Shona Bible
std::string sword_logic_get_installed_module (std::string line)
{
  line = filter::strings::trim (line);
  if (line.length () > 10) {
    line.erase (0, 1);
    size_t pos = line.find ("]");
    if (pos != std::string::npos) line.erase (pos);
  }
  return line;
}


// Gets the version number of a module of the $line like this:
// [Shona]  (1.1)  - Shona Bible
std::string sword_logic_get_version (std::string line)
{
  line = filter::strings::trim (line);
  if (line.length () > 10) {
    line.erase (0, 3);
  }
  if (line.length () > 10) {
    size_t pos = line.find ("(");
    if (pos != std::string::npos) line.erase (0, pos + 1);
    pos = line.find (")");
    if (pos != std::string::npos) line.erase (pos);
  }
  return line;
}


// Gets the human-readable name of a $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_name (std::string line)
{
  std::vector <std::string> bits = filter::strings::explode (line, '-');
  if (bits.size () >= 2) {
    bits.erase (bits.begin ());
  }
  line = filter::strings::implode (bits, "-");
  line = filter::strings::trim (line);
  return line;
}


// Schedule SWORD module installation.
void sword_logic_install_module_schedule (const std::string& source, const std::string& module)
{
  // No source: Done.
  if (source.empty ()) return;
  
  // No module: Done.
  // There have been cases with more than 6000 scheduled SWORD module installation tasks,
  // all trying to install an empty $module.
  // So it's important to check on that.
  if (module.empty ()) return;
  
  // Check whether the module installation has been scheduled already.
  if (tasks_logic_queued (task::install_sword_module, {source, module})) return;
  
  // Schedule it.
  tasks_logic_queue (task::install_sword_module, {source, module});
}


void sword_logic_install_module (const std::string& source_name, const std::string& module_name)
{
  Database_Logs::log ("Install SWORD module " + module_name + " from source " + source_name);
  std::string sword_path {sword_logic_get_path ()};

  // Installation through SWORD InstallMgr does not yet work.
  // When running it from the ~/.sword/InstallMgr directory, it works.
#ifdef HAVE_SWORD
  
  sword::SWMgr *mgr = new sword::SWMgr();
  
  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);
  
  sword::InstallSourceMap::iterator source = installMgr->sources.find(source_name.c_str ());
  if (source == installMgr->sources.end()) {
    Database_Logs::log ("Could not find remote source " + source_name);
  } else {
    sword::InstallSource *is = source->second;
    sword::SWMgr *rmgr = is->getMgr();
    sword::SWModule *module;
    sword::ModMap::iterator it = rmgr->Modules.find(module_name.c_str());
    if (it == rmgr->Modules.end()) {
      Database_Logs::log ("Remote source " + source_name + " does not make available module " + module_name);
    } else {
      module = it->second;
      int error = installMgr->installModule(mgr, 0, module->getName(), is);
      if (error) {
        Database_Logs::log ("Error installing module " + module_name);
      } else {
        Database_Logs::log ("Installed module " + module_name);
      }
    }
  }

  delete installMgr;
  delete mgr;
  
#else
  
  std::string out_err {};
  std::string command = "cd " + sword_path + "; " + std::string(filter::shell::get_executable(filter::shell::Executable::installmgr))+ " --allow-internet-access-and-risk-tracing-and-jail-or-martyrdom --allow-unverified-tls-peer -ri \"" + source_name + "\" \"" + module_name + "\"";
  Database_Logs::log (command);
  filter::shell::run (command, out_err);
  sword_logic_log (out_err);
  
#endif

  // After the installation is complete, write some temporal some data.
  // This temporal data indicates the last access time for this SWORD module.
  {
    const std::string path = sword_logic_access_tracker (module_name);
    filter_url_file_put_contents (path, "SWORD");
  }
}


void sword_logic_uninstall_module (const std::string& module)
{
  Database_Logs::log ("Uninstall SWORD module " + module);
  std::string out_err;
  const std::string sword_path {sword_logic_get_path ()};
  filter::shell::run ("cd " + sword_path + "; " + std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " -u \"" + module + "\"", out_err);
  sword_logic_log (out_err);
}


// Get available SWORD modules.
std::vector <std::string> sword_logic_get_available ()
{
  const std::string contents = filter_url_file_get_contents (sword_logic_module_list_path ());
  return filter::strings::explode (contents, '\n');
}


// Get installed SWORD modules.
std::vector <std::string> sword_logic_get_installed ()
{
  std::vector <std::string> modules {};
  std::string out_err {};
  const std::string sword_path {sword_logic_get_path ()};
  filter::shell::run ("cd " + sword_path + "; " + std::string(filter::shell::get_executable(filter::shell::Executable::installmgr)) + " -l", out_err);
  std::vector <std::string> lines = filter::strings::explode (out_err, '\n');
  for (auto line : lines) {
    line = filter::strings::trim (line);
    if (line.empty ()) continue;
    if (line.find ("[") == std::string::npos) continue;
    modules.push_back (line);
  }
  return modules;
}


std::string sword_logic_get_text (const std::string& source, const std::string& module, const int book, const int chapter, const int verse)
{
#ifdef HAVE_CLIENT

  // The resource name consists of source and module, e.g. [CrossWire][NET].
  std::string resource = sword_logic_get_resource_name (source, module);

  // Client checks for and optionally creates the cache for this SWORD source/module.
  if (!database::cache::sql::exists (resource, book)) {
    database::cache::sql::create (resource, book);
  }

  // If this module/passage exists in the cache, return it (it updates the access days in the cache).
  if (database::cache::sql::exists (resource, book, chapter, verse)) {
    return database::cache::sql::retrieve (resource, book, chapter, verse);
  }

  // Fetch this SWORD resource from the server.
  std::string address = database::config::general::get_server_address ();
  int port = database::config::general::get_server_port ();
  if (!client_logic_client_enabled ()) {
    // If the client has not been connected to a cloud instance,
    // fetch the SWORD content from the Bibledit Cloud demo.
    address = demo_address ();
    port = demo_port ();
  }
  std::string url = client_logic_url (address, port, sync_resources_url ());
  url = filter_url_build_http_query (url, "r", resource);
  url = filter_url_build_http_query (url, "b", std::to_string(book));
  url = filter_url_build_http_query (url, "c", std::to_string(chapter));
  url = filter_url_build_http_query (url, "v", std::to_string(verse));
  std::string error {};
  std::string html = filter_url_http_get (url, error, true);
  
  // In case of an error, don't cache that error, but let the user see it.
  if (!error.empty ()) return error;

  // Client caches this info for later.
  // Except in case of predefined responses from the Cloud.
  if (html != sword_logic_installing_module_text ()) {
    if (html != sword_logic_fetch_failure_text ()) {
      database::cache::sql::cache (resource, book, chapter, verse, html);
    }
  }
  
  return html;
  
#else

  std::string module_text;
  bool module_available {false};

  const std::string osis = database::books::get_osis_from_id (static_cast<book_id>(book));
  const std::string chapter_verse = std::to_string (chapter) + ":" + std::to_string (verse);

  // See notes on function sword_logic_diatheke
  // for why it is not currently fetching content via a SWORD library call.
  // module_text = sword_logic_diatheke (module, osis, chapter, verse, module_available);
  
  // Running diatheke only works when it runs in the SWORD installation directory.
  const std::string sword_path = sword_logic_get_path ();
  // Running several instances of diatheke simultaneously fails.
  sword_logic_diatheke_run_mutex.lock ();
  // The server fetches the module text as follows:
  // diatheke -b KJV -k Jn 3:16
  // To included, run this instead: $ diatheke -b KJV -o n -k Jn 3:16
  std::vector <std::string> parameters {"-b", module};
  parameters.push_back("-o");
  std::string module_options {};
  if (database::config::general::get_keep_osis_content_in_sword_resources ()) {
    module_options.append("n");
  }
  module_options.append("cvapr"); // Hebrew cantillation / Hebrew vowels / Greek accents / Arabic vowels / Arabic shaping.
  parameters.push_back(module_options);
  parameters.push_back("-k");
  parameters.push_back(osis);
  parameters.push_back(chapter_verse);
  std::string error {};
  const int result = filter::shell::run (sword_path, std::string(filter::shell::get_executable(filter::shell::Executable::diatheke)), parameters, &module_text, &error);
  module_text.append (error);
  sword_logic_diatheke_run_mutex.unlock ();
  if (result != 0) return sword_logic_fetch_failure_text ();
  
  // Touch the temporal file
  // so the server knows that the module has been accessed just now
  // and won't uninstall it too soon.
  {
    const std::string path = sword_logic_access_tracker (module);
    filter_url_file_put_contents (path, "access");
  }

  // If the module has not been installed, the output of "diatheke" will be empty.
  // If the module was installed, but the requested passage is out of range,
  // the output of "diatheke" contains the module name, so it won't be empty.
  module_available = !module_text.empty ();
  
  if (!module_available) {
    
    // Check whether the SWORD module exists.
    std::vector <std::string> modules {sword_logic_get_available ()};
    const std::string smodules = filter::strings::implode (modules, std::string());
    if (smodules.find ("[" + module + "]") != std::string::npos) {
      // Schedule SWORD module installation.
      // (It used to be the case that this function, to get the text,
      // would wait till the SWORD module was installed, and then after installation,
      // return the text from that module.
      // But due to long waiting on Bibledit demo, while it would install multiple modules,
      // the Bibledit demo would become unresponsive.
      // So, it's better to return immediately with an informative text.)
      sword_logic_install_module_schedule (source, module);
      // Return standard 'installing' information. Client knows not to cache this.
      return sword_logic_installing_module_text ();
    } else {
      return "Cannot find SWORD module " + module;
    }
  }
  
  // Clean it up.
  module_text = sword_logic_clean_verse (module, chapter, verse, module_text);
  
  return module_text;

#endif
}


std::map <int, std::string> sword_logic_get_bulk_text (const std::string& module, const int book, const int chapter, const std::vector <int>& verses)
{
  // Touch the cache so the server knows that the module has been accessed and won't uninstall it too soon.
  {
    const std::string path = sword_logic_access_tracker (module);
    filter_url_file_put_contents (path, "bulk");
  }

  // The name of the book to pass to diatheke.
  const std::string osis = database::books::get_osis_from_id (static_cast<book_id>(book));

  // Cannot run more than one "diatheke" per user, so use a mutex for that.
  sword_logic_diatheke_run_mutex.lock ();

  // Here is how to speed up SWORD text retrieval.
  // The main point is to not pass just one verse,
  // but to pass the chapter number without the verse.
  // So it returns the entire chapter in one go.
  // One would even be able to pass the book only, so it returns the entire book.
  // But that would not add much to increasing the speed.
  // cd ~/.sword/InstallMgr
  // diatheke -b AB -k Ezra 5:1
  // diatheke -b AB -k Ezra 5
  // diatheke -b AB -k Ezra
  std::string error {};
  std::string bulk_text {};
  const int result = filter::shell::run (sword_logic_get_path (), std::string(filter::shell::get_executable(filter::shell::Executable::diatheke)), { "-b", module, "-k", osis, std::to_string (chapter) }, &bulk_text, &error);
  bulk_text.append (error);
  if (result != 0) Database_Logs::log (error);
  // This is how the output would look.
  // Malachi 3:1: <verse osisID="Mal.3.1">Behold, I send forth My messenger, and he shall survey the way before Me: and the Lord, whom you seek, shall suddenly come into His temple, even the Messenger of the covenant, whom you take pleasure in: behold, He is coming, says the Lord Almighty.

  sword_logic_diatheke_run_mutex.unlock ();

  // Resulting verse text.
  std::map <int, std::string> output {};

  // Iterate over all requested verses to extract the correct content from the chapter.
  // This works well in general.
  // It has been seen in a sample module, the "AB", that some verses in the SWORD module were empty.
  // In case of such verses, there's no content to extract from the chapter.
  // The cause in such verses is in the module builder.
  for (const auto verse : verses) {
    const std::string starter = " " + std::to_string(chapter) + ":" + std::to_string(verse) + ":";
    size_t pos1 = bulk_text.find (starter);
    if (pos1 == std::string::npos) {
      //Database_Logs::log("Cannot find starter: |" + starter + "|");
      continue;
    }
    const std::string finisher = "\n";
    size_t pos2 = bulk_text.find (finisher, pos1);
    if (pos2 == std::string::npos) pos2 = bulk_text.length() + 1;
    pos1 += starter.length ();
    std::string text = bulk_text.substr (pos1, pos2 - pos1);
    text = sword_logic_clean_verse (module, chapter, verse, text);
    output [verse] = text;
  }
  
  // Done.
  return output;
}


// Checks the installed modules, whether they need to be updated.
void sword_logic_update_installed_modules ()
{
  Database_Logs::log ("Updating installed SWORD modules");

  std::vector <std::string> available_modules = sword_logic_get_available ();

  std::vector <std::string> installed_modules = sword_logic_get_installed ();
  for (const auto& installed_module : installed_modules) {
    const std::string module = sword_logic_get_installed_module (installed_module);
    const std::string installed_version = sword_logic_get_version (installed_module);
    for (const auto& available_module : available_modules) {
      if (sword_logic_get_remote_module (available_module) == module) {
        if (sword_logic_get_version (available_module) != installed_version) {
          const std::string source = sword_logic_get_source (available_module);
          // Uninstall module.
          sword_logic_uninstall_module (module);
          // Schedule module installation.
          sword_logic_install_module_schedule (source, module);
        }
        continue;
      }
    }
  }
  
  Database_Logs::log ("Ready updating installed SWORD modules");
}


// Trims the installed SWORD modules.
void sword_logic_trim_modules ()
{
#ifndef HAVE_CLIENT
  Database_Logs::log ("Trimming the installed SWORD modules");
  const std::vector <std::string> modules = sword_logic_get_installed ();
  for (auto module : modules) {
    module = sword_logic_get_installed_module (module);
    const std::string path = sword_logic_access_tracker (module);
    if (!file_or_dir_exists (path)) {
      sword_logic_uninstall_module (module);
    }
  }
  Database_Logs::log ("Ready trimming the SWORD caches and modules");
#endif
}


// Text saying that the Cloud will install the requested SWORD module.
// Client knows not to cache this.
std::string sword_logic_installing_module_text ()
{
  return "The requested SWORD module is not yet installed. Bibledit Cloud will install it shortly. Please check back after a few minutes.";
}


// Text stating fetch failure.
// Client knows not to cache this.
std::string sword_logic_fetch_failure_text ()
{
  return "Failure to fetch SWORD content.";
}


// Tracker for accessing the SWORD module.
std::string sword_logic_access_tracker (const std::string& module)
{
  const std::string path = filter_url_create_root_path ({filter_url_temp_dir (), "sword_access_tracker_" + module});
  return path;
}


// The functions runs a scheduled module installation.
// The purpose of this function is that only one module installation occurs at a time,
// rather than simultaneously installing modules, which clogs the system.
void sword_logic_run_scheduled_module_install (const std::string& source, const std::string& module)
{
  // If a module is being installed,
  // and a call is made for another module installation,
  // re-schedule this module installation to postpone it,
  // till after this one is ready.
  sword_logic_installer_mutex.lock ();
  const bool installing = sword_logic_installing_module;
  sword_logic_installer_mutex.unlock ();
  if (installing) {
    sword_logic_install_module_schedule (source, module);
    return;
  }

  // Set flag for current module installation running.
  sword_logic_installer_mutex.lock ();
  sword_logic_installing_module = true;
  sword_logic_installer_mutex.unlock ();

  // Run the installer if the module is not yet installed.
  const std::vector <std::string> modules {sword_logic_get_installed ()};
  bool already_installed = false;
  for (const auto& installed_module : modules) {
    if (installed_module.find ("[" + module + "]") != std::string::npos) {
      already_installed = true;
    }
  }
  if (!already_installed) {
    sword_logic_install_module (source, module);
  }
  
  // Clear flag as current module installation is ready.
  sword_logic_installer_mutex.lock ();
  sword_logic_installing_module = false;
  sword_logic_installer_mutex.unlock ();
}


void sword_logic_installmgr_initialize ()
{
#ifdef HAVE_SWORD
  sword::SWMgr *mgr = new sword::SWMgr();
  if (!mgr->config) Database_Logs::log ("ERROR: Please configure SWORD first.");

  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);
  
  sword::SWBuf confPath = baseDir + "/InstallMgr.conf";
  sword::FileMgr::createParent (confPath.c_str());
  remove(confPath.c_str());
  
  sword::InstallSource is("FTP");
  is.caption = "CrossWire";
  is.source = "ftp.crosswire.org";
  is.directory = "/pub/sword/raw";
  
  sword::SWConfig config(confPath.c_str());
  config["General"]["PassiveFTP"] = "true";
  config["Sources"]["FTPSource"] = is.getConfEnt();
  config.Save();

  delete installMgr;
  delete mgr;
#endif
}


bool sword_logic_installmgr_synchronize_configuration_with_master ()
{
  bool success = true;
#ifdef HAVE_SWORD
  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);
  
  if (installMgr->refreshRemoteSourceConfiguration()) {
    success = false;
  }
  
  delete installMgr;
#endif
  return success;
}


void sword_logic_installmgr_list_remote_sources ([[maybe_unused]] const std::vector <std::string>& sources)
{
#ifdef HAVE_SWORD
  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);
  
  for (sword::InstallSourceMap::iterator it = installMgr->sources.begin(); it != installMgr->sources.end(); it++) {
    const std::string caption (it->second->caption);
    sources.push_back (caption);
    /*
    std::string description;
    description.append (caption);
    description.append (" - ");
    description.append (it->second->type);
    description.append (" - ");
    description.append (it->second->source);
    description.append (" - ");
    description.append (it->second->directory);
    Database_Logs::log (description);
    */
  }
  
  delete installMgr;
#endif
}


bool sword_logic_installmgr_refresh_remote_source ([[maybe_unused]] const std::string& name)
{
  bool success = true;
#ifdef HAVE_SWORD
  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);

  sword::InstallSourceMap::iterator source = installMgr->sources.find(name.c_str ());
  if (source == installMgr->sources.end()) {
    Database_Logs::log ("Could not find remote source " + name);
  } else {
    if (installMgr->refreshRemoteSource(source->second)) {
      success = false;
    }
  }
  
  delete installMgr;
#endif
  return success;
}


void sword_logic_installmgr_list_remote_modules ([[maybe_unused]] const std::string& source_name,
                                                 [[maybe_unused]] std::vector <std::string>& modules)
{
#ifdef HAVE_SWORD
  sword::SWMgr *mgr = new sword::SWMgr();
  
  sword::SWBuf baseDir = sword_logic_get_path ().c_str ();
  
  sword::InstallMgr *installMgr = new sword::InstallMgr (baseDir, NULL);
  installMgr->setUserDisclaimerConfirmed (true);
  
  sword::InstallSourceMap::iterator source = installMgr->sources.find(source_name.c_str ());
  if (source == installMgr->sources.end()) {
    Database_Logs::log ("Could not find remote source " + source_name);
  } else {
    sword::SWMgr *otherMgr = source->second->getMgr();
    sword::SWModule *module;
    if (!otherMgr) otherMgr = mgr;
    std::map<sword::SWModule *, int> mods = sword::InstallMgr::getModuleStatus(*mgr, *otherMgr);
    for (std::map<sword::SWModule *, int>::iterator it = mods.begin(); it != mods.end(); it++) {
      module = it->first;
      sword::SWBuf version = module->getConfigEntry("Version");
      sword::SWBuf status = " ";
      if (it->second & sword::InstallMgr::MODSTAT_NEW) status = "*";
      if (it->second & sword::InstallMgr::MODSTAT_OLDER) status = "-";
      if (it->second & sword::InstallMgr::MODSTAT_UPDATED) status = "+";
      std::string module_name (status);
      module_name.append ("[");
      module_name.append (module->getName());
      module_name.append ("]  \t(");
      module_name.append (version);
      module_name.append (")  \t- ");
      module_name.append (module->getDescription());
      // Check if the module is a verse-based Bible or commentary.
      bool verse_based = false;
      std::string module_type = module->getType ();
      if (module_type == "Biblical Texts") verse_based = true;
      if (module_type == "Commentaries") verse_based = true;
      if (verse_based) modules.push_back (module_name);
    }
  }
  
  delete installMgr;
  delete mgr;
#endif
}

/*
 This function works, but there are cases where it crashes as follows:
 
 libsword.so.11v5(_ZN5sword7FileMgr7sysOpenEPNS_8FileDescE+0x39)
 libsword.so.11v5(_ZN5sword8FileDesc5getFdEv+0x20)
 libsword.so.11v5(_ZN5sword8SWConfig4LoadEv+0x16c)
 libsword.so.11v5(_ZN5sword8SWConfigC2EPKc+0xcb)
 libsword.so.11v5(_ZN5sword5SWMgr13loadConfigDirEPKc+0x1af)
 libsword.so.11v5(_ZN5sword5SWMgr4LoadEv+0x201)
 libsword.so.11v5(_ZN5sword5SWMgrC1EPKcbPNS_11SWFilterMgrEbb+0x325)

 And this crash takes down the whole Bibledit Cloud instance.
 */
std::string sword_logic_diatheke ([[maybe_unused]] const std::string& module_name,
                                  [[maybe_unused]] const std::string& osis,
                                  [[maybe_unused]] int chapter,
                                  [[maybe_unused]] int verse,
                                  [[maybe_unused]] bool& available)
{
  std::string rendering {};
#ifdef HAVE_SWORD
  // When accessing the SWORD library from multiple threads simultaneously, the library often crashes.
  // A mutex fixes this behaviour.
  sword_logic_library_access_mutex.lock ();
  
  // The SWORD manager should be pointed to the path of the library, in order to work.
  sword::SWMgr manager (sword_logic_get_path ().c_str ());
  
  manager.setGlobalOption("Footnotes", "Off");
  manager.setGlobalOption("Headings", "Off");
  manager.setGlobalOption("Strong's Numbers", "Off");
  manager.setGlobalOption("Morphological Tags", "Off");
  manager.setGlobalOption("Hebrew Cantillation", "On");
  manager.setGlobalOption("Hebrew Vowel Points", "On");
  manager.setGlobalOption("Greek Accents", "On");
  manager.setGlobalOption("Lemmas", "Off");
  manager.setGlobalOption("Cross-references", "Off");
  manager.setGlobalOption("Words of Christ in Red", "Off");
  manager.setGlobalOption("Arabic Vowel Points", "On");
  manager.setGlobalOption("Glosses", "Off");
  manager.setGlobalOption("Transliterated Forms", "Off");
  manager.setGlobalOption("Enumerations", "Off");
  manager.setGlobalOption("Transliteration", "Off");
  manager.setGlobalOption("Textual Variants", "All Readings");
  //manager.setGlobalOption("Textual Variants", "Secondary Reading");
  //manager.setGlobalOption("Textual Variants", "Primary Reading");
  
  sword::SWModule *module = manager.getModule (module_name.c_str ());
  available = module;
  if (module) {
    std::string key = osis + " " + filter::strings::convert_to_string (chapter) + ":" + filter::strings::convert_to_string (verse);
    module->setKey (key.c_str ());
    rendering = module->renderText();
  }
  sword_logic_library_access_mutex.unlock ();
#endif
  
  return rendering;
}


void sword_logic_log (std::string message)
{
  // Remove less comely stuff, warnings, confusing information.
  message = filter::strings::replace ("-=+*", "", message);
  message = filter::strings::replace ("WARNING", "", message);
  message = filter::strings::replace ("*+=-", "", message);
  message = filter::strings::replace ("enable?", "", message);
  message = filter::strings::replace ("[no]", "", message);
  // Clean message up.
  message = filter::strings::trim (message);
  // Record in the journal.
  Database_Logs::log (message);
}


std::string sword_logic_clean_verse (const std::string& module, int chapter, int verse, std::string text)
{
  // Remove any OSIS elements or make those elements displayable.
  if (database::config::general::get_keep_osis_content_in_sword_resources ()) {
    text = filter::strings::escape_special_xml_characters (text);
  } else {
    filter::strings::replace_between (text, "<", ">", "");
  }
  
  // Remove the passage name that diatheke adds.
  // A reliable signature for this is the chapter and verse plus subsequent colon.
  const std::string chapter_verse = std::to_string (chapter) + ":" + std::to_string (verse);
  size_t pos = text.find (" " + chapter_verse + ":");
  if (pos != std::string::npos) {
    pos += 2;
    pos += chapter_verse.size ();
    text.erase (0, pos);
  }
  
  // Remove the module name that diatheke adds.
  text = filter::strings::replace ("(" + module + ")", "", text);
  
  // Clean whitespace away.
  text = filter::strings::trim (text);

  // Done.
  return text;
}


// Take the SWORD $source and SWORD $module and form it into a canonical resource name.
std::string sword_logic_get_resource_name (const std::string& source, const std::string& module)
{
  return "[" + source + "][" + module + "]";
}

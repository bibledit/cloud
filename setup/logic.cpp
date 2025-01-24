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


#include <setup/logic.h>
#include <webserver/request.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/localization.h>
#include <database/confirm.h>
#include <database/jobs.h>
#include <database/sprint.h>
#include <database/mail.h>
#include <database/navigation.h>
#include <database/mappings.h>
#include <database/noteactions.h>
#include <database/bibleactions.h>
#include <database/versifications.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <database/state.h>
#include <database/login.h>
#include <database/privileges.h>
#include <database/git.h>
#include <database/statistics.h>
#include <database/check.h>
#include <styles/sheets.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/date.h>
#include <config/globals.h>
#include <demo/logic.h>
#include <locale/logic.h>
#include <tasks/logic.h>
#include <database/logic.h>


void setup_conditionally (const char * package)
{
  std::string p (package);
  
  // When the package folder is the same as the document root folder,
  // it may mean that another program installs the data for us.
  // This is the case on Android and on iOS.
  // In that case, wait till the most important data has been installed.
  if (p == config_globals_document_root) setup_wait_till_main_folders_present ();
  
  // Run the setup if the versions differ.
  if (config::logic::version () != database::config::general::getInstalledDatabaseVersion ()) {
    
    std::vector <std::string> messages {};

    // Copy the library into the destination place.
    if (p != config_globals_document_root) {
      messages.push_back ("Copy data from " + p + " to " + config_globals_document_root);
      setup_copy_library (package);
    }
    
    // Ensure write access to certain folders.
    setup_write_access ();
    
    // Create or upgrade the databases and other data.
    messages.push_back ("Initializing and upgrading data");
    setup_initialize_data ();
    
    for (const auto& message : messages) {
      Database_Logs::log (message);
    }
    
#ifndef HAVE_CLIENT
    // Cloud updates the available SWORD modules and web resources.
    tasks_logic_queue (task::refresh_sword_modules);
    tasks_logic_queue (task::refresh_web_resources);
#endif
    
    // Update installed version.
    database::config::general::setInstalledDatabaseVersion (config::logic::version ());
  };

  if (config::logic::version () != database::config::general::get_installed_interface_version ()) {
    
    // In client mode or in demo mode do not display the page for entering the admin's details.
#ifdef HAVE_CLIENT
    setup_complete_gui ();
#endif
    if (config::logic::demo_enabled ()) setup_complete_gui ();
    
    // When the admin's credentials are configured, enter them, and do not display them in the setup page.
    if (!config::logic::admin_username ().empty ()) {
      if (!config::logic::admin_password ().empty ()) {
        setup_set_admin_details (config::logic::admin_username (),
                                 config::logic::admin_password (),
                                 config::logic::admin_email ());
        setup_complete_gui ();
      }
    }
    
  }

  config_globals_data_initialized = true;
  
  // Run the indexers if a flag was set for it.
  // This mechanism is suitable for low power devices as Android and iOS.
  // If Bibles or Notes are scheduled to be indexed, since the tasks take a lot of time,
  // the app may shut down before the tasks have been completed.
  // Next time the app starts, the tasks will be restarted here, and they will run if a flag was set for them.
  // Once the tasks are really complete, they will clear the flag.
  tasks_logic_queue (task::reindex_bibles);
  tasks_logic_queue (task::reindex_notes);
#ifdef HAVE_CLIENT
  // Same for the resource downloader, for the client.
  tasks_logic_queue (task::sync_resources);
#endif
}


// Copies the library from $package to the webroot.
void setup_copy_library (const char * package)
{
  size_t package_length = strlen (package);
  filter_url_mkdir (config_globals_document_root);
  config_globals_setup_message = "scanning";
  std::vector <std::string> package_paths;
  filter_url_recursive_scandir (package, package_paths);
  for (const auto& package_path : package_paths) {
    const std::string dest_path = config_globals_document_root + package_path.substr (package_length);
    config_globals_setup_message = dest_path;
    if (filter_url_is_dir (package_path)) {
      filter_url_mkdir (dest_path);
    } else {
      filter_url_file_cp (package_path, dest_path);
    }
  }
}


void setup_write_access ()
{
  std::vector <std::string> folders = {"exports", "git", "revisions", "dyncss", database_logic_databases (), "bibles", "fonts", "logbook", filter_url_temp_dir ()};
  for (const auto& folder : folders) {
    const std::string path = filter_url_create_root_path ({folder});
    if (!filter_url_get_write_permission (path)) {
      filter_url_set_write_permission (path);
    }
  }
}


// Waits until the main folders for setup are present.
void setup_wait_till_main_folders_present ()
{
  bool present {true};
  do {
    present = true;
    std::vector <std::string> folders = {"dyncss", database_logic_databases (), "databases/config/general", "logbook", "bibles", "processes"};
    for (const auto & folder : folders) {
      const std::string path = filter_url_create_root_path ({folder});
      if (!file_or_dir_exists (path)) {
        present = false;
      }
    }
    if (!present)
      std::this_thread::sleep_for (std::chrono::milliseconds (300));
  } while (!present);
}


void setup_initialize_data ()
{
  // Do the database setup.
  Webserver_Request webserver_request;
  // Display progress in text format.
  // That provides feedback to the user during installation.
  // This alerts the user that installation is in progress, and is not stuck,
  // as the user might think when the install takes longer than expected.
  config_globals_setup_message = "users";
  webserver_request.database_users ()->create ();
  webserver_request.database_users ()->upgrade ();
  config_globals_setup_message = "styles";
  database::styles::create_database ();
  config_globals_setup_message = "bible actions";
  database::bible_actions::create ();
  config_globals_setup_message = "checks";
  database::check::create ();
  setup_generate_locale_databases (false);
#ifdef HAVE_CLOUD
  config_globals_setup_message = "confirmations";
  database::confirm::create ();
  database::confirm::upgrade();
#endif
  config_globals_setup_message = "jobs";
  Database_Jobs database_jobs = Database_Jobs ();
  database_jobs.create ();
#ifdef HAVE_CLOUD
  config_globals_setup_message = "sprint";
  Database_Sprint database_sprint = Database_Sprint ();
  database_sprint.create ();
#endif
  config_globals_setup_message = "mail";
  Database_Mail database_mail (webserver_request);
  database_mail.create ();
  config_globals_setup_message = "navigation";
  Database_Navigation database_navigation = Database_Navigation ();
  database_navigation.create ();
  config_globals_setup_message = "mappings";
  setup_generate_verse_mapping_databases ();
  config_globals_setup_message = "note actions";
  Database_NoteActions database = Database_NoteActions ();
  database.create ();
  config_globals_setup_message = "versifications";
  setup_generate_versification_databases ();
  config_globals_setup_message = "modifications";
  database::modifications::create ();
  config_globals_setup_message = "notes";
  Database_Notes database_notes (webserver_request);
  database_notes.create ();
  config_globals_setup_message = "state";
  Database_State::create ();
  config_globals_setup_message = "login";
  Database_Login::create ();
  Database_Login::optimize ();
  config_globals_setup_message = "privileges";
  DatabasePrivileges::create ();
  DatabasePrivileges::upgrade ();
  DatabasePrivileges::optimize ();
#ifdef HAVE_CLOUD
  config_globals_setup_message = "git";
  database::git::create ();
  config_globals_setup_message = "statistics";
  Database_Statistics::create ();
  Database_Statistics::optimize ();
#endif

  // Create stylesheets.
  config_globals_setup_message = "stylesheets";
  styles_sheets_create_all ();
  
  // Schedule creation of sample Bible if there's no Bible yet.
  // In former versions of Bibledit, creation of the sample Bible was not scheduled,
  // but executed right away.
  // This led to very long app first-run times on low power devices.
  // The installation times were so long that user were tempted to think
  // that the install process was stuck.
  // To make installation fast, the creation of the sample Bible is now done in the background.
  const std::vector <std::string> bibles = database::bibles::get_bibles ();
  if (bibles.empty ()) {
    tasks_logic_queue (task::create_sample_bible);
  }
  
  // Schedule reindexing Bible search data.
  /*
   Re-indexing Bible search data was disabled again in Februari 2016,
   because it takes quite a while on low power devices,
   and the reason for the re-indexing is not clear.
  config_globals_setup_message = "indexes";
  database::config::general::set_index_bibles (true);
  tasks_logic_queue (REINDEXBIBLES);
  */
}


// Store the admin's details.
void setup_set_admin_details (const std::string& username, const std::string& password, const std::string& email)
{
  Database_Users database_users{};
  database_users.removeUser (username);
  database_users.add_user (username, password, Filter_Roles::admin (), email);
}


// Set the GUI setup status as completed.
void setup_complete_gui ()
{
  database::config::general::set_installed_interface_version (config::logic::version ());
}


// Generate the locale databases.
void setup_generate_locale_databases (bool progress)
{
#ifdef HAVE_ANDROID
  // On Android, do not generate the locale databases.
  // On this low power device, generating them would take quite a while, as experience shows.
  // Instead of generating them, the builder and installer put the pre-generated databases into place.
  return;
#endif
#ifdef HAVE_IOS
  // Same story for iOS.
  return;
#endif
  // Generate databases for all the localizations.
  const std::map <std::string, std::string> localizations = locale_logic_localizations ();
  for (const auto& element : localizations) {
    const std::string localization = element.first;
    if (localization.empty ()) continue;
    config_globals_setup_message = "locale " + localization;
    if (progress) 
      std::cout << config_globals_setup_message << std::endl;
    Database_Localization database_localization = Database_Localization (localization);
    const std::string path = filter_url_create_root_path ({"locale", localization + ".po"});
    database_localization.create (path);
  }
}


// Generate the verse mapping databases.
void setup_generate_verse_mapping_databases ()
{
#ifdef HAVE_ANDROID
  // On Android, do not generate the verse mapping databases.
  // On this low power device, generating them would take quite a while, as experience shows.
  // Instead of generating them, the builder and installer put the pre-generated databases into place.
  return;
#endif
#ifdef HAVE_IOS
  // Same story for iOS.
  return;
#endif
  // Generate the verse mappings.
  Database_Mappings database_mappings;
  database_mappings.create1 ();
  database_mappings.defaults ();
  database_mappings.create2 ();
  database_mappings.optimize ();
}


// Generate the versification databases.
void setup_generate_versification_databases ()
{
#ifdef HAVE_ANDROID
  // On Android, do not generate the verse mapping databases.
  // On this low power device, generating them would take quite a while, as experience shows.
  // Instead of generating them, the builder and installer put the pre-generated databases into place.
  return;
#endif
#ifdef HAVE_IOS
  // Same story for iOS.
  return;
#endif
  // Generate the versifications.
  Database_Versifications database_versifications;
  database_versifications.create ();
  database_versifications.defaults ();
}

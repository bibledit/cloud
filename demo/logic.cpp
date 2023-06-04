/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <demo/logic.h>
#include <filter/md5.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/notes.h>
#include <database/sample.h>
#include <database/books.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <styles/logic.h>
#include <styles/sheets.h>
#include <locale/logic.h>
#include <bb/logic.h>
#include <editusfm/index.h>
#include <editone2/index.h>
#include <resource/index.h>
#include <resource/external.h>
#include <resource/logic.h>
#include <workspace/logic.h>
#include <ipc/focus.h>
#include <lexicon/logic.h>
#include <search/logic.h>
#include <book/create.h>
#include <setup/logic.h>
using namespace std;


/*
 
 A demo installation is an open installation.
 Any user is always considered to be logged in as admin.
 
 In October 2015 the demo began to often refuse web connections.
 It appears that the server keeps running most of the times, but also crashed often during certain periods.
 
 The number of parallel connections was traced to see if that was the cause.
 The parallel connection count was mostly 0, at times 1, and higher at rare occassions.
 So this should be excluded as the cause.
 
 Continuous crashes of the server are the likely cause.
 The page requests are now being logged to see what happens.
 After logging them, it appears that the crash often comes after /resource/get
 
 Next a crash handler was installed, which gives some sort of backtrace in the Journal.
 This showed one crash during the nights. The crash was fixed.
 
 */


// Returns true if the credentials are correct for a demo installation.
bool demo_acl (string user, string pass)
{
  if (config::logic::demo_enabled ()) {
    if (user == session_admin_credentials ()) {
      if ((pass == session_admin_credentials ()) || (pass == md5 (session_admin_credentials ()))) {
        return true;
      }
    }
  }
  return false;
}


// Returns a warning in case the client is connected to the open demo server.
string demo_client_warning ()
{
  string warning {};
  if (client_logic_client_enabled ()) {
    string address = Database_Config_General::getServerAddress ();
    if (address == demo_address () || address == demo_address_secure ()) {
      int port = Database_Config_General::getServerPort ();
      if (port == demo_port () || port == demo_port_secure ()) {
        warning.append (translate("You are connected to a public demo of Bibledit Cloud."));
        warning.append (" ");
        warning.append (translate("Everybody can modify the data on that server."));
        warning.append (" ");
        warning.append (translate("After send and receive your data will reflect the data on the server."));
      }
    }
  }
  return warning;
}


// Cleans and resets the data in the Bibledit installation.
void demo_clean_data ()
{
  Database_Logs::log ("Cleaning up the demo data");
  
  
  Webserver_Request request {};
  
  
  // Set user to the demo credentials (admin).
  // This is the user who is always logged-in in a demo installation.
  request.session_logic ()->set_username (session_admin_credentials ());
  
  
  // Delete empty stylesheet that may have been there.
  request.database_styles()->revokeWriteAccess ("", styles_logic_standard_sheet ());
  request.database_styles()->deleteSheet ("");
  styles_sheets_create_all ();
  
  
  // Set both stylesheets to "Standard" for all Bibles.
  vector <string> bibles = request.database_bibles()->getBibles ();
  for (const auto & bible : bibles) {
    Database_Config_Bible::setExportStylesheet (bible, styles_logic_standard_sheet ());
    Database_Config_Bible::setEditorStylesheet (bible, styles_logic_standard_sheet ());
  }
  
  
  // Regenerate versification databases.
  setup_generate_versification_databases ();

  
  // Set the site language to "Default"
  Database_Config_General::setSiteLanguage (string());


  // Ensure the default users are there.
  map <string, int> users = {
    pair ("guest", Filter_Roles::guest ()),
    pair ("member", Filter_Roles::member ()),
    pair ("consultant", Filter_Roles::consultant ()),
    pair ("translator", Filter_Roles::translator ()),
    pair ("manager", Filter_Roles::manager ()),
    pair (session_admin_credentials (), Filter_Roles::admin ())
  };
  for (const auto & element : users) {
    if (!request.database_users ()->usernameExists (element.first)) {
      request.database_users ()->add_user(element.first, element.first, element.second, "");
    }
    request.database_users ()->set_level (element.first, element.second);
  }
  
  
  // Create / update sample Bible.
  if (config::logic::default_bibledit_configuration ()) {
    demo_create_sample_bible ();
  }


  // Create sample notes.
  if (config::logic::default_bibledit_configuration ()) {
    demo_create_sample_notes (&request);
  }


  // Create samples for the workspaces.
  if (config::logic::default_bibledit_configuration ()) {
    demo_create_sample_workspaces (&request);
  }
  
  
  // Set navigator to John 3:16.
  if (config::logic::default_bibledit_configuration ()) {
    Ipc_Focus::set (&request, 43, 3, 16);
  }


  // Set and/or trim resources to display.
  // Too many resources crash the demo: Limit the amount.
  vector <string> resources = request.database_config_user()->getActiveResources ();
  bool reset_resources {false};
  size_t max_resource {25};
  if (resources.size () > max_resource) reset_resources = true;
  // Check if all the current resource exists in the default.
  vector <string> defaults = demo_logic_default_resources ();
  for (const auto & name : defaults) {
    if (!in_array (name, resources)) reset_resources = true;
  }
  if (reset_resources) {
    resources = demo_logic_default_resources ();
    request.database_config_user()->setActiveResources (resources);
  }
  
  
  // No flipped basic <> advanded mode.
  request.database_config_user ()->setBasicInterfaceMode (false);
}


// The name of the sample Bible.
string demo_sample_bible_name ()
{
  return "Sample";
}


// Creates a sample Bible.
// Creating a Sample Bible used to take a relatively long time, in particular on low power devices.
// The new and current method does a simple copy operation and that is fast.
void demo_create_sample_bible ()
{
  Database_Logs::log ("Creating sample Bible");
  
  // Remove and create the sample Bible.
  Database_Bibles database_bibles {};
  database_bibles.deleteBible (demo_sample_bible_name ());
  database_bibles.createBible (demo_sample_bible_name ());
  
  // Remove index for the sample Bible.
  search_logic_delete_bible (demo_sample_bible_name ());

  // Copy the sample Bible data and search index into place.
  vector <int> rowids = Database_Sample::get ();
  for (auto rowid : rowids) {
    string file {};
    string data {};
    Database_Sample::get (rowid, file, data);
    // Remove the "./" from the start.
    file.erase (0, 2);
    // Since the filename contains the foward slash for on Linux,
    // and since Windows needs the backslash as directory separator,
    // replace these on Windows.
    file = filter_url_update_directory_separator_if_windows (file);
    // The name of the Sample Bible should be part of the filename.
    // If that is not the case,
    // * it means that Bibledit uses a different name for the Sample Bible,
    // * and the file needs an update.
    size_t pos = file.find(demo_sample_bible_name());
    if (pos == string::npos) {
      string filename = "Sample";
      file = filter::strings::replace(filename, demo_sample_bible_name(), file);
    }
    // Proceed with the path.
    file = filter_url_create_root_path ({file});
    string path = filter_url_dirname (file);
    if (!file_or_dir_exists (path)) filter_url_mkdir (path);
    filter_url_file_put_contents (file, data);
  }
  
  Database_Logs::log ("Sample Bible was created");
}


// Prepares a sample Bible.
// The output will be in database "sample".
// This data is intended for quickly creating a sample Bible.
// This way it is fast even on low power devices.
void demo_prepare_sample_bible ()
{
  Database_Bibles database_bibles {};
  Database_Sample::create ();
  // Remove the sample Bible plus all related data.
  database_bibles.deleteBible (demo_sample_bible_name ());
  search_logic_delete_bible (demo_sample_bible_name ());
  // Create a new sample Bible.
  database_bibles.createBible (demo_sample_bible_name ());
  // Location of the source USFM files for the sample Bible.
  string directory = filter_url_create_root_path ({"demo"});
  vector <string> files = filter_url_scandir (directory);
  for (auto file : files) {
    // Process only USFM files, skipping others.
    if (filter_url_get_extension (file) == "usfm") {
      // Read the USFM and clean it up.
      file = filter_url_create_path ({directory, file});
      string usfm = filter_url_file_get_contents (file);
      usfm = filter::strings::collapse_whitespace (usfm);
      // Import the USFM into the sample Bible.
      vector <filter::usfm::BookChapterData> book_chapter_data = filter::usfm::usfm_import (usfm, styles_logic_standard_sheet ());
      for (const auto & data : book_chapter_data) {
        int book = data.m_book;
        if (book) {
          // There is license information at the top of each USFM file.
          // This results in a book with number 0.
          // This book gets skipped here, so the license information is skipped as well.
          int chapter {data.m_chapter};
          string usfm2 {data.m_data};
          bible_logic::store_chapter (demo_sample_bible_name (), book, chapter, usfm2);
        }
      }
    }
  }
  // Copy the Bible data to the sample database.
  directory = database_bibles.bibleFolder (demo_sample_bible_name ());
  files.clear ();
  filter_url_recursive_scandir (directory, files);
  for (const auto & file : files) {
    if (!filter_url_is_dir (file)) {
      string data = filter_url_file_get_contents (file);
      Database_Sample::store (file, data);
    }
  }
  // Copy the search index data to the sample database.
  directory = search_logic_index_folder ();
  files.clear ();
  filter_url_recursive_scandir (directory, files);
  for (const auto & file : files) {
    if (file.find (demo_sample_bible_name ()) != string::npos) {
      string data = filter_url_file_get_contents (file);
      Database_Sample::store (file, data);
    }
  }
  // The sample Bible is now in the standard location and editable by the users: Remove it.
  database_bibles.deleteBible (demo_sample_bible_name ());
  // Same for the search index.
  search_logic_delete_bible (demo_sample_bible_name ());
  // Clean up the remaining artifacts that were created along the way.
#ifdef HAVE_CLOUD
  [[maybe_unused]] int result;
  result = system ("find . -path '*logbook/15*' -delete");
  result = system ("find . -name state.sqlite -delete");
  result = system ("find . -name 'Sample.*' -delete");
#endif
}


// Create sample notes.
void demo_create_sample_notes (void * webserver_request)
{
  Database_Notes database_notes (webserver_request);
  vector <int> identifiers = database_notes.get_identifiers ();
  if (identifiers.size () < 10) {
    for (int i = 1; i <= 10; i++) {
      database_notes.store_new_note (demo_sample_bible_name (), i, i, i, "Sample Note " + filter::strings::convert_to_string (i), "Sample Contents for note " + filter::strings::convert_to_string (i), false);
    }
  }
}


string demo_workspace ()
{
  return "Translation";
}


void demo_create_sample_workspaces (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  map <int, string> urls {};
  map <int, string> widths {};
  for (int i = 0; i < 15; i++) {
    string url {};
    string width {};
    if (i == 0) {
      url = editusfm_index_url ();
      width = "45%";
    }
    if (i == 1) {
      url = resource_index_url ();
      width = "45%";
    }
    urls [i] = url;
    widths [i] = width;
  }
  map <int, string> row_heights = {
    pair (0, "90%"),
    pair (1, ""),
    pair (2, "")
  };
  
  request->database_config_user()->setActiveWorkspace ("USFM");
  workspace_set_urls (request, urls);
  workspace_set_widths (request, widths);
  workspace_set_heights (request, row_heights);
  
  urls[0] = editone2_index_url ();
  urls[1] = resource_index_url ();
  
  request->database_config_user()->setActiveWorkspace (demo_workspace ());
  workspace_set_urls (request, urls);
  workspace_set_widths (request, widths);
  workspace_set_heights (request, row_heights);
}


vector <string> demo_logic_default_resources ()
{
  vector <string> resources {};
  if (config::logic::default_bibledit_configuration ()) {
    // Add a few resources that are also safe in an obfuscated version.
    resources = {
      demo_sample_bible_name (),
      resource_logic_violet_divider ()
    };
    // For demo purposes, add some more resources to show-case some of the capabilities.
    if (config::logic::demo_enabled ()) {
      resources.push_back (resource_external_biblehub_interlinear_name ());
      resources.push_back (resource_external_net_bible_name ());
      resources.push_back (SBLGNT_NAME);
    }
  }
  // Done.
  return resources;
}

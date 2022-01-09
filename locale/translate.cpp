/*
Copyright (©) 2003-2022 Teus Benschop.

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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <database/config/general.h>
#include <database/localization.h>
#include <filter/string.h>


//string localization;

//void check_user_localization_preference (void * webserver_request)
//{
//  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
//
//  string system_settings = Database_Config_General::getSiteLanguage ();
//
//  string user_preference = request->database_config_user ()->getSiteLanguage ();
//
//  bool is_logged_in = request->session_logic ()->loggedIn ();
//
//  if (user_preference.empty () && is_logged_in == true) {
//    localization = "";
//  } else if (system_settings != user_preference && !user_preference.empty ()) {
//    localization = user_preference;
//  } else {
//    localization = system_settings;
//  }
//}

// Storage for the user interface obfuscation strings.
vector <string> locale_translate_obfuscation_search;
vector <string> locale_translate_obfuscation_replace;


// Translates $english to its localized string.
string translate (string english)
{
  // Start off with the English message.
  string result (english);
  // Check whether a language has been set on the website or the app.
  string localization = Database_Config_General::getSiteLanguage ();
  if (!localization.empty ()) {
    // Localize it.
    Database_Localization database_localization = Database_Localization (localization);
    result = database_localization.translate (english);
  }
  // Check whether there's obfuscation to be done.
  if (!locale_translate_obfuscation_search.empty ()) {
    // Obfuscate: Search and replace text.
    // It replaces strings, not whole words as having certain boundaries.
    for (unsigned int i = 0; i < locale_translate_obfuscation_search.size(); i++) {
      result = filter_string_str_replace (locale_translate_obfuscation_search [i], locale_translate_obfuscation_replace [i], result);
    }
  }
  // Ready.
  return result;
}


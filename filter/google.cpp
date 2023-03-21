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


#include <filter/google.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>
#include <database/logs.h>
#include <config/logic.h>
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <jsonxx/jsonxx.h>
#pragma GCC diagnostic pop
using namespace std;
using namespace jsonxx;


namespace filter::google {


tuple <string, string> get_json_key_value_error ()
{
  string path = config::logic::google_translate_json_key_path ();

  if (!file_or_dir_exists (path)) {
    return { string(), "Cannot find the JSON key to access Google Translate. Looking for this file: " + path };
  }
  
  string error;
  string value = filter_url_file_get_contents (path);
  if (value.empty()) error = "The key at " + path + " is empty";

  return { value, error };
}


// This runs $ gcloud auth activate-service-account --key-file=key.json.
// It returns whether activation was successful,
// plus the resulting output of the command.
tuple <bool, string> activate_service_account ()
{
  stringstream command;
  command << "gcloud auth activate-service-account --quiet --key-file=";
  command << quoted(config::logic::google_translate_json_key_path ());
  string out_err;
  int result = filter_shell_run (command.str(), out_err);
  return { (result == 0), out_err };
}


string google_access_token {};

// This runs $ gcloud auth application-default print-access-token.
// It returns whether the command was successful,
// plus the resulting output of the command.
tuple <bool, string> print_store_access_token ()
{
  // Set the path to the JSON key in the environment for gcloud to use.
#ifdef HAVE_CLOUD
  setenv("GOOGLE_APPLICATION_CREDENTIALS", config::logic::google_translate_json_key_path ().c_str(), 1);
#endif
  // Print the access token.
  string command {"gcloud auth application-default print-access-token"};
  string out_err;
  int result = filter_shell_run (command.c_str(), out_err);
  // Check on success.
  bool success = (result == 0);
  // Store the token if it was received, else clear it.
  // Trim the token to remove any new line it likely contains.
  if (success) google_access_token = filter_string_trim(out_err);
  else google_access_token.clear();
  // Done.
  return { success, out_err };
}


// Refreshes the Google access token.
void refresh_access_token ()
{
  // Check whether the JSON keys exists, if not, bail out.
  if (!file_or_dir_exists(config::logic::google_translate_json_key_path ())) {
    return;
  }

  // Refresh the token.
  auto [ success, token ] = print_store_access_token ();
  
  Database_Logs::log ("Google access token: " + token);
}


// This makes an authenticated call to the Google Translate API.
// Pass the text to be translated.
// Pass the source language code and the target language code.
// It returns whether the call was successful, plus the translated text, plus the error
tuple <bool, string, string> translate (const string text, const char * source, const char * target)
{
  // From the shell, run these two commands to translate a string.
  // $ export GOOGLE_APPLICATION_CREDENTIALS=`pwd`"/googletranslate.json"
  // $ curl -s -X POST -H "Content-Type: application/json" -H "Authorization: Bearer "$(gcloud auth application-default print-access-token) --data "{ 'q': 'The quick brown fox jumps over the lazy dog', 'source': 'en', 'target': 'fr', 'format': 'text' }" "https://translation.googleapis.com/language/translate/v2"

  // The URL of the translation REST API.
  const string url { "https://translation.googleapis.com/language/translate/v2" };

  // Create the JSON data to post.
  Object translation_data;
  translation_data << "q" << text;
  translation_data << "source" << string (source);
  translation_data << "target" << string (target);
  translation_data << "format" << "text";
  string postdata = translation_data.json ();
  
  string error;
  bool burst { false };
  bool check_certificate { false };
  const vector <pair <string, string> > headers {
    { "Content-Type", "application/json" },
    { "Authorization", "Bearer " + google_access_token }
  };
  string translation = filter_url_http_post (url, postdata, {}, error, burst, check_certificate, headers);
  bool success { error.empty() };

  // Parse the translation JSON.
  // Example:
  // {
  //   "data": {
  //     "translations": [
  //       {
  //       "translatedText": "Ιησούς ο Χριστός ο Μεσσίας"
  //       }
  //     ]
  //   }
  // }
  if (error.empty()) {
    try {
      Object json_object;
      json_object.parse (translation);
      Object data = json_object.get<Object> ("data");
      Array translations = data.get<Array> ("translations");
      Object translated = translations.get<Object>(0);
      translation = translated.get<String> ("translatedText");
    } catch (const exception & exception) {
      error = exception.what();
      error.append (" - ");
      error.append(translation);
      success = false;
      translation.clear();
    }
  }

  if (!error.empty()) {
    Database_Logs::log("Error while translating text: " + error);
  }

  // Done.
  return { success, translation, error };
}


// This asks the Google Translate API for the list of supported languages.
// It returns a container with a pair of <language code, language name>.
// The language name is given in the $target language.
vector <pair <string, string> > get_languages (const string & target)
{

  // From the shell, run these two commands to translate a string.
  // $ export GOOGLE_APPLICATION_CREDENTIALS=`pwd`"/googletranslate.json"
  // $ curl -s -X POST -H "Content-Type: application/json; charset=utf-8" -H "Authorization: Bearer "$(gcloud auth application-default print-access-token) --data "{ 'target': 'en' }" "https://translation.googleapis.com/language/translate/v2/languages"
  
  // The URL of the translation REST API.
  const string url { "https://translation.googleapis.com/language/translate/v2/languages" };
  
  // Create the JSON data to post.
  Object request_data;
  request_data << "target" << target;
  string postdata = request_data.json ();
  
  string error;
  bool burst { false };
  bool check_certificate { false };
  const vector <pair <string, string> > headers {
    { "Content-Type", "application/json; charset=utf-8" },
    { "Authorization", "Bearer " + google_access_token }
  };
  string result_json = filter_url_http_post (url, postdata, {}, error, burst, check_certificate, headers);
  
  // Parse the resulting JSON.
  // Example:
  // {
  //   "data": {
  //     "languages": [
  //       {
  //         "language": "zh-CN",
  //         "name": "Chinese (Simplified)"
  //       },
  //       {
  //         "language": "he",
  //         "name": "Hebrew"
  //       },
  //       {
  //         "language": "zu",
  //         "name": "Zulu"
  //       }
  //     ]
  //   }
  // }
  vector <pair <string, string> > language_codes_names;
  if (error.empty()) {
    try {
      Object json_object;
      json_object.parse (result_json);
      Object data = json_object.get<Object> ("data");
      Array languages = data.get<Array> ("languages");
      for (size_t i = 0; i < languages.size(); i++) {
        Object language_name = languages.get<Object>(static_cast<unsigned>(i));
        string language = language_name.get<String>("language");
        string name = language_name.get<String>("name");
        language_codes_names.push_back({language, name});
      }
    } catch (const exception & exception) {
      error = exception.what();
      error.append (" - ");
      error.append(result_json);
      language_codes_names.clear();
    }
  }
  
  if (!error.empty()) {
    Database_Logs::log("Error while getting Google Translate supported languages: " + error);
    language_codes_names.clear();
  }
  
  // Done.
  return language_codes_names;
}


}

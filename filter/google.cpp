/*
Copyright (©) 2003-2026 Teus Benschop.

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


namespace filter::google {


std::tuple <std::string, std::string> get_json_key_value_error ()
{
  std::string path = config::logic::google_translate_json_key_path ();

  if (!file_or_dir_exists (path)) {
    return { std::string(), "Cannot find the JSON key to access Google Translate. Looking for this file: " + path };
  }
  
  std::string error;
  std::string value = filter_url_file_get_contents (path);
  if (value.empty()) error = "The key at " + path + " is empty";

  return { value, error };
}


// This runs $ gcloud auth activate-service-account --key-file=key.json.
// It returns whether activation was successful,
// plus the resulting output of the command.
std::tuple <bool, std::string> activate_service_account ()
{
  std::stringstream command;
  command << filter::shell::get_executable(filter::shell::Executable::gcloud);
  command << " auth activate-service-account --quiet --key-file=";
  command << std::quoted(config::logic::google_translate_json_key_path ());
  std::string out_err;
  int result = filter::shell::run (command.str(), out_err);
  return { (result == 0), out_err };
}


std::string google_access_token {};

// This runs $ gcloud auth application-default print-access-token.
// It returns whether the command was successful,
// plus the resulting output of the command.
std::tuple <bool, std::string> print_store_access_token ()
{
  // Set the path to the JSON key in the environment for gcloud to use.
#ifdef HAVE_CLOUD
  setenv("GOOGLE_APPLICATION_CREDENTIALS", config::logic::google_translate_json_key_path ().c_str(), 1);
#endif
  // Print the access token.
  const std::string command {std::string(filter::shell::get_executable(filter::shell::Executable::gcloud)) + " auth application-default print-access-token"};
  std::string out_err;
  const int result = filter::shell::run (command.c_str(), out_err);
  // Check on success.
  bool success = (result == 0);
  // Store the token if it was received, else clear it.
  // Trim the token to remove any new line it likely contains.
  if (success) google_access_token = filter::string::trim(out_err);
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
std::tuple <bool, std::string, std::string> translate (const std::string text, const char * source, const char * target)
{
  // From the shell, run these two commands to translate a string.
  // $ export GOOGLE_APPLICATION_CREDENTIALS=`pwd`"/googletranslate.json"
  // $ curl -s -X POST -H "Content-Type: application/json" -H "Authorization: Bearer "$(gcloud auth application-default print-access-token) --data "{ 'q': 'The quick brown fox jumps over the lazy dog', 'source': 'en', 'target': 'fr', 'format': 'text' }" "https://translation.googleapis.com/language/translate/v2"

  // The URL of the translation REST API.
  const std::string url { "https://translation.googleapis.com/language/translate/v2" };

  // Create the JSON data to post.
  jsonxx::Object translation_data;
  translation_data << "q" << text;
  translation_data << "source" << std::string (source);
  translation_data << "target" << std::string (target);
  translation_data << "format" << "text";
  std::string postdata = translation_data.json ();
  
  std::string error;
  bool burst { false };
  bool check_certificate { false };
  const std::vector <std::pair <std::string, std::string> > headers {
    { "Content-Type", "application/json" },
    { "Authorization", "Bearer " + google_access_token }
  };
  std::string translation = filter_url_http_post (url, postdata, {}, error, burst, check_certificate, headers);
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
      jsonxx::Object json_object;
      json_object.parse (translation);
      jsonxx::Object data = json_object.get<jsonxx::Object> ("data");
      jsonxx::Array translations = data.get<jsonxx::Array> ("translations");
      jsonxx::Object translated = translations.get<jsonxx::Object>(0);
      translation = translated.get<jsonxx::String> ("translatedText");
    } catch (const std::exception & exception) {
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
std::vector <std::pair <std::string, std::string> > get_languages (const std::string& target)
{

  // From the shell, run these two commands to translate a string.
  // $ export GOOGLE_APPLICATION_CREDENTIALS=`pwd`"/googletranslate.json"
  // $ curl -s -X POST -H "Content-Type: application/json; charset=utf-8" -H "Authorization: Bearer "$(gcloud auth application-default print-access-token) --data "{ 'target': 'en' }" "https://translation.googleapis.com/language/translate/v2/languages"
  
  // The URL of the translation REST API.
  const std::string url { "https://translation.googleapis.com/language/translate/v2/languages" };
  
  // Create the JSON data to post.
  jsonxx::Object request_data;
  request_data << "target" << target;
  std::string postdata = request_data.json ();
  
  std::string error;
  bool burst { false };
  bool check_certificate { false };
  const std::vector <std::pair <std::string, std::string> > headers {
    { "Content-Type", "application/json; charset=utf-8" },
    { "Authorization", "Bearer " + google_access_token }
  };
  std::string result_json = filter_url_http_post (url, postdata, {}, error, burst, check_certificate, headers);
  
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
  std::vector <std::pair <std::string, std::string> > language_codes_names;
  if (error.empty()) {
    try {
      jsonxx::Object json_object;
      json_object.parse (result_json);
      jsonxx::Object data = json_object.get<jsonxx::Object> ("data");
      jsonxx::Array languages = data.get<jsonxx::Array> ("languages");
      for (size_t i = 0; i < languages.size(); i++) {
        jsonxx::Object language_name = languages.get<jsonxx::Object>(static_cast<unsigned>(i));
        std::string language = language_name.get<jsonxx::String>("language");
        std::string name = language_name.get<jsonxx::String>("name");
        language_codes_names.push_back({language, name});
      }
    } catch (const std::exception & exception) {
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

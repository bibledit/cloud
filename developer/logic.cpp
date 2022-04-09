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


#include <developer/logic.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/logs.h>
#include <database/books.h>
#include <filter/usfm.h>


mutex log_network_mutex;
vector <string> log_network_cache;


void developer_logic_log_network_write ()
{
  if (!log_network_cache.empty ()) {
    log_network_mutex.lock ();
    string lines;
    for (auto line : log_network_cache) {
      lines.append (line);
      lines.append ("\n");
    }
    log_network_cache.clear ();
    log_network_mutex.unlock ();
    string path = filter_url_create_root_path ({filter_url_temp_dir(), "log-network.csv"});
    if (!file_or_dir_exists (path)) {
      filter_url_file_put_contents_append (path, "date,IPaddress,URL,query,username\n");
    }
    filter_url_file_put_contents_append (path, lines);
  }
}


Developer_Logic_Tracer::Developer_Logic_Tracer(void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  seconds1 = filter::date::seconds_since_epoch ();
  microseconds1 = filter::date::numerical_microseconds();
  rfc822 = filter::date::rfc822 (seconds1);
  remote_address = request->remote_address;
  request_get = request->get;
  for (auto element : request->query) {
    request_query.append(" ");
    request_query.append(element.first);
    request_query.append("=");
    request_query.append(element.second);
  }
  username = request->session_logic()->currentUser();
}


Developer_Logic_Tracer::~Developer_Logic_Tracer()
{
  int seconds2 = filter::date::seconds_since_epoch();
  int microseconds2 = filter::date::numerical_microseconds();
  int microseconds = (seconds2 - seconds1) * 1000000 + microseconds2 - microseconds1;
  vector <string> bits = {rfc822, convert_to_string (microseconds), request_get, request_query, username};
  string entry = filter_string_implode(bits, ",");
  log_network_mutex.lock();
  log_network_cache.push_back(entry);
  log_network_mutex.unlock();
}


void developer_logic_import_changes_save (string bible, int book, int chapter, int verse, string & text)
{
  cout << "saving verse " << verse << endl;
  cout << text << endl;
  if (text.empty()) {
    return;
  }
  
  Webserver_Request webserver_request;
  string explanation = "import changes";
  string message = usfm_safely_store_verse (&webserver_request, bible, book, chapter, verse, text, explanation, false);
  if (!message.empty()) Database_Logs::log (message);
  text.clear ();
}


void developer_logic_import_changes ()
{
  string home_path = ".";
  char * home = getenv ("HOME");
  if (home) home_path = home;
  string file_path = filter_url_create_path ({home_path, "Desktop", "changes.usfm"});
  string bible = "test";
  Database_Logs::log ("Import changes from " + file_path + " into Bible " + bible);
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibles.getBibles ();
  if (!in_array(bible, bibles)) {
    Database_Logs::log ("Cannot locate Bible " + bible);
    return;
  }
  if (!file_or_dir_exists (file_path)) {
    Database_Logs::log ("Cannot locate " + file_path);
    return;
  }
  string contents = filter_url_file_get_contents(file_path);
  vector<string> lines = filter_string_explode(contents, "\n");

  vector <int> book_ids = Database_Books::getIDs ();

  Passage passage (bible, 0, 0, "");
  string text;

  for (auto line : lines) {
    if (line.empty()) continue;
    
    int book = 0;
    int chapter = -1;
    int verse = -1;

    // Locate and extract the book identifier.
    for (auto book_id : book_ids) {
      string s = Database_Books::getEnglishFromId(book_id);
      size_t pos = line.find(s);
      if (pos != 3) continue;
      book = book_id;
      line.erase (0, pos + s.length());
      break;
    }
    
    // Extract chapter and verse.
    bool passage_found = false;
    if (book) {
      size_t pos = line.find (":");
      if (pos != string::npos) {
        vector <string> bits = filter_string_explode(line.substr (0, pos), ".");
        if (bits.size() == 2) {
          chapter = convert_to_int(filter_string_trim(bits[0]));
          verse = convert_to_int(filter_string_trim(bits[1]));
          line.erase (0, pos + 2);
          passage_found = (book) && (chapter >= 0) && (verse >= 0);
        }
      }
    }
    
    // If a new passage was found:
    // 1. Save the accumulated text to the existing passage.
    // 2. Update the passage to point to the new one.
    if (passage_found) {
      developer_logic_import_changes_save (passage.bible, passage.book, passage.chapter, convert_to_int (passage.verse), text);
      passage = Passage(bible, book, chapter, convert_to_string(verse));
    }
    // Accumulate the text.
    if (!text.empty()) text.append ("\n");
    //if (verse == 1) text.append ("\\p\n");
    text.append(line);

  }
  
  developer_logic_import_changes_save (passage.bible, passage.book, passage.chapter, convert_to_int (passage.verse), text);

}

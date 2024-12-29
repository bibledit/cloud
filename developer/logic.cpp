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


#include <developer/logic.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/logs.h>
#include <database/books.h>
#include <filter/usfm.h>


// Internal function declarations.
void developer_logic_import_changes_save (std::string bible, int book, int chapter, int verse, std::string& text);


std::mutex log_network_mutex {};
std::vector <std::string> log_network_cache {};


void developer_logic_log_network_write ()
{
  if (!log_network_cache.empty ()) {
    log_network_mutex.lock ();
    std::string lines {};
    for (const auto& line : log_network_cache) {
      lines.append (line);
      lines.append ("\n");
    }
    log_network_cache.clear ();
    log_network_mutex.unlock ();
    const std::string path = filter_url_create_root_path ({filter_url_temp_dir(), "log-network.csv"});
    if (!file_or_dir_exists (path)) {
      filter_url_file_put_contents_append (path, "date,IPaddress,URL,query,username\n");
    }
    filter_url_file_put_contents_append (path, lines);
  }
}


Developer_Logic_Tracer::Developer_Logic_Tracer(Webserver_Request& webserver_request)
{
  seconds1 = filter::date::seconds_since_epoch ();
  microseconds1 = filter::date::numerical_microseconds();
  rfc822 = filter::date::rfc822 (seconds1);
  remote_address = webserver_request.remote_address;
  request_get = webserver_request.get;
  for (const auto& element : webserver_request.query) {
    request_query.append(" ");
    request_query.append(element.first);
    request_query.append("=");
    request_query.append(element.second);
  }
  username = webserver_request.session_logic ()->get_username();
}


Developer_Logic_Tracer::~Developer_Logic_Tracer()
{
  int seconds2 = filter::date::seconds_since_epoch();
  int microseconds2 = filter::date::numerical_microseconds();
  int microseconds = (seconds2 - seconds1) * 1000000 + microseconds2 - microseconds1;
  std::vector <std::string> bits = {rfc822, std::to_string (microseconds), request_get, request_query, username};
  std::string entry = filter::strings::implode(bits, ",");
  log_network_mutex.lock();
  log_network_cache.push_back(entry);
  log_network_mutex.unlock();
}


void developer_logic_import_changes_save (std::string bible, int book, int chapter, int verse, std::string& text)
{
  std::cout << "saving verse " << verse << std::endl;
  std::cout << text << std::endl;
  if (text.empty()) {
    return;
  }
  
  Webserver_Request webserver_request {};
  std::string explanation = "import changes";
  const std::string message = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse, text, explanation, false);
  if (!message.empty()) Database_Logs::log (message);
  text.clear ();
}


void developer_logic_import_changes ()
{
  std::string home_path = ".";
  char * home = getenv ("HOME");
  if (home) 
    home_path = home;
  const std::string file_path = filter_url_create_path ({home_path, "Desktop", "changes.usfm"});
  const std::string bible = "test";
  Database_Logs::log ("Import changes from " + file_path + " into Bible " + bible);
  const std::vector <std::string> bibles = database::bibles::get_bibles ();
  if (!in_array(bible, bibles)) {
    Database_Logs::log ("Cannot locate Bible " + bible);
    return;
  }
  if (!file_or_dir_exists (file_path)) {
    Database_Logs::log ("Cannot locate " + file_path);
    return;
  }
  const std::string contents = filter_url_file_get_contents(file_path);
  const std::vector<std::string> lines = filter::strings::explode(contents, "\n");

  const std::vector <book_id> book_ids = database::books::get_ids ();

  Passage passage (bible, 0, 0, std::string());
  std::string text {};

  for (auto line : lines) {
    if (line.empty()) continue;
    
    book_id book {book_id::_unknown};
    int chapter {-1};
    int verse {-1};

    // Locate and extract the book identifier.
    for (const auto book_num : book_ids) {
      std::string s = database::books::get_english_from_id(static_cast<book_id>(book_num));
      const size_t pos = line.find(s);
      if (pos != 3) continue;
      book = book_num;
      line.erase (0, pos + s.length());
      break;
    }
    
    // Extract chapter and verse.
    bool passage_found {false};
    if (book != book_id::_unknown) {
      size_t pos = line.find (":");
      if (pos != std::string::npos) {
        const std::vector <std::string> bits = filter::strings::explode(line.substr (0, pos), ".");
        if (bits.size() == 2) {
          chapter = filter::strings::convert_to_int(filter::strings::trim(bits[0]));
          verse = filter::strings::convert_to_int(filter::strings::trim(bits[1]));
          line.erase (0, pos + 2);
          passage_found = (book != book_id::_unknown) && (chapter >= 0) && (verse >= 0);
        }
      }
    }
    
    // If a new passage was found:
    // 1. Save the accumulated text to the existing passage.
    // 2. Update the passage to point to the new one.
    if (passage_found) {
      developer_logic_import_changes_save (passage.m_bible, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse), text);
      passage = Passage(bible, static_cast<int>(book), chapter, std::to_string(verse));
    }
    // Accumulate the text.
    if (!text.empty()) text.append ("\n");
    //if (verse == 1) text.append ("\\p\n");
    text.append(line);

  }
  
  developer_logic_import_changes_save (passage.m_bible, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse), text);

}

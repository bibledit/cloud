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


#include <sync/usfmresources.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/usfmresources.h>
#include <database/books.h>
#include <database/mail.h>
#include <database/modifications.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
using namespace std;


string sync_usfmresources_url ()
{
  return "sync/usfmresources";
}


string sync_usfmresources (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Sync_Logic sync_logic = Sync_Logic (webserver_request);
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }

  int action = filter::strings::convert_to_int (request->post ["a"]);
  string resource = request->post ["r"];
  int book = filter::strings::convert_to_int (request->post ["b"]);
  int chapter = filter::strings::convert_to_int (request->post ["c"]);
  
  if (action == Sync_Logic::usfmresources_get_total_checksum) {
    return Sync_Logic::usfm_resources_checksum ();
  }
  
  else if (action == Sync_Logic::usfmresources_get_resources) {
    vector <string> resources = database_usfmresources.getResources ();
    return filter::strings::implode (resources, "\n");
  }
  
  else if (action == Sync_Logic::usfmresources_get_resource_checksum) {
    return Sync_Logic::usfm_resource_checksum (resource);
  }
  
  else if (action == Sync_Logic::usfmresources_get_books) {
    vector <int> resource_books = database_usfmresources.getBooks (resource);
    vector <string> sbooks;
    for (auto & resource_book : resource_books) sbooks.push_back (filter::strings::convert_to_string (resource_book));
    return filter::strings::implode (sbooks, "\n");    
  }
  
  else if (action == Sync_Logic::usfmresources_get_book_checksum) {
    return Sync_Logic::usfm_resource_book_checksum (resource, book);
  }
  
  else if (action == Sync_Logic::usfmresources_get_chapters) {
    vector <int> res_chapters = database_usfmresources.getChapters (resource, book);
    vector <string> s_chapters;
    for (auto & res_chapter : res_chapters) s_chapters.push_back (filter::strings::convert_to_string (res_chapter));
    return filter::strings::implode (s_chapters, "\n");
  }
  
  else if (action == Sync_Logic::usfmresources_get_chapter_checksum) {
    return Sync_Logic::usfm_resource_chapter_checksum (resource, book, chapter);
  }
  
  else if (action == Sync_Logic::usfmresources_get_chapter) {
    return database_usfmresources.getUsfm (resource, book, chapter);
  }

  // Bad request. Delay flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}


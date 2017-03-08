/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <xrefs/next.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/abbreviations.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/volatile.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <ipc/focus.h>
#include <xrefs/index.h>
#include <bible/logic.h>


string xrefs_next_url ()
{
  return "xrefs/next";
}


bool xrefs_next_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_next (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string bible = request->database_config_user()->getTargetXrefBible ();
  
  
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  Passage currentPassage = Passage ("", currentBook, currentChapter, "1");
  int currentLocation = filter_passage_to_integer (currentPassage);
  
  
  vector <int> books = request->database_bibles()->getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = request->database_bibles()->getChapters (bible, book);
    for (auto chapter : chapters) {
      if (chapter == 0) continue;
      Passage passage = Passage ("", book, chapter, "1");
      int location = filter_passage_to_integer (passage);
      if (location > currentLocation) {
        string usfm = request->database_bibles()->getChapter (bible, book, chapter);
        auto xrefs = usfm_extract_notes (usfm, {"x"});
        if (xrefs.empty ()) {
          Ipc_Focus::set (webserver_request, book, chapter, 1);
          redirect_browser (request, xrefs_index_url ());
          return "";
        }
      }
    }
  }
  
  
  redirect_browser (request, xrefs_index_url ());
  return "";
}

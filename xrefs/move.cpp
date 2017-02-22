/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <xrefs/move.h>
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
#include <xrefs/translate.h>
#include <bible/logic.h>


string xrefs_move_url ()
{
  return "xrefs/move";
}


bool xrefs_move_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_move (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  

  string bible = request->database_config_user()->getTargetXrefBible ();
  
  
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int verse = convert_to_int (request->query ["verse"]);
  int focus = convert_to_int (request->query ["focus"]);
  int move = convert_to_int (request->query ["move"]);
  
  
  string usfmString = request->database_bibles()->getChapter (bible, book, chapter);
  if (usfmString.empty ()) return "";
  
  
  map <int, string> usfmMap;
  vector <int> verses = usfm_get_verse_numbers (usfmString);
  verses = array_unique (verses);
  for (auto vs : verses) {
    usfmMap [vs] = usfm_get_verse_text (usfmString, vs);
  }
  
  
  string usfm = usfmMap [verse];
  if (usfm.empty ()) return "";
  
  
  usfm = usfm_move_note (usfm, move, focus);
  if (usfm.empty ()) return "";
  
  
  usfmMap [verse] = usfm;
  
  
  usfm.clear ();
  for (auto element : usfmMap) {
    if (!usfm.empty ()) usfm.append ("\n");
    usfm.append (element.second);
  }
  bible_logic_store_chapter (bible, book, chapter, usfm);

  
  return "";
}

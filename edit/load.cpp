/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <edit/load.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <editor/usfm2html.h>
#include <edit/logic.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <database/logs.h>
#include <quill/logic.h>
#include <database/config/bible.h>


string edit_load_url ()
{
  return "edit/load";
}


bool edit_load_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string edit_load (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  
  // Store a copy of the USFM loaded in the editor for later reference.
  storeLoadedUsfm (webserver_request, bible, book, chapter, "editql");
  
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.quill ();
  editor_usfm2html.run ();
  
  string html = editor_usfm2html.get ();
  
  // To make editing empty verses easier, convert spaces to non-breaking spaces, so they appear in the editor.
  if (usfm_contains_empty_verses (usfm)) {
    string search = "<span> </span>";
    string replace = "<span>" + unicode_non_breaking_space_entity () + "</span>";
    html = filter_string_str_replace (search, replace, html);
  }
  
  string user = request->session_logic ()->currentUser ();
  bool write = access_bible_book_write (webserver_request, user, bible, book);
  
  return Checksum_Logic::send (html, write);
}

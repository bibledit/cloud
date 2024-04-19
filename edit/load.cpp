/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


std::string edit_load_url ()
{
  return "edit/load";
}


bool edit_load_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string edit_load (Webserver_Request& webserver_request)
{
  const std::string bible = webserver_request.query ["bible"];
  const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  const int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  const std::string unique_id = webserver_request.query ["id"];

  // Store a copy of the USFM loaded in the editor for later reference.
  storeLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);
  
  const std::string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  const std::string usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  
  std::string html = editor_usfm2html.get ();
  
  // To make editing empty verses easier, convert spaces to non-breaking spaces, so they appear in the editor.
  if (filter::usfm::contains_empty_verses (usfm)) {
    const std::string search = "<span> </span>";
    const std::string replace = "<span>" + filter::strings::unicode_non_breaking_space_entity () + "</span>";
    html = filter::strings::replace (search, replace, html);
  }
  
  const std::string user = webserver_request.session_logic ()->currentUser ();
  const bool write = access_bible::book_write (webserver_request, user, bible, book);
  
  return checksum_logic::send (html, write);
}

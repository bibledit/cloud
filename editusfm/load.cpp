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


#include <editusfm/load.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <edit/logic.h>
#include <access/bible.h>


std::string editusfm_load_url ()
{
  return "editusfm/load";
}


bool editusfm_load_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editusfm_load (Webserver_Request& webserver_request)
{
  std::string bible = webserver_request.query ["bible"];
  int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  std::string unique_id = webserver_request.query ["id"];

  // Store a copy of the USFM loaded in the editor for later reference.
  store_loaded_usfm (webserver_request, bible, book, chapter, unique_id);

  std::string usfm = database::bibles::get_chapter (bible, book, chapter);

  // Escape the XML special characters so they load properly in the editor.
  usfm = filter::strings::escape_special_xml_characters (usfm);

  const std::string& user = webserver_request.session_logic ()->get_username ();
  bool write = access_bible::book_write (webserver_request, user, bible, book);

  return checksum_logic::send (usfm, write);
}

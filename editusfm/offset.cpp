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


#include <editusfm/offset.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <access/bible.h>


std::string editusfm_offset_url ()
{
  return "editusfm/offset";
}


bool editusfm_offset_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


// This receives the position of the caret in the editor,
// and translates that to a verse number,
// and focuses that verse number.
std::string editusfm_offset (Webserver_Request& webserver_request)
{
  std::string bible = webserver_request.query ["bible"];
  int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  unsigned int offset = static_cast<unsigned> (filter::strings::convert_to_int (webserver_request.query ["offset"]));
  std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  std::vector <int> verses = filter::usfm::offset_to_versenumber (usfm, offset);
  // Only update navigation in case the verse differs.
  // This avoids unnecessary focus operations in the clients.
  if (!in_array (Ipc_Focus::getVerse (webserver_request), verses)) {
    if (!verses.empty ()) {
      Ipc_Focus::set (webserver_request, book, chapter, verses[0]);
    }
  }
  return std::string();
}


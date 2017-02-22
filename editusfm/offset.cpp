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


#include <editusfm/offset.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <access/bible.h>


string editusfm_offset_url ()
{
  return "editusfm/offset";
}


bool editusfm_offset_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editusfm_offset (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int offset = convert_to_int (request->query ["offset"]);
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  vector <int> verses = usfm_offset_to_versenumber (usfm, offset);
  // Only update navigation in case the verse differs.
  // This avoids unnecessary focus operations in the clients.
  if (!in_array (Ipc_Focus::getVerse (request), verses)) {
    if (!verses.empty ()) {
      Ipc_Focus::set (request, book, chapter, verses[0]);
    }
  }
  return "";
}


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


#include <editusfm/load.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <edit/logic.h>
#include <access/bible.h>


string editusfm_load_url ()
{
  return "editusfm/load";
}


bool editusfm_load_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editusfm_load (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  
  // Store a copy of the USFM loaded in the editor for later reference.
  storeLoadedUsfm (webserver_request, bible, book, chapter, "editusfm");

  string usfm = request->database_bibles()->getChapter (bible, book, chapter);

  string user = request->session_logic ()->currentUser ();
  bool write = access_bible_book_write (webserver_request, user, bible, book);

  return Checksum_Logic::send (usfm, write);
}

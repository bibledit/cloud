/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <editor/id.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <database/ipc.h>
#include <access/bible.h>


string editor_id_url ()
{
  return "editor/id";
}


bool editor_id_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = AccessBible::Any (webserver_request);
  return read;
}


string editor_id (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  // Update the timestamp indicating that the Bible editor is alive.
  request->database_config_user()->setLiveBibleEditor (filter::date::seconds_since_epoch ());

  
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int id = request->database_bibles()->getChapterId (bible, book, chapter);
  return convert_to_string (id);
}

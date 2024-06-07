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


#include <ipc/notes.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/navigation.h>
#include <webserver/request.h>


// Deals with the consultation notes stuff.


void Ipc_Notes::open (Webserver_Request& webserver_request, int identifier)
{
  const std::string& user = webserver_request.session_logic ()->get_username ();
  webserver_request.database_ipc()->storeMessage (user, "", "opennote", std::to_string (identifier));
}


int Ipc_Notes::get (Webserver_Request& webserver_request)
{
  Database_Ipc_Message data = webserver_request.database_ipc()->getNote ();
  return filter::strings::convert_to_int (data.message);
}


void Ipc_Notes::erase (Webserver_Request& webserver_request)
{
  Database_Ipc_Message data = webserver_request.database_ipc()->getNote ();
  int counter = 0;
  while (data.id && (counter < 100)) {
    int id = data.id;
    webserver_request.database_ipc()->deleteMessage (id);
    counter++;
  }
}


// If $set is true, it sets the alive status of the notes editor.
// If $set is false, it returns the alive status.
bool Ipc_Notes::alive (Webserver_Request& webserver_request, bool set, bool alive)
{
  const std::string& user = webserver_request.session_logic ()->get_username ();
  if (set) {
    webserver_request.database_ipc()->storeMessage (user, "", "notesalive", filter::strings::convert_to_string (alive));
  } else {
    return webserver_request.database_ipc()->getNotesAlive ();
  }
  return false;
}

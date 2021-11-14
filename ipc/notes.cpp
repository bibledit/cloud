/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


void Ipc_Notes::open (void * webserver_request, int identifier)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic()->currentUser ();
  request->database_ipc()->storeMessage (user, "", "opennote", convert_to_string (identifier));
}


int Ipc_Notes::get (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Ipc_Message data = request->database_ipc()->getNote ();
  return convert_to_int (data.message);
}


void Ipc_Notes::erase (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Ipc_Message data = request->database_ipc()->getNote ();
  int counter = 0;
  while (data.id && (counter < 100)) {
    int id = data.id;
    request->database_ipc()->deleteMessage (id);
    counter++;
  }
}


// If $set is true, it sets the alive status of the notes editor.
// If $set is false, it returns the alive status.
bool Ipc_Notes::alive (void * webserver_request, bool set, bool alive)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic()->currentUser ();
  if (set) {
    request->database_ipc()->storeMessage (user, "", "notesalive", convert_to_string (alive));
  } else {
    return request->database_ipc()->getNotesAlive ();
  }
  return false;
}


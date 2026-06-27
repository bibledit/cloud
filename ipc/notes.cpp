/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <webserver/request.h>
#include <database/ipc.h>


// Deals with the consultation notes stuff.

namespace ipc_notes {
void open(Webserver_Request& webserver_request, const int identifier)
{
    const std::string& user = webserver_request.session_logic()->get_username();
    database_ipc::store_message(user, "", "opennote", std::to_string(identifier));
}


int get(Webserver_Request& webserver_request)
{
    const database_ipc::Message data = database_ipc::get_note(webserver_request);
    return filter::string::convert_to_int(data.message);
}


void erase(Webserver_Request& webserver_request)
{
    const database_ipc::Message data = database_ipc::get_note(webserver_request);
    int counter = 0;
    while (data.id && counter < 100)
    {
        const int id = data.id;
        database_ipc::delete_message(id);
        counter++;
    }
}


// If $set is true, it sets the alive status of the note editor.
// If $set is false, it returns the alive status.
bool alive(Webserver_Request& webserver_request, const bool set, const bool alive)
{
    const std::string& user = webserver_request.session_logic()->get_username();
    if (set)
        database_ipc::store_message(user, "", "notesalive", filter::string::convert_to_string(alive));
    else
        return database_ipc::get_notes_alive(webserver_request);
    return false;
}
}

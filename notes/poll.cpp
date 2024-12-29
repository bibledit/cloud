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


#include <notes/poll.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <ipc/notes.h>
#include <access/logic.h>
#include <developer/logic.h>


std::string notes_poll_url ()
{
  return "notes/poll";
}


bool notes_poll_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_notes (webserver_request);
}


std::string notes_poll (Webserver_Request& webserver_request)
{
  std::string action = webserver_request.query ["action"];
  if (action == "alive") {
    Ipc_Notes::alive (webserver_request, true, true);
    int identifier = Ipc_Notes::get (webserver_request);
    if (identifier) {
      Ipc_Notes::erase (webserver_request);
      std::string url = "note?id=" + std::to_string (identifier);
      return url;
    }
  } else if (action == "unload") {
    Ipc_Notes::alive (webserver_request, true, false);
  }
  return std::string();
}

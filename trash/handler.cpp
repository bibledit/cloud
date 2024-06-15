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


#include <trash/handler.h>
#include <filter/string.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <database/notes.h>
#include <webserver/request.h>


void trash_change_notification (Webserver_Request& webserver_request, int id)
{
  Passage passage = database::modifications::getNotificationPassage (id);
  std::string passageText = filter_passage_display_inline ({passage});
  std::string modification = database::modifications::getNotificationModification (id);
  const std::string& username = webserver_request.session_logic ()->get_username ();
  Database_Logs::log (username + " removed change notification " + passageText + " : " + modification);
}


void trash_consultation_note (Webserver_Request& webserver_request, int id)
{
  Database_Notes database_notes (webserver_request);
  std::vector <Passage> passages = database_notes.get_passages (id);
  std::string passageText = filter_passage_display_inline (passages);
  std::string summary = database_notes.get_summary (id);
  std::string contents = database_notes.get_contents (id);
  contents = filter::strings::html2text (contents);
  std::string username = webserver_request.session_logic ()->get_username ();
  if (username.empty ()) username = "This app";
  Database_Logs::log (username + " deleted or marked for deletion consultation note " + passageText + " | " + summary + " | " + contents);
}

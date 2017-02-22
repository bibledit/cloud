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


#include <trash/handler.h>
#include <filter/string.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <database/notes.h>
#include <webserver/request.h>


void trash_change_notification (void * webserver_request, int id)
{
  Database_Modifications database_modifications;
  Passage passage = database_modifications.getNotificationPassage (id);
  string passageText = filter_passage_display_inline ({passage});
  string modification = database_modifications.getNotificationModification (id);
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string username = request->session_logic()->currentUser ();
  Database_Logs::log (username + " removed change notification " + passageText + " : " + modification);
}


void trash_consultation_note (void * webserver_request, int id)
{
  Database_Notes database_notes (webserver_request);
  vector <Passage> passages = database_notes.getPassages (id);
  string passageText = filter_passage_display_inline (passages);
  string summary = database_notes.getSummary (id);
  string contents = database_notes.getContents (id);
  contents = filter_string_html2text (contents);
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string username = request->session_logic()->currentUser ();
  if (username.empty ()) username = "Bibledit";
  Database_Logs::log (username + " deleted or marked for deletion consultation note " + passageText + " | " + summary + " | " + contents);
}

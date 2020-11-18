/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <navigation/paratext.h>
#include <database/logs.h>
#include <database/books.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <ipc/focus.h>


string navigation_paratext_url ()
{
  return "navigation/paratext";
}


string navigation_paratext (void * webserver_request)
{
  // The request from the client.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  // Handle any reference received that was obtained from Paratext.
  static string previous_from;
  string from = request->query ["from"];
  // Reference should differ from the previous one.
  if (!from.empty () && (from != previous_from)) {
    previous_from = from;
    Database_Logs::log("Paratext is at " + from);
    // User should have set to receive references from Paratext.
    if (request->database_config_user ()->getReceiveFocusedReferenceFromParatext ()) {
      // Parse the reference from Paratext.
      vector<string> book_rest = filter_string_explode (from, ' ');
      if (book_rest.size() == 2) {
        int book = Database_Books::getIdFromUsfm (book_rest[0]);
        vector <string> chapter_verse = filter_string_explode(book_rest[1], ':');
        if (chapter_verse.size() == 2) {
          int chapter = convert_to_int(chapter_verse[0]);
          int verse = convert_to_int(chapter_verse[1]);
          // Set the user name to the first one in the database.
          Database_Users database_users;
          vector <string> users = database_users.getUsers ();
          if (!users.empty()) {
            string user = users [0];
            request->session_logic()->setUsername(user);
            // Set the focused passage for Bibledit.
            Ipc_Focus::set (request, book, chapter, verse);
          }
        }
      }
    }
  }
  return "";
}

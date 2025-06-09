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


#include <navigation/paratext.h>
#include <database/logs.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/mappings.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <ipc/focus.h>
#include <client/logic.h>


std::string navigation_paratext_url ()
{
  return "navigation/paratext";
}


std::string navigation_paratext (Webserver_Request& webserver_request)
{
  // Handle any reference received that was obtained from Paratext.
  static std::string previous_from;
  std::string from = webserver_request.query ["from"];
  // Reference should differ from the previous one.
  if (!from.empty () && (from != previous_from)) {
    previous_from = from;
    Database_Logs::log("Paratext is at " + from);
    // User should have set to receive references from Paratext.
    if (webserver_request.database_config_user ()->get_receive_focused_reference_from_paratext ()) {
      // Parse the reference from Paratext.
      std::vector<std::string> book_rest = filter::strings::explode (from, ' ');
      if (book_rest.size() == 2) {
        int book = static_cast<int>(database::books::get_id_from_usfm (book_rest[0]));
        std::vector <std::string> chapter_verse = filter::strings::explode(book_rest[1], ':');
        if (chapter_verse.size() == 2) {
          int chapter = filter::strings::convert_to_int(chapter_verse[0]);
          int verse = filter::strings::convert_to_int(chapter_verse[1]);
          // Set the user name on this client device.
          const std::string& user = client_logic_get_username ();
          webserver_request.session_logic()->set_username(user);
          // "I believe how SantaFe works on Windows is
          // that it always sends a standardised verse reference.
          // So, for instance, a reference of Psalm 13:3 in the Hebrew Bible
          // will instead send the standardised (KJV-like) Psalm 13:2."
          // Assuming this to be the case for receiving a reference from Paratext,
          // it means that the reference from Paratext
          // may need to be mapped to the local versification system.
          // Get the active Bible and its versification system.
          std::string bible = webserver_request.database_config_user ()->get_bible ();
          std::string versification = database::config::bible::get_versification_system (bible);
          std::vector <Passage> passages;
          Database_Mappings database_mappings;
          if ((versification != filter::strings::english()) && !versification.empty ()) {
            passages = database_mappings.translate (filter::strings::english (), versification, book, chapter, verse);
          } else {
            passages.push_back (Passage ("", book, chapter, std::to_string (verse)));
          }
          if (passages.empty()) return std::string();
          chapter = passages[0].m_chapter;
          verse = filter::strings::convert_to_int (passages[0].m_verse);
          // Set the focused passage for Bibledit.
          Ipc_Focus::set (webserver_request, book, chapter, verse);
        }
      }
    }
  }
  return std::string();
}

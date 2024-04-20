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


#include <notes/verses.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/actions.h>


std::string notes_verses_url ()
{
  return "notes/verses";
}


bool notes_verses_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string notes_verses (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);


  std::string page;
  Assets_Header header = Assets_Header (translate("Passages"), webserver_request);
  page += header.run ();
  Assets_View view;
  std::string success, error;
  
  
  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  view.set_variable ("id", filter::strings::convert_to_string (id));


  if (webserver_request.post.count ("submit")) {
    std::vector <std::string> verses = filter::strings::explode (webserver_request.post["verses"], '\n');
    std::vector <Passage> passages;
    Passage previousPassage = Passage ("", 1, 1, "1");
    for (auto & line : verses) {
      line = filter::strings::trim (line);
      if (line != "") {
        Passage passage = filter_passage_interpret_passage (previousPassage, line);
        if (passage.m_book != 0) {
          passages.push_back (passage);
          previousPassage = passage;
        }
      }
    }
    if (passages.empty ()) {
      error = translate ("The note should have one or more passages assigned.");
    } else {
      notes_logic.setPassages (id, passages);
      redirect_browser (webserver_request, notes_actions_url () + "?id=" + filter::strings::convert_to_string (id));
      return std::string();
    }
  }
  
  
  std::string verses = filter_passage_display_multiline (database_notes.get_passages (id));
  view.set_variable ("verses", verses);
  
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "verses");
  page += assets_page::footer ();
  return page;
}

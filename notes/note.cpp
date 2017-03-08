/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <notes/note.h>
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
#include <notes/index.h>
#include <access/logic.h>


string notes_note_url ()
{
  return "notes/note";
}


bool notes_note_acl (void * webserver_request)
{
  return access_logic_privilege_view_notes (webserver_request);
}


string notes_note (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Note"), request);
  header.setNavigator ();

  
  // After adding a comment to a note, when doing nothing for several seconds,
  // the browser then returns to the list of notes.
  if (request->query.count ("temporal")) {
    header.refresh (5, "index");
  }

  
  page += header.run ();
  Assets_View view;
  string success;

  
  int id = convert_to_int (request->query ["id"]);
  
  
  // When a note is opened, then the passage navigator should go to the passage that belongs to that note.
  vector <Passage> passages = database_notes.getPassages (id);
  if (!passages.empty ()) {
    int desired_book = passages[0].book;
    int desired_chapter = passages[0].chapter;
    int desired_verse = convert_to_int (passages[0].verse);
    int focused_book = Ipc_Focus::getBook (webserver_request);
    int focused_chapter = Ipc_Focus::getChapter (webserver_request);
    int focused_verse = Ipc_Focus::getVerse (webserver_request);
    // Only set passage and track history if the desired passage differs from the focused passage.
    if ((desired_book != focused_book) || (desired_chapter != focused_chapter) || (desired_verse != focused_verse)) {
      Ipc_Focus::set (webserver_request, desired_book, desired_chapter, desired_verse);
      Navigation_Passage::recordHistory (webserver_request, desired_book, desired_chapter, desired_verse);
    }
  }
  
  
  view.set_variable ("id", convert_to_string (id));
  

  string summary = database_notes.getSummary (id);
  view.set_variable ("summary", summary);


  if (request->session_logic ()->currentLevel () >= Filter_Roles::manager ()) {
    view.enable_zone ("editlevel");
  }
  
  
  string content = database_notes.getContents (id);
  view.set_variable ("content", content);

  
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) {
    view.enable_zone ("consultant");
  }
  if (access_logic_privilege_create_comment_notes (webserver_request)) {
    view.enable_zone ("comment");
  }
  
  view.set_variable ("success", success);
  page += view.render ("notes", "note");
  page += Assets_Page::footer ();
  return page;
}

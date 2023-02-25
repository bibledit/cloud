/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
#include <filter/html.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/index.h>
#include <access/logic.h>
using namespace std;


string notes_note_url ()
{
  return "notes/note";
}


bool notes_note_acl (void * webserver_request)
{
  return access_logic::privilege_view_notes (webserver_request);
}


string notes_note (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Note"), request);
  header.set_navigator ();

  
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
  vector <Passage> passages = database_notes.get_passages (id);
  if (!passages.empty ()) {
    Passage focused_passage;
    focused_passage.m_book = Ipc_Focus::getBook (webserver_request);
    focused_passage.m_chapter = Ipc_Focus::getChapter (webserver_request);
    focused_passage.m_verse = convert_to_string (Ipc_Focus::getVerse (webserver_request));
    // Only set passage and track history if the focused passage
    // differs from all of the passages of the note.
    // If the focused passage is already at any of the passages belonging to the note,
    // no further focus operations are needed.
    bool passage_focused = false;
    for (auto passage : passages) {
      if (focused_passage.equal (passage)) passage_focused = true;
    }
    if (!passage_focused) {
      int desired_book = passages[0].m_book;
      int desired_chapter = passages[0].m_chapter;
      int desired_verse = convert_to_int (passages[0].m_verse);
      Ipc_Focus::set (webserver_request, desired_book, desired_chapter, desired_verse);
      Navigation_Passage::record_history (webserver_request, desired_book, desired_chapter, desired_verse);
    }
  }
  
  
  view.set_variable ("id", convert_to_string (id));


  view.set_variable ("passage", filter_passage_display_inline (passages));
  

  string summary = database_notes.get_summary (id);
  view.set_variable ("summary", summary);

  
  bool show_note_status = request->database_config_user ()->getShowNoteStatus ();
  if (show_note_status) {
    string status = database_notes.get_status (id);
    view.set_variable ("status", status);
  }
  
  
  if (request->session_logic ()->currentLevel () >= Filter_Roles::translator ()) {
    view.enable_zone ("editlevel");
  }
  
  
  string content = database_notes.get_contents (id);
  view.set_variable ("content", content);

  
  // Extra space at the bottom of the page.
  // See issues:
  // https://github.com/bibledit/cloud/issues/321
  // https://github.com/bibledit/cloud/issues/310
  view.set_variable ("brs", filter_html_android_brs ());
  

  if (request->database_config_user ()->getQuickNoteEditLink ()) {
    view.enable_zone ("editcontent");
  }
    
  
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) {
    view.enable_zone ("consultant");
  }
  if (access_logic::privilege_create_comment_notes (webserver_request)) {
    view.enable_zone ("comment");
  }
  
  view.set_variable ("success", success);
  page += view.render ("notes", "note");
  page += assets_page::footer ();
  return page;
}

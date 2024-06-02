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


#include <notes/comment.h>
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
#include <notes/note.h>
#include <access/logic.h>


std::string notes_comment_url ()
{
  return "notes/comment";
}


bool notes_comment_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_create_comment_notes (webserver_request);
}


std::string notes_comment (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Comment"), webserver_request);
  page += header.run ();
  Assets_View view;
  std::string success;
  
  
  int id;
  if (webserver_request.query.count ("id")) 
    id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  else 
    id = filter::strings::convert_to_int (webserver_request.post ["id"]);
  
  
  if (webserver_request.post.count ("body")) {
    std::string comment = filter::strings::trim (webserver_request.post ["body"]);
    comment = filter_url_tag_to_plus (comment);
    notes_logic.addComment (id, comment);
    redirect_browser (webserver_request, notes_note_url () + "?id=" + std::to_string (id) + "&temporal=");
    return std::string();
  }
  
  
  if (webserver_request.post.count ("cancel")) {
    redirect_browser (webserver_request, notes_note_url () + "?id=" + std::to_string (id));
    return std::string();
  }
  
  
  view.set_variable ("id", std::to_string (id));
  std::string script = "var noteId = '" + std::to_string (id) + "';\n";
  view.set_variable ("script", script);


  std::vector <Passage> passages = database_notes.get_passages (id);
  view.set_variable ("passage", filter_passage_display_inline (passages));
  
  
  std::string summary = database_notes.get_summary (id);
  view.set_variable ("summary", summary);

  
  bool show_note_status = webserver_request.database_config_user ()->getShowNoteStatus ();
  if (show_note_status) {
    std::string status = database_notes.get_status (id);
    view.set_variable ("status", status);
  }

  
  std::string content = database_notes.get_contents (id);
  view.set_variable ("content", content);
  
  
  view.set_variable ("success", success);
  page += view.render ("notes", "comment");
  page += assets_page::footer ();
  return page;
}

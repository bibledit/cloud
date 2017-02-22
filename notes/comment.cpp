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


string notes_comment_url ()
{
  return "notes/comment";
}


bool notes_comment_acl (void * webserver_request)
{
  return access_logic_privilege_create_comment_notes (webserver_request);
}


string notes_comment (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);

  
  string page;
  Assets_Header header = Assets_Header (translate("Comment"), request);
  page += header.run ();
  Assets_View view;
  string success;
  
  
  int id;
  if (request->query.count ("id")) id = convert_to_int (request->query ["id"]);
  else id = convert_to_int (request->post ["id"]);
  
  
  if (request->post.count ("submit")) {
    string comment = filter_string_trim (request->post ["comment"]);
    notes_logic.addComment (id, comment);
    redirect_browser (request, notes_note_url () + "?id=" + convert_to_string (id) + "&temporal=");
    return "";
  }
  
  
  if (request->post.count ("cancel")) {
    redirect_browser (request, notes_note_url () + "?id=" + convert_to_string (id));
    return "";
  }
  
  
  view.set_variable ("id", convert_to_string (id));
  
  
  string summary = database_notes.getSummary (id);
  view.set_variable ("summary", summary);
  
  
  string content = database_notes.getContents (id);
  view.set_variable ("content", content);
  
  
  view.set_variable ("success", success);
  page += view.render ("notes", "comment");
  page += Assets_Page::footer ();
  return page;
}

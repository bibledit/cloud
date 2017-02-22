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


#include <public/comment.h>
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
#include <public/note.h>


string public_comment_url ()
{
  return "public/comment";
}


bool public_comment_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_comment (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);

  
  string page;
  Assets_Header header = Assets_Header (translate("Feedback"), request);
  page += header.run ();
  Assets_View view;
  
  
  int id = convert_to_int (request->query ["id"]);
  view.set_variable ("id", convert_to_string (id));
  
  
  if (request->post.count ("submit")) {
    string comment = filter_string_trim (request->post ["comment"]);
    notes_logic.addComment (id, comment);
    redirect_browser (request, public_note_url () + "?id=" + convert_to_string (id));
    return "";
  }
  
  
  if (request->post.count ("cancel")) {
    redirect_browser (request, public_note_url () + "?id=" + convert_to_string (id));
    return "";
  }
  
  
  string summary = database_notes.getSummary (id);
  view.set_variable ("summary", summary);
  
  
  string content = database_notes.getContents (id);
  view.set_variable ("content", content);
  
  
  page += view.render ("public", "comment");
  page += Assets_Page::footer ();
  return page;
}

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


#include <notes/assign-n.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/noteassignment.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/actions.h>


string notes_assign_n_url ()
{
  return "notes/assign-n";
}


bool notes_assign_n_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string notes_assign_n (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  Database_NoteAssignment database_noteassignment;

  
  string page;
  Assets_Header header = Assets_Header (translate("Assign notes"), request);
  page += header.run ();
  Assets_View view;

  
  string user = request->session_logic ()->currentUser ();
 
  
  // Notes can be assigned to the assignees.
  string userblock;
  vector <string> assignees = database_noteassignment.assignees (user);
  for (auto & assignee : assignees) {
    userblock.append ("<li><a href=\"bulk?assign=" + assignee + "\">" + assignee + "</a></li>\n");
  }
  view.set_variable ("userblock", userblock);
  
  
  page += view.render ("notes", "assign-n");
  page += Assets_Page::footer ();
  return page;
}

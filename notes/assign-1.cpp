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


#include <notes/assign-1.h>
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


std::string notes_assign_1_url ()
{
  return "notes/assign-1";
}


bool notes_assign_1_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string notes_assign_1 (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);
  Database_NoteAssignment database_noteassignment;
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Assign note"), webserver_request);
  page += header.run ();
  Assets_View view;
  std::string success, error;

  
  std::string user = webserver_request.session_logic ()->currentUser ();

  
  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  view.set_variable ("id", filter::strings::convert_to_string (id));

  
  if (webserver_request.query.count ("assign")) {
    std::string assign = webserver_request.query ["assign"];
    if (database_noteassignment.exists (user, assign)) {
      notes_logic.assignUser (id, assign);
    }
    redirect_browser (webserver_request, notes_actions_url () + "?id=" + filter::strings::convert_to_string (id));
    return std::string();
  }


  // Note assignees.
  std::stringstream userblock;
  std::vector <std::string> assignees = database_noteassignment.assignees (user);
  for (auto & assignee : assignees) {
    userblock << "<li><a href=" << std::quoted ("assign-1?id=" + filter::strings::convert_to_string (id) + "&assign=" + assignee) << ">" << assignee << "</a></li>" << std::endl;
  }
  view.set_variable ("userblock", userblock.str());
  
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "assign-1");
  page += assets_page::footer ();
  return page;
}

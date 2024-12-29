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


std::string notes_assign_n_url ()
{
  return "notes/assign-n";
}


bool notes_assign_n_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string notes_assign_n (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Database_NoteAssignment database_noteassignment;

  
  std::string page{};
  Assets_Header header = Assets_Header (translate("Assign notes"), webserver_request);
  page += header.run ();
  Assets_View view;

  
  const std::string& user = webserver_request.session_logic ()->get_username ();
 
  
  // Notes can be assigned to the assignees.
  std::stringstream userblock{};
  const std::vector <std::string> assignees = database_noteassignment.assignees (user);
  for (const auto& assignee : assignees) {
    userblock << "<li><a href=" << std::quoted ("bulk?assign=" + assignee) << ">" << assignee << "</a></li>" << std::endl;
  }
  view.set_variable ("userblock", userblock.str());
  
  
  page += view.render ("notes", "assign-n");
  page += assets_page::footer ();
  return page;
}

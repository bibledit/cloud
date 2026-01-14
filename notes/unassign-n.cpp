/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <notes/unassign-n.h>
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


std::string notes_unassign_n_url ()
{
  return "notes/unassign-n";
}


bool notes_unassign_n_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string notes_unassign_n (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Unassign notes"), webserver_request);
  page += header.run ();
  Assets_View view;


  // Notes can be unassigned from users who have access to the Bibles
  // the currently logged-in user has access to, and who have notes assigned.
  std::stringstream userblock;
  std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  std::vector <std::string> users = database_notes.get_all_assignees (bibles);
  for (const auto& user : users) {
    userblock << "<li><a href=" << std::quoted ("bulk?unassign=" + user) << ">" << user << "</a></li>" << std::endl;
  }
  view.set_variable ("userblock", userblock.str());
  
  
  page += view.render ("notes", "unassign-n");
  page += assets_page::footer ();
  return page;
}

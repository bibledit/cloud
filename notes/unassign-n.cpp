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


string notes_unassign_n_url ()
{
  return "notes/unassign-n";
}


bool notes_unassign_n_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string notes_unassign_n (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Unassign notes"), request);
  page += header.run ();
  Assets_View view;


  // Notes can be unassigned from users who have access to the Bibles
  // the currently logged-in user has access to, and who have notes assigned.
  string userblock;
  vector <string> bibles = access_bible_bibles (webserver_request);
  vector <string> users = database_notes.getAllAssignees (bibles);
  for (auto & user : users) {
    userblock.append ("<li><a href=\"bulk?unassign=" + user + "\">" + user + "</a></li>\n");
  }
  view.set_variable ("userblock", userblock);
  
  
  page += view.render ("notes", "unassign-n");
  page += Assets_Page::footer ();
  return page;
}

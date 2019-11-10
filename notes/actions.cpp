/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <notes/actions.h>
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
#include <trash/handler.h>
#include <database/logs.h>
#include <styles/logic.h>
#include <access/logic.h>


string notes_actions_url ()
{
  return "notes/actions";
}


bool notes_actions_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string notes_actions (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);

  
  string page;
  Assets_Header header = Assets_Header (translate("Actions"), request);
  header.setNavigator ();
  page += header.run ();
  Assets_View view;
  string success, error;

  
  string user = request->session_logic()->currentUser ();
  int level = request->session_logic()->currentLevel ();

  
  int id;
  if (request->query.count ("id")) id = convert_to_int (request->query ["id"]);
  else id = convert_to_int (request->post ["id"]);

  
  if (request->query.count ("unsubscribe")) {
    notes_logic.unsubscribe (id);
  }
  
  
  if (request->query.count ("subscribe")) {
    notes_logic.subscribe (id);
  }
  
  
  if (request->query.count ("unassign")) {
    string unassign = request->query["unassign"];
    notes_logic.unassignUser (id, unassign);
  }
  
  
  if (request->query.count ("done")) {
    notes_logic.unassignUser (id, user);
  }
  
  
  if (request->query.count ("markdel")) {
    notes_logic.markForDeletion (id);
    success = translate("The note will be deleted after a week.") + " " + translate ("Adding a comment to the note cancels the deletion.");
  }
  
  
  if (request->query.count ("unmarkdel")) {
    notes_logic.unmarkForDeletion (id);
  }
  
  
  if (request->query.count ("delete")) {
    notes_logic.erase (id);
    redirect_browser (request, notes_index_url ());
    return "";
  }
  
  
  if (request->query.count ("publicnote")) {
    bool state = database_notes.get_public (id);
    database_notes.set_public (id, !state);
  }

  
  view.set_variable ("id", convert_to_string (id));
  
                      
  string summary = database_notes.get_summary (id);
  view.set_variable ("summary", summary);
                                          
                                          
  bool subscribed = database_notes.is_subscribed (id, user);
  if (subscribed) view.enable_zone ("subscribed");
  else view.enable_zone ("subscribe");
  

  vector <string> assignees = database_notes.get_assignees (id);
  string assigneeblock;
  for (auto & assignee : assignees) {
    assigneeblock.append (assignee);
    if (level >= Filter_Roles::manager ()) {
      assigneeblock.append ("<a href=\"?id=" + convert_to_string (id) + "&unassign=" + assignee + "\"> [" + translate("unassign") + "]</a>");
      assigneeblock.append (" | ");
    }
  }
  view.set_variable ("assigneeblock", assigneeblock);
  if (level >= Filter_Roles::manager ()) view.enable_zone ("assign");

  
  bool assigned = database_notes.is_assigned (id, user);
  if (assigned) view.enable_zone ("assigned");
  
  
  string status = database_notes.get_status (id);
  view.set_variable ("status", status);
  if (Filter_Roles::translator ()) view.enable_zone ("editstatus");
  else view.enable_zone ("viewstatus");

  
  string verses = filter_passage_display_inline (database_notes.get_passages (id));
  view.set_variable ("verses", verses);
                                          
                                          
  string severity = database_notes.get_severity (id);
  view.set_variable ("severity",  severity);

  
  string bible = database_notes.get_bible (id);
  view.set_variable ("bible", bible);
  if (bible.empty ()) view.enable_zone ("nobible");

  
  if (level >= Filter_Roles::manager ()) view.enable_zone ("rawedit");
  

  if (access_logic_privilege_delete_consultation_notes (webserver_request)) view.enable_zone ("deletenote");
  bool marked = database_notes.is_marked_for_deletion (id);
  if (marked) view.enable_zone ("marked");
  else view.enable_zone ("mark");
  
  
#ifndef HAVE_CLIENT
  view.enable_zone ("cloud");
  string on_off = styles_logic_off_on_inherit_toggle_text (database_notes.get_public (id));
  view.set_variable ("publicnote", on_off);
#endif
  // Roles of translator or higher can edit the public visibility of a note.
  if (level >= Filter_Roles::translator ()) view.enable_zone ("translator");

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "actions");
  page += Assets_Page::footer ();
  return page;
}

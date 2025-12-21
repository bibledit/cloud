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


std::string notes_actions_url ()
{
  return "notes/actions";
}


bool notes_actions_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string notes_actions (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Actions"), webserver_request);
  header.set_navigator ();
  page += header.run ();
  Assets_View view;
  std::string success, error;

  
  const std::string& user = webserver_request.session_logic ()->get_username ();
  int level = webserver_request.session_logic()->get_level ();

  
  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  if (!id) id = filter::strings::convert_to_int (webserver_request.post_get("val1"));

  
  std::string checkbox = webserver_request.post_get("checkbox");
  bool checked = filter::strings::convert_to_bool (webserver_request.post_get("checked"));


  if (webserver_request.query.count ("unsubscribe")) {
    notes_logic.unsubscribe (id);
  }
  
  
  if (webserver_request.query.count ("subscribe")) {
    notes_logic.subscribe (id);
  }
  
  
  if (webserver_request.query.count ("unassign")) {
    std::string unassign = webserver_request.query["unassign"];
    notes_logic.unassignUser (id, unassign);
  }
  
  
  if (webserver_request.query.count ("done")) {
    notes_logic.unassignUser (id, user);
  }
  
  
  if (webserver_request.query.count ("markdel")) {
    notes_logic.markForDeletion (id);
    success = translate("The note will be deleted after a week.") + " " + translate ("Adding a comment to the note cancels the deletion.");
  }
  
  
  if (webserver_request.query.count ("unmarkdel")) {
    notes_logic.unmarkForDeletion (id);
  }
  
  
  if (webserver_request.query.count ("delete")) {
    notes_logic.erase (id);
    redirect_browser (webserver_request, notes_index_url ());
    return std::string();
  }
  
  
  if (checkbox == "public") {
    database_notes.set_public (id, checked);
    return std::string();
  }

  
  view.set_variable ("id", std::to_string (id));
  
                      
  std::string summary = database_notes.get_summary (id);
  view.set_variable ("summary", summary);
                                          
                                          
  bool subscribed = database_notes.is_subscribed (id, user);
  if (subscribed) view.enable_zone ("subscribed");
  else view.enable_zone ("subscribe");
  

  std::vector <std::string> assignees = database_notes.get_assignees (id);
  std::stringstream assigneeblock;
  for (auto & assignee : assignees) {
    assigneeblock << assignee;
    if (level >= roles::manager) {
      assigneeblock << "<a href=" << std::quoted ("?id=" + std::to_string (id) + "&unassign=" + assignee) << "> [" << translate("unassign") << "]</a>";
      assigneeblock << " | ";
    }
  }
  view.set_variable ("assigneeblock", assigneeblock.str());
  if (level >= roles::manager) view.enable_zone ("assign");

  
  bool assigned = database_notes.is_assigned (id, user);
  if (assigned) view.enable_zone ("assigned");
  
  
  std::string status = database_notes.get_status (id);
  view.set_variable ("status", status);
  if (roles::translator) view.enable_zone ("editstatus");
  else view.enable_zone ("viewstatus");

  
  std::string verses = filter_passage_display_inline (database_notes.get_passages (id));
  view.set_variable ("verses", verses);
                                          
                                          
  std::string severity = database_notes.get_severity (id);
  view.set_variable ("severity",  severity);

  
  std::string bible = database_notes.get_bible (id);
  view.set_variable ("bible", bible);
  if (bible.empty ()) view.enable_zone ("nobible");

  
  if (level >= roles::manager) view.enable_zone ("rawedit");
  

  if (access_logic::privilege_delete_consultation_notes (webserver_request))
    view.enable_zone ("deletenote");
  bool marked = database_notes.is_marked_for_deletion (id);
  if (marked) view.enable_zone ("marked");
  else view.enable_zone ("mark");
  
  
#ifdef HAVE_CLOUD
  view.enable_zone ("cloud");
  view.set_variable ("public", filter::strings::get_checkbox_status (database_notes.get_public (id)));
#endif
  // Roles of translator or higher can edit the public visibility of a note.
  if (level < roles::translator) view.set_variable(filter::strings::get_disabled(), filter::strings::get_disabled());

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "actions");
  page += assets_page::footer ();
  return page;
}

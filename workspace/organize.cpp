/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#include <workspace/organize.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <workspace/logic.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <menu/logic.h>


string workspace_organize_url ()
{
  return "workspace/organize";
}


bool workspace_organize_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string workspace_organize (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string success;
  string error;

  
  if (request->post.count ("add")) {
    string add = request->post["add"];
    request->database_config_user()->setActiveWorkspace (add);
    workspace_set_urls    (request, workspace_get_default_urls (0));
    workspace_set_widths  (request, workspace_get_default_widths (0));
    workspace_set_heights (request, workspace_get_default_heights (0));
    success = translate ("The workspace was added");
  }
  
  
  // Re-ordering workspaces.
  if (request->query.count ("up")) {
    size_t item = convert_to_int (request->query ["up"]);
    vector <string> workspaces = workspace_get_names (request);
    array_move_up_down (workspaces, item, true);
    workspace_reorder (request, workspaces);
    success = translate ("The workspace was moved up");
  }
  if (request->query.count ("down")) {
    size_t item = convert_to_int (request->query ["down"]);
    vector <string> workspaces = workspace_get_names (request);
    array_move_up_down (workspaces, item, false);
    workspace_reorder (request, workspaces);
    success = translate ("The workspace was moved down");
  }
  
  
  // Create and reset all default workspaces.
  if (request->query.count ("defaults")) {
    workspace_create_defaults (webserver_request);
    success = translate ("The default workspaces were created");
  }
  
  
  string page;
  
  
  Assets_Header header = Assets_Header (translate("Workspaces"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  
  if (request->query.count ("remove")) {
    string remove = request->query["remove"];
    string confirm = request->query["confirm"];
    if (confirm.empty ()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("organize", translate("Would you like to delete this workspace configuration?"));
      dialog_yes.add_query ("remove", remove);
      page += dialog_yes.run ();
      return page;
    }
    if (confirm == "yes") {
      workspace_delete (request, remove);
      success = translate ("The workspace was removed");
    }
  }
  
  
  // Copy workspace.
  if (request->query.count ("copy")) {
    string workspace = request->query ["copy"];
    Dialog_Entry dialog_entry ("organize", translate("Please enter a name for the new workspace"), "", "destination", "");
    dialog_entry.add_query ("source", workspace);
    page.append (dialog_entry.run ());
    return page;
  }
  if (request->query.count ("source")) {
    string source = request->query ["source"];
    string destination = request->post ["entry"];
    workspace_copy (webserver_request, source, destination);
    success = translate ("The workspace was copied");
  }

  
  // Send workspace to all users.
  string send = request->query ["send"];
  if (!send.empty ()) {
    string me = request->session_logic ()->currentUser ();
    vector <string> users = request->database_users ()->getUsers ();
    for (auto user : users) {
      if (user != me) {
        workspace_send (webserver_request, send, user);
      }
    }
    success = translate ("The workspace was sent to all users");
  }

  
  Assets_View view;
  
  
  vector <string> workspaceblock;
  vector <string> workspaces = workspace_get_names (request, false);
  for (size_t i = 0; i < workspaces.size (); i++) {
    string workspace = workspaces [i];
    workspaceblock.push_back ("<p>");
    workspaceblock.push_back ("<a href=\"?remove=" + workspace + "\" title=\"" + translate("Delete workspace") + "\">" + emoji_wastebasket () + "</a>");
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<a href=\"?up=" + convert_to_string (i) + "\" title=\"" + translate("Move workspace up") + "\"> " + unicode_black_up_pointing_triangle () + " </a>");
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<a href=\"?down=" + convert_to_string (i) + "\" title=\"" + translate("Move workspace down") + "\"> " + unicode_black_down_pointing_triangle () + " </a>");
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<a href=\"settings?name=" + workspace + "\" title=\"" + translate("Edit workspace") + "\"> ✎ </a>");
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<a href=\"?copy=" + workspace + "\" title=\"" + translate("Copy workspace") + "\"> ⎘ </a>");
#ifndef HAVE_CLIENT
    // In the Cloud, one can send the workspace configuration to other users.
    // On a client, sending a workspace to other users does not work.
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<a href=\"?send=" + workspace + "\" title=\"" + translate("Send workspace to all users") + "\"> ✉ </a>");
#endif
    workspaceblock.push_back ("|");
    workspaceblock.push_back ("<span>" + workspace + "</span>");
    workspaceblock.push_back ("</p>");
  }
  view.set_variable ("workspaceblock", filter_string_implode (workspaceblock, "\n"));

  
#ifndef HAVE_CLIENT
    view.enable_zone ("cloud");
#endif

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);


  page += view.render ("workspace", "organize");
  
  
  page += Assets_Page::footer ();
  
  
  return page;
}

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


std::string workspace_organize_url ()
{
  return "workspace/organize";
}


bool workspace_organize_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string workspace_organize (Webserver_Request& webserver_request)
{
  std::string success;
  std::string error;

  
  if (webserver_request.post_count("add")) {
    const std::string add = webserver_request.post_get("add");
    webserver_request.database_config_user()->set_active_workspace (add);
    workspace_set_urls (webserver_request, workspace_get_default_urls (0));
    workspace_set_widths (webserver_request, workspace_get_default_widths (0));
    workspace_set_heights (webserver_request, workspace_get_default_heights (0));
    success = translate ("The workspace was added");
  }
  
  
  // Re-ordering workspaces.
  if (webserver_request.query.count ("up")) {
    const size_t item = static_cast<size_t>(filter::strings::convert_to_int (webserver_request.query ["up"]));
    std::vector <std::string> workspaces = workspace_get_names (webserver_request);
    filter::strings::array_move_up_down (workspaces, item, true);
    workspace_reorder (webserver_request, workspaces);
    success = translate ("The workspace was moved up");
  }
  if (webserver_request.query.count ("down")) {
    const size_t item = static_cast<size_t>(filter::strings::convert_to_int (webserver_request.query ["down"]));
    std::vector <std::string> workspaces = workspace_get_names (webserver_request);
    filter::strings::array_move_up_down (workspaces, item, false);
    workspace_reorder (webserver_request, workspaces);
    success = translate ("The workspace was moved down");
  }
  
  
  // Create and reset all default workspaces.
  if (webserver_request.query.count ("defaults")) {
    workspace_create_defaults (webserver_request);
    success = translate ("The default workspaces were created");
  }
  
  
  std::string page {};
  
  
  Assets_Header header = Assets_Header (translate("Workspaces"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  
  if (webserver_request.query.count ("remove")) {
    const std::string remove = webserver_request.query["remove"];
    const std::string confirm = webserver_request.query["confirm"];
    if (confirm.empty ()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("organize", translate("Would you like to delete this workspace configuration?"));
      dialog_yes.add_query ("remove", remove);
      page += dialog_yes.run ();
      return page;
    }
    if (confirm == "yes") {
      workspace_delete (webserver_request, remove);
      success = translate ("The workspace was removed");
    }
  }
  
  
  // Copy workspace.
  if (webserver_request.query.count ("copy")) {
    const std::string workspace = webserver_request.query ["copy"];
    Dialog_Entry dialog_entry ("organize", translate("Please enter a name for the new workspace"), "", "destination", "");
    dialog_entry.add_query ("source", workspace);
    page.append (dialog_entry.run ());
    return page;
  }
  if (webserver_request.query.count ("source")) {
    const std::string source = webserver_request.query ["source"];
    const std::string destination = webserver_request.post_get("entry");
    workspace_copy (webserver_request, source, destination);
    success = translate ("The workspace was copied");
  }

  
  // Send workspace to all users.
  const std::string send = webserver_request.query ["send"];
  if (!send.empty ()) {
    const std::string& me = webserver_request.session_logic ()->get_username ();
    const std::vector <std::string> users = webserver_request.database_users ()->get_users ();
    for (const auto& user : users) {
      if (user != me) {
        workspace_send (webserver_request, send, user);
      }
    }
    success = translate ("The workspace was sent to all users");
  }

  
  Assets_View view;
  
  
  pugi::xml_document document{};
  const std::vector <std::string> workspaces = workspace_get_names (webserver_request, false);
  for (size_t i {0}; i < workspaces.size(); i++) {
    const std::string workspace = workspaces.at(i);
    pugi::xml_node p_node = document.append_child ("p");
    const auto add_operation = [&p_node] (const std::string href, const std::string title, const std::string text) {
      pugi::xml_node a_node = p_node.append_child("a");
      a_node.append_attribute("href") = href.c_str();
      a_node.append_attribute("title") = title.c_str();
      a_node.text().set(text.c_str());
    };
    const auto add_text = [&p_node] (const std::string text) {
      pugi::xml_node span_node = p_node.append_child("span");
      span_node.text().set(text.c_str());
    };
    add_operation("?remove=" + workspace, translate("Delete workspace"), filter::strings::emoji_wastebasket());
    add_text(" | ");
    add_operation("?up=" + std::to_string(i), translate("Move workspace up"), filter::strings::unicode_black_up_pointing_triangle());
    add_text(" | ");
    add_operation("?down=" + std::to_string(i), translate("Move workspace down"), filter::strings::unicode_black_down_pointing_triangle());
    add_text(" | ");
    add_operation("settings?name=" + workspace, translate("Edit workspace"), " ✎ ");
    add_text(" | ");
    add_operation("?copy=" + workspace, translate("Copy workspace"), " ⎘ ");
#ifndef HAVE_CLIENT
    // In the Cloud, one can send the workspace configuration to other users.
    // On a client, sending a workspace to other users does not work.
    add_text(" | ");
    add_operation("?send=" + workspace, translate("Send workspace to all users"), " ✉ ");
#endif
    add_text(" | ");
    add_text(workspace);
  }
  std::stringstream workspaceblock{};
  document.print(workspaceblock, "", pugi::format_raw);
  view.set_variable ("workspaceblock", workspaceblock.str());

  
#ifndef HAVE_CLIENT
    view.enable_zone ("cloud");
#endif

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);


  page += view.render ("workspace", "organize");
  
  
  page += assets_page::footer ();
  
  
  return page;
}

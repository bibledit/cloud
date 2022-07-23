/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <workspace/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/notes.h>
#include <database/cache.h>
#include <workspace/logic.h>
#include <menu/logic.h>
#include <ipc/focus.h>
#include <navigation/passage.h>


string workspace_index_url ()
{
  return "workspace/index";
}


bool workspace_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string workspace_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <string> workspaces = workspace_get_names (request);

  // Set the requested workspace as the active one.
  if (request->query.count ("bench")) {
    size_t bench = static_cast <size_t> (convert_to_int (request->query ["bench"]));
    if (bench < workspaces.size ()) {
      string workspace = workspaces [bench];
      request->database_config_user()->setActiveWorkspace (workspace);
    }
  }
  
  
  // Check that the active workspace exists, else set the first available workspace as the active one.
  {
    string workspace = request->database_config_user ()->getActiveWorkspace ();
    if (!in_array (workspace, workspaces)) {
      if (!workspaces.empty ()) {
        request->database_config_user ()->setActiveWorkspace (workspaces [0]);
      }
    }
  }
  
  
  // Create default set of workspaces if there are none.
  bool create = workspaces.empty ();
  if (!create) {
    create = (workspaces [0] == workspace_get_default_name ());
  }
  if (create) {
    workspace_create_defaults (webserver_request);
  }

  
  // In case the workspace is opened from a consultation note email,
  // read the note, and set the active passage to the passage the note refers to.
  int noteid = convert_to_int (request->query ["note"]);
  if (noteid) {
    Database_Notes database_notes (webserver_request);
    vector <Passage> passages = database_notes.get_passages (noteid);
    if (!passages.empty ()) {
      Ipc_Focus::set (webserver_request, passages[0].m_book, passages[0].m_chapter, convert_to_int (passages[0].m_verse));
      Navigation_Passage::record_history (webserver_request, passages[0].m_book, passages[0].m_chapter, convert_to_int (passages[0].m_verse));
    }
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Workspace"), request);
  header.setNavigator ();
  header.setFadingMenu (menu_logic_workspace_category (webserver_request));
  page = header.run ();
  Assets_View view;

  
  map <int, string> urls = workspace_get_urls (request, true);
  map <int, string> widths = workspace_get_widths (request);
  // The Bible editor number, starting from 1, going up.
  map <int, int> editor_numbers = workspace_add_bible_editor_number (urls);
  for (int key = 0; key < 15; key++) {
    string url = urls [key];
    string width = widths [key];
    int editor_number = editor_numbers [key];
    int row = static_cast<int> (round (key / 5)) + 1;
    int column = key % 5 + 1;
    string variable = "url" + convert_to_string (row) + convert_to_string (column);
    view.set_variable (variable, url);
    variable = "width" + convert_to_string (row) + convert_to_string (column);
    view.set_variable (variable, width);
    if (convert_to_int (width) > 0) view.enable_zone (variable);
    variable = "editorno" + convert_to_string (row) + convert_to_string (column);
    view.set_variable (variable, convert_to_string (editor_number));
  }
  
  
  map <int, string> heights = workspace_get_heights (request);
  for (int key = 0; key < 3; key++) {
    string height = heights [key];
    int row = key + 1;
    string variable = "height" + convert_to_string (row);
    view.set_variable (variable, height);
    if (convert_to_int (height) > 0) view.enable_zone (variable);
  }
  
  
  string workspacewidth = workspace_get_entire_width (request);
  if (!workspacewidth.empty ()) {
    workspacewidth.insert (0, "width: ");
    workspacewidth.append (";");
  }
  view.set_variable ("workspacewidth", workspacewidth);
  
  
  // The rendered template disables framekillers through the "sandbox" attribute on the iframe elements.
  page += view.render ("workspace", "index");
  page += Assets_Page::footer ();
  return page;
}


// It used to use a sandbox restriction in the iframe.
// sandbox="allow-forms allow-pointer-lock allow-same-origin allow-scripts allow-modals"
// But that led to issues:
// https://github.com/bibledit/cloud/issues/744

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


std::string workspace_index_url ()
{
  return "workspace/index";
}


bool workspace_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string workspace_index (Webserver_Request& webserver_request)
{
  const std::vector <std::string> workspaces = workspace_get_names (webserver_request);

  // Set the requested workspace as the active one.
  if (webserver_request.query.count ("bench")) {
    const size_t bench = static_cast <size_t> (filter::strings::convert_to_int (webserver_request.query ["bench"]));
    if (bench < workspaces.size ()) {
      const std::string workspace = workspaces [bench];
      webserver_request.database_config_user()->setActiveWorkspace (workspace);
    }
  }
  
  
  // Check that the active workspace exists, else set the first available workspace as the active one.
  {
    const std::string workspace = webserver_request.database_config_user ()->getActiveWorkspace ();
    if (!in_array (workspace, workspaces)) {
      if (!workspaces.empty ()) {
        webserver_request.database_config_user ()->setActiveWorkspace (workspaces [0]);
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
  const int note_id = filter::strings::convert_to_int (webserver_request.query ["note"]);
  if (note_id) {
    Database_Notes database_notes (webserver_request);
    const std::vector <Passage> passages = database_notes.get_passages (note_id);
    if (!passages.empty ()) {
      Ipc_Focus::set (webserver_request, passages[0].m_book, passages[0].m_chapter, filter::strings::convert_to_int (passages[0].m_verse));
      Navigation_Passage::record_history (webserver_request, passages[0].m_book, passages[0].m_chapter, filter::strings::convert_to_int (passages[0].m_verse));
    }
  }
  
  
  std::string page{};
  Assets_Header header = Assets_Header (translate("Workspace"), webserver_request);
  header.set_navigator ();
  header.set_fading_menu (menu_logic_workspace_category (webserver_request));
  page = header.run ();
  Assets_View view;

  
  std::map <int, std::string> urls = workspace_get_urls (webserver_request, true);
  std::map <int, std::string> widths = workspace_get_widths (webserver_request);
  // The Bible editor number, starting from 1, going up.
  std::map <int, int> editor_numbers = workspace_add_bible_editor_number (urls);
  for (int key = 0; key < 15; key++) {
    const std::string url = urls [key];
    const std::string width = widths [key];
    const int editor_number = editor_numbers [key];
    const int row = static_cast<int> (round (key / 5)) + 1;
    const int column = key % 5 + 1;
    std::string variable = "url" + filter::strings::convert_to_string (row) + filter::strings::convert_to_string (column);
    view.set_variable (variable, url);
    variable = "width" + filter::strings::convert_to_string (row) + filter::strings::convert_to_string (column);
    view.set_variable (variable, width);
    if (filter::strings::convert_to_int (width) > 0) 
      view.enable_zone (variable);
    variable = "editorno" + filter::strings::convert_to_string (row) + filter::strings::convert_to_string (column);
    view.set_variable (variable, filter::strings::convert_to_string (editor_number));
  }
  
  
  std::map <int, std::string> heights = workspace_get_heights (webserver_request);
  for (int key = 0; key < 3; key++) {
    const std::string height = heights [key];
    const int row = key + 1;
    const std::string variable = "height" + filter::strings::convert_to_string (row);
    view.set_variable (variable, height);
    if (filter::strings::convert_to_int (height) > 0) 
      view.enable_zone (variable);
  }
  
  
  std::string workspacewidth = workspace_get_entire_width (webserver_request);
  if (!workspacewidth.empty ()) {
    workspacewidth.insert (0, "width: ");
    workspacewidth.append (";");
  }
  view.set_variable ("workspacewidth", workspacewidth);
  
  
  // The rendered template disables framekillers through the "sandbox" attribute on the iframe elements.
  page += view.render ("workspace", "index");
  page += assets_page::footer ();
  return page;
}


// It used to use a sandbox restriction in the iframe.
// sandbox="allow-forms allow-pointer-lock allow-same-origin allow-scripts allow-modals"
// But that led to issues:
// https://github.com/bibledit/cloud/issues/744

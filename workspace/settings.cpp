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


#include <workspace/settings.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <workspace/logic.h>
#include <workspace/index.h>
#include <dialog/yes.h>
#include <filter/url.h>
#include <menu/logic.h>
#include <workspace/organize.h>


std::string workspace_settings_url ()
{
  return "workspace/settings";
}


bool workspace_settings_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string workspace_settings (Webserver_Request& webserver_request)
{
  std::string name = webserver_request.query ["name"];
  webserver_request.database_config_user()->set_active_workspace (name);
  
  if (webserver_request.query.count ("preset")) {
    int preset = filter::string::convert_to_int (webserver_request.query ["preset"]);
    workspace_set_urls (webserver_request, workspace_get_default_urls (preset));
    workspace_set_widths (webserver_request, workspace_get_default_widths (preset));
    workspace_set_heights (webserver_request, workspace_get_default_heights (preset));
  }
  
  if (webserver_request.post_count("save")) {
    std::map <int, std::string> urls;
    std::map <int, std::string> widths;
    std::map <int, std::string> row_heights;
    int to14 = 0;
    int to2 = 0;
    for (int row = 1; row <= 3; row++) {
      for (int column = 1; column <= 5; column++) {
        std::string key = std::to_string (row) + std::to_string (column);
        urls [to14] = webserver_request.post_get("url" + key);
        widths [to14] = webserver_request.post_get("width" + key);
        to14++;
      }
      std::string key = std::to_string (row);
      row_heights [to2] = webserver_request.post_get("height" + key);
      to2++;
    }
    workspace_set_urls (webserver_request, urls);
    workspace_set_widths (webserver_request, widths);
    workspace_set_heights (webserver_request, row_heights);
    // If no "px" or "%" is given, then default to "%".
    // https://github.com/bibledit/cloud/issues/643
    std::string workspacewidth = filter::string::trim(webserver_request.post_get("workspacewidth"));
    if (!workspacewidth.empty()) {
      size_t pos_px = workspacewidth.find ("px");
      size_t pos_pct = workspacewidth.find ("%");
      if (pos_px == std::string::npos) {
        if (pos_pct == std::string::npos) {
          workspacewidth.append("%");
        }
      }
    }
    workspace_set_entire_width (webserver_request, workspacewidth);
    redirect_browser (webserver_request, workspace_index_url ());
    return std::string();
  }
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Edit workspace"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (workspace_organize_url (), menu_logic_workspace_organize_text ());
  page = header.run ();
  
  Assets_View view;
  
  std::map <int, std::string> urls = workspace_get_urls (webserver_request, false);
  std::map <int, std::string> widths = workspace_get_widths (webserver_request);
  for (const auto & element : urls) {
    int key = element.first;
    int row = static_cast<int>(round (key / 5)) + 1;
    int column = key % 5 + 1;
    std::string variable = "url" + std::to_string (row) + std::to_string (column);
    view.set_variable (variable, urls[key]);
    variable = "width" + std::to_string (row) + std::to_string (column);
    view.set_variable (variable, widths[key]);
  }
  
  std::map <int, std::string> row_heights = workspace_get_heights (webserver_request);
  for (auto & element : row_heights) {
    int key = element.first;
    int row = key + 1;
    std::string variable = "height" + std::to_string (row);
    view.set_variable (variable, row_heights [key]);
  }

  std::string workspacewidth = workspace_get_entire_width (webserver_request);
  view.set_variable ("workspacewidth", workspacewidth);
  
  view.set_variable ("name", name);
  
  
  std::vector <std::string> samples = workspace_get_default_names ();
  for (size_t i = 0; i < samples.size (); i++) {
    std::string sample = "<a href=\"settings?name=##name##&preset=" + std::to_string (i + 1) + "\">" + samples[i] + "</a>";
    samples [i] = sample;
  }
  view.set_variable ("samples", filter::string::implode (samples, "\n|\n"));
  
  
  view.set_variable ("help", translate ("See the help below for information about what to enter"));
 
  page += view.render ("workspace", "settings");
  
  page += assets_page::footer ();
  
  return page;
}

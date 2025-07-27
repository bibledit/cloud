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


#include <styles/indexm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <access/user.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <styles/logic.h>
#include <assets/header.h>
#include <menu/logic.h>


std::string styles_indexm_url ()
{
  return "styles/indexm";
}


bool styles_indexm_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string styles_indexm (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Styles"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view {};
  
  const std::string& username {webserver_request.session_logic ()->get_username ()};
  int userlevel {webserver_request.session_logic ()->get_level ()};
  
  if (webserver_request.post.count ("new")) {
    std::string name {webserver_request.post["entry"]};
    // Remove spaces at the ends of the name for the new stylesheet.
    // Because predictive keyboards can add a space to the name,
    // and the stylesheet system is not built for whitespace at the start / end of the name of the stylesheet.
    name = filter::strings::trim (name);
    std::vector <std::string> existing {database::styles::get_sheets ()};
    if (find (existing.begin(), existing.end (), name) != existing.end ()) {
      page += assets_page::error (translate("This stylesheet already exists"));
    } else {
      database::styles::create_sheet (name);
      database::styles::grant_write_access (username, name);
      styles_sheets_create_all ();
      page += assets_page::success (translate("The stylesheet has been created"));
    }
  }
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indexm", translate("Please enter the name for the new stylesheet"), std::string(), "new", std::string());
    page += dialog_entry.run();
    return page;
  }
  
  if (webserver_request.query.count ("delete")) {
    std::string del {webserver_request.query ["delete"]};
    if (!del.empty()) {
      std::string confirm {webserver_request.query ["confirm"]};
      if (confirm == "yes") {
        bool write = database::styles::has_write_access (username, del);
        if (userlevel >= roles::admin) write = true;
        if (write) {
          database::styles::delete_sheet (del);
          database::styles::revoke_write_access (std::string(), del);
          page += assets_page::success (translate("The stylesheet has been deleted"));
        }
      } if (confirm.empty()) {
        Dialog_Yes dialog_yes = Dialog_Yes ("indexm", translate("Would you like to delete this stylesheet?"));
        dialog_yes.add_query ("delete", del);
        page += dialog_yes.run ();
        return page;
      }
    }
  }
 
  // Delete empty sheet that may have been there.
  database::styles::delete_sheet (std::string());

  std::vector <std::string> sheets = database::styles::get_sheets();
  std::stringstream sheetblock {};
  for (auto & sheet : sheets) {
    sheetblock << "<p>";
    sheetblock << sheet;
    bool editable = database::styles::has_write_access (username, sheet);
    if (userlevel >= roles::admin) editable = true;
    // Cannot edit the Standard stylesheet.
    if (sheet == stylesv2::standard_sheet ()) editable = false;
    if (editable) {
      sheetblock << "<a href=" << std::quoted ("sheetm?name=" + sheet) << ">[" << translate("edit") << "]</a>";
    }
    sheetblock << "</p>";
  }
  
  view.set_variable ("sheetblock", sheetblock.str());

  page += view.render ("styles", "indexm");
  
  page += assets_page::footer ();
  
  return page;
}

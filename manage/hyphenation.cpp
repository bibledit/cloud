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


#include <manage/hyphenation.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <dialog/list.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <tasks/logic.h>
#include <database/config/bible.h>
#include <menu/logic.h>
#include <journal/index.h>
#include <webserver/request.h>


const char * manage_hyphenation_url ()
{
  return "manage/hyphenation";
}


bool manage_hyphenation_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string manage_hyphenation (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate ("Hyphenation"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  
  
  std::string success;
  std::string error;
  
  
  // Character sets submission.
  if (webserver_request.post.count ("sets")) {
    std::string firstset = webserver_request.post["firstset"];
    database::config::bible::set_hyphenation_first_set (bible, firstset);
    std::string secondset = webserver_request.post["secondset"];
    database::config::bible::set_hyphenation_second_set (bible, secondset);
    success = translate("The two sets of characters were saved");
  }
  
  
  if (webserver_request.query.count ("bible")) {
    std::string bible2 = webserver_request.query ["bible"];
    if (bible2.empty()) {
      Dialog_List dialog_list = Dialog_List ("", translate("Which Bible would you like to take the data from?"), "", "");
      std::vector <std::string> bibles = access_bible::bibles (webserver_request);
      for (auto list_bible : bibles) {
        dialog_list.add_row (list_bible, "bible", list_bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (bible2);
    }
  }
  bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  
  
  std::string firstset = database::config::bible::get_hyphenation_first_set (bible);
  std::string secondset = database::config::bible::get_hyphenation_second_set (bible);
  if (webserver_request.query.count ("run")) {
    if (bible == "") {
      error = translate("No Bible given");
    } else if (firstset == "") {
      error = translate("No first set of characters given");
    } else if (secondset == "") {
      error = translate("No second set of characters given");
    } else {
      tasks_logic_queue (task::hyphenate, {bible, webserver_request.session_logic ()->get_username ()});
      redirect_browser (webserver_request, journal_index_url ());
      return std::string();
    }
  }
  
  
  view.set_variable ("firstset", firstset);
  view.set_variable ("secondset", secondset);
  view.set_variable ("bible", bible);
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  
  page += view.render ("manage", "hyphenation");
  page += assets_page::footer ();
  return page;
}

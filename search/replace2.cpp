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


#include <search/replace2.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/temporal.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <dialog/select.h>


std::string search_replace2_url ()
{
  return "search/replace2";
}


bool search_replace2_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string search_replace2 (Webserver_Request& webserver_request)
{
  // Build the advanced replace page.
  std::string bible = webserver_request.database_config_user()->get_bible ();
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Replace"), webserver_request);
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  Assets_View view;
  {
    constexpr const char* identification {"bibleselect"};
    if (webserver_request.post.count (identification)) {
      bible = webserver_request.post.at(identification);
      webserver_request.database_config_user ()->set_bible (bible);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = access_bible::bibles (webserver_request),
      .selected = bible,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  std::stringstream script {};
  script << "var searchBible = " << std::quoted(bible) << ";";
  view.set_variable ("script", script.str());
  page += view.render ("search", "replace2");
  page += assets_page::footer ();
  return page;
}

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


#include <checks/settingspatterns.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/bible.h>
#include <access/bible.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <checks/settings.h>


std::string checks_settingspatterns_url ()
{
  return "checks/settingspatterns";
}


bool checks_settingspatterns_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return true;
#else
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


std::string checks_settingspatterns (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate ("Patterns"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (checks_settings_url (), menu_logic_checks_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  
  
  if (webserver_request.post.count ("patterns")) {
    const std::string patterns = webserver_request.post ["patterns"];
    if (!bible.empty ()) database::config::bible::set_checking_patterns (bible, patterns);
    view.set_variable ("success", translate("The patterns were saved"));
  }
  
  
  view.set_variable ("bible", bible);
  view.set_variable ("patterns", database::config::bible::get_checking_patterns (bible));
                                          
                                          
  page += view.render ("checks", "settingspatterns");
  page += assets_page::footer ();
  return page;
}

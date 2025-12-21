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


#include <checks/settingspairs.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <access/bible.h>
#include <menu/logic.h>
#include <checks/settings.h>


std::string checks_settingspairs_url ()
{
  return "checks/settingspairs";
}


bool checks_settingspairs_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return true;
#else
  return roles::access_control (webserver_request, roles::manager);
#endif
}


std::string checks_settingspairs (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate ("Matching pairs"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (checks_settings_url (), menu_logic_checks_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  view.set_variable ("bible", bible);
  
  
  if (webserver_request.post_count ("pairs")) {
    const std::string fragment = webserver_request.post_get("pairs");
    std::vector <std::string> errors {};
    std::vector <std::string> pairs = filter::strings::explode (fragment, ' ');
    bool okay {true};
    for (const auto& pair : pairs) {
      const size_t length = filter::strings::unicode_string_length (pair);
      if (length != 2) {
        errors.push_back (translate ("A pair should consist of two characters:") + " " + pair);
        okay = false;
      }
    }
    if (okay) {
      database::config::bible::set_matching_pairs (bible, fragment);
      view.set_variable ("success", translate("The pairs were saved"));
    } else {
      view.set_variable ("error", filter::strings::implode (errors, " | "));
    }
  }
  view.set_variable ("pairs", database::config::bible::get_matching_pairs (bible));
  
  
  page += view.render ("checks", "settingspairs");
  page += assets_page::footer ();
  return page;
}

/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


string checks_settingspairs_url ()
{
  return "checks/settingspairs";
}


bool checks_settingspairs_acl (void * webserver_request)
{
#ifdef HAVE_CLIENT
  (void) webserver_request;
  return true;
#else
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


string checks_settingspairs (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  Assets_Header header = Assets_Header (translate ("Matching pairs"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (checks_settings_url (), menu_logic_checks_settings_text ());
  page = header.run ();
  Assets_View view;
  
  
  string bible = access_bible_clamp (webserver_request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);
  
  
  if (request->post.count ("pairs")) {
    string fragment = request->post["pairs"];
    vector <string> errors;
    vector <string> pairs = filter_string_explode (fragment, ' ');
    bool okay = true;
    for (auto & pair : pairs) {
      size_t length = unicode_string_length (pair);
      if (length != 2) {
        errors.push_back (translate ("A pair should consist of two characters:") + " " + pair);
        okay = false;
      }
    }
    if (okay) {
      Database_Config_Bible::setMatchingPairs (bible, fragment);
      view.set_variable ("success", translate("The pairs were saved"));
    } else {
      view.set_variable ("error", filter_string_implode (errors, " | "));
    }
  }
  view.set_variable ("pairs", Database_Config_Bible::getMatchingPairs (bible));
  
  
  page += view.render ("checks", "settingspairs");
  page += Assets_Page::footer ();
  return page;
}

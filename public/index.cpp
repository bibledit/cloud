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


#include <public/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <public/logic.h>
#include <dialog/list.h>
#include <public/login.h>
#include <fonts/logic.h>
#include <database/config/bible.h>
#include <styles/css.h>
#include <config/logic.h>


std::string public_index_url ()
{
  return "public/index";
}


bool public_index_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string public_index (Webserver_Request& webserver_request)
{
  // If the person providing public feedback is not logged in, foward to the page for entering details.
  if (!webserver_request.session_logic ()->get_logged_in ()) {
    redirect_browser (webserver_request, public_login_url ());
    return std::string();
  }

  
  // Take the Bible for this user, and ensure that it is one of the Bibles that have public feedback enabled.
  std::string bible = webserver_request.database_config_user()->getBible ();
  const std::vector <std::string> public_bibles = public_logic_bibles ();
  if (!in_array (bible, public_bibles)) {
    bible.clear ();
    if (!public_bibles.empty ()) {
      bible = public_bibles [0];
    }
    webserver_request.database_config_user()->setBible (bible);
  }
  
  
  // Switch Bible before displaying the passage navigator because the navigator contains the active Bible.
  if (webserver_request.query.count ("bible")) {
    bible = webserver_request.query ["bible"];
    if (bible.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to display"), "", "");
      for (const auto& public_bible : public_bibles) {
        dialog_list.add_row (public_bible, "bible", public_bible);
      }
      Assets_Header header = Assets_Header ("", webserver_request);
      std::string page = header.run ();
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (bible);
    }
  }
  
  
  std::string page {};
  Assets_Header header = Assets_Header (translate ("Public feedback"), webserver_request);
  header.set_navigator ();
  header.set_stylesheet ();
  page = header.run ();
  Assets_View view {};
  

  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);

  
  bible = webserver_request.database_config_user()->getBible ();
  view.set_variable ("bible", bible);
  
  
  // If there's more than one Bible with public feedback enabled, the public can select a Bible.
  if (public_bibles.size () > 1) {
    view.enable_zone ("bibles");
  }

  
  const std::string clss = Filter_Css::getClass (bible);
  const std::string font = fonts::logic::get_text_font (bible);
  const int direction = database::config::bible::get_text_direction (bible);
  const int lineheight = database::config::bible::get_line_height (bible);
  const int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("custom_class", clss);
  view.set_variable ("custom_css", Filter_Css::get_css (clss, fonts::logic::get_font_path (font),
                                                        direction, lineheight, letterspacing));
  
  Styles_Css styles_css (webserver_request, stylesheet);
  styles_css.exports ();
  styles_css.generate ();
  const std::string css = styles_css.css ();
  view.set_variable ("exports_css", css);
  
  page += view.render ("public", "index");
  page += assets_page::footer ();
  return page;
}

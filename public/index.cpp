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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
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


string public_index_url ()
{
  return "public/index";
}


bool public_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);


  // If the person providing public feedback is not logged in, foward to the page for entering details.
  if (!request->session_logic ()->loggedIn ()) {
    redirect_browser (request, public_login_url ());
    return "";
  }

  
  // Take the Bible for this user, and ensure that it is one of the Bibles that have public feedback enabled.
  string bible = request->database_config_user()->getBible ();
  vector <string> public_bibles = public_logic_bibles (webserver_request);
  if (!in_array (bible, public_bibles)) {
    bible.clear ();
    if (!public_bibles.empty ()) {
      bible = public_bibles [0];
    }
    request->database_config_user()->setBible (bible);
  }
  
  
  // Switch Bible before displaying the passage navigator because the navigator contains the active Bible.
  if (request->query.count ("bible")) {
    bible = request->query ["bible"];
    if (bible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to display"), "", "");
      for (auto & bible : public_bibles) {
        dialog_list.add_row (bible, "bible", bible);
      }
      Assets_Header header = Assets_Header ("", request);
      string page = header.run ();
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (bible);
    }
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate ("Public feedback"), request);
  header.setNavigator ();
  header.setStylesheet ();
  page = header.run ();
  Assets_View view;
  

  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  
  bible = request->database_config_user()->getBible ();
  view.set_variable ("bible", bible);
  
  
  // If there's more than one Bible with public feedback enabled, the public can select a Bible.
  if (public_bibles.size () > 1) {
    view.enable_zone ("bibles");
  }

  
  string clss = Filter_Css::getClass (bible);
  string font = Fonts_Logic::getTextFont (bible);
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  view.set_variable ("custom_class", clss);
  view.set_variable ("custom_css", Filter_Css::getCss (clss,
                                                       Fonts_Logic::getFontPath (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
  Styles_Css styles_css = Styles_Css (request, stylesheet);
  styles_css.exports ();
  styles_css.generate ();
  string css = styles_css.css ();
  view.set_variable ("exports_css", css);
  
  page += view.render ("public", "index");
  page += Assets_Page::footer ();
  return page;
}

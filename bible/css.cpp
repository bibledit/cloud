/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <bible/css.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <database/books.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <fonts/logic.h>
#include <menu/logic.h>
#include <bible/manage.h>


string bible_css_url ()
{
  return "bible/css";
}


bool bible_css_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string bible_css (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Font and text direction"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", filter_string_sanitize_html (bible));
  
  // Data submission.
  if (request->post.count ("submit")) {
    
    string font = request->post ["font"];
    font = filter_string_trim (font);
#ifdef HAVE_CLIENT
    // Bibledit client storage.
    Database_Config_Bible::setTextFontClient (bible, font);
#else
    // Bibledit Cloud storage.
    Database_Config_Bible::setTextFont (bible, font);
#endif
    
    string s_direction = request->post ["direction"];
    int i_direction = Filter_Css::directionValue (s_direction);
    
    string s_mode = request->post ["mode"];
    int i_mode = Filter_Css::writingModeValue (s_mode);
    
    Database_Config_Bible::setTextDirection (bible, i_mode * 10 + i_direction);
    
    int lineheight = convert_to_int (request->post["lineheight"]);
    if (lineheight < 50) lineheight = 50;
    if (lineheight > 300) lineheight = 300;
    Database_Config_Bible::setLineHeight (bible, lineheight);

    float letterspacing = convert_to_float (request->post["letterspacing"]);
    if (letterspacing < -3) letterspacing = -3;
    if (letterspacing > 3) letterspacing = 3;
    Database_Config_Bible::setLetterSpacing (bible, 10 * letterspacing);
    
    page += Assets_Page::success ("The information was saved.");
    
  }

#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#endif
  
  string font = Fonts_Logic::getTextFont (bible);
  view.set_variable ("font", font);

  int direction = Database_Config_Bible::getTextDirection (bible);
  
  view.set_variable ("direction_none", Filter_Css::directionUnspecified (direction));
  view.set_variable ("direction_ltr", Filter_Css::directionLeftToRight (direction));
  view.set_variable ("direction_rtl", Filter_Css::directionRightToLeft (direction));
  
  view.set_variable ("mode_none", Filter_Css::writingModeUnspecified (direction));
  view.set_variable ("mode_tblr", Filter_Css::writingModeTopBottomLeftRight (direction));
  view.set_variable ("mode_tbrl", Filter_Css::writingModeTopBottomRightLeft (direction));
  view.set_variable ("mode_btlr", Filter_Css::writingModeBottomTopLeftRight (direction));
  view.set_variable ("mode_btrl", Filter_Css::writingModeBottomTopRightLeft (direction));

  int lineheight = Database_Config_Bible::getLineHeight (bible);
  view.set_variable ("lineheight", convert_to_string (lineheight));

  float letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  letterspacing /= 10;
  view.set_variable ("letterspacing", convert_to_string (letterspacing));

  string cls = Filter_Css::getClass (bible);
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::getCss
                     (cls,
                      Fonts_Logic::getFontPath (font), direction,
                      lineheight,
                      Database_Config_Bible::getLetterSpacing (bible)));

  page += view.render ("bible", "css");
  
  page += Assets_Page::footer ();
  
  return page;
}

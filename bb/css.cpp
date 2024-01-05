/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#include <bb/css.h>
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
#include <bb/manage.h>


std::string bible_css_url ()
{
  return "bible/css";
}


bool bible_css_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string bible_css (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Font and text direction"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  // The name of the Bible.
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  view.set_variable ("bible", filter::strings::escape_special_xml_characters (bible));
  
  // Data submission.
  if (webserver_request.post.count ("submit")) {
    
    std::string font = webserver_request.post ["font"];
    font = filter::strings::trim (font);
#ifdef HAVE_CLIENT
    // Bibledit client storage.
    Database_Config_Bible::setTextFontClient (bible, font);
#else
    // Bibledit Cloud storage.
    Database_Config_Bible::setTextFont (bible, font);
#endif
    
    const std::string s_direction = webserver_request.post ["direction"];
    const int i_direction = Filter_Css::directionValue (s_direction);
    
    const std::string s_mode = webserver_request.post ["mode"];
    const int i_mode = Filter_Css::writingModeValue (s_mode);
    
    Database_Config_Bible::setTextDirection (bible, i_mode * 10 + i_direction);
    
    int lineheight = filter::strings::convert_to_int (webserver_request.post["lineheight"]);
    if (lineheight < 50) lineheight = 50;
    if (lineheight > 300) lineheight = 300;
    Database_Config_Bible::setLineHeight (bible, lineheight);

    float letterspacing = filter::strings::convert_to_float (webserver_request.post["letterspacing"]);
    if (letterspacing < -3) letterspacing = -3;
    if (letterspacing > 3) letterspacing = 3;
    Database_Config_Bible::setLetterSpacing (bible, static_cast<int>(10 * letterspacing));
    
    page += assets_page::success ("The information was saved.");
  }

#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#endif
  
  const std::string font = fonts::logic::get_text_font (bible);
  view.set_variable ("font", font);

  const int direction = Database_Config_Bible::getTextDirection (bible);
  
  view.set_variable ("direction_none", Filter_Css::directionUnspecified (direction));
  view.set_variable ("direction_ltr", Filter_Css::directionLeftToRight (direction));
  view.set_variable ("direction_rtl", Filter_Css::directionRightToLeft (direction));
  
  view.set_variable ("mode_none", Filter_Css::writingModeUnspecified (direction));
  view.set_variable ("mode_tblr", Filter_Css::writingModeTopBottomLeftRight (direction));
  view.set_variable ("mode_tbrl", Filter_Css::writingModeTopBottomRightLeft (direction));
  view.set_variable ("mode_btlr", Filter_Css::writingModeBottomTopLeftRight (direction));
  view.set_variable ("mode_btrl", Filter_Css::writingModeBottomTopRightLeft (direction));

  const int lineheight = Database_Config_Bible::getLineHeight (bible);
  view.set_variable ("lineheight", filter::strings::convert_to_string (lineheight));

  float letterspacing = static_cast<float> (Database_Config_Bible::getLetterSpacing (bible));
  letterspacing /= 10;
  view.set_variable ("letterspacing", filter::strings::convert_to_string (letterspacing));

  const std::string custom_class = Filter_Css::getClass (bible);
  view.set_variable ("custom_class", custom_class);
  const std::string custom_css = Filter_Css::get_css (custom_class,
                                                      fonts::logic::get_font_path (font), direction,
                                                      lineheight,
                                                      Database_Config_Bible::getLetterSpacing (bible));
  view.set_variable ("custom_css", custom_css);

  page += view.render ("bb", "css");
  
  page += assets_page::footer ();
  
  return page;
}

/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <assets/header.h>
#include <filter/css.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <navigation/passage.h>
#include <menu/logic.h>
#include <index/index.h>
#include <webserver/request.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/cache.h>
using namespace std;


Assets_Header::Assets_Header (string title, void * webserver_request)
{
  m_webserver_request = webserver_request;
  m_view = new Assets_View ();
  m_view->set_variable ("title", title);
}


Assets_Header::~Assets_Header ()
{
  delete m_view;
}


void Assets_Header::jquery_touch_on ()
{
  m_jquery_touch_on = true;
}


void Assets_Header::touch_css_on ()
{
  m_touch_css_on = true;
}


void Assets_Header::notify_it_on ()
{
  m_notify_it_on = true;
}


// Display the passage navigator.
void Assets_Header::set_navigator ()
{
  m_display_navigator = true;
}


// Display the user's basic stylesheet.css.
void Assets_Header::set_stylesheet ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(m_webserver_request);
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  m_included_stylesheet = stylesheet;
}


// Display the user's editor stylesheet.css.
void Assets_Header::set_editor_stylesheet ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(m_webserver_request);
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  m_included_editor_stylesheet = stylesheet;
}


// Whether to display the topbar.
bool Assets_Header::display_topbar ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(m_webserver_request);
  // If the topbar is in the query: Don't display the top bar.
  if (request->query.count ("topbar")) {
    return false;
  }
  // Display the topbar.
  return true;
}


// Sets the page to refresh after "seconds".
void Assets_Header::refresh (int seconds, string url)
{
  string content = filter::strings::convert_to_string (seconds);
  if (!url.empty ()) content.append (";URL=" + url);
  stringstream ss;
  ss << "<META HTTP-EQUIV=" << quoted("refresh") << " CONTENT=" << quoted(content) << ">";
  m_head_lines.push_back (ss.str());
}


// Adds a menu item to the fading menu.
void Assets_Header::set_fading_menu (string html)
{
  m_fading_menu = html;
}


// Add one breadcrumb $item with $text.
void Assets_Header::add_bread_crumb (string item, string text)
{
  m_bread_crumbs.push_back (pair (item, text));
}


// Runs the header.
string Assets_Header::run ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(m_webserver_request);

  string page;
  
  // Include the software version number in the stylesheet and javascript URL
  // to refresh the browser's cache after a software upgrade.
  m_view->set_variable("VERSION", config::logic::version ());

  if (m_jquery_touch_on) {
    m_view->enable_zone ("include_jquery_touch");
  }

  if (request->session_logic ()->touchEnabled ()) {
    touch_css_on();
  }
  if (!request->session_logic ()->loggedIn ()) {
    touch_css_on();
  }
  if (m_touch_css_on) {
    m_view->enable_zone ("include_touch_css");
  } else {
    m_view->enable_zone ("include_mouse_css");
  }
  
  if (m_notify_it_on) {
    m_view->enable_zone ("include_notif_it");
  }
  
  string headlines;
  for (auto & headline : m_head_lines) {
    if (!headlines.empty ()) headlines.append ("\n");
    headlines.append (headline);
  }
  m_view->set_variable ("head_lines", headlines);

  if (!m_included_stylesheet.empty ()) {
    m_view->enable_zone ("include_stylesheet");
    m_view->set_variable ("included_stylesheet", m_included_stylesheet);
  }
  if (!m_included_editor_stylesheet.empty ()) {
    m_view->enable_zone ("include_editor_stylesheet");
    m_view->set_variable ("included_editor_stylesheet", m_included_editor_stylesheet);
  }

  bool basic_mode = config::logic::basic_mode (m_webserver_request);
  string basicadvanced;
  if (basic_mode) basicadvanced = "basic";
  else basicadvanced = "advanced";
  m_view->set_variable ("basicadvanced", basicadvanced);

  if (display_topbar ()) {
    m_view->enable_zone ("display_topbar");
    
    // In basic mode there's no back button in a bare browser.
    if (basic_mode) {
      m_view->disable_zone ("bare_browser");
    }
    
    // The start button to be displayed only when there's no menu.
    bool start_button = true;
    
    // Whether tabbed mode is on.
    bool tabbed_mode_on = menu_logic_can_do_tabbed_mode () && Database_Config_General::getMenuInTabbedViewOn ();
    
    string menublock;
    string item = request->query ["item"];
    bool main_menu_always_on = false;
    if (item.empty ())
      if (request->database_config_user ()->getMainMenuAlwaysVisible ()) {
        main_menu_always_on = true;
        // Add the main menu status as a Javascript variable.
        m_view->set_variable ("mainmenualwayson", filter::strings::convert_to_string (main_menu_always_on));
			}
    if ((item == "main") || main_menu_always_on) {
      if (basic_mode) {
        // Basic mode gives basic menu, but nothing in tabbed mode.
        if (!tabbed_mode_on) {
          menublock = menu_logic_basic_categories (m_webserver_request);
        }
      } else {
        string devnull;
        menublock = menu_logic_main_categories (m_webserver_request, devnull);
      }
      start_button = false;
    } else if (item == menu_logic_translate_menu ()) {
      menublock = menu_logic_translate_category (m_webserver_request);
    } else if (item == menu_logic_search_menu ()) {
      menublock = menu_logic_search_category (m_webserver_request);
    } else if (item == menu_logic_tools_menu ()) {
      menublock = menu_logic_tools_category (m_webserver_request);
    } else if (item == menu_logic_settings_menu ()) {
      menublock = menu_logic_settings_category (m_webserver_request);
    } else if (item == menu_logic_settings_resources_menu ()) {
      menublock = menu_logic_settings_resources_category (m_webserver_request);
    } else if (item == "help") {
      menublock = menu_logic_help_category (m_webserver_request);
    }
    m_view->set_variable ("mainmenu", menublock);

    // Not to display the "start button" in tabbed mode.
    // That would take up screen space unnecessarily.
    if (tabbed_mode_on) start_button = false;

    if (start_button) {
      m_view->enable_zone ("start_button");
      string tooltip;
      menu_logic_main_categories (m_webserver_request, tooltip);
      m_view->set_variable ("starttooltip", tooltip);
    }
    
    if (!m_fading_menu.empty ()) {
      m_view->enable_zone ("fading_menu");
      m_view->set_variable ("fadingmenu", m_fading_menu);
      string delay = filter::strings::convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()) + "000";
      m_view->set_variable ("fadingmenudelay", delay);
      m_fading_menu.clear ();
    }

    if (m_display_navigator) {
      m_view->enable_zone ("display_navigator");
      // string bible = access_bible::clamp (request, request->database_config_user()->getBible ());
      // The clamping above does not work for public feedback as it would reset the Bible always.
      string bible = request->database_config_user()->getBible ();
      m_view->set_variable ("navigation_code", Navigation_Passage::code (bible));
    }
  }

  vector <string> embedded_css;
  int fontsize = request->database_config_user ()->getGeneralFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back ("body { font-size: " + filter::strings::convert_to_string (fontsize) + "%; }");
  }
  fontsize = request->database_config_user ()->getMenuFontSize ();
  string filename = menu_font_size_filebased_cache_filename (request->session_identifier);
  if (fontsize != 100) {
    embedded_css.push_back (".menu-advanced, .menu-basic { font-size: " + filter::strings::convert_to_string (fontsize) + "%; }");
  }
  fontsize = request->database_config_user ()->getBibleEditorsFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".bibleeditor { font-size: " + filter::strings::convert_to_string (fontsize) + "% !important; }");
  }
  fontsize = request->database_config_user ()->getResourcesFontSize ();
  filename = resource_font_size_filebased_cache_filename (request->session_identifier);
  if (fontsize != 100) {
    embedded_css.push_back (".resource { font-size: " + filter::strings::convert_to_string (fontsize) + "% !important; }");
  }
  fontsize = request->database_config_user ()->getHebrewFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".hebrew { font-size: " + filter::strings::convert_to_string (fontsize) + "%!important; }");
  }
  fontsize = request->database_config_user ()->getGreekFontSize ();
  filename = greek_font_size_filebased_cache_filename (request->session_identifier);
  if (fontsize != 100) {
    embedded_css.push_back (".greek { font-size: " + filter::strings::convert_to_string (fontsize) + "%!important; }");
  }
  if (!embedded_css.empty ()) {
    m_view->set_variable ("embedded_css", filter::strings::implode (embedded_css, "\n"));
  }

  int current_theme_index = request->database_config_user ()->getCurrentTheme ();
  filename = current_theme_filebased_cache_filename (request->session_identifier);
  // Add the theme color css class selector name on the body element,..
  m_view->set_variable ("body_theme_color", Filter_Css::theme_picker (current_theme_index, 0));
  // ..workspacewrapper div element..
  m_view->set_variable ("workspace_theme_color", Filter_Css::theme_picker (current_theme_index, 4));
  // ..and as a variable for JavaScript.
  m_view->set_variable ("themecolorfortabs", Filter_Css::theme_picker (current_theme_index, 1));

  if (request->database_config_user ()->getDisplayBreadcrumbs ()) {
    if (!m_bread_crumbs.empty ()) {
      // No bread crumbs in basic mode.
      // The crumbs would be incorrect anyway, because they show the trail of advanced mode.
      if (!config::logic::basic_mode (m_webserver_request)) {
        stringstream track;
        track << "<a href=" << quoted(index_index_url ()) << ">";
        track << menu_logic_menu_text ("") << "</a>";
        for (auto & crumb : m_bread_crumbs) {
          track << " » ";
          if (!crumb.first.empty ()) {
            track << "<a href=" << quoted("/" + menu_logic_menu_url (crumb.first)) << ">";
          }
          track << crumb.second;
          if (!crumb.first.empty ()) {
            track << "</a>";
          }
        }
        m_view->enable_zone("breadcrumbs");
        m_view->set_variable ("breadcrumbs", track.str());
      }
    }
  }

  page += m_view->render("assets", "xhtml_start");
  page += m_view->render("assets", "header");
  page += m_view->render("assets", "workspacewrapper_start");

  return page;
}



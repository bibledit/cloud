/*
Copyright (©) 2003-2021 Teus Benschop.

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


Assets_Header::Assets_Header (string title, void * webserver_request_in)
{
  includeJQueryTouch = false;
  includeTouchCSS = false;
  includeNotifIt = false;
  displayNavigator = false;
  webserver_request = webserver_request_in;
  view = new Assets_View ();
  view->set_variable ("title", title);
}


Assets_Header::~Assets_Header ()
{
  delete view;
}


void Assets_Header::jQueryTouchOn ()
{
  includeJQueryTouch = true;
}


void Assets_Header::touchCSSOn ()
{
  includeTouchCSS = true;
}


void Assets_Header::notifItOn ()
{
  includeNotifIt = true;
}


// Display the passage navigator.
void Assets_Header::setNavigator ()
{
  displayNavigator = true;
}


// Display the user's basic stylesheet.css.
void Assets_Header::setStylesheet ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  includedStylesheet = stylesheet;
}


// Display the user's editor stylesheet.css.
void Assets_Header::setEditorStylesheet ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  includedEditorStylesheet = stylesheet;
}


// Whether to display the topbar.
bool Assets_Header::displayTopbar ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
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
  string content = convert_to_string (seconds);
  if (!url.empty ()) content.append (";URL=" + url);
  string headline = "<META HTTP-EQUIV=\"refresh\" CONTENT=\"" + content + "\">";
  headLines.push_back (headline);
}


// Adds a menu item to the fading menu.
void Assets_Header::setFadingMenu (string html)
{
  fadingmenu = html;
}


// Add one breadcrumb $item with $text.
void Assets_Header::addBreadCrumb (string item, string text)
{
  breadcrumbs.push_back (make_pair (item, text));
}


// Runs the header.
string Assets_Header::run ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;
  
  // Include the software version number in the stylesheet and javascript URL
  // to refresh the browser's cache after a software upgrade.
  view->set_variable("VERSION", config_logic_version ());

  if (includeJQueryTouch) {
    view->enable_zone ("include_jquery_touch");
  }

  if (request->session_logic ()->touchEnabled ()) {
    touchCSSOn();
  }
  if (!request->session_logic ()->loggedIn ()) {
    touchCSSOn();
  }
  if (includeTouchCSS) {
    view->enable_zone ("include_touch_css");
  } else {
    view->enable_zone ("include_mouse_css");
  }
  
  if (includeNotifIt) {
    view->enable_zone ("include_notif_it");
  }
  
  string headlines;
  for (auto & headline : headLines) {
    if (!headlines.empty ()) headlines.append ("\n");
    headlines.append (headline);
  }
  view->set_variable ("head_lines", headlines);

  if (!includedStylesheet.empty ()) {
    view->enable_zone ("include_stylesheet");
    view->set_variable ("included_stylesheet", includedStylesheet);
  }
  if (!includedEditorStylesheet.empty ()) {
    view->enable_zone ("include_editor_stylesheet");
    view->set_variable ("included_editor_stylesheet", includedEditorStylesheet);
  }

  bool basic_mode = config_logic_basic_mode (webserver_request);
  string basicadvanced;
  if (basic_mode) basicadvanced = "basic";
  else basicadvanced = "advanced";
  view->set_variable ("basicadvanced", basicadvanced);

  if (displayTopbar ()) {
    view->enable_zone ("display_topbar");
    
    // In basic mode there's no back button in a bare browser.
    if (basic_mode) {
      view->disable_zone ("bare_browser");
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
        view->set_variable ("mainmenualwayson", convert_to_string (main_menu_always_on));
			}
    if ((item == "main") || main_menu_always_on) {
      if (basic_mode) {
        // Basic mode gives basic menu, but nothing in tabbed mode.
        if (!tabbed_mode_on) {
          menublock = menu_logic_basic_categories (webserver_request);
        }
      } else {
        string devnull;
        menublock = menu_logic_main_categories (webserver_request, devnull);
      }
      start_button = false;
    } else if (item == menu_logic_translate_menu ()) {
      menublock = menu_logic_translate_category (webserver_request);
    } else if (item == menu_logic_search_menu ()) {
      menublock = menu_logic_search_category (webserver_request);
    } else if (item == menu_logic_tools_menu ()) {
      menublock = menu_logic_tools_category (webserver_request);
    } else if (item == menu_logic_settings_menu ()) {
      menublock = menu_logic_settings_category (webserver_request);
    } else if (item == menu_logic_settings_resources_menu ()) {
      menublock = menu_logic_settings_resources_category (webserver_request);
    } else if (item == "help") {
      menublock = menu_logic_help_category (webserver_request);
    }
    view->set_variable ("mainmenu", menublock);

    // Not to display the "start button" in tabbed mode.
    // That would take up screen space unnecessarily.
    if (tabbed_mode_on) start_button = false;

    if (start_button) {
      view->enable_zone ("start_button");
      string tooltip;
      menu_logic_main_categories (webserver_request, tooltip);
      view->set_variable ("starttooltip", tooltip);
    }
    
    if (!fadingmenu.empty ()) {
      view->enable_zone ("fading_menu");
      view->set_variable ("fadingmenu", fadingmenu);
      string delay = convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()) + "000";
      view->set_variable ("fadingmenudelay", delay);
      fadingmenu.clear ();
    }

    if (displayNavigator) {
      view->enable_zone ("display_navigator");
      // string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
      // The clamping above does not work for public feedback as it would reset the Bible always.
      string bible = request->database_config_user()->getBible ();
      view->set_variable ("navigation_code", Navigation_Passage::code (bible));
    }
  }

  vector <string> embedded_css;
  int fontsize = request->database_config_user ()->getGeneralFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back ("body { font-size: " + convert_to_string (fontsize) + "%; }");
  }
  fontsize = request->database_config_user ()->getMenuFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".menu-advanced, .menu-basic { font-size: " + convert_to_string (fontsize) + "%; }");
  }
  fontsize = request->database_config_user ()->getBibleEditorsFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".bibleeditor { font-size: " + convert_to_string (fontsize) + "% !important; }");
  }
  fontsize = request->database_config_user ()->getResourcesFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".resource { font-size: " + convert_to_string (fontsize) + "% !important; }");
  }
  fontsize = request->database_config_user ()->getHebrewFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".hebrew { font-size: " + convert_to_string (fontsize) + "%!important; }");
  }
  fontsize = request->database_config_user ()->getGreekFontSize ();
  if (fontsize != 100) {
    embedded_css.push_back (".greek { font-size: " + convert_to_string (fontsize) + "%!important; }");
  }
  if (!embedded_css.empty ()) {
    view->set_variable ("embedded_css", filter_string_implode (embedded_css, "\n"));
  }

  int current_theme_index = convert_to_int (request->database_config_user ()->getCurrentTheme ());
  // Add the theme color css class selector name on the body element,..
  view->set_variable ("body_theme_color", Filter_Css::theme_picker (current_theme_index, 0));
  // ..workspacewrapper div element..
  view->set_variable ("workspace_theme_color", Filter_Css::theme_picker (current_theme_index, 4));
  // ..and as a variable for JavaScript.
  view->set_variable ("themecolorfortabs", Filter_Css::theme_picker (current_theme_index, 1));

  if (request->database_config_user ()->getDisplayBreadcrumbs ()) {
    if (!breadcrumbs.empty ()) {
      // No bread crumbs in basic mode.
      // The crumbs would be incorrect anyway, because they show the trail of advanced mode.
      if (!config_logic_basic_mode (webserver_request)) {
        string track;
        track.append ("<a href=\"/");
        track.append (index_index_url ());
        track.append ("\">");
        track.append (menu_logic_menu_text (""));
        track.append ("</a>");
        for (auto & crumb : breadcrumbs) {
          track.append (" » ");
          if (!crumb.first.empty ()) {
            track.append ("<a href=\"/");
            track.append (menu_logic_menu_url (crumb.first));
            track.append ("\">");
          }
          track.append (crumb.second);
          if (!crumb.first.empty ()) {
            track.append ("</a>");
          }
        }
        view->enable_zone("breadcrumbs");
        view->set_variable ("breadcrumbs", track);
      }
    }
  }

  page += view->render("assets", "xhtml_start");
  page += view->render("assets", "header");
  page += view->render("assets", "workspacewrapper_start");

  return page;
}



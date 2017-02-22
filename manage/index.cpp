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


#include <manage/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <fonts/logic.h>
#include <client/logic.h>
#include <assets/header.h>
#include <menu/logic.h>


string manage_index_url ()
{
  return "manage/index";
}


bool manage_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string manage_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;

  Assets_Header header = Assets_Header (translate("Manage"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  Assets_View view;
  string success;
  string error;
  
  
  // Force re-index Bibles.
  if (request->query ["reindex"] == "bibles") {
    Database_Config_General::setIndexBibles (true);
    tasks_logic_queue (REINDEXBIBLES, {"1"});
    redirect_browser (request, journal_index_url ());
    return "";
  }
  
  
  // Re-index consultation notes.
  if (request->query ["reindex"] == "notes") {
    Database_Config_General::setIndexNotes (true);
    tasks_logic_queue (REINDEXNOTES);
    redirect_browser (request, journal_index_url ());
    return "";
  }

  
  // Delete a font.
  string deletefont = request->query ["deletefont"];
  if (!deletefont.empty ()) {
    string font = filter_url_basename_web (deletefont);
    bool font_in_use = false;
    vector <string> bibles = request->database_bibles ()->getBibles ();
    for (auto & bible : bibles) {
      if (font == Fonts_Logic::getTextFont (bible)) font_in_use = true;
    }
    if (!font_in_use) {
      // Only delete a font when it is not in use.
      Fonts_Logic::erase (font);
    } else {
      error = translate("The font could not be deleted because it is in use");
    }
  }
  
  
  // Upload a font.
  if (request->post.count ("uploadfont")) {
    string filename = request->post ["filename"];
    string path = filter_url_create_root_path ("fonts", filename);
    string fontdata = request->post ["fontdata"];
    filter_url_file_put_contents (path, fontdata);
    success = translate("The font has been uploaded.");
  }
  

  // Assemble the font block html.
  vector <string> fonts = Fonts_Logic::getFonts ();
  vector <string> fontsblock;
  for (auto & font : fonts) {
    fontsblock.push_back ("<p>");
#ifndef HAVE_CLIENT
    fontsblock.push_back ("<a href=\"?deletefont=" + font+ "\" title=\"" + translate("Delete font") + "\">" + emoji_wastebasket () + "</a>");
#endif
    fontsblock.push_back (font);
    fontsblock.push_back ("</p>");
  }
  view.set_variable ("fontsblock", filter_string_implode (fontsblock, "\n"));

  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  view.set_variable ("cloudlink", client_logic_link_to_cloud (manage_index_url (), ""));
#else
  view.enable_zone ("server");
#endif
  
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("manage", "index");

  page += Assets_Page::footer ();
  
  return page;
}

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


#include <bible/abbreviations.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/abbreviations.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <bible/manage.h>


string bible_abbreviations_url ()
{
  return "bible/abbreviations";
}


bool bible_abbreviations_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string bible_abbreviations (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Abbreviations"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  string success_message;
  string error_message;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", filter_string_sanitize_html (bible));
  
  // Data submission.
  if (request->post.count ("submit")) {
    string data = request->post ["data"];
    data = filter_string_trim (data);
    if (data != "") {
      if (unicode_string_is_valid (data)) {
        Database_Config_Bible::setBookAbbreviations (bible, data);
        success_message = translate("The abbreviations were saved.");
      } else {
        error_message = translate("Please supply valid Unicode UTF-8 text.");
      }
    } else {
      success_message = translate("Nothing was saved.");
    }
  }

  string data = Database_Config_Bible::getBookAbbreviations (bible);
  data = filter_abbreviations_display (data);
  view.set_variable ("data", data);
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  page += view.render ("bible", "abbreviations");
  
  page += Assets_Page::footer ();
  
  return page;
}

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


#include <checks/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/check.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <access/bible.h>
#include <menu/logic.h>
#include <checks/settings.h>


std::string checks_index_url ()
{
  return "checks/index";
}


bool checks_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string checks_index (Webserver_Request& webserver_request)
{
  Database_Check database_check {};

  
  std::string page {};
  Assets_Header header = Assets_Header (translate("Checks"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view {};
  

  if (webserver_request.query.count ("approve")) {
    const int approve = filter::strings::convert_to_int (webserver_request.query["approve"]);
    database_check.approve (approve);
    view.set_variable ("success", translate("The entry was suppressed."));
  }
  
                        
  if (webserver_request.query.count ("delete")) {
    const int erase = filter::strings::convert_to_int (webserver_request.query["delete"]);
    database_check.erase (erase);
    view.set_variable ("success", translate("The entry was deleted for just now."));
  }

  
  // Get the Bibles the user has write-access to.
  std::vector <std::string> bibles {};
  {
    const std::vector <std::string>& all_bibles = webserver_request.database_bibles()->get_bibles ();
    for (const auto& bible : all_bibles) {
      if (access_bible::write (webserver_request, bible)) {
        bibles.push_back (bible);
      }
    }
  }
  
  
  std::stringstream resultblock {};
  const std::vector <Database_Check_Hit>& hits = database_check.getHits ();
  for (const auto& hit : hits) {
    std::string bible = hit.bible;
    if (find (bibles.begin(), bibles.end (), bible) != bibles.end ()) {
      const int id = hit.rowid;
      bible = filter::strings::escape_special_xml_characters (bible);
      int book = hit.book;
      int chapter = hit.chapter;
      int verse = hit.verse;
      const std::string link = filter_passage_link_for_opening_editor_at (book, chapter, filter::strings::convert_to_string (verse));
      const std::string information = filter::strings::escape_special_xml_characters (hit.data);
      resultblock << "<p>\n";
      resultblock << "<a href=" << std::quoted("index?approve=" + filter::strings::convert_to_string (id)) << "> ✔ </a>\n";
      resultblock << "<a href=" << std::quoted ("index?delete=" + filter::strings::convert_to_string (id)) << ">" << filter::strings::emoji_wastebasket () << "</a>\n";
      resultblock << bible;
      resultblock << " ";
      resultblock << link;
      resultblock << " ";
      resultblock << information;
      resultblock << "</p>\n";
    }
  }
  view.set_variable ("resultblock", resultblock.str());

  
  if (checks_settings_acl (webserver_request)) {
    view.enable_zone ("can_enable");
  } else {
    view.enable_zone ("cannot_enable");
  }


  page += view.render ("checks", "index");
  page += assets_page::footer ();
  return page;
}

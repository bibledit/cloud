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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


std::string checks_index_url ()
{
  return "checks/index";
}


bool checks_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string checks_index (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Checks"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view {};
  

  if (webserver_request.query.count ("approve")) {
    const int approve = filter::strings::convert_to_int (webserver_request.query["approve"]);
    database::check::approve (approve);
    view.set_variable ("success", translate("The entry was suppressed."));
  }
  
                        
  if (webserver_request.query.count ("delete")) {
    const int erase = filter::strings::convert_to_int (webserver_request.query["delete"]);
    database::check::erase (erase);
    view.set_variable ("success", translate("The entry was deleted for just now."));
  }

  
  // Get the Bibles the user has write-access to.
  std::vector <std::string> bibles {};
  {
    const std::vector <std::string>& all_bibles = database::bibles::get_bibles ();
    for (const auto& bible : all_bibles) {
      if (access_bible::write (webserver_request, bible)) {
        bibles.push_back (bible);
      }
    }
  }
  
  
  pugi::xml_document document {};
  const std::vector <database::check::Hit>& hits = database::check::get_hits ();
  for (const auto& hit : hits) {
    std::string bible = hit.bible;
    if (find (bibles.begin(), bibles.end (), bible) == bibles.end ())
      continue;
    const int id = hit.rowid;
    bible = filter::strings::escape_special_xml_characters (bible);
    const int book = hit.book;
    const int chapter = hit.chapter;
    const int verse = hit.verse;
    const std::string information = filter::strings::escape_special_xml_characters (hit.data);
    pugi::xml_node p = document.append_child("p");
    // Add the Bible.
    p.append_child("span").text().set(bible.c_str());
    // Add space.
    p.append_child("span").text().set(" ");
    // Add the link to open the passage in a Bible editor.
    filter_passage_link_for_opening_editor_at (p, book, chapter, std::to_string (verse));
    // Add space.
    p.append_child("span").text().set(" ");
    // Add the checking info.
    p.append_child("span").text().set(information.c_str());
    {
      // Add link to approve this checking result.
      pugi::xml_node a = p.append_child("a");
      const std::string href = "index?approve=" + std::to_string (id);
      a.append_attribute("href") = href.c_str();
      const std::string text = " [" + translate("approve") + "]";
      a.text ().set(text.c_str());
    }
    {
      // Add link to delete this checking result.
      pugi::xml_node a = p.append_child("a");
      const std::string href = "index?delete=" + std::to_string (id);
      a.append_attribute("href") = href.c_str();
      const std::string text = " [" + translate("delete") + "] ";
      a.text ().set(text.c_str());
    }
  }
  {
    std::stringstream ss {};
    document.print(ss, "", pugi::format_raw);
    view.set_variable ("resultblock", std::move(ss).str());
  }

  
  if (checks_settings_acl (webserver_request)) {
    view.enable_zone ("can_enable");
  } else {
    view.enable_zone ("cannot_enable");
  }


  page += view.render ("checks", "index");
  page += assets_page::footer ();
  return page;
}

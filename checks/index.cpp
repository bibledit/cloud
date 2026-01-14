/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <checks/issues.h>


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
  Assets_Header header = Assets_Header (translate("Checks"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  std::string page = header.run ();
  Assets_View view {};
  

  if (webserver_request.query.count ("approve")) {
    const int approve = filter::string::convert_to_int (webserver_request.query["approve"]);
    database::check::approve (approve);
    view.set_variable ("success", translate("The entry was suppressed."));
  }
  
                        
  if (webserver_request.query.count ("delete")) {
    const int id = filter::string::convert_to_int (webserver_request.query["delete"]);
    database::check::delete_id (id);
    view.set_variable ("success", translate("The entry was deleted for just now."));
  }

  
  if (webserver_request.query.count ("deleteall")) {
    database::check::delete_output(std::string());
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

  
  if (webserver_request.query.count ("deletegroup")) {
    const int group = filter::string::convert_to_int (webserver_request.query.at("deletegroup"));
    using namespace checks::issues;
    if (group > static_cast<int>(issue::start_boundary) and group < static_cast<int>(issue::stop_boundary)) {
      const auto fragment = text(static_cast<enum issue>(group));
      const std::vector <database::check::Hit>& hits = database::check::get_hits ();
      for (const auto& hit : hits) {
        if (std::find (bibles.begin(), bibles.end (), hit.bible) == bibles.end ())
          continue;
        if (hit.data.find(fragment) != std::string::npos) {
          const int id = hit.rowid;
          database::check::delete_id (id);
        }
      }
      view.set_variable ("success", translate("The matching entries were deleted for just now."));
    } else {
      view.set_variable ("error", translate("This group of issues is not known."));
    }
  }
  
  
  {
    pugi::xml_document document {};
    const std::vector <database::check::Hit>& hits = database::check::get_hits ();
    for (const auto& hit : hits) {
      std::string bible = hit.bible;
      if (find (bibles.begin(), bibles.end (), bible) == bibles.end ())
        continue;
      const int id = hit.rowid;
      bible = filter::string::escape_special_xml_characters (bible);
      const int book = hit.book;
      const int chapter = hit.chapter;
      const int verse = hit.verse;
      const std::string information = filter::string::escape_special_xml_characters (hit.data);
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
  }


  // Assemble a block with zero or more fragments of checking results to delete all in one go.
  {
    const auto get_fragments = [&bibles]() {
      std::set<std::string> fragments{};
      const std::vector <database::check::Hit>& hits = database::check::get_hits ();
      for (const auto& hit : hits) {
        if (std::find (bibles.begin(), bibles.end(), hit.bible) == bibles.end())
          continue;
        fragments.insert(hit.data);
      }
      return fragments;
    };
    const std::set<std::string> fragments{get_fragments()};
    pugi::xml_document document {};
    using namespace checks::issues;
    constexpr auto start {static_cast<int>(issue::start_boundary)};
    constexpr auto stop {static_cast<int>(issue::stop_boundary)};
    for (int i {start + 1}; i < stop; i++) {
      const auto issue = static_cast<enum issue>(i);
      const auto translation = text(issue);
      const auto is_among_fragments = [&fragments] (const auto& translation) {
        for (const auto& fragment : fragments)
          if (fragment.find(translation) != std::string::npos)
            return true;
        return false;
      };
      if (!is_among_fragments(translation))
        continue;
      pugi::xml_node p = document.append_child("p");
      pugi::xml_node a = p.append_child("a");
      const std::string href = "?deletegroup=" + std::to_string (i);
      a.append_attribute("href") = href.c_str();
      std::stringstream text {};
      text << translate("Delete items with") << " " << std::quoted(translation);
      a.text ().set(text.str().c_str());
    }
    {
      std::stringstream ss {};
      document.print(ss, "", pugi::format_raw);
      view.set_variable ("deleteblock", std::move(ss).str());
    }
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

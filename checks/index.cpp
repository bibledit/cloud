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


string checks_index_url ()
{
  return "checks/index";
}


bool checks_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string checks_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Check database_check;

  
  string page;
  Assets_Header header = Assets_Header (translate("Checks"), webserver_request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  

  if (request->query.count ("approve")) {
    int approve = convert_to_int (request->query["approve"]);
    database_check.approve (approve);
    view.set_variable ("success", translate("The entry was suppressed."));
  }
  
                        
  if (request->query.count ("delete")) {
    int erase = convert_to_int (request->query["delete"]);
    database_check.erase (erase);
    view.set_variable ("success", translate("The entry was deleted for just now."));
  }

  
  // Get the Bibles the user has write-access to.
  vector <string> bibles;
  {
    vector <string> all_bibles = request->database_bibles()->getBibles ();
    for (auto bible : all_bibles) {
      if (access_bible_write (webserver_request, bible)) {
        bibles.push_back (bible);
      }
    }
  }
  
  
  string resultblock;
  vector <Database_Check_Hit> hits = database_check.getHits ();
  for (auto hit : hits) {
    string bible = hit.bible;
    if (find (bibles.begin(), bibles.end (), bible) != bibles.end ()) {
      int id = hit.rowid;
      bible = filter_string_sanitize_html (bible);
      int book = hit.book;
      int chapter = hit.chapter;
      int verse = hit.verse;
      string link = filter_passage_link_for_opening_editor_at (book, chapter, convert_to_string (verse));
      string information = filter_string_sanitize_html (hit.data);
      resultblock.append ("<p>\n");
      resultblock.append ("<a href=\"index?approve=" + convert_to_string (id) + "\"> ✔ </a>\n");
      resultblock.append ("<a href=\"index?delete=" + convert_to_string (id) + "\">" + emoji_wastebasket () + "</a>\n");
      resultblock.append (bible);
      resultblock.append (" ");
      resultblock.append (link);
      resultblock.append (" ");
      resultblock.append (information);
      resultblock.append ("</p>\n");
    }
  }
  view.set_variable ("resultblock", resultblock);

  
  if (checks_settings_acl (webserver_request)) {
    view.enable_zone ("can_enable");
  } else {
    view.enable_zone ("cannot_enable");
  }


  page += view.render ("checks", "index");
  page += Assets_Page::footer ();
  return page;
}

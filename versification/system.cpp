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


#include <versification/system.h>
#include <versification/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <database/books.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <assets/header.h>
#include <menu/logic.h>


string versification_system_url ()
{
  return "versification/system";
}


bool versification_system_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string versification_system (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Versification system"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (versification_index_url (), menu_logic_versification_index_text ());
  page = header.run ();
  
  Assets_View view;
  
  Database_Versifications database_versifications = Database_Versifications();

  string name = request->query["name"];
  view.set_variable ("name", filter_string_sanitize_html (name));

  if (request->post.count ("submit")) {
    string data = request->post["data"];
    if (data != "") {
      database_versifications.input (data, name);
    }
  }

  vector <string> data;
  vector <Passage> passages = database_versifications.getBooksChaptersVerses (name);
  for (auto & passage : passages) {
    int book = passage.book;
    int chapter = passage.chapter;
    string verse = passage.verse;
    string bookname = Database_Books::getEnglishFromId (book);
    data.push_back ("<tr>");
    data.push_back ("<td>" + bookname + "</td>");
    data.push_back ("<td>" + convert_to_string (chapter) + "</td>");
    data.push_back ("<td>" + convert_to_string (verse) + "</td>");
    data.push_back ("</tr>");
  }
  view.set_variable ("data", filter_string_implode (data, "\n"));

  string output = database_versifications.output (name);
  view.set_variable ("output", output);

  page += view.render ("versification", "system");
  
  page += Assets_Page::footer ();
  
  return page;
}

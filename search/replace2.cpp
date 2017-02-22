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


#include <search/replace2.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/volatile.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <menu/logic.h>


string search_replace2_url ()
{
  return "search/replace2";
}


bool search_replace2_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string search_replace2 (void * webserver_request)
{
  // Build the advanced replace page.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->database_config_user()->getBible ();
  string page;
  Assets_Header header = Assets_Header (translate("Replace"), request);
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  Assets_View view;
  view.set_variable ("bible", bible);
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);
  page += view.render ("search", "replace2");
  page += Assets_Page::footer ();
  return page;
}

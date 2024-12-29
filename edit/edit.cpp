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


#include <edit/edit.h>
#include <edit/index.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <locale/translate.h>
#include <assets/view.h>
#include <access/bible.h>


std::string edit_edit_url ()
{
  return "edit/edit";
}


bool edit_edit_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [read, write] = access_bible::any (webserver_request);
  return read;
}


std::string edit_edit (Webserver_Request& webserver_request)
{
  std::string passage_query = webserver_request.query ["passage"];
  Passage passage = filter_integer_to_passage (filter::strings::convert_to_int (passage_query));
  Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  Navigation_Passage::record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  
  // Check whether a Bible editor is alive.
  int timestamp = webserver_request.database_config_user()->getLiveBibleEditor ();
  bool alive = (timestamp > (filter::date::seconds_since_epoch () - 5));
  
  if (alive) 
    return translate ("The passage has been opened in the existing Bible editor");
  
  return R"(<a href="../editone/index">)" + translate ("Open a Bible editor to edit the passage") + "</a>";
}

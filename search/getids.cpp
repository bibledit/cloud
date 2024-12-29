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


#include <search/getids.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <search/logic.h>
#include <access/bible.h>


std::string search_getids_url ()
{
  return "search/getids";
}


bool search_getids_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string search_getids (Webserver_Request& webserver_request)
{
  // Get search variables from the query.
  std::string bible = webserver_request.query ["b"];
  std::string searchfor = webserver_request.query ["q"];
  bool casesensitive = (webserver_request.query ["c"] == "true");

  // Do the search.
  std::vector <Passage> passages;
  if (casesensitive) {
    passages = search_logic_search_bible_text_case_sensitive (bible, searchfor);
  } else {
    passages = search_logic_search_bible_text (bible, searchfor);
  }

  // Output identifiers of the search results.
  std::string output;
  for (const auto& passage : passages) {
    if (!output.empty ()) output.append ("\n");
    output.append (passage.encode ());
  }
  return output;
}

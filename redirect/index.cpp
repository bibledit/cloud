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


#include <redirect/index.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <editone2/index.h>


std::string editone_index_url ()
{
  return "editone/index";
}


bool editone_index_acl ()
{
  return true;
}


std::string editone_index (Webserver_Request& webserver_request)
{
  std::string url = editone2_index_url ();
  for (auto query : webserver_request.query) {
    url = filter_url_build_http_query (url, query.first, query.second);
  }
  redirect_browser (webserver_request, url);
  return std::string();
}

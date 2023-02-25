/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <help/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <locale/translate.h>
#include <assets/header.h>
#include <assets/external.h>
using namespace std;


string help_index_html (const string& url)
{
  string path (url);
  size_t pos = url.find ("/");
  if (pos != string::npos) path.erase (0, ++pos);
  path.append (".html");
  path = filter_url_create_root_path ({"help", path});
  return path;
}


bool help_index_url (const string& url)
{
  size_t pos = url.find ("help/");
  if (pos != 0) return false;
  return file_or_dir_exists (help_index_html (url));
}


bool help_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string help_index (void * webserver_request, const string& url)
{
  string page {};

  Assets_Header header = Assets_Header (translate("Help"), webserver_request);
  page = header.run ();

  Assets_View view {};

  view.set_variable ("version", config::logic::version ());

  view.set_variable ("external", assets_external_logic_link_addon ());

  view.set_variable ("config", filter_url_create_root_path ({config::logic::config_folder ()}));
  
  string filename (url);
  size_t pos = url.find ("/");
  if (pos != string::npos) filename.erase (0, ++pos);
 
  page += view.render ("help", filename);

  page += assets_page::footer ();

  return page;
}

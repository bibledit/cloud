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


#include <index/listing.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <locale/translate.h>
#include <menu/logic.h>


bool index_listing_match (string url)
{
  if (url.length () >= 9) if (url.substr (0, 9) == "revisions") return true;
  if (url.length () >= 7) if (url.substr (0, 7) == "exports") return true;
  return false;
}


string index_listing_url (string url)
{
  if (index_listing_match (url)) return url;
  return "\\";
}


bool index_listing_acl (void * webserver_request, string url)
{
  // Bible exports are public.
  if (url.find ("exports") == 0) {
    return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
  }
  // Any other files are for people with at least a member role.
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


string index_listing (void * webserver_request, string url)
{
  string page;
  page = Assets_Page::header ("Bibledit", webserver_request);
  // No breadcrumbs because the user can arrive here from more than one place.
  Assets_View view;
  url = filter_url_urldecode (url);
  url = filter_url_create_path ("", url);
  url = filter_string_str_replace ("\\", "/", url);
  view.set_variable ("url", url);
  string parent = filter_url_dirname_web (url);
  if (parent.length () > 1) {
    view.enable_zone ("parent");
    view.set_variable ("parent", parent);
  }
  string directory = filter_url_create_root_path (url);
  if (!file_or_dir_exists (directory) || filter_url_is_dir (directory)) {
    vector <string> files = filter_url_scandir (directory);
    for (auto & file : files) {
      string path = filter_url_create_path (directory, file);
      string line;
      line.append ("<tr>");
      line.append ("<td>");
      line.append ("<a href=\"" + filter_url_create_path (url, file) + "\">");
      line.append (file);
      line.append ("</a>");
      line.append ("</td>");
      line.append ("<td>");
      if (!filter_url_is_dir (path)) {
        line.append (convert_to_string (filter_url_filesize (path)));
      }
      line.append ("</td>");
      line.append ("</tr>");
      file = line;
    }
    string listing = filter_string_implode (files, "\n");
    if (listing.empty ()) listing = translate ("No files in this folder");
    else {
      listing.insert (0, "<table>");
      listing.append ("</table>");
    }
    view.set_variable ("listing", listing);
  } else {
    string filename = filter_url_create_root_path (url);
    return filter_url_file_get_contents (filename);
  }
  page += view.render ("index", "listing");
  page += Assets_Page::footer ();
  return page;
}

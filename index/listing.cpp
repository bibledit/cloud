/*
Copyright (©) 2003-2022 Teus Benschop.

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop

using namespace pugi;


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
  page = Assets_Page::header (translate ("Bibledit"), webserver_request);
  // No breadcrumbs because the user can arrive here from more than one place.
  Assets_View view;
  url = filter_url_urldecode (url);
  url = filter_url_create_path ({string(), url});
  url = filter_string_str_replace (R"(\)", "/", url);
  view.set_variable ("url", url);
  string parent = filter_url_dirname_web (url);
  if (parent.length () > 1) {
    view.enable_zone ("parent");
    view.set_variable ("parent", parent);
  }
  string directory = filter_url_create_root_path ({url});
  if (!file_or_dir_exists (directory) || filter_url_is_dir (directory)) {
    // The document that contains the listing.
    xml_document listing_document;
    string listing;
    // Check the files in this folder.
    vector <string> files = filter_url_scandir (directory);
    // Handle empty folder.
    if (files.empty()) {
      xml_node span_node = listing_document.append_child("span");
      span_node.text().set(translate ("No files in this folder").c_str());
    }
    // Handle file / folder listing.
    else {
      xml_node table_node = listing_document.append_child("table");
      for (auto & file : files) {
        // Open a new row.
        xml_node tr_node = table_node.append_child("tr");
        // Add the link to the file in the first column.
        xml_node td_node = tr_node.append_child("td");
        xml_node a_node = td_node.append_child("a");
        string href = filter_url_create_path ({url, file});
        a_node.append_attribute("href") = href.c_str();
        a_node.text().set(file.c_str());
        // Implement force download for USFM files.
        // https://github.com/bibledit/cloud/issues/771
        string suffix = filter_url_get_extension (file);
        if (suffix == "usfm") {
          a_node.append_attribute("download") = file.c_str();
        }
        // Optionally add the file size.
        string path = filter_url_create_path ({directory, file});
        if (!filter_url_is_dir (path)) {
          td_node = tr_node.append_child("td");
          td_node.text().set(convert_to_string (filter_url_filesize (path)).c_str());
        }
      }
    }
    stringstream ss;
    listing_document.print (ss, "", format_raw);
    view.set_variable ("listing", ss.str());
  } else {
    string filename = filter_url_create_root_path ({url});
    return filter_url_file_get_contents (filename);
  }
  page += view.render ("index", "listing");
  page += Assets_Page::footer ();
  return page;
}

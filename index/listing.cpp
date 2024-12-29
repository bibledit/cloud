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


#include <forward_list>
#include <array>
#include <string_view>
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
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


constexpr const std::string_view revisions {"revisions"};
constexpr const std::string_view exports {"exports"};


static bool index_listing_match (const std::string& url)
{
  if (url.length () >= revisions.size())
    if (url.substr (0, revisions.size()) == revisions)
      return true;
  if (url.length () >= exports.size())
    if (url.substr (0, exports.size()) == exports)
      return true;
  return false;
}


std::string index_listing_url (const std::string& url)
{
  if (index_listing_match (url)) 
    return url;
  return R"(\)";
}


bool index_listing_acl (Webserver_Request& webserver_request, std::string url)
{
  // Bible exports are public.
  if (url.find (exports) == 0) {
    return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
  }
  // Any other files are for people with at least a member role.
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


std::string index_listing (Webserver_Request& webserver_request, std::string url)
{
  std::string page = assets_page::header (translate ("Bibledit"), webserver_request);
  // No breadcrumbs because the user can arrive here from more than one place.
  Assets_View view;
  url = filter_url_urldecode (url);
  url = filter_url_create_path_web ({std::string(), url});
  view.set_variable ("url", url);
  const std::string parent = filter_url_dirname_web (url);
  if (parent.length () > 1) {
    view.enable_zone ("parent");
    view.set_variable ("parent", parent);
  }
  const std::string directory = filter_url_create_root_path ({url});
  if (!file_or_dir_exists (directory) || filter_url_is_dir (directory)) {
    // The document that contains the listing.
    pugi::xml_document listing_document;
    std::string listing;
    // Check the files in this folder.
    std::vector <std::string> files = filter_url_scandir (directory);
    // Handle empty folder.
    if (files.empty()) {
      pugi::xml_node span_node = listing_document.append_child("span");
      span_node.text().set(translate ("No files in this folder").c_str());
    }
    // Handle file / folder listing.
    else {
      pugi::xml_node table_node = listing_document.append_child("table");
      for (const auto& file : files) {
        // Open a new row.
        pugi::xml_node tr_node = table_node.append_child("tr");
        // Add the link to the file in the first column.
        pugi::xml_node td_node = tr_node.append_child("td");
        pugi::xml_node a_node = td_node.append_child("a");
        const std::string href = filter_url_create_path ({url, file});
        a_node.append_attribute("href") = href.c_str();
        a_node.text().set(file.c_str());
        // Implement force download for USFM files and other files.
        // https://github.com/bibledit/cloud/issues/771
        constexpr std::array<std::string_view, 5> downloadable_suffixes {
          "bblx", "exp", "odt", "zip", "usfm"
        };
        const std::string suffix = filter_url_get_extension (file);
        if (std::find (downloadable_suffixes.begin(), downloadable_suffixes.end(), suffix) != downloadable_suffixes.end()) {
          a_node.append_attribute("download") = file.c_str();
        }
        // Optionally add the file size.
        const std::string path = filter_url_create_path ({directory, file});
        if (!filter_url_is_dir (path)) {
          td_node = tr_node.append_child("td");
          td_node.text().set(std::to_string (filter_url_filesize (path)).c_str());
        }
      }
    }
    std::stringstream ss {};
    listing_document.print (ss, "", pugi::format_raw);
    view.set_variable ("listing", ss.str());
  } else {
    const std::string filename = filter_url_create_root_path ({url});
    return filter_url_file_get_contents (filename);
  }
  page += view.render ("index", "listing");
  page += assets_page::footer ();
  return page;
}

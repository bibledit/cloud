/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <dialog/books.h>
#include <assets/view.h>
#include <assets/page.h>
#include <locale/translate.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/books.h>


// Constructs a Bible books selection dialog
// $action - GET action to take: Will be added to the base url upon selection.
// $inclusions - vector of book IDs to include, empty vector does nothing.
// $exclusions - vector of book IDs to exclude, empty vector does nothing.
Dialog_Books::Dialog_Books (std::string url, std::string header, std::string info_top, std::string info_bottom, std::string action, std::vector <int> inclusions, std::vector <int> exclusions)
{
  base_url = url;
  assets_view.set_variable ("header", header);
  assets_view.set_variable ("info_top", info_top);
  assets_view.set_variable ("info_bottom", info_bottom);
  selection_action = action;
  include = inclusions;
  exclude = exclusions;
}


Dialog_Books::~Dialog_Books ()
{
}


// Add "parameter" and "value" to be added in the base query, or base url.
// If any $query is passed, if Cancel is clicked in this dialog, it should go go back
// to the original caller page  with the $query added.
// Same for when a selection is made: It adds the $query to the page where to go.
void Dialog_Books::add_query (std::string parameter, std::string value)
{
  base_url = filter_url_build_http_query(base_url, {{parameter, value}});
}


std::string Dialog_Books::run ()
{
  assets_view.set_variable ("base_url", base_url);

  std::vector <int> book_ids {};
  {
    std::vector <book_id> book_enums = database::books::get_ids ();
    for (const auto book : book_enums) book_ids.push_back(static_cast<int>(book));
  }
  if (!include.empty ()) {
    book_ids = include;
  }
  if (!exclude.empty()) {
    std::vector <int> ids;
    for (const auto & book_id : book_ids) {
      if (find (exclude.begin(), exclude.end(), book_id) == exclude.end ()) {
        ids.push_back (book_id);
      }
    }
    book_ids = ids;
  }

  std::stringstream book_block;
  for (const auto & id : book_ids) {
    book_block << "<a href=";
    book_block << std::quoted(filter_url_build_http_query(base_url, {{selection_action, std::to_string (id)}}));
    book_block << ">" << database::books::get_english_from_id (static_cast<book_id>(id)) << "</a>\n";
  }
  assets_view.set_variable ("book_block", book_block.str());
  
  std::string page = assets_view.render ("dialog", "books");
  page += assets_page::footer ();
  return page;
}

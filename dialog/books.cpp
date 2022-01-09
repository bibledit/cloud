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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <dialog/books.h>
#include <assets/view.h>
#include <assets/page.h>
#include <locale/translate.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/books.h>
#include <sstream>
#include <iomanip>


// Constructs a Bible books selection dialog
// $action - GET action to take: Will be added to the base url upon selection.
// $inclusions - vector of book IDs to include, empty vector does nothing.
// $exclusions - vector of book IDs to exclude, empty vector does nothing.
Dialog_Books::Dialog_Books (string url, string header, string info_top, string info_bottom, string action, vector <int> inclusions, vector <int> exclusions)
{
  Assets_View * view = new Assets_View ();
  base_url = url;
  view->set_variable ("header", header);
  view->set_variable ("info_top", info_top);
  view->set_variable ("info_bottom", info_bottom);
  selection_action = action;
  include = inclusions;
  exclude = exclusions;
  assets_view = view;
}


Dialog_Books::~Dialog_Books ()
{
  Assets_View * view = static_cast<Assets_View *>(assets_view);
  delete view;
}


// Add "parameter" and "value" to be added in the base query, or base url.
// If any $query is passed, if Cancel is clicked in this dialog, it should go go back
// to the original caller page  with the $query added.
// Same for when a selection is made: It adds the $query to the page where to go.
void Dialog_Books::add_query (string parameter, string value)
{
  base_url = filter_url_build_http_query (base_url, parameter, value);
}


string Dialog_Books::run ()
{
  Assets_View * view = static_cast<Assets_View *>(assets_view);
  view->set_variable ("base_url", base_url);

  vector <int> book_ids = Database_Books::getIDs ();
  if (!include.empty ()) {
    book_ids = include;
  }
  if (!exclude.empty()) {
    vector <int> ids;
    for (auto & book_id : book_ids) {
      if (find (exclude.begin(), exclude.end(), book_id) == exclude.end ()) {
        ids.push_back (book_id);
      }
    }
    book_ids = ids;
  }

  stringstream book_block;
  for (auto & id : book_ids) {
    book_block << "<a href=";
    book_block << quoted(filter_url_build_http_query (base_url, selection_action, convert_to_string (id)));
    book_block << ">" << Database_Books::getEnglishFromId (id) << "</a>\n";
  }
  view->set_variable ("book_block", book_block.str());
  
  string page = view->render ("dialog", "books");
  page += Assets_Page::footer ();
  return page;
}

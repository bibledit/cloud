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


#include <dialog/color.h>
#include <assets/view.h>
#include <assets/page.h>
#include <locale/translate.h>
#include <filter/url.h>


Dialog_Color::Dialog_Color (std::string url, std::string question)
{
  base_url = url;
  assets_view.set_variable ("question", question);
}


Dialog_Color::~Dialog_Color ()
{
}


// Add "parameter" and "value" to be added in the base query, or base url.
// If any $query is passed, if Cancel is clicked in this dialog, it should go go back
// to the original caller page  with the $query added.
// Same for when a selection is made: It adds the $query to the page where to go.
void Dialog_Color::add_query (std::string parameter, std::string value)
{
  base_url = filter_url_build_http_query (base_url, parameter, value);
}


std::string Dialog_Color::run ()
{
  assets_view.set_variable ("base_url", base_url);
  std::string page = assets_view.render ("dialog", "color");
  page += assets_page::footer ();
  return page;
}

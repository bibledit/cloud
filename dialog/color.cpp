/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


Dialog_Color::Dialog_Color (string url, string question)
{
  Assets_View * view = new Assets_View ();
  base_url = url;
  view->set_variable ("question", question);
  assets_view = view;
}


Dialog_Color::~Dialog_Color ()
{
  Assets_View * view = (Assets_View *) assets_view;
  delete view;
}


// Add "parameter" and "value" to be added in the base query, or base url.
// If any $query is passed, if Cancel is clicked in this dialog, it should go go back
// to the original caller page  with the $query added.
// Same for when a selection is made: It adds the $query to the page where to go.
void Dialog_Color::add_query (string parameter, string value)
{
  base_url = filter_url_build_http_query (base_url, parameter, value);
}


string Dialog_Color::run ()
{
  Assets_View * view = (Assets_View *) assets_view;
  view->set_variable ("base_url", base_url);
  string page = view->render ("dialog", "color");
  page += Assets_Page::footer ();
  return page;
}

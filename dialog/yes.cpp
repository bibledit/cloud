/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#include <dialog/yes.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/url.h>


// Dialog that asks the user for confirmation to perform an action.
// $url: The url of the page where to go to on clicking Cancel or Yes.
// $question: The question to ask.
Dialog_Yes::Dialog_Yes (string url, string question)
{
  Assets_View * view = new Assets_View ();
  base_url = url;
  view->set_variable ("question", question);
  assets_view = view;
}


Dialog_Yes::~Dialog_Yes ()
{
  Assets_View * view = (Assets_View *) assets_view;
  delete view;
}


// Adds a query to the URL for going to the page on clicking Cancel or Yes.
void Dialog_Yes::add_query (string parameter, string value)
{
  base_url = filter_url_build_http_query (base_url, parameter, value);
}


string Dialog_Yes::run ()
{
  Assets_View * view = (Assets_View *) assets_view;
  string yes = filter_url_build_http_query (base_url, "confirm", "yes");
  string cancel = filter_url_build_http_query (base_url, "confirm", "cancel");
  view->set_variable ("yes", yes);
  view->set_variable ("cancel", cancel);
  string page = view->render ("dialog", "yes");
  page += Assets_Page::footer ();
  return page;
}


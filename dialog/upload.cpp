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


#include <dialog/upload.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/url.h>


// Dialog that enables the user to upload a file.
// $url: The url of the page where to go to on clicking Cancel or Upload.
// $question: The question to ask.
Dialog_Upload::Dialog_Upload (string url, string question)
{
  Assets_View * view = new Assets_View ();
  base_url = url;
  view->set_variable ("question", question);
  assets_view = view;
}


Dialog_Upload::~Dialog_Upload ()
{
  Assets_View * view = (Assets_View *) assets_view;
  delete view;
}


// Adds a query to the URL for going to the page on clicking Upload.
void Dialog_Upload::add_upload_query (string parameter, string value)
{
  upload_query [parameter] = value;
}


string Dialog_Upload::run ()
{
  Assets_View * view = (Assets_View *) assets_view;
  string import;
  for (auto & element : upload_query) {
    import = filter_url_build_http_query (base_url, element.first, element.second);
  }
  view->set_variable ("import", import);
  view->set_variable ("cancel", base_url);
  string page = view->render ("dialog", "upload");
  page += Assets_Page::footer ();
  return page;
}

/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <dialog/list.h>
#include <assets/view.h>
#include <assets/page.h>
#include <locale/translate.h>
#include <filter/url.h>


// Entry dialog constructor
// $url: The base URL of the page.
// $question: The question to be asked.
// $info_top : Information.
// $info_bottom: Information.
// $post: causes the result to be sent via the POST method rather than the default GET method.
Dialog_List::Dialog_List (string url, string question, string info_top, string info_bottom, bool post)
{
  Assets_View * view = new Assets_View ();
  base_url = url;
  view->set_variable ("question", question);
  if (info_top == "") info_top = translate("Here are the various options:");
  view->set_variable ("info_top", info_top);
  if (info_bottom == "") info_bottom = translate("Please pick one.");
  view->set_variable ("info_bottom", info_bottom);
  post_result = post;
  assets_view = view;
}


Dialog_List::~Dialog_List ()
{
  Assets_View * view = (Assets_View *) assets_view;
  delete view;
}


// Add "parameter" and "value" to be added in the base query, or base url.
// If any $query is passed, if Cancel is clicked in this dialog, it should go go back
// to the original caller page  with the $query added.
// Same for when a selection is made: It adds the $query to the page where to go.
void Dialog_List::add_query (string parameter, string value)
{
  base_url = filter_url_build_http_query (base_url, parameter, value);
}


void Dialog_List::add_row (string text, string parameter, string value)
{
  if (!list_block.empty ()) list_block.append ("\n");
  list_block.append ("<li>");
  if (post_result) {
    list_block.append ("\n");
    list_block.append ("<form action=\"" + base_url + "\" method=\"post\">\n");
    list_block.append ("<a href=\"javascript:;\" onclick=\"parentNode.submit();\">" + text + "</a>\n");
    list_block.append ("<input type=\"hidden\" name=\"add\" value=\"" + value + "\" />\n");
    list_block.append ("</form>\n");
  } else {
    string href = filter_url_build_http_query (base_url, parameter, value);
    list_block.append ("<a href=\"" + href + "\">" + text + "</a>");
  }
  list_block.append ("</li>");
}


string Dialog_List::run ()
{
  Assets_View * view = (Assets_View *) assets_view;
  view->set_variable ("base_url", base_url);
  view->set_variable ("list_block", list_block);
  string page = view->render ("dialog", "list");
  page += Assets_Page::footer ();
  return page;
}

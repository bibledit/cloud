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


#include <public/chapter.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <config/logic.h>


std::string public_chapter_url ()
{
  return "public/chapter";
}


bool public_chapter_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string public_chapter (Webserver_Request& webserver_request)
{
  const std::string bible = webserver_request.query ["bible"];
  const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  const int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.html_text_standard->custom_class = Filter_Css::getClass (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesheet);

  const std::string html = filter_text.html_text_standard->get_inner_html ();
  
  return html;
}

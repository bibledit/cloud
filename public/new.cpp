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


#include <public/new.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <assets/view.h>
#include <assets/header.h>
#include <assets/page.h>
#include <locale/translate.h>
#include <read/index.h>
#include <config/logic.h>


std::string public_new_url ()
{
  return "public/new";
}


bool public_new_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string public_new (Webserver_Request& webserver_request)
{
  if (!webserver_request.query.empty ()) {
    const std::string bible = webserver_request.query ["bible"];
    const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
    const int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
    const int verse = filter::strings::convert_to_int (webserver_request.query ["verse"]);

    const std::string chapter_usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
    const std::string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    const std::string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new HtmlText (bible);
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    return filter_text.html_text_standard->get_inner_html ();
  }

  
  std::string page {};
  Assets_Header header = Assets_Header (translate ("New feedback"), webserver_request);
  header.set_navigator ();
  header.set_stylesheet ();
  page = header.run ();
  Assets_View view;


  const std::string cancellation_url = "index";
  view.set_variable ("cancellation_url", cancellation_url);


  page += view.render ("public", "new");
  page += assets_page::footer ();
  return page;
}

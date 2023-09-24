/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


string public_new_url ()
{
  return "public/new";
}


bool public_new_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_new (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);


  if (!request->query.empty ()) {
    string bible = request->query ["bible"];
    int book = filter::strings::convert_to_int (request->query ["book"]);
    int chapter = filter::strings::convert_to_int (request->query ["chapter"]);
    int verse = filter::strings::convert_to_int (request->query ["verse"]);

    string chapter_usfm = request->database_bibles()->get_chapter (bible, book, chapter);
    string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new HtmlText (bible);
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    return filter_text.html_text_standard->get_inner_html ();
  }

  
  string page;
  Assets_Header header = Assets_Header (translate ("New feedback"), request);
  header.set_navigator ();
  header.set_stylesheet ();
  page = header.run ();
  Assets_View view;


  string cancellation_url = "index";

  view.set_variable ("cancellation_url", cancellation_url);


  page += view.render ("public", "new");
  page += assets_page::footer ();
  return page;
}

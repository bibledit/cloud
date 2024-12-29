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


#include <resource/img.h>
#include <resource/image.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/imageresources.h>
#include <database/books.h>
#include <database/temporal.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>


std::string resource_img_url ()
{
  return "resource/img";
}


bool resource_img_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string resource_img (Webserver_Request& webserver_request)
{
  Database_ImageResources database_imageresources;

  
  std::string page {};
  Assets_Header header = Assets_Header (translate("Image resources"), webserver_request);
  page = header.run ();
  Assets_View view {};
  std::string error, success;
  book_id book1 {book_id::_unknown};
  book_id book2 {book_id::_unknown};
  int chapter1, verse1, chapter2, verse2;
  
  
  std::string name = webserver_request.query ["name"];
  view.set_variable ("name", name);

  
  std::string image = webserver_request.query ["image"];
  view.set_variable ("image", image);

  
  int userid = filter::strings::user_identifier (webserver_request);

  
  if (webserver_request.post.count ("submit")) {
    
    std::vector <std::string> errors {};
    
    std::string book = webserver_request.post ["book1"];
    book1 = filter_passage_interpret_book_v2 (book);
    if (book1 == book_id::_unknown) errors.push_back (translate ("Unknown starting book."));

    chapter1 = filter::strings::convert_to_int (webserver_request.post ["chapter1"]);
    if (chapter1 < 0) errors.push_back (translate ("Negative starting chapter."));
    if (chapter1 > 200) errors.push_back (translate ("High starting chapter."));
    
    verse1 = filter::strings::convert_to_int (webserver_request.post ["verse1"]);
    if (chapter1 < 0) errors.push_back (translate ("Negative starting verse."));
    if (chapter1 > 200) errors.push_back (translate ("High starting verse."));
    
    book = webserver_request.post ["book2"];
    book2 = filter_passage_interpret_book_v2 (book);
    if (book2 == book_id::_unknown) errors.push_back (translate ("Unknown ending book."));
    
    chapter2 = filter::strings::convert_to_int (webserver_request.post ["chapter2"]);
    if (chapter2 < 0) errors.push_back (translate ("Negative ending chapter."));
    if (chapter2 > 200) errors.push_back (translate ("High ending chapter."));
    
    verse2 = filter::strings::convert_to_int (webserver_request.post ["verse2"]);
    if (chapter2 < 0) errors.push_back (translate ("Negative ending verse."));
    if (chapter2 > 200) errors.push_back (translate ("High ending verse."));
    
    int start = filter_passage_to_integer (Passage (std::string(), static_cast<int>(book1), chapter1, std::to_string (verse1)));
    int end = filter_passage_to_integer (Passage (std::string(), static_cast<int>(book2), chapter2, std::to_string (verse2)));
    if (start > end) {
      errors.push_back (translate ("The starting passage is beyond the ending passage."));
    }

    database_imageresources.assign (name, image, static_cast<int>(book1), chapter1, verse1, static_cast<int>(book2), chapter2, verse2);
    
    database::temporal::set_value (userid, "imageresources", std::to_string (end));

    error = filter::strings::implode (errors, " ");
    if (errors.empty ()) {
      redirect_browser (webserver_request, filter_url_build_http_query (resource_image_url (), "name", name));
      return std::string();
    }
  }
  
  
  // Retrieve passage range for this image.
  {
    int ibook1 {static_cast<int>(book1)};
    int ibook2 {static_cast<int>(book2)};
    database_imageresources.get (name, image, ibook1, chapter1, verse1, ibook2, chapter2, verse2);
    book1 = static_cast<book_id>(ibook1);
    book2 = static_cast<book_id>(ibook2);
  }
  if ((book1 == book_id::_unknown) || (book2 == book_id::_unknown)) {
    std::string end = database::temporal::get_value (userid, "imageresources");
    Passage passage = filter_integer_to_passage (filter::strings::convert_to_int (end));
    book1 = book2 = static_cast<book_id>(passage.m_book);
    chapter1 = chapter2 = passage.m_chapter;
    verse1 = verse2 = filter::strings::convert_to_int (passage.m_verse);
    if (book1 == book_id::_unknown) book1 = book_id::_genesis;
    if (book2 == book_id::_unknown) book2 = book_id::_genesis;
  }
  view.set_variable ("book1", database::books::get_english_from_id (book1));
  view.set_variable ("chapter1", std::to_string (chapter1));
  view.set_variable ("verse1", std::to_string (verse1));
  view.set_variable ("book2", database::books::get_english_from_id (book2));
  view.set_variable ("chapter2", std::to_string (chapter2));
  view.set_variable ("verse2", std::to_string (verse2));
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "img");
  page += assets_page::footer ();
  return page;
}

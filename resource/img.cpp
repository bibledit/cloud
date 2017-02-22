/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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
#include <database/volatile.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>


string resource_img_url ()
{
  return "resource/img";
}


bool resource_img_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_img (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_ImageResources database_imageresources;

  
  string page;
  Assets_Header header = Assets_Header (translate("Image resources"), request);
  page = header.run ();
  Assets_View view;
  string error, success;
  int book1, chapter1, verse1, book2, chapter2, verse2;
  
  
  string name = request->query ["name"];
  view.set_variable ("name", name);

  
  string image = request->query ["image"];
  view.set_variable ("image", image);

  
  int userid = filter_string_user_identifier (webserver_request);

  
  if (request->post.count ("submit")) {
    
    vector <string> errors;
    
    string book = request->post ["book1"];
    book1 = filter_passage_interpret_book (book);
    if (book1 == 0) errors.push_back (translate ("Unknown starting book."));

    chapter1 = convert_to_int (request->post ["chapter1"]);
    if (chapter1 < 0) errors.push_back (translate ("Negative starting chapter."));
    if (chapter1 > 200) errors.push_back (translate ("High starting chapter."));
    
    verse1 = convert_to_int (request->post ["verse1"]);
    if (chapter1 < 0) errors.push_back (translate ("Negative starting verse."));
    if (chapter1 > 200) errors.push_back (translate ("High starting verse."));
    
    book = request->post ["book2"];
    book2 = filter_passage_interpret_book (book);
    if (book2 == 0) errors.push_back (translate ("Unknown ending book."));
    
    chapter2 = convert_to_int (request->post ["chapter2"]);
    if (chapter2 < 0) errors.push_back (translate ("Negative ending chapter."));
    if (chapter2 > 200) errors.push_back (translate ("High ending chapter."));
    
    verse2 = convert_to_int (request->post ["verse2"]);
    if (chapter2 < 0) errors.push_back (translate ("Negative ending verse."));
    if (chapter2 > 200) errors.push_back (translate ("High ending verse."));
    
    int start = filter_passage_to_integer (Passage ("", book1, chapter1, convert_to_string (verse1)));
    int end = filter_passage_to_integer (Passage ("", book2, chapter2, convert_to_string (verse2)));
    if (start > end) {
      errors.push_back (translate ("The starting passage is beyond the ending passage."));
    }

    database_imageresources.assign (name, image, book1, chapter1, verse1, book2, chapter2, verse2);
    
    Database_Volatile::setValue (userid, "imageresources", convert_to_string (end));

    error = filter_string_implode (errors, " ");
    if (errors.empty ()) {
      redirect_browser (request, filter_url_build_http_query (resource_image_url (), "name", name));
      return "";
    }
  }
  
  
  // Retrieve passage range for this image.
  database_imageresources.get (name, image, book1, chapter1, verse1, book2, chapter2, verse2);
  if ((book1 == 0) || (book2 == 0)) {
    string end = Database_Volatile::getValue (userid, "imageresources");
    Passage passage = filter_integer_to_passage (convert_to_int (end));
    book1 = book2 = passage.book;
    chapter1 = chapter2 = passage.chapter;
    verse1 = verse2 = convert_to_int (passage.verse);
    if (book1 == 0) book1 = 1;
    if (book2 == 0) book2 = 1;
  }
  view.set_variable ("book1", Database_Books::getEnglishFromId (book1));
  view.set_variable ("chapter1", convert_to_string (chapter1));
  view.set_variable ("verse1", convert_to_string (verse1));
  view.set_variable ("book2", Database_Books::getEnglishFromId (book2));
  view.set_variable ("chapter2", convert_to_string (chapter2));
  view.set_variable ("verse2", convert_to_string (verse2));
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "img");
  page += Assets_Page::footer ();
  return page;
}

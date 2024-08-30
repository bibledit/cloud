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

#include <navigation/update.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <navigation/passage.h>
#include <database/books.h>


std::string navigation_update_url ()
{
  return "navigation/update";
}


bool navigation_update_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string navigation_update (Webserver_Request& webserver_request)
{
  // It used to take the Bible from the Bible variable stored in Javascript.
  // But then, when switching from a Bible that has the NT only, to a Bible that has OT and NT,
  // the navigator would only show the NT books.
  // Now, by taking the Bible from the database, it will show the books of the last selected Bible.
  std::string bible = webserver_request.database_config_user()->getBible ();
  int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  int verse = filter::strings::convert_to_int (webserver_request.query ["verse"]);
  
  
  
  // Build the keyboard navigation fragment.
  if (webserver_request.query.count ("keyboard")) {
    return Navigation_Passage::get_keyboard_navigator (webserver_request, bible);
  }

  
  else if (webserver_request.query.count ("passage")) {
    std::string passage = webserver_request.query["passage"];
    Navigation_Passage::interpret_keyboard_navigator (webserver_request, bible, passage);
  }


  else if (webserver_request.query.count ("goback")) {
    Navigation_Passage::go_back (webserver_request);
  }
  
  
  else if (webserver_request.query.count ("goforward")) {
    Navigation_Passage::go_forward (webserver_request);
  }
  
  
  else if (webserver_request.query.count ("getbooks")) {
    return Navigation_Passage::get_books_fragment (webserver_request, bible);
  }
  
  
  else if (webserver_request.query.count ("applybook")) {
    std::string msg = webserver_request.query ["applybook"];
    if (msg.find ("cancel") == std::string::npos) {
      int apply_book = filter::strings::convert_to_int (msg);
      if (apply_book) Navigation_Passage::set_book (webserver_request, apply_book);
    }
  }
  
  
  // Get the list of available chapters in the current book.
  else if (webserver_request.query.count ("getchapters")) {
    return Navigation_Passage::get_chapters_fragment (bible, book, chapter);
  }
  

  // Select a chapter, go to previous or next chapter, or cancel.
  else if (webserver_request.query.count ("applychapter")) {
    std::string msg = webserver_request.query ["applychapter"];
    if (msg.find ("previous") != std::string::npos) {
      Navigation_Passage::goto_previous_chapter (webserver_request, bible);
    } else if (msg.find ("next") != std::string::npos) {
      Navigation_Passage::goto_next_chapter (webserver_request, bible);
    }
    else if (msg.find ("cancel") != std::string::npos) {
    } else {
      int apply_chapter = filter::strings::convert_to_int (msg);
      Navigation_Passage::set_chapter (webserver_request, apply_chapter);
    }
  }
  
  
  // Get the list of available verses in the current chapter.
  else if (webserver_request.query.count ("getverses")) {
    return Navigation_Passage::get_verses_fragment (bible, book, chapter, verse);
  }

  
  // Select a verse, go to the previous or next verse, or cancel.
  else if (webserver_request.query.count ("applyverse")) {
    std::string msg = webserver_request.query ["applyverse"];
    if (msg.find ("previous") != std::string::npos) {
      Navigation_Passage::goto_previous_verse (webserver_request, bible);
    } else if (msg.find ("next") != std::string::npos) {
      Navigation_Passage::goto_next_verse (webserver_request, bible);
    }
    else if (msg.find ("cancel") != std::string::npos) {
    } else {
      int apply_verse = filter::strings::convert_to_int (msg);
      Navigation_Passage::set_verse (webserver_request, apply_verse);
    }
  }
  

  // Go to the previous verse.
  else if (webserver_request.query.count ("previousverse")) {
    Navigation_Passage::goto_previous_verse (webserver_request, bible);
  }
  
  
  // Go to the next verse.
  else if (webserver_request.query.count ("nextverse")) {
    Navigation_Passage::goto_next_verse (webserver_request, bible);
  }
  

  // Go to the previous chapter.
  else if (webserver_request.query.count ("previouschapter")) {
    Navigation_Passage::goto_previous_chapter (webserver_request, bible);
  }
  
  
  // Go to the next chapter.
  else if (webserver_request.query.count ("nextchapter")) {
    Navigation_Passage::goto_next_chapter (webserver_request, bible);
  }
  
  
  // Provide html for history going back.
  else if (webserver_request.query.count ("historyback")) {
    return Navigation_Passage::get_history_back (webserver_request);
  }

  
  // Provide html for history going forward.
  else if (webserver_request.query.count ("historyforward")) {
    return Navigation_Passage::get_history_forward (webserver_request);
  }

  // Apply the selected history items to the navigation system.
  else if (webserver_request.query.count ("applyhistory")) {
    std::string message = webserver_request.query ["applyhistory"];
    if (message.find ("cancel") == std::string::npos) {
      Navigation_Passage::go_history(webserver_request, message);
    }
  }
  
  // Build the navigation html fragment.
  return Navigation_Passage::get_mouse_navigator (webserver_request, bible);
}

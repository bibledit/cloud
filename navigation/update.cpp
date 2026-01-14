/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <ipc/focus.h>


std::string navigation_update_url ()
{
  return "navigation/update";
}


bool navigation_update_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string navigation_update (Webserver_Request& webserver_request)
{
  // It used to take the Bible from the Bible variable stored in Javascript.
  // But then, when switching from a Bible that has the NT only, to a Bible that has OT and NT,
  // the navigator would only show the NT books.
  // Now, by taking the Bible from the database, it will show the books of the last selected Bible.
  const std::string bible = webserver_request.database_config_user()->get_bible ();
  const int book = filter::string::convert_to_int (webserver_request.query ["book"]);
  const int chapter = filter::string::convert_to_int (webserver_request.query ["chapter"]);
  const int verse = filter::string::convert_to_int (webserver_request.query ["verse"]);
  const int focus_group = ipc_focus::get_focus_group(webserver_request);
  
  
  // Build the keyboard navigation fragment.
  if (webserver_request.query.count ("keyboard")) {
    return navigation_passage::get_keyboard_navigator (webserver_request, bible);
  }

  
  else if (webserver_request.query.count ("passage")) {
    std::string passage = webserver_request.query["passage"];
    navigation_passage::interpret_keyboard_navigator (webserver_request, bible, passage);
  }


  else if (webserver_request.query.count ("goback")) {
    navigation_passage::go_back (webserver_request);
  }
  
  
  else if (webserver_request.query.count ("goforward")) {
    navigation_passage::go_forward (webserver_request);
  }
  
  
  else if (webserver_request.query.count ("getbooks")) {
    return navigation_passage::get_books_fragment (webserver_request, bible);
  }
  
  
  else if (webserver_request.query.count ("applybook")) {
    const std::string msg = webserver_request.query ["applybook"];
    if (msg.find ("cancel") == std::string::npos) {
      int apply_book = filter::string::convert_to_int (msg);
      if (apply_book) navigation_passage::set_book (webserver_request, apply_book);
    }
  }
  
  
  // Get the list of available chapters in the current book.
  else if (webserver_request.query.count ("getchapters")) {
    return navigation_passage::get_chapters_fragment (bible, book, chapter);
  }
  

  // Select a chapter, go to previous or next chapter, or cancel.
  else if (webserver_request.query.count ("applychapter")) {
    std::string msg = webserver_request.query ["applychapter"];
    if (msg.find ("previous") != std::string::npos) {
      navigation_passage::goto_previous_chapter (webserver_request, bible);
    } else if (msg.find ("next") != std::string::npos) {
      navigation_passage::goto_next_chapter (webserver_request, bible);
    }
    else if (msg.find ("cancel") != std::string::npos) {
    } else {
      int apply_chapter = filter::string::convert_to_int (msg);
      navigation_passage::set_chapter (webserver_request, apply_chapter);
    }
  }
  
  
  // Get the list of available verses in the current chapter.
  else if (webserver_request.query.count ("getverses")) {
    return navigation_passage::get_verses_fragment (bible, book, chapter, verse);
  }

  
  // Select a verse, go to the previous or next verse, or cancel.
  else if (webserver_request.query.count ("applyverse")) {
    std::string msg = webserver_request.query ["applyverse"];
    if (msg.find ("previous") != std::string::npos) {
      navigation_passage::goto_previous_verse (webserver_request, bible);
    } else if (msg.find ("next") != std::string::npos) {
      navigation_passage::goto_next_verse (webserver_request, bible);
    }
    else if (msg.find ("cancel") != std::string::npos) {
    } else {
      int apply_verse = filter::string::convert_to_int (msg);
      navigation_passage::set_verse (webserver_request, apply_verse);
    }
  }
  

  // Go to the previous verse.
  else if (webserver_request.query.count ("previousverse")) {
    navigation_passage::goto_previous_verse (webserver_request, bible);
  }
  
  
  // Go to the next verse.
  else if (webserver_request.query.count ("nextverse")) {
    navigation_passage::goto_next_verse (webserver_request, bible);
  }
  

  // Go to the previous chapter.
  else if (webserver_request.query.count ("previouschapter")) {
    navigation_passage::goto_previous_chapter (webserver_request, bible);
  }
  
  
  // Go to the next chapter.
  else if (webserver_request.query.count ("nextchapter")) {
    navigation_passage::goto_next_chapter (webserver_request, bible);
  }

  
  // Go to the previous book.
  else if (webserver_request.query.count ("previousbook")) {
    navigation_passage::goto_previous_book (webserver_request, bible);
  }
  
  
  // Go to the next book.
  else if (webserver_request.query.count ("nextbook")) {
    navigation_passage::goto_next_book (webserver_request, bible);
  }

  
  // Provide html for history going back.
  else if (webserver_request.query.count ("historyback")) {
    return navigation_passage::get_history_back (webserver_request);
  }

  
  // Provide html for history going forward.
  else if (webserver_request.query.count ("historyforward")) {
    return navigation_passage::get_history_forward (webserver_request);
  }

  // Apply the selected history items to the navigation system.
  else if (webserver_request.query.count ("applyhistory")) {
    std::string message = webserver_request.query ["applyhistory"];
    if (message.find ("cancel") == std::string::npos) {
      navigation_passage::go_history(webserver_request, message);
    }
  }
  
  // Build the navigation html fragment.
  return navigation_passage::get_mouse_navigator (webserver_request, bible);
}

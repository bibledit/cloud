/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


string navigation_update_url ()
{
  return "navigation/update";
}


bool navigation_update_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string navigation_update (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  // It used to take the Bible from the Bible variable stored in Javascript.
  // But then, when switching from a Bible that has the NT only, to a Bible that has OT and NT,
  // the navigator would only show the NT books.
  // Now, by taking the Bible from the database, it will show the books of the last selected Bible.
  string bible = request->database_config_user()->getBible ();
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int verse = convert_to_int (request->query ["verse"]);
  
  
  
  // Build the keyboard navigation fragment.
  if (request->query.count ("keyboard")) {
    return Navigation_Passage::get_keyboard_navigator (request, bible);
  }

  
  else if (request->query.count ("passage")) {
    string passage = request->query["passage"];
    Navigation_Passage::interpret_keyboard_navigator (request, bible, passage);
  }


  else if (request->query.count ("goback")) {
    Navigation_Passage::go_back (request);
  }
  
  
  else if (request->query.count ("goforward")) {
    Navigation_Passage::go_forward (request);
  }
  
  
  else if (request->query.count ("getbooks")) {
    return Navigation_Passage::get_books_fragment (request, bible);
  }
  
  
  else if (request->query.count ("applybook")) {
    string msg = request->query ["applybook"];
    if (msg.find ("cancel") == string::npos) {
      int book = convert_to_int (msg);
      if (book) Navigation_Passage::set_book (request, book);
    }
  }
  
  
  // Get the list of available chapters in the current book.
  else if (request->query.count ("getchapters")) {
    return Navigation_Passage::get_chapters_fragment (request, bible, book, chapter);
  }
  

  // Select a chapter, go to previous or next chapter, or cancel.
  else if (request->query.count ("applychapter")) {
    string msg = request->query ["applychapter"];
    if (msg.find ("previous") != string::npos) {
      Navigation_Passage::goto_previous_chapter (webserver_request, bible);
    } else if (msg.find ("next") != string::npos) {
      Navigation_Passage::goto_next_chapter (webserver_request, bible);
    }
    else if (msg.find ("cancel") != string::npos) {
    } else {
      int chapter = convert_to_int (msg);
      Navigation_Passage::set_chapter (request, chapter);
    }
  }
  
  
  // Get the list of available verses in the current chapter.
  else if (request->query.count ("getverses")) {
    return Navigation_Passage::get_verses_fragment (request, bible, convert_to_int (book), chapter, verse);
  }

  
  // Select a verse, go to the previous or next verse, or cancel.
  else if (request->query.count ("applyverse")) {
    string msg = request->query ["applyverse"];
    if (msg.find ("previous") != string::npos) {
      Navigation_Passage::goto_previous_verse (webserver_request, bible);
    } else if (msg.find ("next") != string::npos) {
      Navigation_Passage::goto_next_verse (webserver_request, bible);
    }
    else if (msg.find ("cancel") != string::npos) {
    } else {
      int verse = convert_to_int (msg);
      Navigation_Passage::set_verse (request, verse);
    }
  }
  

  // Go to the previous verse.
  else if (request->query.count ("previousverse")) {
    Navigation_Passage::goto_previous_verse (webserver_request, bible);
  }
  
  
  // Go to the next verse.
  else if (request->query.count ("nextverse")) {
    Navigation_Passage::goto_next_verse (webserver_request, bible);
  }
  

  // Go to the previous chapter.
  else if (request->query.count ("previouschapter")) {
    Navigation_Passage::goto_previous_chapter (webserver_request, bible);
  }
  
  
  // Go to the next chapter.
  else if (request->query.count ("nextchapter")) {
    Navigation_Passage::goto_next_chapter (webserver_request, bible);
  }
  
  
  // Provide html for history going back.
  else if (request->query.count ("historyback")) {
    return Navigation_Passage::get_history_back (request);
  }

  
  // Provide html for history going forward.
  else if (request->query.count ("historyforward")) {
    return Navigation_Passage::get_history_forward (request);
  }

  // Apply the selected history items to the navigation system.
  else if (request->query.count ("applyhistory")) {
    string message = request->query ["applyhistory"];
    if (message.find ("cancel") == string::npos) {
      Navigation_Passage::go_history(request, message);
    }
  }
  
  // Build the navigation html fragment.
  return Navigation_Passage::get_mouse_navigator (request, bible);
}


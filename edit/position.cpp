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


#include <edit/position.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <editor/usfm2html.h>
#include <access/bible.h>
#include <database/config/bible.h>


std::string edit_position_url ()
{
  return "edit/position";
}


bool edit_position_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) 
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string edit_position (Webserver_Request& webserver_request)
{
  // Get Bible: If an empty Bible is given, bail out.
  const std::string bible = webserver_request.query ["bible"];
  if (bible.empty ())
    return std::string();
  // Get book: If no book is given: Bail out.
  const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  if (!book) return std::string();
  // Get chapter.
  const int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  
  
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  const int verse = Ipc_Focus::getVerse (webserver_request);


  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  
  int starting_offset = 0;
  int ending_offset = 0;
  // To deal with a combined verse, go through the offsets, and pick the correct one.
  for (auto element : editor_usfm2html.m_verse_start_offsets) {
    const int vs = element.first;
    const int offset = element.second;
    if (vs <= verse)
      starting_offset = offset;
    if (ending_offset == 0) {
      if (vs > verse) {
        ending_offset = offset;
      }
    }
  }
  if (verse) {
    starting_offset += static_cast<int> (std::to_string (verse).length () + 1);
  }
  if (ending_offset) {
    ending_offset--;
  } else {
    ending_offset = static_cast<int>(editor_usfm2html.m_text_tength);
  }
  
  std::string data = std::to_string (starting_offset);
  data.append ("\n");
  data.append (std::to_string (ending_offset));
  
  return data;
}

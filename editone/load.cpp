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


#include <editone/load.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <editor/usfm2html.h>
#include <config/globals.h>
#include <access/bible.h>
#include <editone/logic.h>
#include <edit/logic.h>
#include <database/config/bible.h>


std::string editone_load_url ()
{
  return "editone/load";
}


bool editone_load_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editone_load (Webserver_Request& webserver_request)
{
  const std::string bible = webserver_request.query ["bible"];
  const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  const int chapter = filter::strings::convert_to_int (webserver_request.query ["chapter"]);
  const int verse = filter::strings::convert_to_int (webserver_request.query ["verse"]);
  const std::string unique_id = webserver_request.query ["id"];
  
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);

  const std::string chapter_usfm = database::bibles::get_chapter (bible, book, chapter);

  const std::vector <int> verses = filter::usfm::get_verse_numbers (chapter_usfm);
  int highest_verse = 0;
  if (!verses.empty ())
    highest_verse = verses.back ();
  
  // The Quill-based editor removes empty paragraphs at the end.
  // Therefore do not include them.
  const std::string editable_usfm = filter::usfm::get_verse_text_quill (chapter_usfm, verse);
  
  const std::string prefix_usfm = filter::usfm::get_verse_range_text (chapter_usfm, 0, verse - 1, editable_usfm, true);
  const std::string suffix_usfm = filter::usfm::get_verse_range_text (chapter_usfm, verse + 1, highest_verse, editable_usfm, true);

  // Store a copy of the USFM loaded in the editor for later reference.
  // Note that this verse editor has been tested that it uses the correct sequence
  // while storing this snapshot.
  // When the user goes to another verse in the editor, the system follows this sequence:
  // 1. It saves the edited text in the current verse.
  // 2. It updates the chapter snapshot.
  // 3. It loads the other verse.
  // 4. It updates the chapter snapshot.
  store_loaded_usfm (webserver_request, bible, book, chapter, unique_id);
  
  std::string prefix_html;
  std::string not_used;
  editone_logic_prefix_html (prefix_usfm, stylesheet, prefix_html, not_used);
  
  // The focused editable verse also has any footnotes contained in that verse.
  // It is convenient to have the footnote as near as possible to the verse text.
  // This is helpful for editing the verse and note.
  std::string focused_verse_html;
  editone_logic_editable_html (editable_usfm, stylesheet, focused_verse_html);
  
  std::string suffix_html;
  editone_logic_suffix_html ("", suffix_usfm, stylesheet, suffix_html);
  
  // If the verse was empty, ensure that it has a non-breaking space as the last character,
  // for easier text entry in the verse.
  std::string plain_text = filter::strings::html2text (focused_verse_html);
  plain_text = filter::strings::trim (plain_text);
  const std::string vs = std::to_string (verse);
  const bool editable_verse_is_empty = plain_text == vs;
  if (editable_verse_is_empty) {
    const std::string search = "<span> </span></p>";
    const std::string replace = "<span>" + filter::strings::unicode_non_breaking_space_entity () + "</span></p>";
    focused_verse_html = filter::strings::replace (search, replace, focused_verse_html);
  }

  // Moves any notes from the prefix to the suffix.
  editone_logic_move_notes (prefix_html, suffix_html);
  
  std::string data;
  data.append (prefix_html);
  data.append ("#_be_#");
  data.append (focused_verse_html);
  data.append ("#_be_#");
  data.append (suffix_html);
  
  const std::string& user = webserver_request.session_logic ()->get_username ();
  const bool write = access_bible::book_write (webserver_request, user, bible, book);
  data = checksum_logic::send (data, write);

  return data;
}

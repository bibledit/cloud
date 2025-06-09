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


#include <notes/notes.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <access/logic.h>


std::string notes_notes_url ()
{
  return "notes/notes";
}


bool notes_notes_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_notes (webserver_request);
}


std::string notes_notes (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);

  
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible());
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);

  
  int passage_selector = webserver_request.database_config_user()->get_consultation_notes_passage_selector();
  int edit_selector = webserver_request.database_config_user()->get_consultation_notes_edit_selector();
  int non_edit_selector = webserver_request.database_config_user()->get_consultation_notes_non_edit_selector();
  std::string status_selector = webserver_request.database_config_user()->get_consultation_notes_status_selector();
  std::string bible_selector = webserver_request.database_config_user()->get_consultation_notes_bible_selector();
  std::string assignment_selector = webserver_request.database_config_user()->get_consultation_notes_assignment_selector();
  bool subscription_selector = webserver_request.database_config_user()->get_consultation_notes_subscription_selector();
  int severity_selector = webserver_request.database_config_user()->get_consultation_notes_severity_selector();
  int text_selector = webserver_request.database_config_user()->get_consultation_notes_text_selector();
  std::string search_text = webserver_request.database_config_user()->get_consultation_notes_search_text();
  int passage_inclusion_selector = webserver_request.database_config_user()->get_consultation_notes_passage_inclusion_selector();
  int text_inclusion_selector = webserver_request.database_config_user()->get_consultation_notes_text_inclusion_selector();

  
  // The Bibles the current user has access to.
  std::vector <std::string> bibles = access_bible::bibles (webserver_request, webserver_request.session_logic ()->get_username ());
  
  
  // The admin disables notes selection on Bibles,
  // so the admin sees all notes, including notes referring to non-existing Bibles.
  if (webserver_request.session_logic ()->get_level () == roles::admin) bibles.clear ();
  
  
  std::vector <int> identifiers = database_notes.select_notes (bibles, book, chapter, verse, passage_selector, edit_selector, non_edit_selector, status_selector, bible_selector, assignment_selector, subscription_selector, severity_selector, text_selector, search_text, -1);
  
  
  // In case there aren't too many notes, there's enough time to sort them in passage order.
  if (identifiers.size () <= 200) {
    std::vector <int> passage_sort_keys;
    for (auto & identifier : identifiers) {
      int passage_sort_key = 0;
      std::vector <double> numeric_passages;
      std::vector <Passage> passages = database_notes.get_passages (identifier);
      for (auto & passage : passages) {
        numeric_passages.push_back (filter_passage_to_integer (passage));
      }
      if (!numeric_passages.empty ()) {
        double average = static_cast<double>(accumulate (numeric_passages.begin (), numeric_passages.end (), 0)) / static_cast<double>(numeric_passages.size ());
        passage_sort_key = static_cast<int> (round(average));
      }
      passage_sort_keys.push_back (passage_sort_key);
    }
    filter::strings::quick_sort (passage_sort_keys, identifiers, 0, static_cast <unsigned> (identifiers.size ()));
  }


  const bool show_bible_in_notes_list = webserver_request.database_config_user ()->get_show_bible_in_notes_list ();
  const bool show_note_status = webserver_request.database_config_user ()->get_show_note_status ();
  const bool color_note_status = webserver_request.database_config_user ()->get_use_colored_note_status_labels ();
  std::stringstream notesblock;
  for (auto & identifier : identifiers) {

    std::string summary = database_notes.get_summary (identifier);
    std::vector <Passage> passages = database_notes.get_passages (identifier);
    std::string verses = filter_passage_display_inline (passages);
    if (show_note_status) {
      std::string status_text = database_notes.get_status (identifier);
      std::string raw_status;
      if (color_note_status) {
        // The class properties are in the stylesheet.
        // Distinct colors were generated through https://mokole.com/palette.html.
        raw_status = database_notes.get_raw_status (identifier);
        raw_status = filter::strings::unicode_string_casefold (raw_status);
        raw_status = filter::strings::replace (" ", "", raw_status);
        std::string css_class;
        if (raw_status == "new") css_class = Filter_Css::distinction_set_notes (0);
        else if (raw_status == "pending") css_class = Filter_Css::distinction_set_notes (1);
        else if (raw_status == "inprogress") css_class = Filter_Css::distinction_set_notes (2);
        else if (raw_status == "done") css_class = Filter_Css::distinction_set_notes (3);
        else if (raw_status == "reopened") css_class = Filter_Css::distinction_set_notes (4);
        else css_class = Filter_Css::distinction_set_notes (5);
        if (!css_class.empty()) status_text.insert (0, R"(<span class=")" + css_class + R"(">)");
        status_text.append ("</span>");
      }
      verses.insert (0, status_text + " ");
    }
    if (show_bible_in_notes_list) {
      std::string note_bible = database_notes.get_bible (identifier);
      verses.insert (0, note_bible + " ");
    }
    // A simple way to make it easier to see the individual notes in the list,
    // when the summaries of some notes are long, is to display the passage first.
    summary.insert (0, verses + " | ");

    std::string verse_text;
    if (passage_inclusion_selector) {
      std::vector <Passage> include_passages = database_notes.get_passages (identifier);
      for (auto & passage : include_passages) {
        std::string usfm = database::bibles::get_chapter (bible, passage.m_book, passage.m_chapter);
        std::string text = filter::usfm::get_verse_text (usfm, filter::strings::convert_to_int (passage.m_verse));
        if (!verse_text.empty ()) verse_text.append ("<br>");
        verse_text.append (text);
      }
    }
    
    std::string content;
    if (text_inclusion_selector) {
      content = database_notes.get_contents (identifier);
    }

    notesblock << "<a name=" << std::quoted ("note" + std::to_string (identifier)) << "></a>" << std::endl;
    notesblock << "<p><a href=" << std::quoted ("note?id=" + std::to_string (identifier)) << ">" << summary << "</a></p>" << std::endl;
    if (!verse_text.empty ()) notesblock << "<p>" << verse_text << "</p>" << std::endl;
    if (!content.empty ()) notesblock << "<p>" << content << "</p>" << std::endl;
  }

  
  if (identifiers.empty ()) {
    return translate("This selection does not display any notes.");
  }
  return notesblock.str();
}

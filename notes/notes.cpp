/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
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
#include <numeric>
#include <sstream>
#include <iomanip>
#include <cmath>


string notes_notes_url ()
{
  return "notes/notes";
}


bool notes_notes_acl (void * webserver_request)
{
  return access_logic_privilege_view_notes (webserver_request);
}


string notes_notes (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);

  
  string bible = AccessBible::Clamp (webserver_request, request->database_config_user()->getBible());
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);

  
  int passage_selector = request->database_config_user()->getConsultationNotesPassageSelector();
  int edit_selector = request->database_config_user()->getConsultationNotesEditSelector();
  int non_edit_selector = request->database_config_user()->getConsultationNotesNonEditSelector();
  string status_selector = request->database_config_user()->getConsultationNotesStatusSelector();
  string bible_selector = request->database_config_user()->getConsultationNotesBibleSelector();
  string assignment_selector = request->database_config_user()->getConsultationNotesAssignmentSelector();
  bool subscription_selector = request->database_config_user()->getConsultationNotesSubscriptionSelector();
  int severity_selector = request->database_config_user()->getConsultationNotesSeveritySelector();
  int text_selector = request->database_config_user()->getConsultationNotesTextSelector();
  string search_text = request->database_config_user()->getConsultationNotesSearchText();
  int passage_inclusion_selector = request->database_config_user()->getConsultationNotesPassageInclusionSelector();
  int text_inclusion_selector = request->database_config_user()->getConsultationNotesTextInclusionSelector();

  
  // The Bibles the current user has access to.
  vector <string> bibles = AccessBible::Bibles (webserver_request, request->session_logic()->currentUser ());
  
  
  // The admin disables notes selection on Bibles,
  // so the admin sees all notes, including notes referring to non-existing Bibles.
  if (request->session_logic ()->currentLevel () == Filter_Roles::admin ()) bibles.clear ();
  
  
  vector <int> identifiers = database_notes.select_notes (bibles, book, chapter, verse, passage_selector, edit_selector, non_edit_selector, status_selector, bible_selector, assignment_selector, subscription_selector, severity_selector, text_selector, search_text, -1);
  
  
  // In case there aren't too many notes, there's enough time to sort them in passage order.
  if (identifiers.size () <= 200) {
    vector <int> passage_sort_keys;
    for (auto & identifier : identifiers) {
      int passage_sort_key = 0;
      vector <double> numeric_passages;
      vector <Passage> passages = database_notes.get_passages (identifier);
      for (auto & passage : passages) {
        numeric_passages.push_back (filter_passage_to_integer (passage));
      }
      if (!numeric_passages.empty ()) {
        double average = accumulate (numeric_passages.begin (), numeric_passages.end (), 0) / numeric_passages.size ();
        passage_sort_key = round(average);
      }
      passage_sort_keys.push_back (passage_sort_key);
    }
    quick_sort (passage_sort_keys, identifiers, 0, (int)identifiers.size ());
  }


  bool show_bible_in_notes_list = request->database_config_user ()->getShowBibleInNotesList ();
  bool show_note_status = request->database_config_user ()->getShowNoteStatus ();
  bool color_note_status = request->database_config_user ()->getUseColoredNoteStatusLabels ();
  stringstream notesblock;
  for (auto & identifier : identifiers) {

    string summary = database_notes.get_summary (identifier);
    vector <Passage> passages = database_notes.get_passages (identifier);
    string verses = filter_passage_display_inline (passages);
    if (show_note_status) {
      string status_text = database_notes.get_status (identifier);
      string raw_status;
      if (color_note_status) {
        // The class properties are in the stylesheet.
        // Distinct colors were generated through https://mokole.com/palette.html.
        raw_status = database_notes.get_raw_status (identifier);
        raw_status = unicode_string_casefold (raw_status);
        raw_status = filter_string_str_replace (" ", "", raw_status);
        string css_class;
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
      string bible = database_notes.get_bible (identifier);
      verses.insert (0, bible + " ");
    }
    // A simple way to make it easier to see the individual notes in the list,
    // when the summaries of some notes are long, is to display the passage first.
    summary.insert (0, verses + " | ");

    string verse_text;
    if (passage_inclusion_selector) {
      vector <Passage> passages = database_notes.get_passages (identifier);
      for (auto & passage : passages) {
        string usfm = request->database_bibles()->getChapter (bible, passage.book, passage.chapter);
        string text = usfm_get_verse_text (usfm, convert_to_int (passage.verse));
        if (!verse_text.empty ()) verse_text.append ("<br>");
        verse_text.append (text);
      }
    }
    
    string content;
    if (text_inclusion_selector) {
      content = database_notes.get_contents (identifier);
    }

    notesblock << "<a name=" << quoted ("note" + convert_to_string (identifier)) << "></a>" << endl;
    notesblock << "<p><a href=" << quoted ("note?id=" + convert_to_string (identifier)) << ">" << summary << "</a></p>" << endl;
    if (!verse_text.empty ()) notesblock << "<p>" << verse_text << "</p>" << endl;
    if (!content.empty ()) notesblock << "<p>" << content << "</p>" << endl;
  }

  
  if (identifiers.empty ()) {
    return translate("This selection does not display any notes.");
  }
  return notesblock.str();
}

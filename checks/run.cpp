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


#include <checks/run.h>
#include <webserver/request.h>
#include <database/logs.h>
#include <database/check.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <database/modifications.h>
#include <database/git.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <client/logic.h>
#include <checks/sentences.h>
#include <checks/versification.h>
#include <checks/headers.h>
#include <checks/space.h>
#include <checks/usfm.h>
#include <checks/verses.h>
#include <checks/pairs.h>
#include <checks/index.h>
#include <checks/settings.h>
#include <checks/french.h>
#include <email/send.h>
#include <sendreceive/logic.h>
#include <rss/logic.h>


void checks_run (std::string bible)
{
  Webserver_Request webserver_request {};

  
  if (bible.empty()) return;
  
  
  Database_Logs::log ("Check " + bible + ": Start", Filter_Roles::translator ());
  
  
  database::check::truncate_output (bible);
  
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  
  bool check_double_spaces_usfm = database::config::bible::get_check_double_spaces_usfm (bible);
  bool check_full_stop_in_headings = database::config::bible::get_check_full_stop_in_headings (bible);
  bool check_space_before_punctuation = database::config::bible::get_check_space_before_punctuation (bible);
  bool check_space_before_final_note_marker = database::config::bible::get_check_space_before_final_note_marker (bible);
  bool check_sentence_structure = database::config::bible::get_check_sentence_structure (bible);
  bool check_paragraph_structure = database::config::bible::get_check_paragraph_structure (bible);
  Checks_Sentences checks_sentences;
  checks_sentences.enter_capitals (database::config::bible::get_sentence_structure_capitals (bible));
  checks_sentences.enter_small_letters (database::config::bible::get_sentence_structure_small_letters (bible));
  std::string end_marks = database::config::bible::get_sentence_structure_end_punctuation (bible);
  checks_sentences.enter_end_marks (end_marks);
  std::string center_marks = database::config::bible::get_sentence_structure_middle_punctuation (bible);
  checks_sentences.enter_center_marks (center_marks);
  std::string disregards = database::config::bible::get_sentence_structure_disregards (bible);
  checks_sentences.enter_disregards (disregards);
  checks_sentences.enter_names (database::config::bible::get_sentence_structure_names (bible));
  std::vector <std::string> within_sentence_paragraph_markers = filter::strings::explode (database::config::bible::get_sentence_structure_within_sentence_markers (bible), ' ');
  bool check_books_versification = database::config::bible::get_check_books_versification (bible);
  bool check_chapters_verses_versification = database::config::bible::get_check_chaptes_verses_versification (bible);
  bool check_well_formed_usfm = database::config::bible::get_check_well_formed_usfm (bible);
  Checks_Usfm checks_usfm = Checks_Usfm (bible);
  bool check_missing_punctuation_end_verse = database::config::bible::get_check_missing_punctuation_end_verse (bible);
  bool check_patterns = database::config::bible::get_check_patterns (bible);
  std::string s_checking_patterns = database::config::bible::get_checking_patterns (bible);
  std::vector <std::string> checking_patterns = filter::strings::explode (s_checking_patterns, '\n');
  bool check_matching_pairs = database::config::bible::get_check_matching_pairs (bible);
  std::vector <std::pair <std::string, std::string> > matching_pairs;
  {
    const std::string fragment = database::config::bible::get_matching_pairs (bible);
    std::vector <std::string> pairs = filter::strings::explode (fragment, ' ');
    for (auto& pair : pairs) {
      pair = filter::strings::trim (pair);
      const size_t length = filter::strings::unicode_string_length (pair);
      if (length == 2) {
        const std::string opener = filter::strings::unicode_string_substr (pair, 0, 1);
        const std::string closer = filter::strings::unicode_string_substr (pair, 1, 1);
        matching_pairs.push_back ({opener, closer});
      }
    }
  }
  bool check_space_end_verse = database::config::bible::get_check_space_end_verse (bible);
  bool check_french_punctuation = database::config::bible::get_check_french_punctuation (bible);
  bool check_french_citation_style = database::config::bible::get_check_french_citation_style (bible);
  bool transpose_fix_space_in_notes = database::config::bible::get_transpose_fix_spaces_notes (bible);
  bool check_valid_utf8_text = database::config::bible::get_check_valid_utf8_text (bible);

  
  const std::vector <int> books = database::bibles::get_books (bible);
  if (check_books_versification) checks_versification::books (bible, books);
  
  
  for (auto book : books) {
    
    
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (check_chapters_verses_versification) checks_versification::chapters (bible, book, chapters);
    
    
    for (auto chapter : chapters) {
      std::string chapterUsfm = database::bibles::get_chapter (bible, book, chapter);
    
      
      // Transpose and fix spacing around certain markers in footnotes and cross references.
      if (transpose_fix_space_in_notes) {
        std::string old_usfm (chapterUsfm);
        const bool transposed = checks::space::transpose_note_space (chapterUsfm);
        if (transposed) {
#ifndef HAVE_CLIENT
          const int oldID = database::bibles::get_chapter_id (bible, book, chapter);
#endif
          database::bibles::store_chapter(bible, book, chapter, chapterUsfm);
#ifndef HAVE_CLIENT
          const int newID = database::bibles::get_chapter_id (bible, book, chapter);
          const std::string username = "Bibledit";
          database::modifications::recordUserSave (username, bible, book, chapter, oldID, old_usfm, newID, chapterUsfm);
          if (sendreceive_git_repository_linked (bible)) {
            database::git::store_chapter (username, bible, book, chapter, old_usfm, chapterUsfm);
          }
          rss_logic_schedule_update (username, bible, book, chapter, old_usfm, chapterUsfm);
#endif
          Database_Logs::log ("Transposed and fixed double spaces around markers in footnotes or cross references in " + filter_passage_display (book, chapter, "") + " in Bible " + bible);
        }
      }
      
      
      std::vector <int> verses = filter::usfm::get_verse_numbers (chapterUsfm);
      if (check_chapters_verses_versification) checks_versification::verses (bible, book, chapter, verses);
      
      
      for (auto verse : verses) {
        const std::string verseUsfm = filter::usfm::get_verse_text (chapterUsfm, verse);
        if (check_double_spaces_usfm) {
          checks::space::double_space_usfm (bible, book, chapter, verse, verseUsfm);
        }
        if (check_valid_utf8_text) {
          if (!filter::strings::unicode_string_is_valid (verseUsfm)) {
            const std::string msg = "Invalid UTF-8 Unicode in verse text";
            database::check::record_output (bible, book, chapter, verse, msg);
          }
        }
        if (check_space_before_final_note_marker) {
          checks::space::space_before_final_note_markup(bible, book, chapter, verse, verseUsfm);
        }
      }
      
      
      Filter_Text filter_text = Filter_Text (bible);
      filter_text.initializeHeadingsAndTextPerVerse (false);
      filter_text.add_usfm_code (chapterUsfm);
      filter_text.run (stylesheet);
      std::map <int, std::string> verses_headings = filter_text.verses_headings;
      std::map <int, std::string> verses_text = filter_text.getVersesText ();
      std::vector <std::map <int, std::string>> verses_paragraphs = filter_text.verses_paragraphs;
      if (check_full_stop_in_headings) {
        checks_headers::no_punctuation_at_end (bible, book, chapter, verses_headings, center_marks, end_marks);
      }
      if (check_space_before_punctuation) {
        checks::space::space_before_punctuation (bible, book, chapter, verses_text);
      }
      
      if (check_sentence_structure || check_paragraph_structure) {
        checks_sentences.initialize ();
        if (check_sentence_structure) checks_sentences.check (verses_text);
        if (check_paragraph_structure) {
          checks_sentences.paragraphs (filter_text.paragraph_starting_markers,
                                       within_sentence_paragraph_markers,
                                       verses_paragraphs);
        }
        
        const std::vector <std::pair<int, std::string>> results = checks_sentences.get_results ();
        for (const auto& result : results) {
          const int verse = result.first;
          const std::string msg = result.second;
          database::check::record_output (bible, book, chapter, verse, msg);
        }
      }

      if (check_well_formed_usfm) {
        checks_usfm.initialize (book, chapter);
        checks_usfm.check (chapterUsfm);
        checks_usfm.finalize ();
        std::vector <std::pair<int, std::string>> results = checks_usfm.get_results ();
        for (const auto& element : results) {
          const int verse = element.first;
          const std::string msg = element.second;
          database::check::record_output (bible, book, chapter, verse, msg);
        }
      }

      if (check_missing_punctuation_end_verse) {
        checks_verses::missing_punctuation_at_end (bible, book, chapter, verses_text, center_marks, end_marks, disregards);
      }
      
      if (check_patterns) {
        checks_verses::patterns (bible, book, chapter, verses_text, checking_patterns);
      }
      
      if (check_matching_pairs) {
        checks_pairs::run (bible, book, chapter, verses_text, matching_pairs, check_french_citation_style);
      }
      
      if (check_space_end_verse) {
        checks::space::space_end_verse (bible, book, chapter, chapterUsfm);
      }
      
      if (check_french_punctuation) {
        checks_french::space_before_after_punctuation (bible, book, chapter, verses_headings);
        checks_french::space_before_after_punctuation (bible, book, chapter, verses_text);
      }
      
      if (check_french_citation_style) {
        checks_french::citation_style (bible, book, chapter, verses_paragraphs);
      }
      
    }
  }
  
  
  // Create an email with the checking results for this bible.
  std::vector <std::string> emailBody;
  std::vector <database::check::Hit> hits = database::check::get_hits ();
  for (const auto & hit : hits) {
    if (hit.bible == bible) {
      const std::string passage = filter_passage_display_inline ({Passage ("", hit.book, hit.chapter, std::to_string (hit.verse))});
      const std::string data = filter::strings::escape_special_xml_characters (hit.data);
      const std::string result = "<p>" + passage + " " + data + "</p>";
      emailBody.push_back (result);
    }
  }
  
  
  // Add a link to the online checking results.
  if (!emailBody.empty ()) {
    Webserver_Request webserver_request;
    const std::string siteUrl = config::logic::site_url (webserver_request);
    std::stringstream body1 {};
    body1 << "<p><a href=" << std::quoted (siteUrl + checks_index_url ()) << ">" << translate("Checking results online") << "</a></p>";
    emailBody.push_back (body1.str());
    std::stringstream body2 {};
    body2 << "<p><a href=" << std::quoted(siteUrl + checks_settings_url ()) << ">" << translate ("Settings") << "</a></p>";
    emailBody.push_back (body2.str());
  }
  
  
  // Send email to users with access to the Bible and a subscription to the notification.
  if (!emailBody.empty ()) {
    const std::string subject = translate("Bible Checks") + " " + bible;
    const std::string body = filter::strings::implode (emailBody, "\n");
    std::vector <std::string> users = webserver_request.database_users ()->get_users ();
    for (const auto& user : users) {
      if (webserver_request.database_config_user()->getUserBibleChecksNotification (user)) {
        if (access_bible::read (webserver_request, bible, user)) {
          if (!client_logic_client_enabled ()) {
            email_schedule (user, subject, body);
          }
        }
      }
    }
  }
  
  
  Database_Logs::log ("Check " + bible + ": Complete", Filter_Roles::translator ());
}

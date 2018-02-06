/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


void checks_run (string bible)
{
  Webserver_Request request;
  Database_Check database_check;
  
  
  if (bible == "") return;
  
  
  Database_Logs::log ("Check " + bible + ": Start", Filter_Roles::translator ());
  
  
  database_check.truncateOutput (bible);
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  bool check_double_spaces_usfm = Database_Config_Bible::getCheckDoubleSpacesUsfm (bible);
  bool check_full_stop_in_headings = Database_Config_Bible::getCheckFullStopInHeadings (bible);
  bool check_space_before_punctuation = Database_Config_Bible::getCheckSpaceBeforePunctuation (bible);
  bool check_sentence_structure = Database_Config_Bible::getCheckSentenceStructure (bible);
  bool check_paragraph_structure = Database_Config_Bible::getCheckParagraphStructure (bible);
  Checks_Sentences checks_sentences;
  checks_sentences.enterCapitals (Database_Config_Bible::getSentenceStructureCapitals (bible));
  checks_sentences.enterSmallLetters (Database_Config_Bible::getSentenceStructureSmallLetters (bible));
  string end_marks = Database_Config_Bible::getSentenceStructureEndPunctuation (bible);
  checks_sentences.enterEndMarks (end_marks);
  string center_marks = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  checks_sentences.enterCenterMarks (center_marks);
  string disregards = Database_Config_Bible::getSentenceStructureDisregards (bible);
  checks_sentences.enterDisregards (disregards);
  checks_sentences.enterNames (Database_Config_Bible::getSentenceStructureNames (bible));
  vector <string> within_sentence_paragraph_markers = filter_string_explode (Database_Config_Bible::getSentenceStructureWithinSentenceMarkers (bible), ' ');
  bool check_books_versification = Database_Config_Bible::getCheckBooksVersification (bible);
  bool check_chapters_verses_versification = Database_Config_Bible::getCheckChaptesVersesVersification (bible);
  bool check_well_formed_usfm = Database_Config_Bible::getCheckWellFormedUsfm (bible);
  Checks_Usfm checks_usfm = Checks_Usfm (bible);
  bool check_missing_punctuation_end_verse = Database_Config_Bible::getCheckMissingPunctuationEndVerse (bible);
  bool check_patterns = Database_Config_Bible::getCheckPatterns (bible);
  string s_checking_patterns = Database_Config_Bible::getCheckingPatterns (bible);
  vector <string> checking_patterns = filter_string_explode (s_checking_patterns, '\n');
  bool check_matching_pairs = Database_Config_Bible::getCheckMatchingPairs (bible);
  vector <pair <string, string> > matching_pairs;
  {
    string fragment = Database_Config_Bible::getMatchingPairs (bible);
    vector <string> pairs = filter_string_explode (fragment, ' ');
    for (auto & pair : pairs) {
      pair = filter_string_trim (pair);
      size_t length = unicode_string_length (pair);
      if (length == 2) {
        string opener = unicode_string_substr (pair, 0, 1);
        string closer = unicode_string_substr (pair, 1, 1);
        matching_pairs.push_back (make_pair (opener, closer));
      }
    }
  }
  bool check_space_end_verse = Database_Config_Bible::getCheckSpaceEndVerse (bible);
  bool check_french_punctuation = Database_Config_Bible::getCheckFrenchPunctuation (bible);
  bool check_french_citation_style = Database_Config_Bible::getCheckFrenchCitationStyle (bible);

  
  vector <int> books = request.database_bibles()->getBooks (bible);
  if (check_books_versification) Checks_Versification::books (bible, books);
  
  
  for (auto book : books) {
    
    
    vector <int> chapters = request.database_bibles()->getChapters (bible, book);
    if (check_chapters_verses_versification) Checks_Versification::chapters (bible, book, chapters);
    
    
    for (auto chapter : chapters) {
      string chapterUsfm = request.database_bibles()->getChapter (bible, book, chapter);
      
      
      vector <int> verses = usfm_get_verse_numbers (chapterUsfm);
      if (check_chapters_verses_versification) Checks_Versification::verses (bible, book, chapter, verses);
      
      
      for (auto verse : verses) {
        string verseUsfm = usfm_get_verse_text (chapterUsfm, verse);
        if (check_double_spaces_usfm) {
          Checks_Space::doubleSpaceUsfm (bible, book, chapter, verse, verseUsfm);
        }
      }
      
      
      Filter_Text filter_text = Filter_Text (bible);
      filter_text.initializeHeadingsAndTextPerVerse (false);
      filter_text.addUsfmCode (chapterUsfm);
      filter_text.run (stylesheet);
      map <int, string>  verses_headings = filter_text.verses_headings;
      map <int, string> verses_text = filter_text.getVersesText ();
      vector <map <int, string>> verses_paragraphs = filter_text.verses_paragraphs;
      if (check_full_stop_in_headings) {
        Checks_Headers::noPunctuationAtEnd (bible, book, chapter, verses_headings, center_marks, end_marks);
      }
      if (check_space_before_punctuation) {
        Checks_Space::spaceBeforePunctuation (bible, book, chapter, verses_text);
      }
      
      if (check_sentence_structure || check_paragraph_structure) {
        checks_sentences.initialize ();
        if (check_sentence_structure) checks_sentences.check (verses_text);
        if (check_paragraph_structure) {
          checks_sentences.paragraphs (filter_text.paragraph_starting_markers,
                                       within_sentence_paragraph_markers,
                                       verses_paragraphs);
        }
        
        vector <pair<int, string>> results = checks_sentences.getResults ();
        for (auto result : results) {
          int verse = result.first;
          string msg = result.second;
          database_check.recordOutput (bible, book, chapter, verse, msg);
        }
      }

      if (check_well_formed_usfm) {
        checks_usfm.initialize (book, chapter);
        checks_usfm.check (chapterUsfm);
        checks_usfm.finalize ();
        vector <pair<int, string>>  results = checks_usfm.getResults ();
        for (auto element : results) {
          int verse = element.first;
          string msg = element.second;
          database_check.recordOutput (bible, book, chapter, verse, msg);
        }
      }

      if (check_missing_punctuation_end_verse) {
        Checks_Verses::missingPunctuationAtEnd (bible, book, chapter, verses_text, center_marks, end_marks, disregards);
      }
      
      if (check_patterns) {
        Checks_Verses::patterns (bible, book, chapter, verses_text, checking_patterns);
      }
      
      if (check_matching_pairs) {
        Checks_Pairs::run (bible, book, chapter, verses_text, matching_pairs, check_french_citation_style);
      }
      
      if (check_space_end_verse) {
        Checks_Space::spaceEndVerse (bible, book, chapter, chapterUsfm);
      }
      
      if (check_french_punctuation) {
        Checks_French::spaceBeforeAfterPunctuation (bible, book, chapter, verses_headings);
        Checks_French::spaceBeforeAfterPunctuation (bible, book, chapter, verses_text);
      }
      
      if (check_french_citation_style) {
        Checks_French::citationStyle (bible, book, chapter, verses_paragraphs);
      }
      
    }
  }
  
  
  // Create an email with the checking results for this bible.
  vector <string> emailBody;
  vector <Database_Check_Hit> hits = database_check.getHits ();
  for (auto hit : hits) {
    if (hit.bible == bible) {
      string passage = filter_passage_display_inline ({Passage ("", hit.book, hit.chapter, convert_to_string (hit.verse))});
      string data = escape_special_xml_characters (hit.data);
      string result = "<p>" + passage + " " + data + "</p>";
      emailBody.push_back (result);
    }
  }
  
  
  // Add a link to the online checking results.
  if (!emailBody.empty ()) {
    string siteUrl = config_logic_site_url (NULL);
    emailBody.push_back ("<p><a href=\"" + siteUrl + checks_index_url () + "\">" + translate("Checking results online") + "</a></p>");
    emailBody.push_back ("<p><a href=\"" + siteUrl + checks_settings_url () + "\">" + translate ("Settings") + "</a></p>");
  }
  
  
  // Send email to users with access to the Bible and a subscription to the notification.
  if (!emailBody.empty ()) {
    string subject = translate("Bible Checks") + " " + bible;
    string body = filter_string_implode (emailBody, "\n");
    vector <string> users = request.database_users ()->getUsers ();
    for (auto user : users) {
      if (request.database_config_user()->getUserBibleChecksNotification (user)) {
        if (access_bible_read (&request, bible, user)) {
          if (!client_logic_client_enabled ()) {
            email_schedule (user, subject, body);
          }
        }
      }
    }
  }
  
  
  Database_Logs::log ("Check " + bible + ": Complete", Filter_Roles::translator ());
}

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


#include <nmt/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/text.h>
#include <filter/usfm.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/versifications.h>
#include <database/config/bible.h>
#include <database/mappings.h>


void nmt_logic_export (std::string referencebible, std::string translatingbible)
{
  Database_Logs::log (R"(Exporting reference Bible ")" + referencebible + R"(" plus translated Bible ")" + translatingbible + R"(" for a neural machine translation training job)");
  
  Database_Bibles database_bibles;
  Database_Versifications database_versifications;
  Database_Mappings database_mappings;

  std::vector <std::string> reference_lines;
  std::vector <std::string> translation_lines;
  
  // Get the versification systems of both Bibles.
  std::string reference_versification = Database_Config_Bible::getVersificationSystem (referencebible);
  std::string translating_versification = Database_Config_Bible::getVersificationSystem (translatingbible);
  
  std::vector <int> books = database_bibles.get_books (referencebible);
  for (auto book : books) {
  
    // Take books that contain text, leave others, like front matter, out.
    book_type type = database::books::get_type (static_cast<book_id>(book));
    if ((type != book_type::old_testament) && (type != book_type::new_testament) && (type != book_type::apocryphal)) continue;
    
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    Database_Logs::log ("Exporting " + bookname);
    
    std::vector <int> chapters = database_bibles.get_chapters (referencebible, book);
    for (auto reference_chapter : chapters) {
      
      // Skip chapter 0.
      // It won't contain Bible text.
      if (reference_chapter == 0) continue;

      std::vector <int> verses = database_versifications.getMaximumVerses (book, reference_chapter);
      for (auto & reference_verse : verses) {
       
        // Verse 0 won't contain Bible text: Skip it.
        if (reference_verse == 0) continue;

        // Use the versification system to get the matching chapter and verse of the Bible in translation.
        std::vector <Passage> translation_passages;
        if ((reference_versification != translating_versification) && !translating_versification.empty ()) {
          translation_passages = database_mappings.translate (reference_versification, translating_versification, book, reference_chapter, reference_verse);
        } else {
          translation_passages.push_back (Passage ("", book, reference_chapter, filter::strings::convert_to_string (reference_verse)));
        }

        // If the conversion from one versification system to another
        // leads to one verse for the reference Bible,
        // and two verses for the Bible in translation,
        // then this would indicate a mismatch in verse contents between the two Bibles.
        // This mismatch would disturb the neural machine translation training process.
        // So such versea are skipped for that reason.
        if (translation_passages.size() != 1) {
          std::string referencetext = filter_passage_display_inline (translation_passages);
          std::string message = "Skipping reference Bible verse " + filter::strings::convert_to_string (reference_verse) + " and translated Bible " + referencetext;
          Database_Logs::log (message);
          continue;
        }
        
        int translation_chapter = translation_passages[0].m_chapter;
        int translation_verse = filter::strings::convert_to_int (translation_passages[0].m_verse);

        // Convert the verse USFM of the reference Bible to plain verse text.
        std::string reference_text;
        {
          std::string chapter_usfm = database_bibles.get_chapter (referencebible, book, reference_chapter);
          const std::string stylesheet = styles_logic_standard_sheet ();
          Filter_Text filter_text = Filter_Text ("");
          filter_text.initializeHeadingsAndTextPerVerse (false);
          filter_text.add_usfm_code (chapter_usfm);
          filter_text.run (stylesheet);
          std::map <int, std::string> output = filter_text.getVersesText ();
          reference_text = output [reference_verse];
          
          // The text may contain new lines, so remove these,
          // because the NMT training files should not contain new lines mid-text,
          // as that would cause misalignments in the two text files used for training.
          reference_text = filter::strings::replace ("\n", " ", reference_text);
        }

        // Convert the verse USFM of the Bible being translated to plain verse text.
        std::string translation_text;
        {
          std::string chapter_usfm = database_bibles.get_chapter (translatingbible, book, translation_chapter);
          const std::string stylesheet = styles_logic_standard_sheet ();
          Filter_Text filter_text = Filter_Text ("");
          filter_text.initializeHeadingsAndTextPerVerse (false);
          filter_text.add_usfm_code (chapter_usfm);
          filter_text.run (stylesheet);
          std::map <int, std::string> output = filter_text.getVersesText ();
          translation_text = output [translation_verse];

          // The text may contain new lines, so remove these,
          // because the NMT training files should not contain new lines mid-text,
          // as that would cause misalignments in the two text files used for training.
          translation_text = filter::strings::replace ("\n", " ", translation_text);
        }

        // If any of the two texts contains nothing, skip everything.
        if (reference_text.empty ()) continue;
        if (translation_text.empty ()) continue;

        // Split the texts set on matching punctuation markers.
        std::vector <std::string> reference_bits, translating_bits;
        nmt_logic_split (reference_text, translation_text, reference_bits, translating_bits);

        // Store the texts set.
        for (auto s : reference_bits) reference_lines.push_back (s);
        for (auto s : translating_bits) translation_lines.push_back (s);
      }
    }
  }

  std::string reference_text = filter::strings::implode (reference_lines, "\n");
  std::string translation_text = filter::strings::implode (translation_lines, "\n");
  std::string reference_path = filter_url_create_root_path ({filter_url_temp_dir (), "reference_bible_nmt_training_text.txt"});
  std::string translation_path = filter_url_create_root_path ({filter_url_temp_dir (), "translation_bible_nmt_training_text.txt"});
  filter_url_file_put_contents (reference_path, reference_text);
  filter_url_file_put_contents (translation_path, translation_text);
  Database_Logs::log ("The text of the reference Bible was exported to ", reference_path);
  Database_Logs::log ("The text of the Bible being translated was exported to ", translation_path);
  Database_Logs::log ("Ready exporting for neural machine translation training");
}


void nmt_logic_split (std::string reference_text, std::string translating_text,
                      std::vector <std::string> & reference_bits, std::vector <std::string> & translating_bits)
{
  // Define the punctuation marks to split the texts on.
  // The largest sets of punctuation will be tried first,
  // and then the smaller sets.
  // The first set of punctuation that leads to equally split texts is taken.
  std::vector <std::string> punctuations = { ".!?:;", ".!?:", ".!?", ".!?:;,", ".!?:,", ".!?," };
  for (auto punctuation : punctuations) {
    reference_bits = filter::strings::explode (reference_text, punctuation);
    translating_bits = filter::strings::explode (translating_text, punctuation);
    if (reference_bits.size() > 1) {
      if (reference_bits.size() == translating_bits.size()) {
        for (auto & s : reference_bits) {
          s = filter::strings::trim (s);
        }
        for (auto & s : translating_bits) {
          s = filter::strings::trim (s);
        }
        return;
      }
    }
  }

  // If the text sets could not be split equally,
  // take the original text sets unsplit.
  reference_bits.clear ();
  translating_bits.clear ();
  reference_bits.push_back (reference_text);
  translating_bits.push_back (translating_text);
}

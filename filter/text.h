/*
Copyright (©) 2003-2023 Teus Benschop.

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


#pragma once


#include <config/libraries.h>
#include <database/styles.h>
#include <odf/text.h>
#include <olb/text.h>
#include <html/text.h>
#include <text/text.h>
#include <esword/text.h>
#include <tbsx/text.h>
#include <filter/note.h>


namespace filter::text {

class passage_marker_value
{
public:
  passage_marker_value (int book, int chapter, std::string verse, std::string marker, std::string value);
  int m_book {0};
  int m_chapter {0};
  std::string m_verse {};
  std::string m_marker {};
  std::string m_value {};
};

}


class Filter_Text
{
public:
  Filter_Text (std::string bible);
  ~Filter_Text ();
  Filter_Text(const Filter_Text&) = delete;
  Filter_Text operator=(const Filter_Text&) = delete;
  
private:
  std::string m_bible {};

public:
  void add_usfm_code (std::string usfm);
private:
  // Container holding USFM, alternating between markup and text.
  std::vector <std::string> m_usfm_markers_and_text {};
  unsigned int usfm_markers_and_text_ptr {0};
  bool unprocessed_usfm_code_available ();
  void get_usfm_next_chapter ();
  
public:
  void run (std::string stylesheet);
private:
  // Container holding a chapter of USFM code, alternating between USFM and text.
  std::vector <std::string> chapter_usfm_markers_and_text {};
  unsigned int chapter_usfm_markers_and_text_pointer {0};

public:
  void get_styles (std::string stylesheet);
private:
  // A map of marker -> object with style information.
  std::map <std::string, Database_Styles_Item> styles {};
  // Usually this is: c
  std::string chapterMarker {};
  // Array holding styles created in Odf_Text class.
  std::vector <std::string> createdStyles {};

public:
  void pre_process_usfm ();
private:
  // Book identifier, e.g. 1, 2, 3, and so on.
  int m_current_book_identifier {0};
  // Chapter number, e.g. 1, 2, 3, etc.
  int m_current_chapter_number {0};
  // Verse number, e.g. "0", "1", "2", and so on.
  std::string currentVerseNumber {};
  std::string getCurrentPassageText ();
  // Map of (book, chapter number).
  std::map <int, int> numberOfChaptersPerBook {};
  void process_usfm ();
  void processNote ();
  // Opening a new paragraph.
  void create_paragraph_style (const Database_Styles_Item & style, bool keepWithNext);
  void new_paragraph (const Database_Styles_Item & style, bool keepWithNext);
  void applyDropCapsToCurrentParagraph (int dropCapsLength);
  void putChapterNumberInFrame (std::string chapterText);
  std::string getNoteCitation (const Database_Styles_Item & style);
  void ensureNoteParagraphStyle (std::string marker, const Database_Styles_Item & style);

public:
  // Container with objects (book, chapter, verse, marker, header value).
  std::vector <filter::text::passage_marker_value> runningHeaders {};
  // Container with objects (book, chapter, verse, marker, TOC value).
  std::vector <filter::text::passage_marker_value> longTOCs {};
  // Container with objects (book, chapter, verse, marker, TOC value).
  std::vector <filter::text::passage_marker_value> shortTOCs {};
  // Container with objects (book, chapter, verse, marker, abbreviation value).
  std::vector <filter::text::passage_marker_value> bookAbbreviations {};

public:
  // Vector with objects (book, chapter, verse, marker, label value).
  std::vector <filter::text::passage_marker_value> chapterLabels {};
  // Vector with object (book, chapter, verse, marker, marker value).
  std::vector <filter::text::passage_marker_value> publishedChapterMarkers {};
  // Vector with object (book, chapter, verse, marker, marker value).
  std::vector <filter::text::passage_marker_value> publishedVerseMarkers {};
private:
  // std::string holding the chapter number or text to output at the first verse.
  std::string output_chapter_text_at_first_verse {};

public:
  // Object for creating OpenDocument with text in standard form.
  odf_text * odf_text_standard {nullptr};
  // Object for creating OpenDocument with text only.
  odf_text * odf_text_text_only {nullptr};
  // Object for creating OpenDocument with text and note citations.
  odf_text * odf_text_text_and_note_citations {nullptr};
  // Object for creating OpenDocument with the notes only.
  odf_text * odf_text_notes {nullptr};

public:
  void produceInfoDocument (std::string path);
  void produceFalloutDocument (std::string path);
  std::vector <std::string> info {};
  std::vector <std::string> fallout {};
private:
  void addToInfo (std::string text, bool next = false);
  void addToFallout (std::string text, bool next = false);
  void addToWordList (std::vector <std::string> & list);
  std::vector <std::string> wordListGlossaryDictionary {};
  std::vector <std::string> hebrewWordList {};
  std::vector <std::string> greekWordList {};
  std::vector <std::string> subjectIndex {};

private:
  // Information for the citations for the notes.
  filter::note::citations note_citations {};

  std::string standardContentMarkerFootEndNote {};
  std::string standardContentMarkerCrossReference {};

public:
  // Object for creating standard web documents.
  Html_Text * html_text_standard {nullptr};
  // Object for creating interlinked web documents.
  Html_Text * html_text_linked {nullptr};

public:
  // Object for creating the input file for the Online Bible compiler.
  OnlineBible_Text * onlinebible_text {nullptr};

public:
  // Object for creating the Bible module for eSword.
  Esword_Text * esword_text {nullptr};

public:
  // Object for exporting to plain text.
  Text_Text * text_text { nullptr };

public:
  // Object for exporting to TBS online bible format.
  Tbsx_Text * tbsx_text { nullptr };
  
public:
  void initializeHeadingsAndTextPerVerse (bool start_text_now);
  std::map <int, std::string> getVersesText ();
  // Vector with objects to hold verse numbers and the text of the headings.
  std::map <int, std::string> verses_headings {};
  // Markers that started the above paragraph start positions.
  std::vector <std::string> paragraph_starting_markers {};
  // Complete paragraphs keyed to verse numbers.
  std::vector <std::map <int, std::string>> verses_paragraphs {};
private:
  // Flags for headings per verse processor.
  bool headings_text_per_verse_active { false };
  bool heading_started  { false };
  // Holds verse numbers and the plain text in that verse, without anything extra.
  std::map <int, std::string> verses_text {};
  // Flag for text per verse processor.
  bool text_started {false};
  void store_verses_paragraphs ();
  std::map <int, std::string> actual_verses_paragraph {};
  
private:
  std::string space_type_after_verse {}; // The type of space to follow a verse number.
  std::map <int, bool> book_has_chapter_label {}; // Whether \cl was found in the book.
  
public:
  // The notes plain text.
  std::vector <std::pair <std::string, std::string> > notes_plain_text {};
  // Holds the positions of the notes in the plain text, keyed to the verse numbers.
  std::map <int, std::vector <int>> verses_text_note_positions {};
private:
  // Flag to keep track of open note.
  bool note_open_now { false };
  // The joined fragments.
  std::string notes_plain_text_buffer {};
  // Handler.
  void notes_plain_text_handler ();
  
public:
  // The images that have been encountered to use.
  std::vector <std::string> image_sources {};
private:
  // Flag for whether the processor is now within figure markup.
  bool is_within_figure_markup { false };
  std::string figure_marker {};
  
public:
private:
  // Flag for whether to left-align certain poetry styles
  // in exports to OpenDocument format.
  bool odt_left_align_verse_in_poetry_styles { false };
  
};

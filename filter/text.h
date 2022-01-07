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


#pragma once


#include <string>
#include <vector>
#include <map>
using namespace std;
#include <database/styles.h>
#include <odf/text.h>
#include <olb/text.h>
#include <html/text.h>
#include <text/text.h>
#include <esword/text.h>
#include <tbsx/text.h>


class Filter_Text_Passage_Marker_Value
{
public:
  Filter_Text_Passage_Marker_Value (int book_in, int chapter_in, string verse_in, string marker_in, string value_in);
  int book;
  int chapter;
  string verse;
  string marker;
  string value;
};


class Filter_Text_Note_Citation
{
public:
  Filter_Text_Note_Citation ();
  Filter_Text_Note_Citation (vector <string> sequence_in, string restart_in);
  vector <string> sequence;
  string restart;
  unsigned int pointer;
};


class Filter_Text
{
public:
  Filter_Text (string bible_in);
  ~Filter_Text ();

private:
  string bible;

public:
  void add_usfm_code (string usfm);
private:
  // Container holding USFM, alternating between markup and text.
  vector <string> usfmMarkersAndText;
  unsigned int usfmMarkersAndTextPointer;
  bool unprocessedUsfmCodeAvailable ();
  void getUsfmNextChapter ();
  
public:
  void run (string stylesheet);
private:
  // Container holding a chapter of USFM code, alternating between USFM and text.
  vector <string> chapter_usfm_markers_and_text;
  unsigned int chapter_usfm_markers_and_text_pointer;

public:
  void getStyles (string stylesheet);
private:
  // A map of marker -> object with style information.
  map <string, Database_Styles_Item> styles;
  // Usually this is: c
  string chapterMarker;
  // Array holding styles created in Odf_Text class.
  vector <string> createdStyles;

public:
  void pre_process_usfm ();
private:
  // Book identifier, e.g. 1, 2, 3, and so on.
  int currentBookIdentifier;
  // Chapter number, e.g. 1, 2, 3, etc.
  int currentChapterNumber;
  // Verse number, e.g. "0", "1", "2", and so on.
  string currentVerseNumber;
  string getCurrentPassageText ();
  // Map of (book, chapter number).
  map <int, int> numberOfChaptersPerBook;
  void process_usfm ();
  void processNote ();
  // Opening a new paragraph.
  void new_paragraph (Database_Styles_Item style, bool keepWithNext);
  void applyDropCapsToCurrentParagraph (int dropCapsLength);
  void putChapterNumberInFrame (string chapterText);
  void createNoteCitation (Database_Styles_Item style);
  string getNoteCitation (Database_Styles_Item style);
  void resetNoteCitations (string moment);
  void ensureNoteParagraphStyle (string marker, Database_Styles_Item style);

public:
  // Container with objects (book, chapter, verse, marker, header value).
  vector <Filter_Text_Passage_Marker_Value> runningHeaders;
  // Container with objects (book, chapter, verse, marker, TOC value).
  vector <Filter_Text_Passage_Marker_Value> longTOCs;
  // Container with objects (book, chapter, verse, marker, TOC value).
  vector <Filter_Text_Passage_Marker_Value> shortTOCs;
  // Container with objects (book, chapter, verse, marker, abbreviation value).
  vector <Filter_Text_Passage_Marker_Value> bookAbbreviations;

public:
  // Vector with objects (book, chapter, verse, marker, label value).
  vector <Filter_Text_Passage_Marker_Value> chapterLabels;
  // Vector with object (book, chapter, verse, marker, marker value).
  vector <Filter_Text_Passage_Marker_Value> publishedChapterMarkers;
  // Vector with object (book, chapter, verse, marker, marker value).
  vector <Filter_Text_Passage_Marker_Value> publishedVerseMarkers;
private:
  // String holding the chapter number or text to output at the first verse.
  string output_chapter_text_at_first_verse;

public:
  // Object for creating OpenDocument with text in standard form.
  Odf_Text * odf_text_standard;
  // Object for creating OpenDocument with text only.
  Odf_Text * odf_text_text_only;
  // Object for creating OpenDocument with text and note citations.
  Odf_Text * odf_text_text_and_note_citations;
  // Object for creating OpenDocument with the notes only.
  Odf_Text * odf_text_notes;

public:
  void produceInfoDocument (string path);
  void produceFalloutDocument (string path);
  vector <string> info;
  vector <string> fallout;
private:
  void addToInfo (string text, bool next = false);
  void addToFallout (string text, bool next = false);
  void addToWordList (vector <string>  & list);
  vector <string> wordListGlossaryDictionary;
  vector <string> hebrewWordList;
  vector <string> greekWordList;
  vector <string> subjectIndex;

private:
  map <string, Filter_Text_Note_Citation> notecitations; // Information for the citations for the notes.
  string standardContentMarkerFootEndNote;
  string standardContentMarkerCrossReference;

public:
  // Object for creating standard web documents.
  Html_Text * html_text_standard;
  // Object for creating interlinked web documents.
  Html_Text * html_text_linked;

public:
  // Object for creating the input file for the Online Bible compiler.
  OnlineBible_Text * onlinebible_text;

public:
  // Object for creating the Bible module for eSword.
  Esword_Text * esword_text;

public:
  // Object for exporting to plain text.
  Text_Text * text_text;

public:
  // Object for exporting to TBS online bible format.
  Tbsx_Text * tbsx_text;
  
public:
  void initializeHeadingsAndTextPerVerse (bool start_text_now);
  map <int, string> getVersesText ();
  // Vector with objects to hold verse numbers and the text of the headings.
  map <int, string> verses_headings;
  // Markers that started the above paragraph start positions.
  vector <string> paragraph_starting_markers;
  // Complete paragraphs keyed to verse numbers.
  vector <map <int, string>> verses_paragraphs;
private:
  // Flags for headings per verse processor.
  bool headings_text_per_verse_active;
  bool heading_started;
  // Holds verse numbers and the plain text in that verse, without anything extra.
  map <int, string> verses_text;
  // Flag for text per verse processor.
  bool text_started;
  void storeVersesParagraphs ();
  map <int, string> actual_verses_paragraph;

  
private:
  string space_type_after_verse; // The type of space to follow a verse number.
  map <int, bool> book_has_chapter_label; // Whether \cl was found in the book.
  
public:
  // The notes plain text.
  vector <pair <string, string> > notes_plain_text;
  // Holds the positions of the notes in the plain text, keyed to the verse numbers.
  map <int, vector <int>> verses_text_note_positions;
private:
  // Flag to keep track of open note.
  bool note_open_now = false;
  // The joined fragments.
  string notes_plain_text_buffer;
  // Handler.
  void notes_plain_text_handler ();
  
public:
  // The images that have been encountered to use.
  vector <string> image_sources;
private:
  // Flag for whether the processor is now within figure markup.
  bool is_within_figure_markup = false;
  
public:
private:
  // Flag for whether to left-align certain poetry styles
  // in exports to OpenDocument format.
  bool odt_left_align_verse_in_poetry_styles = false;
  
};

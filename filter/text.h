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


#ifndef INCLUDED_FILTER_TEXT_H
#define INCLUDED_FILTER_TEXT_H


#include <config/libraries.h>
#include <database/styles.h>
#include <odf/text.h>
#include <olb/text.h>
#include <html/text.h>
#include <text/text.h>
#include <esword/text.h>


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
  void addUsfmCode (string usfm);
private:
  vector <string> usfmMarkersAndText; // Vector holding USFM, alternating between markup and text.
  unsigned int usfmMarkersAndTextPointer;
  bool unprocessedUsfmCodeAvailable ();
  void getUsfmNextChapter ();
  
public:
  void run (string stylesheet);
private:
  vector <string> chapterUsfmMarkersAndText; // Vector holding a chapter of USFM code, alternating between USFM and text.
  unsigned int chapterUsfmMarkersAndTextPointer;

public:
  void getStyles (string stylesheet);
private:
  map <string, Database_Styles_Item> styles; // A map of marker -> object with style information.
  string chapterMarker; // Usually this is: c
  vector <string> createdStyles; // Array holding styles created in Odf_Text class.

public:
  void preprocessingStage ();
private:
  int currentBookIdentifier; // Book identifier, e.g. 1, 2, 3, and so on.
  int currentChapterNumber; // Chapter number, e.g. 1, 2, 3, etc.
  string currentVerseNumber; // Verse number, e.g. "0", "1", "2", and so on.
  string getCurrentPassageText ();
  map <int, int> numberOfChaptersPerBook; // Map of (book, chapter number).
  void processUsfm ();
  void processNote ();
  void newParagraph (Database_Styles_Item style, bool keepWithNext);
  void applyDropCapsToCurrentParagraph (int dropCapsLength);
  void putChapterNumberInFrame (string chapterText);
  void createNoteCitation (Database_Styles_Item style);
  string getNoteCitation (Database_Styles_Item style);
  void resetNoteCitations (string moment);
  void ensureNoteParagraphStyle (string marker, Database_Styles_Item style);

public:
  vector <Filter_Text_Passage_Marker_Value> runningHeaders; // Vector with objects (book, chapter, verse, marker, header value).
  vector <Filter_Text_Passage_Marker_Value> longTOCs; // Vector with objects (book, chapter, verse, marker, TOC value).
  vector <Filter_Text_Passage_Marker_Value> shortTOCs; // Vector with objects (book, chapter, verse, marker, TOC value).
  vector <Filter_Text_Passage_Marker_Value> bookAbbreviations; // Vector with objects (book, chapter, verse, marker, abbreviation value).

public:
  vector <Filter_Text_Passage_Marker_Value> chapterLabels; // Vector with objects (book, chapter, verse, marker, label value).
  vector <Filter_Text_Passage_Marker_Value> publishedChapterMarkers; // Vector with object (book, chapter, verse, marker, marker value).
private:
  string outputChapterTextAtFirstVerse; // String holding the chapter number or text to output at the first verse.

public:
  Odf_Text * odf_text_standard; // Object for creating OpenDocument with text in standard form.
  Odf_Text * odf_text_text_only; // Object for creating OpenDocument with text only.
  Odf_Text * odf_text_text_and_note_citations; // Object for creating OpenDocument with text and note citations.
  Odf_Text * odf_text_notes; // Object for creating OpenDocument with the notes only.

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
  Html_Text * html_text_standard; // Object for creating standard web documents.
  Html_Text * html_text_linked; // Object for creating interlinked web documents.

public:
  OnlineBible_Text * onlinebible_text; // Object for creating the input file for the Online Bible compiler.

public:
  Esword_Text * esword_text; // Object for creating the Bible module for eSword.

public:
  Text_Text * text_text; // Object for exporting to plain text.

public:
  void initializeHeadingsAndTextPerVerse (bool start_text_now);
  map <int, string> getVersesText ();
  map <int, string> verses_headings; // Vector with objects to hold verse numbers and the text of the headings.
  vector <string> paragraph_starting_markers; // Markers that started the above paragraph start positions.
  vector <map <int, string>> verses_paragraphs; // Complete paragraphs keyed to verse numbers.
private:
  bool headings_text_per_verse_active;
  bool heading_started; // Flag for headings per verse processor.
  map <int, string> verses_text; // Holds verse numbers and the plain text in that verse, without anything extra.
  bool text_started; // Flag for text per verse processor.
  void storeVersesParagraphs ();
  map <int, string> actual_verses_paragraph;
  
private:
  string space_type_after_verse; // The type of space to follow a verse number.
  map <int, bool> book_has_chapter_label; // Whether \cl was found in the book.
};


#endif

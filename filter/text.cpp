/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <filter/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <filter/passage.h>
#include <styles/logic.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <locale/translate.h>


Filter_Text_Passage_Marker_Value::Filter_Text_Passage_Marker_Value (int book_in, int chapter_in, string verse_in, string marker_in, string value_in)
{
  book = book_in;
  chapter = chapter_in;
  verse = verse_in;
  marker = marker_in;
  value = value_in;
}


Filter_Text_Note_Citation::Filter_Text_Note_Citation ()
{
  pointer = 0;
}


Filter_Text_Note_Citation::Filter_Text_Note_Citation (vector <string> sequence_in, string restart_in)
{
  sequence = sequence_in;
  restart = restart_in;
  pointer = 0;
}



// This class filters USFM text, converting it into other formats.



Filter_Text::Filter_Text (string bible_in)
{
  bible = bible_in;
  currentBookIdentifier = 0;
  currentChapterNumber = 0;
  heading_started = false;
  text_started = false;
  odf_text_standard = nullptr;
  odf_text_text_only = nullptr;
  odf_text_text_and_note_citations = nullptr;
  odf_text_notes = nullptr;
  html_text_standard = nullptr;
  html_text_linked = nullptr;
  onlinebible_text = nullptr;
  esword_text = nullptr;
  text_text = nullptr;
  headings_text_per_verse_active = false;
  space_type_after_verse = Database_Config_Bible::getOdtSpaceAfterVerse (bible);
}



Filter_Text::~Filter_Text ()
{
  if (odf_text_standard) delete odf_text_standard;
  if (odf_text_text_only) delete odf_text_text_only;
  if (odf_text_text_and_note_citations) delete odf_text_text_and_note_citations;
  if (odf_text_notes) delete odf_text_notes;
  if (html_text_standard) delete html_text_standard;
  if (html_text_linked) delete html_text_linked;
  if (onlinebible_text) delete onlinebible_text;
  if (esword_text) delete esword_text;
  if (text_text) delete text_text;
}



// This function adds USFM code to the class.
// $code: USFM code.
void Filter_Text::addUsfmCode (string usfm)
{
  // Clean the USFM.
  usfm = filter_string_trim (usfm);
  usfm += "\n";
  // Sort the USFM code out and separate it into markers and text.
  vector <string> markersAndText = usfm_get_markers_and_text (usfm);
  // Add the USFM to the object.
  usfmMarkersAndText.insert (usfmMarkersAndText.end(), markersAndText.begin(), markersAndText.end());
}



// This function runs the filter.
// $stylesheet - The stylesheet to use.
void Filter_Text::run (string stylesheet)
{
  // Get the styles.
  getStyles (stylesheet);

  // Preprocess.
  preprocessingStage ();

  // Process data.
  processUsfm ();

  storeVersesParagraphs ();
  
  // Clear USFM and styles.
  usfmMarkersAndText.clear();
  usfmMarkersAndTextPointer = 0;
  chapterUsfmMarkersAndText.clear();
  chapterUsfmMarkersAndTextPointer = 0;
  styles.clear();
  chapterMarker.clear();
  createdStyles.clear();
}



// This function return true when there is still unprocessed USFM code available.
bool Filter_Text::unprocessedUsfmCodeAvailable ()
{
  return (usfmMarkersAndTextPointer < usfmMarkersAndText.size());
}



// This function stores data in the class:
// The next chapter from the unprocessed USFM code.
void Filter_Text::getUsfmNextChapter ()
{
  // Initialization.
  chapterUsfmMarkersAndText.clear();
  chapterUsfmMarkersAndTextPointer = 0;
  bool firstLine = true;

  // Obtain the standard marker for the chapter number.
  // Deal with the unlikely case that the chapter marker is non-standard.
  if (chapterMarker.empty()) {
    chapterMarker = "c";
    for (const auto & style : styles) {
      if (style.second.type == StyleTypeChapterNumber) {
        chapterMarker = style.second.marker;
        break;
      }
    }
  }

  // Load the USFM code till the next chapter marker.
  while (unprocessedUsfmCodeAvailable ()) {
    string item = usfmMarkersAndText [usfmMarkersAndTextPointer];
    if (!firstLine) {
      if (filter_string_trim (item) == ("\\" + chapterMarker)) {
        return;
      }
    }
    chapterUsfmMarkersAndText.push_back (item);
    firstLine = false;
    usfmMarkersAndTextPointer++;
  }
}



// This function gets the styles from the database,
// and stores them in the object for quicker access.
void Filter_Text::getStyles (string stylesheet)
{
  styles.clear();
  // Get the relevant styles information included.
  if (odf_text_standard) odf_text_standard->createPageBreakStyle ();
  if (odf_text_text_only) odf_text_text_only->createPageBreakStyle ();
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->createPageBreakStyle ();
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->createSuperscriptStyle ();
  Database_Styles database_styles;
  vector <string> markers = database_styles.getMarkers (stylesheet);
  for (auto marker : markers) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    styles [marker] = style;
    if (style.type == StyleTypeFootEndNote) {
      if (style.subtype == FootEndNoteSubtypeStandardContent) {
        standardContentMarkerFootEndNote = style.marker;
      }
    }
    if (style.type == StyleTypeCrossreference) {
      if (style.subtype == CrossreferenceSubtypeStandardContent) {
        standardContentMarkerCrossReference = style.marker;
      }
    }
  }
}



// This function does the preprocessing of the USFM code
// extracting a variety of information, creating note citations, etc.
void Filter_Text::preprocessingStage ()
{
  usfmMarkersAndTextPointer = 0;
  while (unprocessedUsfmCodeAvailable ()) {
    getUsfmNextChapter ();
    for (chapterUsfmMarkersAndTextPointer = 0; chapterUsfmMarkersAndTextPointer < chapterUsfmMarkersAndText.size(); chapterUsfmMarkersAndTextPointer++) {
      string currentItem = chapterUsfmMarkersAndText[chapterUsfmMarkersAndTextPointer];
      if (usfm_is_usfm_marker (currentItem)) {
        string marker = filter_string_trim (currentItem); // Change, e.g. '\id ' to '\id'.
        marker = marker.substr (1); // Remove the initial backslash, e.g. '\id' becomes 'id'.
        if (usfm_is_opening_marker (marker)) {
          if (styles.find (marker) != styles.end()) {
            Database_Styles_Item style = styles [marker];
            switch (style.type) {
              case StyleTypeIdentifier:
                switch (style.subtype) {
                  case IdentifierSubtypeBook:
                  {
                    // Get book number.
                    string s = usfm_get_book_identifier (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    s = filter_string_str_replace (soft_hyphen_u00AD (), "", s); // Remove possible soft hyphen.
                    currentBookIdentifier = Database_Books::getIdFromUsfm (s);
                    // Reset chapter and verse numbers.
                    currentChapterNumber = 0;
                    numberOfChaptersPerBook[currentBookIdentifier] = 0;
                    currentVerseNumber = "0";
                    // Done.
                    break;
                  }
                  case IdentifierSubtypeRunningHeader:
                  {
                    string runningHeader = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    runningHeaders.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, runningHeader));
                    break;
                  }
                  case IdentifierSubtypeLongTOC:
                  {
                    string longTOC = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    longTOCs.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, longTOC));
                    break;
                  }
                  case IdentifierSubtypeShortTOC:
                  {
                    string shortTOC = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    shortTOCs.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, shortTOC));
                    break;
                  }
                  case IdentifierSubtypeBookAbbrev:
                  {
                    string bookAbbreviation = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    bookAbbreviations.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, bookAbbreviation));
                    break;
                  }
                  case IdentifierSubtypeChapterLabel:
                  {
                    // Store the chapter label for this book and chapter.
                    string chapterLabel = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    chapterLabels.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, chapterLabel));
                    // If a chapter label is in the book, there's no drop caps output of the chapter number.
                    book_has_chapter_label [currentBookIdentifier] = true;
                    // Done.
                    break;
                  }
                  case IdentifierSubtypePublishedChapterMarker:
                  {
                    string publishedChapterMarker = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    publishedChapterMarkers.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, publishedChapterMarker));
                    break;
                  }
                  case IdentifierSubtypePublishedVerseMarker:
                  {
                    // It gets the published verse markup.
                    // The marker looks like: ... \vp ၁။\vp* ...
                    // It stores this markup in the object for later reference.
                    string publishedVerseMarker = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                    publishedVerseMarkers.push_back (Filter_Text_Passage_Marker_Value (currentBookIdentifier, currentChapterNumber, currentVerseNumber, marker, publishedVerseMarker));
                    break;
                  }
                }
                break;
              case StyleTypeChapterNumber:
              {
                string number = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                int inumber = convert_to_int (number);
                currentChapterNumber = inumber;
                numberOfChaptersPerBook[currentBookIdentifier] = inumber;
                currentVerseNumber = "0";
                break;
              }
              case StyleTypeVerseNumber:
              {
                string number = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                int inumber = convert_to_int (number);
                currentVerseNumber = convert_to_string (inumber);
                break;
              }
              case StyleTypeFootEndNote:
              {
                switch (style.subtype)
                {
                  case FootEndNoteSubtypeFootnote:
                  case FootEndNoteSubtypeEndnote:
                  {
                    createNoteCitation (style);
                    break;
                  }
                  case FootEndNoteSubtypeStandardContent:
                  case FootEndNoteSubtypeContent:
                  case FootEndNoteSubtypeContentWithEndmarker:
                  case FootEndNoteSubtypeParagraph:
                  {
                    break;
                  }
                }
                break;
              }
              case StyleTypeCrossreference:
              {
                switch (style.subtype)
                {
                  case CrossreferenceSubtypeCrossreference:
                  {
                    createNoteCitation (style);
                    break;
                  }
                  case CrossreferenceSubtypeStandardContent:
                  case CrossreferenceSubtypeContent:
                  case CrossreferenceSubtypeContentWithEndmarker:
                  {
                    break;
                  }
                }
                break;
              }
            }
          }
        }
      }
    }
  }
}



// This function does the processing of the USFM code,
// formatting the document and extracting other useful information.
void Filter_Text::processUsfm ()
{
  // Go through the USFM code.
  int processedBooksCount = 0;
  usfmMarkersAndTextPointer = 0;
  while (unprocessedUsfmCodeAvailable ()) {
    getUsfmNextChapter ();
    for (chapterUsfmMarkersAndTextPointer = 0; chapterUsfmMarkersAndTextPointer < chapterUsfmMarkersAndText.size(); chapterUsfmMarkersAndTextPointer++) {
      string currentItem = chapterUsfmMarkersAndText [chapterUsfmMarkersAndTextPointer];
      if (usfm_is_usfm_marker (currentItem))
      {
        // Indicator describing the marker.
        bool isOpeningMarker = usfm_is_opening_marker (currentItem);
        bool isEmbeddedMarker = usfm_is_embedded_marker (currentItem);
        // Clean up the marker, so we remain with the basic version, e.g. 'id'.
        string marker = usfm_get_marker (currentItem);
        if (styles.find (marker) != styles.end())
        {
          Database_Styles_Item style = styles [marker];
          switch (style.type)
          {
            case StyleTypeIdentifier:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              switch (style.subtype)
              {
                case IdentifierSubtypeBook:
                {
                  // Get book number.
                  string s = usfm_get_book_identifier (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  s = filter_string_str_replace (soft_hyphen_u00AD (), "", s); // Remove possible soft hyphen.
                  currentBookIdentifier = Database_Books::getIdFromUsfm (s);
                  // Reset chapter and verse numbers.
                  currentChapterNumber = 0;
                  currentVerseNumber = "0";
                  // Throw away whatever follows the \id, e.g. 'GEN xxx xxx'.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  // Whether to insert a new page before the book. But never before the first book.
                  if (style.userbool1) {
                    if (processedBooksCount) {
                      if (odf_text_standard) odf_text_standard->newPageBreak ();
                      if (odf_text_text_only) odf_text_text_only->newPageBreak ();
                      if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->newPageBreak ();
                      if (html_text_standard) html_text_standard->newPageBreak ();
                      if (html_text_linked) html_text_linked->newPageBreak ();
                    }
                  }
                  processedBooksCount++;
                  // Reset notes.
                  resetNoteCitations ("book");
                  // Online Bible.
                  if (onlinebible_text) onlinebible_text->storeData ();
                  // eSword.
                  if (esword_text) esword_text->newBook (currentBookIdentifier);
                  // The hidden header in the text normally displays in the running header.
                  // It does this only when it's the first header on the page.
                  // The book starts here.
                  // So create a correct hidden header for displaying in the running header.
                  string runningHeader = Database_Books::getEnglishFromId (currentBookIdentifier);
                  for (auto item : runningHeaders) {
                    if (item.book == currentBookIdentifier) {
                      runningHeader = item.value;
                    }
                  }
                  if (odf_text_standard) odf_text_standard->newHeading1 (runningHeader, true);
                  if (odf_text_text_only) odf_text_text_only->newHeading1 (runningHeader, true);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->newHeading1 (runningHeader, true);
                  if (odf_text_notes) odf_text_notes->newHeading1 (runningHeader, false);
                  // Done.
                  break;
                }
                case IdentifierSubtypeEncoding:
                {
                  addToInfo ("Text encoding: \\" + marker, true);
                  break;
                }
                case IdentifierSubtypeComment:
                {
                  addToInfo ("Comment: \\" + marker, true);
                  break;
                }
                case IdentifierSubtypeRunningHeader:
                {
                  // This information already went into the Info document during the preprocessing stage.
                  // Remove it from the USFM input stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  // Ideally this information should be inserted in the headers of the standard text document.
                  // UserBool2RunningHeaderLeft:
                  // UserBool3RunningHeaderRight:
                  break;
                }
                case IdentifierSubtypeLongTOC:
                {
                  // This information already went into the Info document. Remove it from the USFM stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  break;
                }
                case IdentifierSubtypeShortTOC:
                {
                  // This information already went into the Info document. Remove it from the USFM stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  break;
                }
                case IdentifierSubtypeBookAbbrev:
                {
                  // This information already went into the Info document. Remove it from the USFM stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  break;
                }
                case IdentifierSubtypeChapterLabel:
                {
                  // This information is already in the object. Remove it from the USFM stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  break;
                }
                case IdentifierSubtypePublishedChapterMarker:
                {
                  // This information is already in the object.
                  // Remove it from the USFM stream.
                  usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  break;
                }
                case IdentifierSubtypeCommentWithEndmarker:
                {
                  if (isOpeningMarker) {
                    addToInfo ("Comment: \\" + marker, true);
                  }
                  break;
                }
                case IdentifierSubtypePublishedVerseMarker:
                {
                  if (isOpeningMarker) {
                    // This information is already in the object.
                    // Remove it from the USFM stream at the opening marker.
                    usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
                  } else {
                    // USFM allows the closing marker \vp* to be followed by a space.
                    // But this space should not be converted to text output.
                    // https://github.com/bibledit/cloud/issues/311
                    // It is going to be removed here.
                    size_t pointer = chapterUsfmMarkersAndTextPointer + 1;
                    if (pointer < chapterUsfmMarkersAndText.size()) {
                      string text = chapterUsfmMarkersAndText[pointer];
                      text = filter_string_ltrim (text);
                      chapterUsfmMarkersAndText[pointer] = text;
                    }
                  }
                  break;
                }
                default:
                {
                  addToFallout ("Unknown markup: \\" + marker, true);
                  break;
                }
              }
              break;
            }
            case StyleTypeNotUsedComment:
            {
              addToFallout ("Unknown markup: \\" + marker, true);
              break;
            }
            case StyleTypeNotUsedRunningHeader:
            {
              addToFallout ("Unknown markup: \\" + marker, true);
              break;
            }
            case StyleTypeStartsParagraph:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              switch (style.subtype)
              {
                case ParagraphSubtypeMainTitle:
                case ParagraphSubtypeSubTitle:
                case ParagraphSubtypeSectionHeading:
                {
                  newParagraph (style, true);
                  heading_started = true;
                  text_started = false;
                  break;
                }
                case ParagraphSubtypeNormalParagraph:
                default:
                {
                  newParagraph (style, false);
                  heading_started = false;
                  text_started = true;
                  if (headings_text_per_verse_active) {
                    // If a new paragraph starts within an existing verse,
                    // add a space to the text already in that verse.
                    int iverse = convert_to_int (currentVerseNumber);
                    if (verses_text.count (iverse) && !verses_text [iverse].empty ()) {
                      verses_text [iverse].append (" ");
                    }
                    // Record the style that started this new paragraph.
                    paragraph_starting_markers.push_back (style.marker);
                    // Store previous paragraph, if any, and start recording the new one.
                    storeVersesParagraphs ();
                  }
                  break;
                }
              }
              break;
            }
            case StyleTypeInlineText:
            {
              // Support for a normal and an embedded character style.
              if (isOpeningMarker) {
                if (odf_text_standard) odf_text_standard->openTextStyle (style, false, isEmbeddedMarker);
                if (odf_text_text_only) odf_text_text_only->openTextStyle (style, false, isEmbeddedMarker);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->openTextStyle (style, false, isEmbeddedMarker);
                if (html_text_standard) html_text_standard->openTextStyle (style, false, isEmbeddedMarker);
                if (html_text_linked) html_text_linked->openTextStyle (style, false, isEmbeddedMarker);
              } else {
                if (odf_text_standard) odf_text_standard->closeTextStyle (false, isEmbeddedMarker);
                if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, isEmbeddedMarker);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, isEmbeddedMarker);
                if (html_text_standard) html_text_standard->closeTextStyle (false, isEmbeddedMarker);
                if (html_text_linked) html_text_linked->closeTextStyle (false, isEmbeddedMarker);
              }
              break;
            }
            case StyleTypeChapterNumber:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);

              if (onlinebible_text) onlinebible_text->storeData ();

              // Get the chapter number.
              string number = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
              int inumber = convert_to_int (number);

              // Update this object.
              currentChapterNumber = inumber;
              currentVerseNumber = "0";

              // If there is a published chapter character, the chapter number takes that value.
              for (auto publishedChapterMarker : publishedChapterMarkers) {
                if (publishedChapterMarker.book == currentBookIdentifier) {
                  if (publishedChapterMarker.chapter == currentChapterNumber) {
                    number = publishedChapterMarker.value;
                    inumber = convert_to_int (number);
                  }
                }
              }

              // Enter text for the running headers.
              string runningHeader = Database_Books::getEnglishFromId (currentBookIdentifier);
              for (auto item : runningHeaders) {
                if (item.book == currentBookIdentifier) {
                  runningHeader = item.value;
                }
              }
              runningHeader = runningHeader + " " + number;
              if (odf_text_standard) odf_text_standard->newHeading1 (runningHeader, true);
              if (odf_text_text_only) odf_text_text_only->newHeading1 (runningHeader, true);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->newHeading1 (runningHeader, true);
              if (odf_text_notes) odf_text_notes->newHeading1 (runningHeader, false);

              // This is the phase of outputting the chapter number in the text body.
              // It always outputs the chapter number to the clear text export.
              if (text_text) {
                text_text->paragraph (number);
              }
              // The chapter number is only output when there is more than one chapter in a book.
              if (numberOfChaptersPerBook [currentBookIdentifier] > 1) {
                // Putting the chapter number at the first verse is determined by the style of the \c marker.
                // But if a chapter label (\cl) is found in the current book, that disables the above.
                bool cl_found = book_has_chapter_label[currentBookIdentifier];
                if (style.userbool1 && !cl_found) {
                  // Output the chapter number at the first verse, not here.
                  // Store it for later processing.
                  outputChapterTextAtFirstVerse = number;
                } else {
                  // Output the chapter in a new paragraph.
                  // If the chapter label \cl is entered once before chapter 1 (\c 1)
                  // it represents the text for "chapter" to be used throughout the current book.
                  // If \cl is used after each individual chapter marker, it represents the particular text
                  // to be used for the display of the current chapter heading
                  // (usually done if numbers are being presented as words, not numerals).
                  string labelEntireBook = "";
                  string labelCurrentChapter = "";
                  for (auto pchapterLabel : chapterLabels) {
                    if (pchapterLabel.book == currentBookIdentifier) {
                      if (pchapterLabel.chapter == 0) {
                        labelEntireBook = pchapterLabel.value;
                      }
                      if (pchapterLabel.chapter == currentChapterNumber) {
                        labelCurrentChapter = pchapterLabel.value;
                      }
                    }
                  }
                  if (labelEntireBook != "") {
                    number = labelEntireBook + " " + number;
                  }
                  if (labelCurrentChapter != "") {
                    number = labelCurrentChapter;
                  }
                  // The chapter number shows in a new paragraph.
                  // Keep it together with the next paragraph.
                  newParagraph (style, true);
                  if (odf_text_standard) odf_text_standard->addText (number);
                  if (odf_text_text_only) odf_text_text_only->addText (number);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->addText (number);
                  if (html_text_standard) html_text_standard->addText (number);
                  if (html_text_linked) html_text_linked->addText (number);
                }
              }

              // Output chapter number for other formats.
              if (esword_text) esword_text->newChapter (currentChapterNumber);

              // Open a paragraph for the notes.
              // It takes the style of the footnote content marker, usually 'ft'.
              // This is done specifically for the version that has the notes only.
              ensureNoteParagraphStyle (standardContentMarkerFootEndNote, styles[standardContentMarkerFootEndNote]);
              if (odf_text_notes) odf_text_notes->newParagraph (standardContentMarkerFootEndNote);
              // UserBool2ChapterInLeftRunningHeader -> no headings implemented yet.
              // UserBool3ChapterInRightRunningHeader -> no headings implemented yet.

              // Reset.
              resetNoteCitations ("chapter");

              // Done.
              break;
            }
            case StyleTypeVerseNumber:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              if (onlinebible_text) onlinebible_text->storeData ();
              // Care for the situation that a new verse starts a new paragraph.
              if (style.userbool1) {
                if (odf_text_standard) {
                  if (odf_text_standard->currentParagraphContent != "") {
                    odf_text_standard->newParagraph (odf_text_standard->currentParagraphStyle);
                  }
                }
                if (odf_text_text_only) {
                  if (odf_text_text_only->currentParagraphContent != "") {
                    odf_text_text_only->newParagraph (odf_text_text_only->currentParagraphStyle);
                  }
                }
                if (odf_text_text_and_note_citations) {
                  if (odf_text_text_and_note_citations->currentParagraphContent != "") {
                    odf_text_text_and_note_citations->newParagraph (odf_text_text_and_note_citations->currentParagraphStyle);
                  }
                }
                if (html_text_standard) {
                  if (html_text_standard->currentParagraphContent != "") {
                    html_text_standard->newParagraph (html_text_standard->currentParagraphStyle);
                  }
                }
                if (html_text_linked) {
                  if (html_text_linked->currentParagraphContent != "") {
                    html_text_linked->newParagraph (html_text_linked->currentParagraphStyle);
                  }
                }
                if (text_text) {
                  text_text->paragraph ();
                }
              }
              // Deal with the case of a pending chapter number.
              if (outputChapterTextAtFirstVerse != "") {
                if (!Database_Config_Bible::getExportChapterDropCapsFrames (bible)) {
                  int dropCapsLength = unicode_string_length (outputChapterTextAtFirstVerse);
                  applyDropCapsToCurrentParagraph (dropCapsLength);
                  if (odf_text_standard) odf_text_standard->addText (outputChapterTextAtFirstVerse);
                  if (odf_text_text_only) odf_text_text_only->addText (outputChapterTextAtFirstVerse);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->addText (outputChapterTextAtFirstVerse);
                } else {
                  putChapterNumberInFrame (outputChapterTextAtFirstVerse);
                }
                Database_Styles_Item styleItem = Database_Styles_Item ();
                styleItem.marker = "dropcaps";
                if (html_text_standard) html_text_standard->openTextStyle (styleItem, false, false);
                if (html_text_standard) html_text_standard->addText (outputChapterTextAtFirstVerse);
                if (html_text_standard) html_text_standard->closeTextStyle (false, false);
                if (html_text_linked) html_text_linked->openTextStyle (styleItem, false, false);
                if (html_text_linked) html_text_linked->addText (outputChapterTextAtFirstVerse);
                if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              }
              // Temporarily retrieve the text that follows the \v verse marker.
              string textFollowingMarker = usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
              // Extract the verse number, and store it in the object.
              string v_number = usfm_peek_verse_number (textFollowingMarker);
              currentVerseNumber = v_number;
              // In case there was a published verse marker, use that markup for publishing.
              string v_vp_number = v_number;
              for (auto publishedVerseMarker : publishedVerseMarkers) {
                if (publishedVerseMarker.book == currentBookIdentifier) {
                  if (publishedVerseMarker.chapter == currentChapterNumber) {
                    if (publishedVerseMarker.verse == currentVerseNumber) {
                      v_vp_number = publishedVerseMarker.value;
                    }
                  }
                }
              }
              // Output the verse number. But only if no chapter number was put here.
              if (outputChapterTextAtFirstVerse.empty ()) {
                // If the current paragraph has text already, then insert a space.
                if (odf_text_standard) {
                  if (odf_text_standard->currentParagraphContent != "") {
                    odf_text_standard->addText (" ");
                  }
                }
                if (odf_text_text_only) {
                  if (odf_text_text_only->currentParagraphContent != "") {
                    odf_text_text_only->addText (" ");
                  }
                }
                if (odf_text_text_and_note_citations) {
                  if (odf_text_text_and_note_citations->currentParagraphContent != "") {
                    odf_text_text_and_note_citations->addText (" ");
                  }
                }
                if (html_text_standard) {
                  if (html_text_standard->currentParagraphContent != "") {
                    html_text_standard->addText (" ");
                  }
                }
                if (html_text_linked) {
                  if (html_text_linked->currentParagraphContent != "") {
                    html_text_linked->addText (" ");
                  }
                }
                if (odf_text_standard) odf_text_standard->openTextStyle (style, false, false);
                if (odf_text_text_only) odf_text_text_only->openTextStyle (style, false, false);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->openTextStyle (style, false, false);
                if (html_text_standard) html_text_standard->openTextStyle (style, false, false);
                if (html_text_linked) html_text_linked->openTextStyle (style, false, false);
                if (odf_text_standard) odf_text_standard->addText (v_vp_number);
                if (odf_text_text_only) odf_text_text_only->addText (v_vp_number);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->addText (v_vp_number);
                if (html_text_standard) html_text_standard->addText (v_vp_number);
                if (html_text_linked) html_text_linked->addText (v_vp_number);
                if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
                if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
                if (html_text_standard) html_text_standard->closeTextStyle (false, false);
                if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              }
              // Clear text output.
              if (text_text) {
                if (text_text->line () != "") {
                  text_text->addtext (" ");
                }
                text_text->addtext (v_vp_number);
                // Clear text output always has a space following the verse.
                // Important for outputting the first verse.
                text_text->addtext (" ");
              }
              // If there was any text following the \v marker, remove the verse number,
              // put the remainder back into the object, and update the pointer.
              if (textFollowingMarker != "") {
                size_t pos = textFollowingMarker.find (v_number);
                if (pos != string::npos) {
                  textFollowingMarker = textFollowingMarker.substr (pos + v_number.length ());
                }
                // If a chapter number was put, remove any whitespace from the start of the following text.
                // Remove whitespace from the start of the following text,
                // and replace it with the type of space that the user has set.
                // This could be a fixed-width space, or a non-breaking space, or a combination of the two.
                // These types of spaces make the layout of the text following the verse number look tidier.
                // But if a chapter number was put, than do not put any space at the start of the following verse.
                textFollowingMarker = filter_string_ltrim (textFollowingMarker);
                if (outputChapterTextAtFirstVerse.empty()) {
                  textFollowingMarker = space_type_after_verse + textFollowingMarker;
                }
                chapterUsfmMarkersAndText [chapterUsfmMarkersAndTextPointer] = textFollowingMarker;
                chapterUsfmMarkersAndTextPointer--;
              }
              // Unset the chapter variable, whether it was used or not.
              // This makes it ready for subsequent use.
              outputChapterTextAtFirstVerse.clear();
              // Other export formats.
              if (onlinebible_text) onlinebible_text->newVerse (currentBookIdentifier, currentChapterNumber, convert_to_int (currentVerseNumber));
              if (esword_text) esword_text->newVerse (convert_to_int (currentVerseNumber));
              // Done.
              break;
            }
            case StyleTypeFootEndNote:
            {
              processNote ();
              break;
            }
            case StyleTypeCrossreference:
            {
              processNote ();
              break;
            }
            case StyleTypePeripheral:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              switch (style.subtype)
              {
                case PeripheralSubtypePublication:
                case PeripheralSubtypeTableOfContents:
                case PeripheralSubtypePreface:
                case PeripheralSubtypeIntroduction:
                case PeripheralSubtypeGlossary:
                case PeripheralSubtypeConcordance:
                case PeripheralSubtypeIndex:
                case PeripheralSubtypeMapIndex:
                case PeripheralSubtypeCover:
                case PeripheralSubtypeSpine:
                default:
                {
                  addToFallout ("Unknown pheripheral marker \\" + marker, false);
                  break;
                }
              }
              break;
            }
            case StyleTypePicture:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              addToFallout ("Picture formatting not implemented", true);
              break;
            }
            case StyleTypePageBreak:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              if (odf_text_standard) odf_text_standard->newPageBreak ();
              if (odf_text_text_only) odf_text_text_only->newPageBreak ();
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->newPageBreak ();
              if (html_text_standard) html_text_standard->newPageBreak ();
              if (html_text_linked) html_text_linked->newPageBreak ();
              if (text_text) text_text->paragraph ();
              break;
            }
            case StyleTypeTableElement:
            {
              if (odf_text_standard) odf_text_standard->closeTextStyle (false, false);
              if (odf_text_text_only) odf_text_text_only->closeTextStyle (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->closeTextStyle (false, false);
              if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
              if (html_text_standard) html_text_standard->closeTextStyle (false, false);
              if (html_text_linked) html_text_linked->closeTextStyle (false, false);
              switch (style.subtype)
              {
                case TableElementSubtypeRow:
                {
                  addToFallout ("Table elements not implemented", false);
                  break;
                }
                case TableElementSubtypeHeading:
                case TableElementSubtypeCell:
                {
                  newParagraph (style, false);
                  break;
                }
                default:
                {
                  break;
                }
              }
              // UserInt1TableColumnNumber:
              break;
            }
            case StyleTypeWordlistElement:
            {
              switch (style.subtype)
              {
                case WorListElementSubtypeWordlistGlossaryDictionary:
                {
                  if (isOpeningMarker) {
                    addToWordList (wordListGlossaryDictionary);
                  }
                  break;
                }
                case WorListElementSubtypeHebrewWordlistEntry:
                {
                  if (isOpeningMarker) {
                    addToWordList (hebrewWordList);
                  }
                  break;
                }
                case WorListElementSubtypeGreekWordlistEntry:
                {
                  if (isOpeningMarker) {
                    addToWordList (greekWordList);
                  }
                  break;
                }
                case WorListElementSubtypeSubjectIndexEntry:
                {
                  if (isOpeningMarker) {
                    addToWordList (subjectIndex);
                  }
                  break;
                }
                default:
                {
                  if (isOpeningMarker) {
                    addToFallout ("Unknown word list marker \\" + marker, false);
                  }
                  break;
                }
              }
              // UserString1WordListEntryAddition:
              break;
            }
            default:
            {
              // This marker is not yet implemented.
              // Add it to the fallout, plus any text that follows the marker.
              addToFallout ("Marker not yet implemented \\" + marker + ", possible formatting error:", true);
              break;
            }
          }
        } else {
          // Here is an unknown marker.
          // Add it to the fallout, plus any text that follows the marker.
          addToFallout ("Unknown marker \\" + marker + ", formatting error:", true);
        }
      } else {
        // Here is no marker. Treat it as text.
        if (odf_text_standard) odf_text_standard->addText (currentItem);
        if (odf_text_text_only) odf_text_text_only->addText (currentItem);
        if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->addText (currentItem);
        if (html_text_standard) html_text_standard->addText (currentItem);
        if (html_text_linked) html_text_linked->addText (currentItem);
        if (onlinebible_text) onlinebible_text->addText (currentItem);
        if (esword_text) esword_text->addText (currentItem);
        if (text_text) text_text->addtext (currentItem);
        if (headings_text_per_verse_active && heading_started) {
          int iverse = convert_to_int (currentVerseNumber);
          verses_headings [iverse].append (currentItem);
        }
        if (headings_text_per_verse_active && text_started) {
          int iverse = convert_to_int (currentVerseNumber);
          if (verses_text.count (iverse) && !verses_text [iverse].empty ()) {
            verses_text [iverse].append (currentItem);
            actual_verses_paragraph [iverse].append (currentItem);
          } else {
            // The verse text straight after the \v starts with certain space type.
            // Replace it with a normal space.
            string item = filter_string_str_replace (space_type_after_verse, " ", currentItem);
            verses_text [iverse] = filter_string_ltrim (item);
            actual_verses_paragraph [iverse] = filter_string_ltrim (item);
          }
        }
        if (note_open_now) {
          notes_plain_text_buffer.append (currentItem);
        }
      }
    }
  }
}



// This function does the processing of the USFM code for one note,
// formatting the document and extracting information.
void Filter_Text::processNote ()
{
  for ( ; chapterUsfmMarkersAndTextPointer < chapterUsfmMarkersAndText.size(); chapterUsfmMarkersAndTextPointer++)
  {
    string currentItem = chapterUsfmMarkersAndText[chapterUsfmMarkersAndTextPointer];
    if (usfm_is_usfm_marker (currentItem))
    {
      // Flags about the nature of the marker.
      bool isOpeningMarker = usfm_is_opening_marker (currentItem);
      bool isEmbeddedMarker = usfm_is_embedded_marker (currentItem);
      // Clean up the marker, so we remain with the basic version, e.g. 'f'.
      string marker = usfm_get_marker (currentItem);
      if (styles.find (marker) != styles.end())
      {
        Database_Styles_Item style = styles[marker];
        switch (style.type)
        {
          case StyleTypeVerseNumber:
          {
            // Verse found. The note should have stopped here. Incorrect note markup.
            addToFallout ("The note did not close at the end of the verse. The text is not correct.", false);
            goto noteDone;
          }
          case StyleTypeFootEndNote:
          {
            switch (style.subtype)
            {
              case FootEndNoteSubtypeFootnote:
              {
                if (isOpeningMarker) {
                  ensureNoteParagraphStyle (marker, styles [standardContentMarkerFootEndNote]);
                  string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->addNote (citation, marker);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    vector <string> currentTextStyles = odf_text_text_and_note_citations->currentTextStyle;
                    odf_text_text_and_note_citations->currentTextStyle = {"superscript"};
                    odf_text_text_and_note_citations->addText (citation);
                    odf_text_text_and_note_citations->currentTextStyle = currentTextStyles;
                  }
                  // Add space if the paragraph has text already.
                  if (odf_text_notes) {
                    if (odf_text_notes->currentParagraphContent != "") {
                      odf_text_notes->addText (" ");
                    }
                  }
                  // Add the note citation. And a no-break space after it.
                  if (odf_text_notes) odf_text_notes->addText (citation + non_breaking_space_u00A0());
                  // Open note in the web pages.
                  if (html_text_standard) html_text_standard->addNote (citation, standardContentMarkerFootEndNote);
                  if (html_text_linked) html_text_linked->addNote (citation, standardContentMarkerFootEndNote);
                  // Online Bible. Footnotes do not seem to behave as they ought in the Online Bible compiler. Just leave them out.
                  //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
                  if (text_text) text_text->note ();
                  // Handle opening notes in plain text.
                  notes_plain_text_handler ();
                  // Set flag.
                  note_open_now = true;
                } else {
                  goto noteDone;
                }
                break;
              }
              case FootEndNoteSubtypeEndnote:
              {
                if (isOpeningMarker) {
                  ensureNoteParagraphStyle (marker, styles[standardContentMarkerFootEndNote]);
                  string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->addNote (citation, marker, true);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    vector <string> currentTextStyles = odf_text_text_and_note_citations->currentTextStyle;
                    odf_text_text_and_note_citations->currentTextStyle = {"superscript"};
                    odf_text_text_and_note_citations->addText (citation);
                    odf_text_text_and_note_citations->currentTextStyle = currentTextStyles;
                  }
                  // Open note in the web page.
                  if (html_text_standard) html_text_standard->addNote (citation, standardContentMarkerFootEndNote, true);
                  if (html_text_linked) html_text_linked->addNote (citation, standardContentMarkerFootEndNote, true);
                  // Online Bible: Leave note out.
                  //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
                  if (text_text) text_text->note ();
                  // Handle opening notes in plain text.
                  notes_plain_text_handler ();
                  // Set flag.
                  note_open_now = true;
                } else {
                  goto noteDone;
                }
                break;
              }
              case FootEndNoteSubtypeStandardContent:
              {
                // The style of the standard content is already used in the note's body.
                // If means that the text style should be cleared
                // in order to return to the correct style for the paragraph.
                if (odf_text_standard) odf_text_standard->closeTextStyle (true, false);
                if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
                if (html_text_standard) html_text_standard->closeTextStyle (true, false);
                if (html_text_linked) html_text_linked->closeTextStyle (true, false);
                break;
              }
              case FootEndNoteSubtypeContent:
              case FootEndNoteSubtypeContentWithEndmarker:
              {
                if (isOpeningMarker) {
                  if (odf_text_standard) odf_text_standard->openTextStyle (style, true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->openTextStyle (style, false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->openTextStyle (style, true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->openTextStyle (style, true, isEmbeddedMarker);
                } else {
                  if (odf_text_standard) odf_text_standard->closeTextStyle (true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->closeTextStyle (false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->closeTextStyle (true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->closeTextStyle (true, isEmbeddedMarker);
                }
                break;
              }
              case FootEndNoteSubtypeParagraph:
              {
                // The style of this is not yet implemented.
                if (odf_text_standard) odf_text_standard->closeTextStyle (true, false);
                if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
                if (html_text_standard) html_text_standard->closeTextStyle (true, false);
                if (html_text_linked) html_text_linked->closeTextStyle (true, false);
                if (text_text) text_text->note ();
                break;
              }
              default:
              {
                break;
              }
            }
            // UserBool1NoteAppliesToApocrypha: For xref too?
            break;
          }
          case StyleTypeCrossreference:
          {
            switch (style.subtype)
            {
              case CrossreferenceSubtypeCrossreference:
              {
                if (isOpeningMarker) {
                  ensureNoteParagraphStyle (marker, styles[standardContentMarkerCrossReference]);
                  string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->addNote (citation, marker);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    vector <string> currentTextStyles = odf_text_text_and_note_citations->currentTextStyle;
                    odf_text_text_and_note_citations->currentTextStyle = {"superscript"};
                    odf_text_text_and_note_citations->addText (citation);
                    odf_text_text_and_note_citations->currentTextStyle = currentTextStyles;
                  }
                  // Add a space if the paragraph has text already.
                  if (odf_text_notes) {
                    if (odf_text_notes->currentParagraphContent != "") {
                      odf_text_notes->addText (" ");
                    }
                  }
                  // Add the note citation. And a no-break space (NBSP) after it.
                  if (odf_text_notes) odf_text_notes->addText (citation + non_breaking_space_u00A0());
                  // Open note in the web page.
                  ensureNoteParagraphStyle (standardContentMarkerCrossReference, styles[standardContentMarkerCrossReference]);
                  if (html_text_standard) html_text_standard->addNote (citation, standardContentMarkerCrossReference);
                  if (html_text_linked) html_text_linked->addNote (citation, standardContentMarkerCrossReference);
                  // Online Bible: Skip notes.
                  //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
                  if (text_text) text_text->note ();
                  // Handle opening notes in plain text.
                  notes_plain_text_handler ();
                  // Set flag.
                  note_open_now = true;
                } else {
                  goto noteDone;
                }
                break;
              }
              case CrossreferenceSubtypeStandardContent:
              {
                // The style of the standard content is already used in the note's body.
                // If means that the text style should be cleared
                // in order to return to the correct style for the paragraph.
                if (odf_text_standard) odf_text_standard->closeTextStyle (true, false);
                if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
                if (html_text_standard) html_text_standard->closeTextStyle (true, false);
                if (html_text_linked) html_text_linked->closeTextStyle (true, false);
                break;
              }
              case CrossreferenceSubtypeContent:
              case CrossreferenceSubtypeContentWithEndmarker:
              {
                if (isOpeningMarker) {
                  if (odf_text_standard) odf_text_standard->openTextStyle (style, true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->openTextStyle (style, false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->openTextStyle (style, true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->openTextStyle (style, true, isEmbeddedMarker);
                } else {
                  if (odf_text_standard) odf_text_standard->closeTextStyle (true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->closeTextStyle (false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->closeTextStyle (true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->closeTextStyle (true, isEmbeddedMarker);
                }
                break;
              }
              default:
              {
                break;
              }
            }
            break;
          }
          default:
          {
            addToFallout ("Marker not suitable in note context \\" + marker, false);
            break;
          }
        }
      } else {
        // Here is an unknown marker. Add the marker to fallout, plus any text that follows.
        addToFallout ("Unknown marker \\" + marker, true);
      }
    } else {
      // Here is no marker. Treat it as text.
      if (odf_text_standard) odf_text_standard->addNoteText (currentItem);
      if (odf_text_notes) odf_text_notes->addText (currentItem);
      if (html_text_standard) html_text_standard->addNoteText (currentItem);
      if (html_text_linked) html_text_linked->addNoteText (currentItem);
      if (text_text) text_text->addnotetext (currentItem);
      if (note_open_now) {
        notes_plain_text_buffer.append (currentItem);
      }
    }
  }
  
  noteDone:

  // "Close" the current note, so that any following note text, if malformed,
  // will be added to a new note, not to the last one created.
  if (odf_text_standard) odf_text_standard->closeCurrentNote ();
  if (odf_text_notes) odf_text_notes->closeTextStyle (false, false);
  if (html_text_standard) html_text_standard->closeCurrentNote ();
  if (html_text_linked) html_text_linked->closeCurrentNote ();
  //if ($this->onlinebible_text) $this->onlinebible_text->closeCurrentNote ();
  if (!notes_plain_text_buffer.empty ()) {
    notes_plain_text.push_back (make_pair (currentVerseNumber, notes_plain_text_buffer));
  }
  note_open_now = false;
  notes_plain_text_buffer.clear ();
}



// This creates and saves the information document.
// It contains formatting information, collected from the USFM code.
// $path: Path to the document.
void Filter_Text::produceInfoDocument (string path)
{
  Html_Text information (translate("Information"));

  // Number of chapters per book.
  information.newHeading1 (translate("Number of chapters per book"));
  for (const auto & element : numberOfChaptersPerBook) {
    string line = Database_Books::getEnglishFromId (element.first) + " => " + convert_to_string (element.second);
    information.newParagraph ();
    information.addText (line);
  }

  // Running headers.
  information.newHeading1 (translate("Running headers"));
  for (auto item : runningHeaders) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }

  // Table of Contents entries.
  information.newHeading1 (translate("Long table of contents entries"));
  for (auto item : longTOCs) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }
  information.newHeading1 (translate("Short table of contents entries"));
  for (auto item : shortTOCs) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }

  // Book abbreviations.
  information.newHeading1 (translate("Book abbreviations"));
  for (auto item : bookAbbreviations) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }

  // Chapter specials.
  information.newHeading1 (translate("Publishing chapter labels"));
  for (auto item : chapterLabels) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }
  information.newHeading1 (translate("Publishing alternate chapter numbers"));
  for (auto item : publishedChapterMarkers) {
    string line = Database_Books::getEnglishFromId (item.book) + " (USFM " + item.marker + ") => " + item.value;
    information.newParagraph ();
    information.addText (line);
  }

  // Word lists.
  information.newHeading1 (translate("Word list, glossary, dictionary entries"));
  for (auto item : wordListGlossaryDictionary) {
    information.newParagraph ();
    information.addText (item);
  }
  information.newHeading1 (translate("Hebrew word list entries"));
  for (auto item : hebrewWordList) {
    information.newParagraph ();
    information.addText (item);
  }
  information.newHeading1 (translate("Greek word list entries"));
  for (auto item : greekWordList) {
    information.newParagraph ();
    information.addText (item);
  }
  information.newHeading1 (translate("Subject index entries"));
  for (auto item : subjectIndex) {
    information.newParagraph ();
    information.addText (item);
  }

  // Other info.
  information.newHeading1 (translate("Other information"));
  for (auto item : info) {
    information.newParagraph ();
    information.addText (item);
  }

  information.save (path);
}



// This function produces the text of the current passage, e.g.: Genesis 1:1.
// Returns: The passage text
string Filter_Text::getCurrentPassageText ()
{
  return filter_passage_display (currentBookIdentifier, currentChapterNumber, currentVerseNumber);
}



// This function adds a string to the Info array, prefixed by the current passage.
// $text: String to add to the Info array.
// $next: If true, it also adds the text following the marker to the info,
// and removes this text from the USFM input stream.
void Filter_Text::addToInfo (string text, bool next)
{
  text = getCurrentPassageText() + " " + text;
  if (next) {
    text.append (" " + usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer));
  }
  info.push_back (text);
}



// This function adds a string to the Fallout array, prefixed by the current passage.
// $text: String to add to the Fallout array.
// $next: If true, it also adds the text following the marker to the fallout,
// and removes this text from the USFM input stream.
void Filter_Text::addToFallout (string text, bool next)
{
  text = getCurrentPassageText () + " " + text;
  if (next) {
    text.append (" " + usfm_get_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer));
  }
  fallout.push_back (text);
}



// This function adds something to a word list array, prefixed by the current passage.
// $list: which list to add the text to.
// The word is extracted from the input USFM. The Usfm pointer points to the current marker,
// and the text following that marker is added to the word list array.
void Filter_Text::addToWordList (vector <string>  & list)
{
  string text = usfm_peek_text_following_marker (chapterUsfmMarkersAndText, chapterUsfmMarkersAndTextPointer);
  text.append (" (");
  text.append (getCurrentPassageText ());
  text.append (")");
  list.push_back (text);
}



// This produces and saves the Fallout document.
// $path: Path to the document.
void Filter_Text::produceFalloutDocument (string path)
{
  Html_Text html_text (translate("Fallout"));
  html_text.newHeading1 (translate("Fallout"));
  for (string line : fallout) {
    html_text.newParagraph ();
    html_text.addText (line);
  }
  html_text.save (path);
}



// This function ensures that a certain paragraph style is in the OpenDocument,
// and then opens a paragraph with that style.
// $style: The style to use.
// $keepWithNext: Whether to keep this paragraph with the next one.
void Filter_Text::newParagraph (Database_Styles_Item style, bool keepWithNext)
{
  string marker = style.marker;
  if (find (createdStyles.begin(), createdStyles.end(), marker) == createdStyles.end()) {
    string fontname = Database_Config_Bible::getExportFont (bible);
    float fontsize = style.fontsize;
    int italic = style.italic;
    int bold = style.bold;
    int underline = style.underline;
    int smallcaps = style.smallcaps;
    int alignment = style.justification;
    float spacebefore = style.spacebefore;
    float spaceafter = style.spaceafter;
    float leftmargin = style.leftmargin;
    float rightmargin = style.rightmargin;
    float firstlineindent = style.firstlineindent;
    // Columns are not implemented at present. Reason:
    // Copying and pasting sections with columns between documents in LibreOffice failed to work.
    // int spancolumns = style.spancolumns;
    int dropcaps = 0;
    if (odf_text_standard) odf_text_standard->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_only) odf_text_text_only->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    createdStyles.push_back (marker);
  }
  if (odf_text_standard) odf_text_standard->newParagraph (marker);
  if (odf_text_text_only) odf_text_text_only->newParagraph (marker);
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->newParagraph (marker);
  if (html_text_standard) html_text_standard->newParagraph (marker);
  if (html_text_linked) html_text_linked->newParagraph (marker);
  if (text_text) text_text->paragraph ();
}



// This applies the drop caps setting to the current paragraph style.
// This is for the chapter number to appear in drop caps in the OpenDocument.
// $dropCapsLength: Number of characters to put in drop caps.
void Filter_Text::applyDropCapsToCurrentParagraph (int dropCapsLength)
{
  // To name a style according to the number of characters to put in drop caps,
  // e.g. a style name like p_c1 or p_c2 or p_c3.
  if (odf_text_standard) {
    string combined_style = odf_text_standard->currentParagraphStyle + "_" + chapterMarker + convert_to_string (dropCapsLength);
    if (find (createdStyles.begin(), createdStyles.end(), combined_style) == createdStyles.end()) {
      Database_Styles_Item style = styles[odf_text_standard->currentParagraphStyle];
      string fontname = Database_Config_Bible::getExportFont (bible);
      float fontsize = style.fontsize;
      int italic = style.italic;
      int bold = style.bold;
      int underline = style.underline;
      int smallcaps = style.smallcaps;
      int alignment = style.justification;
      float spacebefore = style.spacebefore;
      float spaceafter = style.spaceafter;
      float leftmargin = style.leftmargin;
      float rightmargin = style.rightmargin;
      float firstlineindent = 0; // First line that contains the chapter number in drop caps is not indented.
      //int spancolumns = style.spancolumns;
      bool keepWithNext = false;
      if (odf_text_standard) odf_text_standard->createParagraphStyle (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      if (odf_text_text_only) odf_text_text_only->createParagraphStyle (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->createParagraphStyle (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      createdStyles.push_back (combined_style);
    }
    if (odf_text_standard) odf_text_standard->updateCurrentParagraphStyle (combined_style);
    if (odf_text_text_only) odf_text_text_only->updateCurrentParagraphStyle (combined_style);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->updateCurrentParagraphStyle (combined_style);
  }
}



// This puts the chapter number in a frame in the current paragraph.
// This is to put the chapter number in a frame so it looks like drop caps in the OpenDocument.
// $chapterText: The text of the chapter indicator to put.
void Filter_Text::putChapterNumberInFrame (string chapterText)
{
  Database_Styles_Item style = styles[chapterMarker];
  if (odf_text_standard) odf_text_standard->placeTextInFrame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
  if (odf_text_text_only) odf_text_text_only->placeTextInFrame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->placeTextInFrame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
}



// This creates an entry in the $this->notecitations array.
// $style: the style: an object with values.
void Filter_Text::createNoteCitation (Database_Styles_Item style)
{
  // Create an entry in the notecitations array in this object, if it does not yet exist.
  if (notecitations.find (style.marker) == notecitations.end()) {
    int numbering = style.userint1;
    string sequence = "1 2 3 4 5 6 7 8 9"; // Fallback sequence.
    if (numbering == NoteNumbering123) sequence = "";
    if (numbering == NoteNumberingAbc) sequence = "a b c d e f g h i j k l m n o p q r s t u v w x y z";
    if (numbering == NoteNumberingUser) sequence = style.userstring1;
    vector <string> sequencevector;
    if (!sequence.empty ()) sequencevector = filter_string_explode (sequence, ' ');
    // Use of the above information:
    // The note will be numbered as follows:
    // If a sequence is given, then this sequence is followed for the citations.
    // If no sequence is given, then the note gets numerical citations.
    string restart = "chapter";
    int userint2 = style.userint2;
    if (userint2 == NoteRestartNumberingNever) restart = "never";
    if (userint2 == NoteRestartNumberingEveryBook) restart = "book";
    if (userint2 == NoteRestartNumberingEveryChapter) restart = "chapter";
    // /Store the citation for later use.
    notecitations [style.marker] = Filter_Text_Note_Citation (sequencevector, restart);
  }
}



// This gets the note citation.
// The first time that a xref is encountered, this function would return, e.g. 'a'.
// The second time, it would return 'b'. Then 'c', 'd', 'e', and so on, up to 'z'.
// Then it would restart with 'a'. And so on.
// The note citation is the character that is put in superscript in the main body of Bible text.
// $style: array with values for the note opening marker.
// Returns: The character for the note citation.
string Filter_Text::getNoteCitation (Database_Styles_Item style)
{
  bool end_of_text_reached = (chapterUsfmMarkersAndTextPointer + 1) >= chapterUsfmMarkersAndText.size ();
  if (end_of_text_reached) return "";

  // Get the raw note citation from the USFM. This could be, e.g. '+'.
  string nextText = chapterUsfmMarkersAndText [chapterUsfmMarkersAndTextPointer + 1];
  string citation = nextText.substr (0, 1);
  nextText = filter_string_ltrim (nextText.substr (1));
  chapterUsfmMarkersAndText [chapterUsfmMarkersAndTextPointer + 1] = nextText;
  citation = filter_string_trim (citation);
  if (citation == "+") {
    string marker = style.marker;
    vector <string> sequence = notecitations[marker].sequence;
    size_t pointer = notecitations[marker].pointer;
    if (sequence.empty()) {
      pointer++;
      citation = convert_to_string (pointer);
    } else {
      citation = sequence [pointer];
      pointer++;
      if (pointer >= sequence.size ()) pointer = 0;
    }
    notecitations[marker].pointer = pointer;
  } else if (citation == "-") {
    citation = "";
  }
  return citation;
}



// This resets selected note citation data.
// Resetting means that the note citations start to count afresh.
// $moment: what type of reset to apply, e.g. 'chapter' or 'book'.
void Filter_Text::resetNoteCitations (string moment)
{
  for (auto & notecitation : notecitations) {
    if (notecitation.second.restart == moment) {
      notecitation.second.pointer = 0;
    }
  }
}



// This function ensures that a certain paragraph style for a note is present in the OpenDocument.
// $marker: Which note, e.g. 'f' or 'x' or 'fe'.
// $style: The style to use.
void Filter_Text::ensureNoteParagraphStyle (string marker, Database_Styles_Item style)
{
  if (find (createdStyles.begin(), createdStyles.end(), marker) == createdStyles.end()) {
    string fontname = Database_Config_Bible::getExportFont (bible);
    float fontsize = style.fontsize;
    int italic = style.italic;
    int bold = style.bold;
    int underline = style.underline;
    int smallcaps = style.smallcaps;
    int alignment = style.justification;
    float spacebefore = style.spacebefore;
    float spaceafter = style.spaceafter;
    float leftmargin = style.leftmargin;
    float rightmargin = style.rightmargin;
    float firstlineindent = style.firstlineindent;
    //bool spancolumns = false;
    bool keepWithNext = false;
    int dropcaps = 0;
    if (odf_text_standard) odf_text_standard->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_only) odf_text_text_only->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_notes) odf_text_notes->createParagraphStyle (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, 0, 0, 0, 0, 0, keepWithNext, dropcaps);
    createdStyles.push_back (marker);
  }
}


// This function initializes the array that holds verse numbers and the text of the headings,
// and the array that holds verse numbers and the plain text of the Bible.
// The object will only use the array when it has been initialized.
// The resulting arrays use the verse numbers as keys. Therefore it only works reliably within one chapter.
void Filter_Text::initializeHeadingsAndTextPerVerse (bool start_text_now)
{
  headings_text_per_verse_active = true;
  // Normally collecting the plain text starts only after the first normal paragraph marker.
  // But this can be force to start immediately
  if (start_text_now) text_started = true;
}


map <int, string> Filter_Text::getVersesText ()
{
  // Trim white space at start and end of each line.
  for (auto & element : verses_text) {
    element.second = filter_string_trim (element.second);
  }
  // Return the result.
  return verses_text;
}


void Filter_Text::storeVersesParagraphs ()
{
  if (!actual_verses_paragraph.empty ()) {
    verses_paragraphs.push_back (actual_verses_paragraph);
    actual_verses_paragraph.clear ();
  }
}


void Filter_Text::notes_plain_text_handler ()
{
  int offset = 0;
  int iverse = convert_to_int (currentVerseNumber);
  if (verses_text.count (iverse)) {
    offset = verses_text [iverse].size ();
  }
  verses_text_note_positions[iverse].push_back (offset);
}

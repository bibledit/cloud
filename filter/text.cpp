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


#include <filter/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <filter/passage.h>
#include <styles/logic.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <locale/translate.h>


namespace filter::text {

passage_marker_value::passage_marker_value (int book, int chapter, std::string verse, std::string marker, std::string value)
{
  m_book = book;
  m_chapter = chapter;
  m_verse = verse;
  m_marker = marker;
  m_value = value;
}

}


// This class filters USFM text, converting it into other formats.


Filter_Text::Filter_Text (std::string bible)
{
  m_bible = bible;
  space_type_after_verse = database::config::bible::get_odt_space_after_verse (m_bible);
  odt_left_align_verse_in_poetry_styles = database::config::bible::get_odt_poetry_verses_left (m_bible);
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
void Filter_Text::add_usfm_code (std::string usfm)
{
  // Check that the USFM is valid UTF-8.
  if (!filter::strings::unicode_string_is_valid (usfm)) {
    Database_Logs::log (translate ("Exporting invalid UTF-8.") + " " + translate ("Please check.") + " " + usfm);
  }
  // Clean the USFM.
  usfm = filter::strings::trim (usfm);
  usfm += "\n";
  // Sort the USFM code out and separate it into markers and text.
  std::vector <std::string> markers_and_text = filter::usfm::get_markers_and_text (usfm);
  // Add the USFM to the object.
  m_usfm_markers_and_text.insert (m_usfm_markers_and_text.end(), markers_and_text.begin(), markers_and_text.end());
}



// This function runs the filter.
// $stylesheet - The stylesheet to use.
void Filter_Text::run (const std::string& stylesheet)
{
  // Get the styles.
  get_styles (stylesheet);

  // Preprocess.
  pre_process_usfm (stylesheet);

  // Process data.
  process_usfm (stylesheet);

  store_verses_paragraphs ();
  
  // Clear USFM and styles.
  m_usfm_markers_and_text.clear();
  usfm_markers_and_text_ptr = 0;
  chapter_usfm_markers_and_text.clear();
  chapter_usfm_markers_and_text_pointer = 0;
  styles.clear();
  chapterMarker.clear();
  createdStyles.clear();
}



// This function return true when there is still unprocessed USFM code available.
bool Filter_Text::unprocessed_usfm_code_available ()
{
  return (usfm_markers_and_text_ptr < m_usfm_markers_and_text.size());
}



// This function stores data in the class:
// The next chapter from the unprocessed USFM code.
void Filter_Text::get_usfm_next_chapter ()
{
  // Initialization.
  chapter_usfm_markers_and_text.clear();
  chapter_usfm_markers_and_text_pointer = 0;
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
  while (unprocessed_usfm_code_available ()) {
    std::string item = m_usfm_markers_and_text [usfm_markers_and_text_ptr];
    if (!firstLine) {
      if (filter::strings::trim (item) == (R"(\)" + chapterMarker)) {
        return;
      }
    }
    chapter_usfm_markers_and_text.push_back (item);
    firstLine = false;
    usfm_markers_and_text_ptr++;
  }
}



// This function gets the styles from the database,
// and stores them in the object for quicker access.
void Filter_Text::get_styles (const std::string& stylesheet)
{
  styles.clear();
  // Get the relevant styles information included.
  if (odf_text_standard) odf_text_standard->create_page_break_style ();
  if (odf_text_text_only) odf_text_text_only->create_page_break_style ();
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->create_page_break_style ();
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->create_superscript_style ();
  std::vector <std::string> markers = database::styles1::get_markers (stylesheet);
  for (const auto& marker : markers) {
    database::styles1::Item style = database::styles1::get_marker_data (stylesheet, marker);
    styles [marker] = style;
    if (style.type == StyleTypeFootEndNote) {
      if (style.subtype == FootEndNoteSubtypeStandardContent) {
        standard_content_marker_foot_end_note = style.marker;
      }
    }
    if (style.type == StyleTypeCrossreference) {
      if (style.subtype == CrossreferenceSubtypeStandardContent) {
        standard_content_marker_cross_reference = style.marker;
      }
    }
  }
}


// This function does the preprocessing of the USFM code
// extracting a variety of information, creating note citations, etc.
void Filter_Text::pre_process_usfm (const std::string& stylesheet)
{
  usfm_markers_and_text_ptr = 0;
  while (unprocessed_usfm_code_available ()) {
    get_usfm_next_chapter ();
    for (chapter_usfm_markers_and_text_pointer = 0; chapter_usfm_markers_and_text_pointer < chapter_usfm_markers_and_text.size(); chapter_usfm_markers_and_text_pointer++) {
      std::string currentItem = chapter_usfm_markers_and_text[chapter_usfm_markers_and_text_pointer];
      if (filter::usfm::is_usfm_marker (currentItem)) {
        std::string marker = filter::strings::trim (currentItem); // Change, e.g. '\id ' to '\id'.
        marker = marker.substr (1); // Remove the initial backslash, e.g. '\id' becomes 'id'.
        if (filter::usfm::is_opening_marker (marker)) {
          if ((styles.find (marker) != styles.end()) && (!stylesv2::marker_moved_to_v2(marker, ""))) {
            database::styles1::Item style = styles [marker];
            note_citations.evaluate_style(style);
            switch (style.type) {
              case StyleTypeIdentifier:
                switch (style.subtype) {
                  case IdentifierSubtypeRunningHeader: // Todo move to v2
                  {
                    const std::string runningHeader = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    runningHeaders.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, runningHeader));
                    break;
                  }
                  case IdentifierSubtypeLongTOC:
                  {
                    const std::string longTOC = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    longTOCs.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, longTOC));
                    break;
                  }
                  case IdentifierSubtypeShortTOC:
                  {
                    const std::string shortTOC = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    shortTOCs.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, shortTOC));
                    break;
                  }
                  case IdentifierSubtypeBookAbbrev:
                  {
                    const std::string bookAbbreviation = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    bookAbbreviations.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, bookAbbreviation));
                    break;
                  }
                  case IdentifierSubtypeChapterLabel:
                  {
                    // Store the chapter label for this book and chapter.
                    const std::string chapterLabel = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    chapterLabels.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, chapterLabel));
                    // If a chapter label is in the book, there's no drop caps output of the chapter number.
                    book_has_chapter_label [m_current_book_identifier] = true;
                    // Done.
                    break;
                  }
                  case IdentifierSubtypePublishedChapterMarker:
                  {
                    const std::string publishedChapterMarker = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    publishedChapterMarkers.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, publishedChapterMarker));
                    break;
                  }
                  case IdentifierSubtypePublishedVerseMarker:
                  {
                    // It gets the published verse markup.
                    // The marker looks like: ... \vp ၁။\vp* ...
                    // It stores this markup in the object for later reference.
                    const std::string publishedVerseMarker = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                    publishedVerseMarkers.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, publishedVerseMarker));
                    break;
                  }
                  default: {
                    break;
                  }
                }
                break;
              case StyleTypeChapterNumber:
              {
                const std::string number = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                m_current_chapter_number = filter::strings::convert_to_int (number);
                m_number_of_chapters_per_book[m_current_book_identifier] = m_current_chapter_number;
                set_to_zero(m_current_verse_number);
                break;
              }
              case StyleTypeVerseNumber:
              {
                const std::string fragment = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                const int number = filter::strings::convert_to_int (fragment);
                m_current_verse_number = std::to_string (number);
                break;
              }
              case StyleTypeFootEndNote:
              {
                switch (style.subtype)
                {
                  case FootEndNoteSubtypeFootnote:
                  case FootEndNoteSubtypeEndnote:
                  case FootEndNoteSubtypeStandardContent:
                  case FootEndNoteSubtypeContent:
                  case FootEndNoteSubtypeContentWithEndmarker:
                  case FootEndNoteSubtypeParagraph:
                  default: {
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
                  case CrossreferenceSubtypeStandardContent:
                  case CrossreferenceSubtypeContent:
                  case CrossreferenceSubtypeContentWithEndmarker:
                  default: {
                    break;
                  }
                }
                break;
              }
              default: {
                break;
              }
            }
          }
          else if (const stylesv2::Style* style {database::styles2::get_marker_data (stylesheet, marker)}; style) {
            switch (style->type) {
              case stylesv2::Type::book_id:
              {
                // Get book number.
                std::string usfm_id = filter::usfm::get_book_identifier (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                // Remove possible soft hyphen.
                usfm_id = filter::strings::replace (filter::strings::soft_hyphen_u00AD (), "", usfm_id);
                // Get Bibledit book number.
                m_current_book_identifier = static_cast<int>(database::books::get_id_from_usfm (usfm_id));
                // Reset chapter and verse numbers.
                m_current_chapter_number = 0;
                m_number_of_chapters_per_book[m_current_book_identifier] = 0;
                set_to_zero(m_current_verse_number);
                // Done.
                break;
              }
              case stylesv2::Type::starting_boundary:
              case stylesv2::Type::none:
              case stylesv2::Type::stopping_boundary:
              default:
                break;
            }
          }
        }
      }
    }
  }
}



// This function does the processing of the USFM code,
// formatting the document and extracting other useful information.
void Filter_Text::process_usfm (const std::string& stylesheet)
{
  // Go through the USFM code.
  int processed_books_count {0};
  usfm_markers_and_text_ptr = 0;
  while (unprocessed_usfm_code_available ()) {
    get_usfm_next_chapter ();
    for (chapter_usfm_markers_and_text_pointer = 0; chapter_usfm_markers_and_text_pointer < chapter_usfm_markers_and_text.size(); chapter_usfm_markers_and_text_pointer++) {
      const std::string current_item = chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer];
      if (filter::usfm::is_usfm_marker (current_item))
      {
        // Indicator describing the marker.
        const bool is_opening_marker = filter::usfm::is_opening_marker (current_item);
        const bool is_embedded_marker = filter::usfm::is_embedded_marker (current_item);
        // Clean up the marker, so we remain with the basic version, e.g. 'id'.
        const std::string marker = filter::usfm::get_marker (current_item);
        // Strip word-level attributes.
        if (is_opening_marker) filter::usfm::remove_word_level_attributes (marker, chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
        if ((styles.find (marker) != styles.end()) && (!stylesv2::marker_moved_to_v2(marker, ""))) // Todo
        {
          // Deal with a known style.
          const database::styles1::Item& style = styles.at(marker);
          switch (style.type)
          {
            case StyleTypeIdentifier:
            {
              switch (style.subtype)
              {
                case IdentifierSubtypeRunningHeader: // Todo move to v2.
                {
                  close_text_style_all();
                  // This information was processed during the preprocessing stage.
                  std::string runningHeader = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  // Ideally this information should be inserted in the headers of the standard text document.
                  // UserBool2RunningHeaderLeft:
                  // UserBool3RunningHeaderRight:
                  break;
                }
                case IdentifierSubtypeLongTOC:
                {
                  close_text_style_all();
                  // This information already went into the Info document. Remove it from the USFM stream.
                  filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  break;
                }
                case IdentifierSubtypeShortTOC:
                {
                  close_text_style_all();
                  // This information already went into the Info document. Remove it from the USFM stream.
                  filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  break;
                }
                case IdentifierSubtypeBookAbbrev:
                {
                  close_text_style_all();
                  // This information already went into the Info document. Remove it from the USFM stream.
                  filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  break;
                }
                case IdentifierSubtypeChapterLabel:
                {
                  close_text_style_all();
                  // This information is already in the object. Remove it from the USFM stream.
                  filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  break;
                }
                case IdentifierSubtypePublishedChapterMarker:
                {
                  close_text_style_all();
                  // This information is already in the object.
                  // Remove it from the USFM stream.
                  filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  break;
                }
                case IdentifierSubtypeCommentWithEndmarker:
                {
                  close_text_style_all();
                  if (is_opening_marker) {
                    add_to_info (R"(Comment: \)" + marker, true);
                  }
                  break;
                }
                case IdentifierSubtypePublishedVerseMarker:
                {
                  close_text_style_all();
                  if (is_opening_marker) {
                    // This information is already in the object.
                    // Remove it from the USFM stream at the opening marker.
                    filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                  } else {
                    // USFM allows the closing marker \vp* to be followed by a space.
                    // But this space should not be converted to text output.
                    // https://github.com/bibledit/cloud/issues/311
                    // It is going to be removed here.
                    const size_t pointer = chapter_usfm_markers_and_text_pointer + 1;
                    if (pointer < chapter_usfm_markers_and_text.size()) {
                      std::string text = chapter_usfm_markers_and_text[pointer];
                      text = filter::strings::ltrim (text);
                      chapter_usfm_markers_and_text[pointer] = text;
                    }
                  }
                  break;
                }
                default:
                {
                  close_text_style_all();
                  addToFallout (R"(Unknown markup: \)" + marker, true);
                  break;
                }
              }
              break;
            }
            case StyleTypeNotUsedComment:
            {
              addToFallout (R"(Unknown markup: \)" + marker, true);
              break;
            }
            case StyleTypeNotUsedRunningHeader:
            {
              addToFallout (R"(Unknown markup: \)" + marker, true);
              break;
            }
            case StyleTypeStartsParagraph:
            {
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);
              switch (style.subtype)
              {
                case ParagraphSubtypeMainTitle:
                case ParagraphSubtypeSubTitle:
                case ParagraphSubtypeSectionHeading:
                {
                  new_paragraph (style, true);
                  heading_started = true;
                  text_started = false;
                  break;
                }
                case ParagraphSubtypeNormalParagraph:
                default:
                {
                  new_paragraph (style, false);
                  heading_started = false;
                  text_started = true;
                  if (headings_text_per_verse_active) {
                    // If a new paragraph starts within an existing verse,
                    // add a space to the text already in that verse.
                    int iverse = filter::strings::convert_to_int (m_current_verse_number);
                    if (m_verses_text.count (iverse) && !m_verses_text [iverse].empty ()) {
                      m_verses_text [iverse].append (" ");
                    }
                    // Record the style that started this new paragraph.
                    paragraph_starting_markers.push_back (style.marker);
                    // Store previous paragraph, if any, and start recording the new one.
                    store_verses_paragraphs ();
                  }
                  break;
                }
              }
              break;
            }
            case StyleTypeInlineText:
            {
              // Support for a normal and an embedded character style.
              if (is_opening_marker) {
                if (odf_text_standard) odf_text_standard->open_text_style (style, false, is_embedded_marker);
                if (odf_text_text_only) odf_text_text_only->open_text_style (style, false, is_embedded_marker);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->open_text_style (style, false, is_embedded_marker);
                if (html_text_standard) html_text_standard->open_text_style (style, false, is_embedded_marker);
                if (html_text_linked) html_text_linked->open_text_style (style, false, is_embedded_marker);
              } else {
                if (odf_text_standard) odf_text_standard->close_text_style (false, is_embedded_marker);
                if (odf_text_text_only) odf_text_text_only->close_text_style (false, is_embedded_marker);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, is_embedded_marker);
                if (html_text_standard) html_text_standard->close_text_style (false, is_embedded_marker);
                if (html_text_linked) html_text_linked->close_text_style (false, is_embedded_marker);
              }
              break;
            }
            case StyleTypeChapterNumber:
            {
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);

              if (onlinebible_text) onlinebible_text->storeData ();

              // Get the chapter number.
              std::string usfm_c_fragment = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              int chapter_number = filter::strings::convert_to_int (usfm_c_fragment);

              // Update this object.
              m_current_chapter_number = chapter_number;
              set_to_zero(m_current_verse_number);

              // If there is a published chapter character, the chapter number takes that value.
              for (const auto& published_chapter_marker : publishedChapterMarkers) {
                if (published_chapter_marker.m_book == m_current_book_identifier) {
                  if (published_chapter_marker.m_chapter == m_current_chapter_number) {
                    usfm_c_fragment = published_chapter_marker.m_value;
                    chapter_number = filter::strings::convert_to_int (usfm_c_fragment);
                  }
                }
              }

              // Enter text for the running headers.
              std::string running_header = database::books::get_english_from_id (static_cast<book_id>(m_current_book_identifier));
              for (const auto& item : runningHeaders) {
                if (item.m_book == m_current_book_identifier) {
                  running_header = item.m_value;
                }
              }
              running_header += (" " + usfm_c_fragment);
              if (odf_text_standard) odf_text_standard->new_heading1 (running_header, true);
              if (odf_text_text_only) odf_text_text_only->new_heading1 (running_header, true);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_heading1 (running_header, true);
              if (odf_text_notes) odf_text_notes->new_heading1 (running_header, false);

              // This is the phase of outputting the chapter number in the text body.
              // It always outputs the chapter number to the clear text export.
              if (text_text) { 
                text_text->paragraph (usfm_c_fragment);
              }
              // The chapter number is only output when there is more than one chapter in a book.
              // if (m_number_of_chapters_per_book [m_current_book_identifier] > 1)
              // This was disabled because of a bug where exporting to web did not output the chapter number
              // for the first chapter. See https://github.com/bibledit/cloud/issues/905 for more info.
              {
                // Putting the chapter number at the first verse is determined by the style of the \c marker.
                // But if a chapter label (\cl) is found in the current book, that disables the above.
                const bool cl_found = book_has_chapter_label[m_current_book_identifier];
                if (style.userbool1 && !cl_found) {
                  // Output the chapter number at the first verse, not here.
                  // Store it for later processing.
                  m_output_chapter_text_at_first_verse = usfm_c_fragment;
                } else {
                  // Output the chapter in a new paragraph.
                  // If the chapter label \cl is entered once before chapter 1 (\c 1)
                  // it represents the text for "chapter" to be used throughout the current book.
                  // If \cl is used after each individual chapter marker, it represents the particular text
                  // to be used for the display of the current chapter heading
                  // (usually done if numbers are being presented as words, not numerals).
                  std::string labelEntireBook {};
                  std::string labelCurrentChapter {};
                  for (const auto& pchapterLabel : chapterLabels) {
                    if (pchapterLabel.m_book == m_current_book_identifier) {
                      if (pchapterLabel.m_chapter == 0) {
                        labelEntireBook = pchapterLabel.m_value;
                      }
                      if (pchapterLabel.m_chapter == m_current_chapter_number) {
                        labelCurrentChapter = pchapterLabel.m_value;
                      }
                    }
                  }
                  if (!labelEntireBook.empty()) {
                    usfm_c_fragment = labelEntireBook + " " + usfm_c_fragment;
                  }
                  if (!labelCurrentChapter.empty()) {
                    usfm_c_fragment = labelCurrentChapter;
                  }
                  // The chapter number shows in a new paragraph.
                  // Keep it together with the next paragraph.
                  new_paragraph (style, true);
                  if (odf_text_standard) odf_text_standard->add_text (usfm_c_fragment);
                  if (odf_text_text_only) odf_text_text_only->add_text (usfm_c_fragment);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (usfm_c_fragment);
                  if (html_text_standard) html_text_standard->add_text (usfm_c_fragment);
                  if (html_text_linked) html_text_linked->add_text (usfm_c_fragment);
                }
              }

              // Output chapter number for other formats.
              if (esword_text) esword_text->newChapter (m_current_chapter_number);

              // Open a paragraph for the notes.
              // It takes the style of the footnote content marker, usually 'ft'.
              // This is done specifically for the version that has the notes only.
              ensureNoteParagraphStyle (standard_content_marker_foot_end_note, styles[standard_content_marker_foot_end_note]);
              if (odf_text_notes) odf_text_notes->new_paragraph (standard_content_marker_foot_end_note);
              // UserBool2ChapterInLeftRunningHeader -> no headings implemented yet.
              // UserBool3ChapterInRightRunningHeader -> no headings implemented yet.

              // Reset.
              note_citations.restart("chapter");

              // Done.
              break;
            }
            case StyleTypeVerseNumber:
            {
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);
              if (onlinebible_text) onlinebible_text->storeData ();
              // Handle a situation that a verse number starts a new paragraph.
              if (style.userbool1) {
                if (odf_text_standard) {
                  if (!odf_text_standard->m_current_paragraph_content.empty()) {
                    odf_text_standard->new_paragraph (odf_text_standard->m_current_paragraph_style);
                  }
                }
                if (odf_text_text_only) {
                  if (!odf_text_text_only->m_current_paragraph_content.empty()) {
                    odf_text_text_only->new_paragraph (odf_text_text_only->m_current_paragraph_style);
                  }
                }
                if (odf_text_text_and_note_citations) {
                  if (!odf_text_text_and_note_citations->m_current_paragraph_content.empty()) {
                    odf_text_text_and_note_citations->new_paragraph (odf_text_text_and_note_citations->m_current_paragraph_style);
                  }
                }
                if (html_text_standard) {
                  if (!html_text_standard->current_paragraph_content.empty()) {
                    html_text_standard->new_paragraph (html_text_standard->current_paragraph_style);
                  }
                }
                if (html_text_linked) {
                  if (!html_text_linked->current_paragraph_content.empty()) {
                    html_text_linked->new_paragraph (html_text_linked->current_paragraph_style);
                  }
                }
                if (text_text) {
                  text_text->paragraph (); 
                }
              }
              // Deal with the case of a pending chapter number.
              if (!m_output_chapter_text_at_first_verse.empty()) {
                if (!database::config::bible::get_export_chapter_drop_caps_frames (m_bible)) {
                  int dropCapsLength = static_cast<int>( filter::strings::unicode_string_length (m_output_chapter_text_at_first_verse));
                  applyDropCapsToCurrentParagraph (dropCapsLength);
                  if (odf_text_standard) odf_text_standard->add_text (m_output_chapter_text_at_first_verse);
                  if (odf_text_text_only) odf_text_text_only->add_text (m_output_chapter_text_at_first_verse);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (m_output_chapter_text_at_first_verse);
                } else {
                  putChapterNumberInFrame (m_output_chapter_text_at_first_verse);
                }
                database::styles1::Item styleItem = database::styles1::Item ();
                styleItem.marker = "dropcaps";
                if (html_text_standard) html_text_standard->open_text_style (styleItem, false, false);
                if (html_text_standard) html_text_standard->add_text (m_output_chapter_text_at_first_verse);
                if (html_text_standard) html_text_standard->close_text_style (false, false);
                if (html_text_linked) html_text_linked->open_text_style (styleItem, false, false);
                if (html_text_linked) html_text_linked->add_text (m_output_chapter_text_at_first_verse);
                if (html_text_linked) html_text_linked->close_text_style (false, false);
              }
              // Temporarily retrieve the text that follows the \v verse marker.
              std::string text_following_v_marker = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              // Extract the verse number, and store it in the object.
              std::string v_number = filter::usfm::peek_verse_number (text_following_v_marker);
              m_current_verse_number = v_number;
              // In case there was a published verse marker, use that markup for publishing.
              std::string v_vp_number = v_number;
              for (const auto& publishedVerseMarker : publishedVerseMarkers) {
                if (publishedVerseMarker.m_book == m_current_book_identifier) {
                  if (publishedVerseMarker.m_chapter == m_current_chapter_number) {
                    if (publishedVerseMarker.m_verse == m_current_verse_number) {
                      v_vp_number = publishedVerseMarker.m_value;
                    }
                  }
                }
              }
              // Output the verse number. But only if no chapter number was put here.
              if (m_output_chapter_text_at_first_verse.empty ()) {
                // If the current paragraph has text already, then insert a space.
                if (odf_text_standard) {
                  if (!odf_text_standard->m_current_paragraph_content.empty()) {
                    odf_text_standard->add_text (" ");
                  }
                }
                if (odf_text_text_only) {
                  if (!odf_text_text_only->m_current_paragraph_content.empty()) {
                    odf_text_text_only->add_text (" ");
                  }
                }
                if (odf_text_text_and_note_citations) {
                  if (!odf_text_text_and_note_citations->m_current_paragraph_content.empty()) {
                    odf_text_text_and_note_citations->add_text (" ");
                  }
                }
                if (html_text_standard) {
                  if (!html_text_standard->current_paragraph_content.empty()) {
                    html_text_standard->add_text (" ");
                  }
                }
                if (html_text_linked) {
                  if (!html_text_linked->current_paragraph_content.empty()) {
                    html_text_linked->add_text (" ");
                  }
                }
                if (odf_text_standard) odf_text_standard->open_text_style (style, false, false);
                if (odf_text_text_only) odf_text_text_only->open_text_style (style, false, false);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->open_text_style (style, false, false);
                if (html_text_standard) html_text_standard->open_text_style (style, false, false);
                if (html_text_linked) html_text_linked->open_text_style (style, false, false);
                if (odf_text_standard) odf_text_standard->add_text (v_vp_number);
                if (odf_text_text_only) odf_text_text_only->add_text (v_vp_number);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (v_vp_number);
                if (html_text_standard) html_text_standard->add_text (v_vp_number);
                if (html_text_linked) html_text_linked->add_text (v_vp_number);
                if (odf_text_standard) odf_text_standard->close_text_style (false, false);
                if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
                if (html_text_standard) html_text_standard->close_text_style (false, false);
                if (html_text_linked) html_text_linked->close_text_style (false, false);
              }
              // Plain text output.
              if (text_text) { 
                if (!text_text->line ().empty()) {
                  text_text->addtext (" ");
                }
                text_text->addtext (v_vp_number);
                // Plain text output always has a space following the verse.
                // Important for outputting the first verse.
                text_text->addtext (" ");
              }
              // If there was any text following the \v marker, remove the verse number,
              // put the remainder back into the object, and update the pointer.
              if (!text_following_v_marker.empty()) {
                size_t pos = text_following_v_marker.find (v_number);
                if (pos != std::string::npos) {
                  text_following_v_marker = text_following_v_marker.substr (pos + v_number.length ());
                }
                // If a verse number was put, do this:
                // Remove any whitespace from the start of the following text.
                text_following_v_marker = filter::strings::ltrim (text_following_v_marker);
                chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer] = text_following_v_marker;
                chapter_usfm_markers_and_text_pointer--;
                // If a verse number was put, do this too:
                // Output the type of space that the user has set.
                // This could be a fixed-width space, or a non-breaking space,
                // or a combination of the two.
                // This space type improves the appearance of the verse plus text.
                // In case the verse numbers in poetry are to be left aligned,
                // then output a tab to OpenDocument instead of the space.
                // Exception:
                // If a chapter number was put, do not output any white space.
                if (m_output_chapter_text_at_first_verse.empty()) {
                  if (odf_text_standard) {
                    bool tab = odt_left_align_verse_in_poetry_styles && filter::usfm::is_standard_q_poetry (odf_text_standard->m_current_paragraph_style);
                    if (tab) odf_text_standard->add_tab();
                    else odf_text_standard->add_text (space_type_after_verse);
                  }
                  if (odf_text_text_only) {
                    bool tab = odt_left_align_verse_in_poetry_styles && filter::usfm::is_standard_q_poetry (odf_text_text_only->m_current_paragraph_style);
                    if (tab) odf_text_text_only->add_tab();
                    else odf_text_text_only->add_text (space_type_after_verse);
                  }
                  if (odf_text_text_and_note_citations) {
                    bool tab = odt_left_align_verse_in_poetry_styles && filter::usfm::is_standard_q_poetry (odf_text_text_and_note_citations->m_current_paragraph_style);
                    if (tab) odf_text_text_and_note_citations->add_tab();
                    else odf_text_text_and_note_citations->add_text (space_type_after_verse);
                  }
                  if (html_text_standard) html_text_standard->add_text (space_type_after_verse);
                  if (html_text_linked) html_text_linked->add_text (space_type_after_verse);
                }
              }
              // Unset the chapter variable, whether it was used or not.
              // This makes it ready for subsequent use.
              m_output_chapter_text_at_first_verse.clear();
              // Other export formats.
              if (onlinebible_text) onlinebible_text->newVerse (m_current_book_identifier, m_current_chapter_number, filter::strings::convert_to_int (m_current_verse_number));
              if (esword_text) esword_text->newVerse (filter::strings::convert_to_int (m_current_verse_number));
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
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);
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
                {
                  addToFallout (R"(Unknown pheripheral marker \)" + marker, false);
                  break;
                }
                case PeripheralSubtypeGeneral:
                {
                  add_to_info(R"(Pheripheral markup: \)" + marker, true);
                  // To start peripheral material o a new page.
                  // https://ubsicap.github.io/usfm/peripherals/index.html
                  if (odf_text_standard) odf_text_standard->new_page_break ();
                  if (odf_text_text_only) odf_text_text_only->new_page_break ();
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_page_break ();
                  if (html_text_standard) html_text_standard->new_page_break ();
                  if (html_text_linked) html_text_linked->new_page_break ();
                  // Done.
                  break;
                }
                default: break;
              }
              break;
            }
            case StyleTypePicture:
            {
              if (is_opening_marker) {
                // Set a flag that the parser is going to be within figure markup and save the style.
                is_within_figure_markup = true;
                figure_marker = marker;
                // Create the style for the figure because it is used within the ODT generator.
                create_paragraph_style (style, false);
                // At the start of the \fig marker, close all text styles that might be open.
                if (odf_text_standard) odf_text_standard->close_text_style (false, false);
                if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
                if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
                if (odf_text_notes) odf_text_notes->close_text_style (false, false);
                if (html_text_standard) html_text_standard->close_text_style (false, false);
                if (html_text_linked) html_text_linked->close_text_style (false, false);
              } else {
                // Closing the \fig* markup.
                // Clear the flag since the parser is no longer within figure markup.
                is_within_figure_markup = false;
              }
              break;
            }
            case StyleTypePageBreak:
            {
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);
              if (odf_text_standard) odf_text_standard->new_page_break ();
              if (odf_text_text_only) odf_text_text_only->new_page_break ();
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_page_break ();
              if (html_text_standard) html_text_standard->new_page_break ();
              if (html_text_linked) html_text_linked->new_page_break ();
              if (text_text) text_text->paragraph (); 
              break;
            }
            case StyleTypeTableElement:
            {
              if (odf_text_standard) odf_text_standard->close_text_style (false, false);
              if (odf_text_text_only) odf_text_text_only->close_text_style (false, false);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->close_text_style (false, false);
              if (odf_text_notes) odf_text_notes->close_text_style (false, false);
              if (html_text_standard) html_text_standard->close_text_style (false, false);
              if (html_text_linked) html_text_linked->close_text_style (false, false);
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
                  new_paragraph (style, false);
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
                  if (is_opening_marker) {
                    addToWordList (wordListGlossaryDictionary);
                  }
                  break;
                }
                case WorListElementSubtypeHebrewWordlistEntry:
                {
                  if (is_opening_marker) {
                    addToWordList (hebrewWordList);
                  }
                  break;
                }
                case WorListElementSubtypeGreekWordlistEntry:
                {
                  if (is_opening_marker) {
                    addToWordList (greekWordList);
                  }
                  break;
                }
                case WorListElementSubtypeSubjectIndexEntry:
                {
                  if (is_opening_marker) {
                    addToWordList (subjectIndex);
                  }
                  break;
                }
                default:
                {
                  if (is_opening_marker) {
                    addToFallout (R"(Unknown word list marker \)" + marker, false);
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
              addToFallout (R"(Marker not yet implemented \)" + marker + ", possible formatting error:", true);
              break;
            }
          }
        }
        else if (const stylesv2::Style* style {database::styles2::get_marker_data (stylesheet, marker)}; style) { // Todo v2
          switch (style->type) {
            case stylesv2::Type::book_id:
            {
              close_text_style_all();
              // Get book number.
              std::string usfm_id = filter::usfm::get_book_identifier (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              usfm_id = filter::strings::replace (filter::strings::soft_hyphen_u00AD (), "", usfm_id); // Remove possible soft hyphen.
              m_current_book_identifier = static_cast<int>(database::books::get_id_from_usfm (usfm_id));
              // Reset chapter and verse numbers.
              m_current_chapter_number = 0;
              set_to_zero(m_current_verse_number);
              // Throw away whatever follows the \id, e.g. 'GEN xxx xxx'.
              filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              // Whether to insert a new page before the book. But never before the first book.
              if (stylesv2::get_bool_parameter(style, stylesv2::Property::starts_new_page)) {
                if (processed_books_count) {
                  if (odf_text_standard) odf_text_standard->new_page_break ();
                  if (odf_text_text_only) odf_text_text_only->new_page_break ();
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_page_break ();
                  if (html_text_standard) html_text_standard->new_page_break ();
                  if (html_text_linked) html_text_linked->new_page_break ();
                }
              }
              processed_books_count++;
              // Reset notes.
              note_citations.restart("book");
              // Online Bible.
              if (onlinebible_text) onlinebible_text->storeData ();
              // eSword.
              if (esword_text) esword_text->newBook (m_current_book_identifier);
              // The hidden header in the text normally displays in the running header.
              // It does this only when it's the first header on the page.
              // The book starts here.
              // So create a correct hidden header for displaying in the running header.
              std::string runningHeader = database::books::get_english_from_id (static_cast<book_id>(m_current_book_identifier));
              for (const auto& item : runningHeaders) {
                if (item.m_book == m_current_book_identifier) {
                  runningHeader = item.m_value;
                }
              }
              if (odf_text_standard) odf_text_standard->new_heading1 (runningHeader, true);
              if (odf_text_text_only) odf_text_text_only->new_heading1 (runningHeader, true);
              if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_heading1 (runningHeader, true);
              if (odf_text_notes) odf_text_notes->new_heading1 (runningHeader, false);
              // Done.
              break;
            }
            case stylesv2::Type::file_encoding:
            {
              close_text_style_all();
              add_to_info (R"(Text encoding: \)" + marker, true);
              break;
            }
            case stylesv2::Type::remark:
            {
              close_text_style_all();
              add_to_info (R"(Comment: \)" + marker, true);
              break;
            }
            case stylesv2::Type::starting_boundary:
            case stylesv2::Type::none:
            case stylesv2::Type::stopping_boundary:
            default:
              break;
          }
        }
        else {
          // Here is an unknown marker.
          // Add it to the fallout, plus any text that follows the marker.
          addToFallout (R"(Unknown marker \)" + marker + ", formatting error:", true);
        }
      } else {
        // Here is no marker, just text.

        // Treat this content as figure directions.
        if (is_within_figure_markup) {
          // Extract the bits for this image / picture / figure.
          std::string caption, alt, src, size, loc, copy, ref;
          filter::usfm::extract_fig (current_item, caption, alt, src, size, loc, copy, ref);
          // Store the name of this image in the object, ready to be copied into place if needed.
          image_sources.push_back(src);
          // Add the image to the various output formats.
          if (odf_text_standard) odf_text_standard->add_image(figure_marker, alt, src, caption);
          if (odf_text_text_only) odf_text_text_only->add_image(figure_marker, alt, src, caption);
          if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_image(figure_marker, alt, src, caption);
          if (html_text_standard) html_text_standard->add_image(figure_marker, alt, src, caption);
          if (html_text_linked) html_text_linked->add_image(figure_marker, alt, src, caption);
        }

        // Treat this content as text.
        else {
          // Handle situation that verses in poetry are to be left aligned.
          // In such a case, if the OpenDocument paragraph is still empty,
          // output a tab before any text.
          if (odf_text_standard)
            if (odt_left_align_verse_in_poetry_styles)
              if (filter::usfm::is_standard_q_poetry (odf_text_standard->m_current_paragraph_style))
                if (odf_text_standard->m_current_paragraph_content.empty())
                  odf_text_standard->add_tab();
          if (odf_text_text_only)
            if (odt_left_align_verse_in_poetry_styles)
              if (filter::usfm::is_standard_q_poetry (odf_text_text_only->m_current_paragraph_style))
                if (odf_text_text_only->m_current_paragraph_content.empty())
                  odf_text_text_only->add_tab();
          if (odf_text_text_and_note_citations)
            if (odt_left_align_verse_in_poetry_styles)
              if (filter::usfm::is_standard_q_poetry (odf_text_text_and_note_citations->m_current_paragraph_style))
                if (odf_text_text_and_note_citations->m_current_paragraph_content.empty())
                  odf_text_text_and_note_citations->add_tab();
          // Output text as normal.
          if (odf_text_standard) odf_text_standard->add_text (current_item);
          if (odf_text_text_only) odf_text_text_only->add_text (current_item);
          if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (current_item);
          if (html_text_standard) html_text_standard->add_text (current_item);
          if (html_text_linked) html_text_linked->add_text (current_item);
          if (onlinebible_text) onlinebible_text->add_text (current_item);
          if (esword_text) esword_text->add_text (current_item);
          if (text_text) text_text->addtext (current_item);
          if (headings_text_per_verse_active && heading_started) {
            int iverse = filter::strings::convert_to_int (m_current_verse_number);
            verses_headings [iverse].append (current_item);
          }
          if (headings_text_per_verse_active && text_started) {
            int iverse = filter::strings::convert_to_int (m_current_verse_number);
            if (m_verses_text.count (iverse) && !m_verses_text [iverse].empty ()) {
              m_verses_text [iverse].append (current_item);
              actual_verses_paragraph [iverse].append (current_item);
            } else {
              // The verse text straight after the \v starts with certain space type.
              // Replace it with a normal space.
              std::string item = filter::strings::replace (space_type_after_verse, " ", current_item);
              m_verses_text [iverse] = filter::strings::ltrim (item);
              actual_verses_paragraph [iverse] = filter::strings::ltrim (item);
            }
          }
          if (note_open_now) {
            notes_plain_text_buffer.append (current_item);
          }
        }
      }
    }
  }
}



// This function does the processing of the USFM code for one note,
// formatting the document and extracting information.
void Filter_Text::processNote ()
{
  for ( ; chapter_usfm_markers_and_text_pointer < chapter_usfm_markers_and_text.size(); chapter_usfm_markers_and_text_pointer++)
  {
    std::string currentItem = chapter_usfm_markers_and_text[chapter_usfm_markers_and_text_pointer];
    if (filter::usfm::is_usfm_marker (currentItem))
    {
      // Flags about the nature of the marker.
      bool is_opening_marker = filter::usfm::is_opening_marker (currentItem);
      bool isEmbeddedMarker = filter::usfm::is_embedded_marker (currentItem);
      // Clean up the marker, so we remain with the basic version, e.g. 'f'.
      std::string marker = filter::usfm::get_marker (currentItem);
      if (styles.find (marker) != styles.end())
      {
        database::styles1::Item style = styles[marker];
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
                if (is_opening_marker) {
                  ensureNoteParagraphStyle (marker, styles [standard_content_marker_foot_end_note]);
                  std::string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->add_note (citation, marker);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    std::vector <std::string> current_text_styles = odf_text_text_and_note_citations->m_current_text_style;
                    odf_text_text_and_note_citations->m_current_text_style = {"superscript"};
                    odf_text_text_and_note_citations->add_text (citation);
                    odf_text_text_and_note_citations->m_current_text_style = current_text_styles;
                  }
                  // Add space if the paragraph has text already.
                  if (odf_text_notes) {
                    if (odf_text_notes->m_current_paragraph_content != "") {
                      odf_text_notes->add_text (" ");
                    }
                  }
                  // Add the note citation. And a no-break space after it.
                  if (odf_text_notes) odf_text_notes->add_text (citation + filter::strings::non_breaking_space_u00A0());
                  // Open note in the web pages.
                  if (html_text_standard) html_text_standard->add_note (citation, standard_content_marker_foot_end_note);
                  if (html_text_linked) html_text_linked->add_note (citation, standard_content_marker_foot_end_note);
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
                if (is_opening_marker) {
                  ensureNoteParagraphStyle (marker, styles[standard_content_marker_foot_end_note]);
                  std::string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->add_note (citation, marker, true);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    std::vector <std::string> current_text_styles = odf_text_text_and_note_citations->m_current_text_style;
                    odf_text_text_and_note_citations->m_current_text_style = {"superscript"};
                    odf_text_text_and_note_citations->add_text (citation);
                    odf_text_text_and_note_citations->m_current_text_style = current_text_styles;
                  }
                  // Open note in the web page.
                  if (html_text_standard) html_text_standard->add_note (citation, standard_content_marker_foot_end_note, true);
                  if (html_text_linked) html_text_linked->add_note (citation, standard_content_marker_foot_end_note, true);
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
                if (odf_text_standard) odf_text_standard->close_text_style (true, false);
                if (odf_text_notes) odf_text_notes->close_text_style (false, false);
                if (html_text_standard) html_text_standard->close_text_style (true, false);
                if (html_text_linked) html_text_linked->close_text_style (true, false);
                break;
              }
              case FootEndNoteSubtypeContent:
              case FootEndNoteSubtypeContentWithEndmarker:
              {
                if (is_opening_marker) {
                  if (odf_text_standard) odf_text_standard->open_text_style (style, true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->open_text_style (style, false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->open_text_style (style, true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->open_text_style (style, true, isEmbeddedMarker);
                } else {
                  if (odf_text_standard) odf_text_standard->close_text_style (true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->close_text_style (false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->close_text_style (true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->close_text_style (true, isEmbeddedMarker);
                }
                break;
              }
              case FootEndNoteSubtypeParagraph:
              {
                // The style of this is not yet implemented.
                if (odf_text_standard) odf_text_standard->close_text_style (true, false);
                if (odf_text_notes) odf_text_notes->close_text_style (false, false);
                if (html_text_standard) html_text_standard->close_text_style (true, false);
                if (html_text_linked) html_text_linked->close_text_style (true, false);
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
                if (is_opening_marker) {
                  ensureNoteParagraphStyle (marker, styles[standard_content_marker_cross_reference]);
                  std::string citation = getNoteCitation (style);
                  if (odf_text_standard) odf_text_standard->add_note (citation, marker);
                  // Note citation in superscript in the document with text and note citations.
                  if (odf_text_text_and_note_citations) {
                    std::vector <std::string> current_text_styles = odf_text_text_and_note_citations->m_current_text_style;
                    odf_text_text_and_note_citations->m_current_text_style = {"superscript"};
                    odf_text_text_and_note_citations->add_text (citation);
                    odf_text_text_and_note_citations->m_current_text_style = current_text_styles;
                  }
                  // Add a space if the paragraph has text already.
                  if (odf_text_notes) {
                    if (odf_text_notes->m_current_paragraph_content != "") {
                      odf_text_notes->add_text (" ");
                    }
                  }
                  // Add the note citation. And a no-break space (NBSP) after it.
                  if (odf_text_notes) odf_text_notes->add_text (citation + filter::strings::non_breaking_space_u00A0());
                  // Open note in the web page.
                  ensureNoteParagraphStyle (standard_content_marker_cross_reference, styles[standard_content_marker_cross_reference]);
                  if (html_text_standard) html_text_standard->add_note (citation, standard_content_marker_cross_reference);
                  if (html_text_linked) html_text_linked->add_note (citation, standard_content_marker_cross_reference);
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
                if (odf_text_standard) odf_text_standard->close_text_style (true, false);
                if (odf_text_notes) odf_text_notes->close_text_style (false, false);
                if (html_text_standard) html_text_standard->close_text_style (true, false);
                if (html_text_linked) html_text_linked->close_text_style (true, false);
                break;
              }
              case CrossreferenceSubtypeContent:
              case CrossreferenceSubtypeContentWithEndmarker:
              {
                if (is_opening_marker) {
                  if (odf_text_standard) odf_text_standard->open_text_style (style, true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->open_text_style (style, false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->open_text_style (style, true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->open_text_style (style, true, isEmbeddedMarker);
                } else {
                  if (odf_text_standard) odf_text_standard->close_text_style (true, isEmbeddedMarker);
                  if (odf_text_notes) odf_text_notes->close_text_style (false, isEmbeddedMarker);
                  if (html_text_standard) html_text_standard->close_text_style (true, isEmbeddedMarker);
                  if (html_text_linked) html_text_linked->close_text_style (true, isEmbeddedMarker);
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
            addToFallout (R"(Marker not suitable in note context \)" + marker, false);
            break;
          }
        }
      } else {
        // Here is an unknown marker. Add the marker to fallout, plus any text that follows.
        addToFallout (R"(Unknown marker \)" + marker, true);
      }
    } else {
      // Here is no marker. Treat it as text.
      if (odf_text_standard) odf_text_standard->add_note_text (currentItem);
      if (odf_text_notes) odf_text_notes->add_text (currentItem);
      if (html_text_standard) html_text_standard->add_note_text (currentItem);
      if (html_text_linked) html_text_linked->add_note_text (currentItem);
      if (text_text) text_text->addnotetext (currentItem); 
      if (note_open_now) {
        notes_plain_text_buffer.append (currentItem);
      }
    }
  }
  
  noteDone:

  // "Close" the current note, so that any following note text, if malformed,
  // will be added to a new note, not to the last one created.
  if (odf_text_standard) odf_text_standard->close_current_note ();
  if (odf_text_notes) odf_text_notes->close_text_style (false, false);
  if (html_text_standard) html_text_standard->close_current_note ();
  if (html_text_linked) html_text_linked->close_current_note ();
  //if ($this->onlinebible_text) $this->onlinebible_text->close_current_note ();
  if (!notes_plain_text_buffer.empty ()) {
    notes_plain_text.push_back (std::pair (m_current_verse_number, notes_plain_text_buffer));
  }
  note_open_now = false;
  notes_plain_text_buffer.clear ();
}



// This creates and saves the information document.
// It contains formatting information, collected from the USFM code.
// $path: Path to the document.
void Filter_Text::produceInfoDocument (std::string path)
{
  HtmlText information (translate("Information"));

  // Number of chapters per book.
  information.new_heading1 (translate("Number of chapters per book"));
  for (const auto& element : m_number_of_chapters_per_book) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(element.first)) + " => " + std::to_string (element.second);
    information.new_paragraph ();
    information.add_text (line);
  }

  // Running headers.
  information.new_heading1 (translate("Running headers"));
  for (const auto& item : runningHeaders) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }

  // Table of Contents entries.
  information.new_heading1 (translate("Long table of contents entries"));
  for (const auto& item : longTOCs) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }
  information.new_heading1 (translate("Short table of contents entries"));
  for (const auto& item : shortTOCs) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }

  // Book abbreviations.
  information.new_heading1 (translate("Book abbreviations"));
  for (const auto& item : bookAbbreviations) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }

  // Chapter specials.
  information.new_heading1 (translate("Publishing chapter labels"));
  for (const auto& item : chapterLabels) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }
  information.new_heading1 (translate("Publishing alternate chapter numbers"));
  for (const auto& item : publishedChapterMarkers) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }

  // Word lists.
  information.new_heading1 (translate("Word list, glossary, dictionary entries"));
  for (const auto& item : wordListGlossaryDictionary) {
    information.new_paragraph ();
    information.add_text (item);
  }
  information.new_heading1 (translate("Hebrew word list entries"));
  for (const auto& item : hebrewWordList) {
    information.new_paragraph ();
    information.add_text (item);
  }
  information.new_heading1 (translate("Greek word list entries"));
  for (const auto& item : greekWordList) {
    information.new_paragraph ();
    information.add_text (item);
  }
  information.new_heading1 (translate("Subject index entries"));
  for (const auto& item : subjectIndex) {
    information.new_paragraph ();
    information.add_text (item);
  }

  // Other info.
  information.new_heading1 (translate("Other information"));
  for (const auto& item : info) {
    information.new_paragraph ();
    information.add_text (item);
  }

  information.save (path);
}



// This function produces the text of the current passage, e.g.: Genesis 1:1.
// Returns: The passage text
std::string Filter_Text::getCurrentPassageText ()
{
  return filter_passage_display (m_current_book_identifier, m_current_chapter_number, m_current_verse_number);
}



// This function adds a string to the Info array, prefixed by the current passage.
// $text: String to add to the Info array.
// $next: If true, it also adds the text following the marker to the info,
// and removes this text from the USFM input stream.
void Filter_Text::add_to_info (std::string text, bool next)
{
  text = getCurrentPassageText() + " " + text;
  if (next) {
    text.append (" " + filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer));
  }
  info.push_back (text);
}



// This function adds a string to the Fallout array, prefixed by the current passage.
// $text: String to add to the Fallout array.
// $next: If true, it also adds the text following the marker to the fallout,
// and removes this text from the USFM input stream.
void Filter_Text::addToFallout (std::string text, bool next)
{
  text = getCurrentPassageText () + " " + text;
  if (next) {
    text.append (" " + filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer));
  }
  fallout.push_back (text);
}



// This function adds something to a word list array, prefixed by the current passage.
// $list: which list to add the text to.
// The word is extracted from the input USFM. The Usfm pointer points to the current marker,
// and the text following that marker is added to the word list array.
void Filter_Text::addToWordList (std::vector <std::string>  & list)
{
  std::string text = filter::usfm::peek_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
  text.append (" (");
  text.append (getCurrentPassageText ());
  text.append (")");
  list.push_back (text);
}



// This produces and saves the Fallout document.
// $path: Path to the document.
void Filter_Text::produceFalloutDocument (std::string path)
{
  HtmlText html_text (translate("Fallout"));
  html_text.new_heading1 (translate("Fallout"));
  for (std::string line : fallout) {
    html_text.new_paragraph ();
    html_text.add_text (line);
  }
  html_text.save (path);
}


// This function ensures that a certain paragraph style is in the OpenDocument.
// $style: The style to use.
// $keepWithNext: Whether to keep this paragraph with the next one.
void Filter_Text::create_paragraph_style (const database::styles1::Item & style, bool keepWithNext)
{
  std::string marker = style.marker;
  if (find (createdStyles.begin(), createdStyles.end(), marker) == createdStyles.end()) {
    std::string fontname = database::config::bible::get_export_font (m_bible);
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
    if (odf_text_standard) odf_text_standard->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_only) odf_text_text_only->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    createdStyles.push_back (marker);
  }
}


// This function ensures that a certain paragraph style is in the OpenDocument,
// and then opens a paragraph with that style.
// $style: The style to use.
// $keepWithNext: Whether to keep this paragraph with the next one.
void Filter_Text::new_paragraph (const database::styles1::Item & style, bool keepWithNext)
{
  create_paragraph_style(style, keepWithNext);
  std::string marker = style.marker;
  if (odf_text_standard) odf_text_standard->new_paragraph (marker);
  if (odf_text_text_only) odf_text_text_only->new_paragraph (marker);
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->new_paragraph (marker);
  if (html_text_standard) html_text_standard->new_paragraph (marker);
  if (html_text_linked) html_text_linked->new_paragraph (marker);
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
    std::string combined_style = odf_text_standard->m_current_paragraph_style + "_" + chapterMarker + std::to_string (dropCapsLength);
    if (find (createdStyles.begin(), createdStyles.end(), combined_style) == createdStyles.end()) {
      database::styles1::Item style = styles[odf_text_standard->m_current_paragraph_style];
      std::string fontname = database::config::bible::get_export_font (m_bible);
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
      if (odf_text_standard) odf_text_standard->create_paragraph_style (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      if (odf_text_text_only) odf_text_text_only->create_paragraph_style (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->create_paragraph_style (combined_style, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropCapsLength);
      createdStyles.push_back (combined_style);
    }
    if (odf_text_standard) odf_text_standard->update_current_paragraph_style (combined_style);
    if (odf_text_text_only) odf_text_text_only->update_current_paragraph_style (combined_style);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->update_current_paragraph_style (combined_style);
  }
}



// This puts the chapter number in a frame in the current paragraph.
// This is to put the chapter number in a frame so it looks like drop caps in the OpenDocument.
// $chapterText: The text of the chapter indicator to put.
void Filter_Text::putChapterNumberInFrame (std::string chapterText)
{
  database::styles1::Item style = styles[chapterMarker];
  if (odf_text_standard) odf_text_standard->place_text_in_frame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
  if (odf_text_text_only) odf_text_text_only->place_text_in_frame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->place_text_in_frame (chapterText, this->chapterMarker, style.fontsize, style.italic, style.bold);
}



// This gets the note citation.
// The first time that a xref is encountered, this function would return, e.g. 'a'.
// The second time, it would return 'b'. Then 'c', 'd', 'e', and so on, up to 'z'.
// Then it would restart with 'a'. And so on.
// The note citation is the character that is put in superscript in the main body of Bible text.
// $style: array with values for the note opening marker.
// Returns: The character for the note citation.
std::string Filter_Text::getNoteCitation (const database::styles1::Item & style)
{
  bool end_of_text_reached = (chapter_usfm_markers_and_text_pointer + 1) >= chapter_usfm_markers_and_text.size ();
  if (end_of_text_reached) return std::string();

  // Extract the raw note citation from the USFM. This could be, e.g. '+'.
  std::string nextText = chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer + 1];
  std::string citation = nextText.substr (0, 1);
  nextText = filter::strings::ltrim (nextText.substr (1));
  chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer + 1] = nextText;
  citation = filter::strings::trim (citation);
  
  // Get the rendered note citation.
  citation = note_citations.get(style.marker, citation);
  return citation;
}



// This function ensures that a certain paragraph style for a note is present in the OpenDocument.
// $marker: Which note, e.g. 'f' or 'x' or 'fe'.
// $style: The style to use.
void Filter_Text::ensureNoteParagraphStyle (std::string marker, const database::styles1::Item & style)
{
  if (find (createdStyles.begin(), createdStyles.end(), marker) == createdStyles.end()) {
    std::string fontname = database::config::bible::get_export_font (m_bible);
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
    if (odf_text_standard) odf_text_standard->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_only) odf_text_text_only->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, spacebefore, spaceafter, leftmargin, rightmargin, firstlineindent, keepWithNext, dropcaps);
    if (odf_text_notes) odf_text_notes->create_paragraph_style (marker, fontname, fontsize, italic, bold, underline, smallcaps, alignment, 0, 0, 0, 0, 0, keepWithNext, dropcaps);
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


std::map <int, std::string> Filter_Text::getVersesText ()
{
  // Trim white space at start and end of each line.
  for (auto& element : m_verses_text) {
    element.second = filter::strings::trim (element.second);
  }
  // Return the result.
  return m_verses_text;
}


void Filter_Text::store_verses_paragraphs ()
{
  if (!actual_verses_paragraph.empty ()) {
    verses_paragraphs.push_back (actual_verses_paragraph);
    actual_verses_paragraph.clear ();
  }
}


void Filter_Text::notes_plain_text_handler ()
{
  int offset {0};
  const int iverse = filter::strings::convert_to_int (m_current_verse_number);
  if (m_verses_text.count (iverse)) {
    offset = static_cast<int>(m_verses_text [iverse].size ());
  }
  verses_text_note_positions[iverse].push_back (offset);
}


void Filter_Text::set_to_zero (std::string& value)
{
  value = "0";
}


void Filter_Text::close_text_style_all()
{
  if (odf_text_standard)
    odf_text_standard->close_text_style (false, false);
  if (odf_text_text_only)
    odf_text_text_only->close_text_style (false, false);
  if (odf_text_text_and_note_citations)
    odf_text_text_and_note_citations->close_text_style (false, false);
  if (odf_text_notes)
    odf_text_notes->close_text_style (false, false);
  if (html_text_standard)
    html_text_standard->close_text_style (false, false);
  if (html_text_linked)
    html_text_linked->close_text_style (false, false);
}

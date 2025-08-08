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


const std::string chapter_marker {"c"};


Filter_Text::Filter_Text (std::string bible)
{
  m_bible = bible;
  space_type_after_verse = database::config::bible::get_odt_space_after_verse (m_bible);
  odt_left_align_verse_in_poetry_styles = database::config::bible::get_odt_poetry_verses_left (m_bible);
}



Filter_Text::~Filter_Text ()
{
  if (odf_text_standard)
    delete odf_text_standard;
  if (odf_text_text_only)
    delete odf_text_text_only;
  if (odf_text_text_and_note_citations)
    delete odf_text_text_and_note_citations;
  if (odf_text_notes)
    delete odf_text_notes;
  if (html_text_standard)
    delete html_text_standard;
  if (html_text_linked)
    delete html_text_linked;
  if (onlinebible_text)
    delete onlinebible_text;
  if (esword_text)
    delete esword_text;
  if (text_text)
    delete text_text;
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
  // Save sthlesheet.
  m_stylesheet = stylesheet;
  
  // Get the styles.
  get_styles();

  // Preprocess.
  pre_process_usfm();

  // Process data.
  process_usfm ();

  store_verses_paragraphs ();
  
  // Clear USFM and styles.
  m_usfm_markers_and_text.clear();
  usfm_markers_and_text_ptr = 0;
  chapter_usfm_markers_and_text.clear();
  chapter_usfm_markers_and_text_pointer = 0;
  created_styles.clear();
}



// This function returns true when there is still unprocessed USFM code available.
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

  // Load the USFM code till the next chapter marker.
  while (unprocessed_usfm_code_available ()) {
    std::string item = m_usfm_markers_and_text [usfm_markers_and_text_ptr];
    if (!firstLine) {
      if (filter::strings::trim (item) == (R"(\)" + chapter_marker)) {
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
void Filter_Text::get_styles ()
{
  // Get the relevant styles information included.
  if (odf_text_standard)
    odf_text_standard->create_page_break_style ();
  if (odf_text_text_only)
    odf_text_text_only->create_page_break_style ();
  if (odf_text_text_and_note_citations)
    odf_text_text_and_note_citations->create_page_break_style ();
  if (odf_text_text_and_note_citations)
    odf_text_text_and_note_citations->create_superscript_style ();
  for (const stylesv2::Style& style : database::styles::get_styles(m_stylesheet)) {
    if (style.type == stylesv2::Type::note_standard_content)
      standard_content_marker_foot_end_note = style.marker;
    if (style.type == stylesv2::Type::crossreference_standard_content)
      standard_content_marker_cross_reference = style.marker;
  }
}


// This function does the preprocessing of the USFM code
// extracting a variety of information, creating note citations, etc.
void Filter_Text::pre_process_usfm ()
{
  usfm_markers_and_text_ptr = 0;
  while (unprocessed_usfm_code_available ()) {
    get_usfm_next_chapter ();
    for (chapter_usfm_markers_and_text_pointer = 0; chapter_usfm_markers_and_text_pointer < chapter_usfm_markers_and_text.size(); chapter_usfm_markers_and_text_pointer++) {
      std::string current_item = chapter_usfm_markers_and_text[chapter_usfm_markers_and_text_pointer];
      if (filter::usfm::is_usfm_marker (current_item)) {
        std::string marker = filter::strings::trim (current_item); // Change, e.g. '\id ' to '\id'.
        marker = marker.substr (1); // Remove the initial backslash, e.g. '\id' becomes 'id'.
        if (filter::usfm::is_opening_marker (marker)) {
          if (const stylesv2::Style* style {database::styles::get_marker_data (m_stylesheet, marker)}; style) {
            switch (style->type) {
              case stylesv2::Type::starting_boundary:
              case stylesv2::Type::none:
                break;
              case stylesv2::Type::book_id:
              {
                // Get book number.
                std::string usfm_id = filter::usfm::get_book_identifier (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                // Remove possible soft hyphen.
                usfm_id = filter::strings::replace (filter::strings::soft_hyphen_u00AD (), std::string(), usfm_id);
                // Get Bibledit book number.
                m_current_book_identifier = static_cast<int>(database::books::get_id_from_usfm (usfm_id));
                // Reset chapter and verse numbers.
                m_current_chapter_number = 0;
                m_number_of_chapters_per_book[m_current_book_identifier] = 0;
                set_to_zero(m_current_verse_number);
                // Done.
                break;
              }
              case stylesv2::Type::usfm_version:
              case stylesv2::Type::file_encoding:
              case stylesv2::Type::remark:
                break;
              case stylesv2::Type::running_header:
              {
                const std::string running_header = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                runningHeaders.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, running_header));
                break;
              }
              case stylesv2::Type::long_toc_text:
              {
                const std::string long_toc = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                longTOCs.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, long_toc));
                break;
              }
              case stylesv2::Type::short_toc_text:
              {
                const std::string short_toc = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                shortTOCs.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, short_toc));
                break;
              }
              case stylesv2::Type::book_abbrev:
              {
                const std::string book_bbreviation = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                bookAbbreviations.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, book_bbreviation));
                break;
              }
              case stylesv2::Type::introduction_end:
                break;
              case stylesv2::Type::title:
              case stylesv2::Type::heading:
              case stylesv2::Type::paragraph:
              {
                break;
              }
              case stylesv2::Type::chapter:
              {
                const std::string number = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                m_current_chapter_number = filter::strings::convert_to_int (number);
                m_number_of_chapters_per_book[m_current_book_identifier] = m_current_chapter_number;
                set_to_zero(m_current_verse_number);
                break;
              }
              case stylesv2::Type::chapter_label:
              {
                // Store the chapter label for this book and chapter.
                const std::string chapter_label = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                chapter_labels.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, chapter_label));
                // If a chapter label is in the book, there's no drop caps output of the chapter number.
                book_has_chapter_label [m_current_book_identifier] = true;
                // Done.
                break;
              }
              case stylesv2::Type::published_chapter_marker:
              {
                const std::string published_chapter_marker = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                published_chapter_markers.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, published_chapter_marker));
                break;
              }
              case stylesv2::Type::alternate_chapter_number:
              {
                const std::string alternate_chapter_number = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                alternate_chapter_numbers.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, alternate_chapter_number));
                break;
              }
              case stylesv2::Type::verse:
              {
                const std::string fragment = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                const int number = filter::strings::convert_to_int (fragment);
                m_current_verse_number = std::to_string (number);
                break;
              }
              case stylesv2::Type::published_verse_marker:
              {
                // It gets the published verse markup.
                // The marker looks like: ... \vp ၁။\vp* ...
                // It stores this markup in the object for later reference.
                const std::string published_verse_marker = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
                published_verse_markers.push_back (filter::text::passage_marker_value (m_current_book_identifier, m_current_chapter_number, m_current_verse_number, marker, published_verse_marker));
                break;
              }
              case stylesv2::Type::alternate_verse_marker:
                break;
              case stylesv2::Type::table_row:
              case stylesv2::Type::table_heading:
              case stylesv2::Type::table_cell:
                break;
              case stylesv2::Type::footnote_wrapper:
              case stylesv2::Type::endnote_wrapper:
                note_citations.evaluate_style(*style);
                break;
              case stylesv2::Type::note_standard_content:
              case stylesv2::Type::note_content:
              case stylesv2::Type::note_content_with_endmarker:
              case stylesv2::Type::note_paragraph:
                break;
              case stylesv2::Type::crossreference_wrapper:
                note_citations.evaluate_style(*style);
                break;
              case stylesv2::Type::crossreference_standard_content:
              case stylesv2::Type::crossreference_content:
              case stylesv2::Type::crossreference_content_with_endmarker:
                break;
              case stylesv2::Type::character:
                break;
              case stylesv2::Type::page_break:
                break;
              case stylesv2::Type::figure:
                break;
              case stylesv2::Type::word_list:
                break;
              case stylesv2::Type::sidebar_begin:
              case stylesv2::Type::sidebar_end:
                break;
              case stylesv2::Type::peripheral:
              case stylesv2::Type::milestone:
                break;
              case stylesv2::Type::stopping_boundary:
              default:
                break;
            }
            if (stylesv2::has_property(style,stylesv2::Property::deprecated)) {
              add_to_info (R"(Deprecated marker: \)" + marker, false);
            }
          }
        }
      }
    }
  }
}



// This function does the processing of the USFM code,
// formatting the document and extracting other useful information.
void Filter_Text::process_usfm ()
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
        if (is_opening_marker)
          filter::usfm::remove_word_level_attributes (marker, chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
        if (const stylesv2::Style* style {database::styles::get_marker_data (m_stylesheet, marker)}; style) {
          switch (style->type) {
            case stylesv2::Type::starting_boundary:
            case stylesv2::Type::none:
              break;
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
              if (stylesv2::get_parameter<bool>(style, stylesv2::Property::starts_new_page)) {
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
              note_citations.restart(stylesv2::notes_numbering_restart_book);
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
            case stylesv2::Type::usfm_version:
            {
              close_text_style_all();
              add_to_info (R"(USFM version: \)" + marker, true);
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
            case stylesv2::Type::running_header:
              // The running header has properties about wheter to output the running header,
              // on the left page, on the right page, or on both.
              // This has not been implemented here.
              // It were better if this had been implemented.
            case stylesv2::Type::long_toc_text:
            case stylesv2::Type::short_toc_text:
            case stylesv2::Type::book_abbrev:
            {
              close_text_style_all();
              // This information already is preprocessed. Remove it from the USFM stream.
              filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              break;
            }
            case stylesv2::Type::introduction_end:
            {
              close_text_style_all();
              add_to_info (R"(Introduction end: \)" + marker, false);
              break;
            }
            case stylesv2::Type::title:
            case stylesv2::Type::heading:
            {
              if (is_opening_marker) {
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
                new_paragraph (style, true);
                heading_started = true;
                text_started = false;
              }
              break;
            }
            case stylesv2::Type::paragraph:
            {
              if (is_opening_marker) {
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
                  paragraph_starting_markers.push_back (marker);
                  // Store previous paragraph, if any, and start recording the new one.
                  store_verses_paragraphs ();
                }
              }
              break;
            }
            case stylesv2::Type::chapter:
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
              if (onlinebible_text)
                onlinebible_text->storeData ();
              
              // Get the chapter number.
              std::string usfm_c_fragment = filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              
              // Update this object.
              m_current_chapter_number = filter::strings::convert_to_int (usfm_c_fragment);
              set_to_zero(m_current_verse_number);
              
              // If there is a published chapter character, the chapter number takes that value.
              for (const auto& published_chapter_marker : published_chapter_markers) {
                if (published_chapter_marker.m_book == m_current_book_identifier) {
                  if (published_chapter_marker.m_chapter == m_current_chapter_number) {
                    usfm_c_fragment = published_chapter_marker.m_value;
                  }
                }
              }

              // If there's an alternate chapter number, append this to the chapter number fragment.
              for (const auto& alternate_chapter_number : alternate_chapter_numbers) {
                if (alternate_chapter_number.m_book == m_current_book_identifier) {
                  if (alternate_chapter_number.m_chapter == m_current_chapter_number) {
                    usfm_c_fragment.append(" (");
                    usfm_c_fragment.append (alternate_chapter_number.m_value);
                    usfm_c_fragment.append(")");
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
              running_header.append(" ");
              running_header.append(usfm_c_fragment);
              if (odf_text_standard)
                odf_text_standard->new_heading1 (running_header, true);
              if (odf_text_text_only)
                odf_text_text_only->new_heading1 (running_header, true);
              if (odf_text_text_and_note_citations)
                odf_text_text_and_note_citations->new_heading1 (running_header, true);
              if (odf_text_notes)
                odf_text_notes->new_heading1 (running_header, false);
              
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
                if (stylesv2::get_parameter<bool>(style, stylesv2::Property::at_first_verse) && !cl_found) {
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
                  for (const auto& chapter_label : chapter_labels) {
                    if (chapter_label.m_book == m_current_book_identifier) {
                      if (chapter_label.m_chapter == 0) {
                        labelEntireBook = chapter_label.m_value;
                      }
                      if (chapter_label.m_chapter == m_current_chapter_number) {
                        labelCurrentChapter = chapter_label.m_value;
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
                  if (odf_text_standard)
                    odf_text_standard->add_text (usfm_c_fragment);
                  if (odf_text_text_only)
                    odf_text_text_only->add_text (usfm_c_fragment);
                  if (odf_text_text_and_note_citations)
                    odf_text_text_and_note_citations->add_text (usfm_c_fragment);
                  if (html_text_standard)
                    html_text_standard->add_text (usfm_c_fragment);
                  if (html_text_linked)
                    html_text_linked->add_text (usfm_c_fragment);
                }
              }
              
              // Output chapter number for other formats.
              if (esword_text)
                esword_text->newChapter (m_current_chapter_number);
              
              // Open a paragraph for the notes.
              // It takes the style of the footnote content marker, usually 'ft'.
              // This is done specifically for the version that has the notes only.
              const stylesv2::Style* ft_style = database::styles::get_marker_data(m_stylesheet, standard_content_marker_foot_end_note);
              ensure_note_paragraph_style (standard_content_marker_foot_end_note, ft_style);
              if (odf_text_notes)
                odf_text_notes->new_paragraph (standard_content_marker_foot_end_note);
              // Property::on_left_page -> no headings implemented yet.
              // Property::on_right_page -> no headings implemented yet.
              
              // Reset.
              note_citations.restart(stylesv2::notes_numbering_restart_chapter);
              
              // Done.
              break;
            }

            case stylesv2::Type::chapter_label:
            case stylesv2::Type::published_chapter_marker:
            {
              close_text_style_all();
              // This information already is preprocessed. Remove it from the USFM stream.
              filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              break;
            }
            case stylesv2::Type::alternate_chapter_number:
            {
              close_text_style_all();
              if (is_opening_marker) {
                // This information is already in the object.
                // Remove it from the USFM stream at the opening marker.
                filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              }
              // Do nothing with the closing marker.
            }
            case stylesv2::Type::verse:
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
              if (onlinebible_text)
                onlinebible_text->storeData ();
              // Handle a situation that a verse number starts a new paragraph.
              if (stylesv2::get_parameter<bool>(style, stylesv2::Property::restart_paragraph)) {
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
                  const int drop_caps_length = static_cast<int>( filter::strings::unicode_string_length (m_output_chapter_text_at_first_verse));
                  apply_drop_caps_to_current_paragraph (drop_caps_length);
                  if (odf_text_standard) odf_text_standard->add_text (m_output_chapter_text_at_first_verse);
                  if (odf_text_text_only) odf_text_text_only->add_text (m_output_chapter_text_at_first_verse);
                  if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (m_output_chapter_text_at_first_verse);
                } else {
                  put_chapter_number_in_frame (m_output_chapter_text_at_first_verse);
                }
                stylesv2::Style dropcaps_style{};
                dropcaps_style.marker = "dropcaps";
                if (html_text_standard) html_text_standard->open_text_style (&dropcaps_style, false, false);
                if (html_text_standard) html_text_standard->add_text (m_output_chapter_text_at_first_verse);
                if (html_text_standard) html_text_standard->close_text_style (false, false);
                if (html_text_linked) html_text_linked->open_text_style (&dropcaps_style, false, false);
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
              for (const auto& publishedVerseMarker : published_verse_markers) {
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
                if (odf_text_standard)
                  odf_text_standard->open_text_style (style, false, false);
                if (odf_text_text_only)
                  odf_text_text_only->open_text_style (style, false, false);
                if (odf_text_text_and_note_citations)
                  odf_text_text_and_note_citations->open_text_style (style, false, false);
                if (html_text_standard)
                  html_text_standard->open_text_style (style, false, false);
                if (html_text_linked)
                  html_text_linked->open_text_style (style, false, false);
                if (odf_text_standard)
                  odf_text_standard->add_text (v_vp_number);
                if (odf_text_text_only)
                  odf_text_text_only->add_text (v_vp_number);
                if (odf_text_text_and_note_citations)
                  odf_text_text_and_note_citations->add_text (v_vp_number);
                if (html_text_standard)
                  html_text_standard->add_text (v_vp_number);
                if (html_text_linked)
                  html_text_linked->add_text (v_vp_number);
                if (odf_text_standard)
                  odf_text_standard->close_text_style (false, false);
                if (odf_text_text_only)
                  odf_text_text_only->close_text_style (false, false);
                if (odf_text_text_and_note_citations)
                  odf_text_text_and_note_citations->close_text_style (false, false);
                if (html_text_standard)
                  html_text_standard->close_text_style (false, false);
                if (html_text_linked)
                  html_text_linked->close_text_style (false, false);
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
            case stylesv2::Type::published_verse_marker:
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
            case stylesv2::Type::alternate_verse_marker:
            {
              if (is_opening_marker) {
                add_to_info("Alternate verse marker:", true);
              }
              break;
            }
            case stylesv2::Type::table_row:
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
              add_to_fallout ("Table elements not implemented", false);
              break;
            }
            case stylesv2::Type::table_heading:
            case stylesv2::Type::table_cell:
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
              new_paragraph (style, false);
              break;
            }
              
            case stylesv2::Type::footnote_wrapper:
            case stylesv2::Type::endnote_wrapper:
            case stylesv2::Type::note_standard_content:
            case stylesv2::Type::note_content:
            case stylesv2::Type::note_content_with_endmarker:
            case stylesv2::Type::note_paragraph:
            {
              processNote ();
              break;
            }

            case stylesv2::Type::crossreference_wrapper:
            case stylesv2::Type::crossreference_standard_content:
            case stylesv2::Type::crossreference_content:
            case stylesv2::Type::crossreference_content_with_endmarker:
            {
              processNote ();
              break;
            }
              
            case stylesv2::Type::character:
            {
              // Support for a normal and an embedded character style.
              if (is_opening_marker) {
                if (odf_text_standard)
                  odf_text_standard->open_text_style (style, false, is_embedded_marker);
                if (odf_text_text_only)
                  odf_text_text_only->open_text_style (style, false, is_embedded_marker);
                if (odf_text_text_and_note_citations)
                  odf_text_text_and_note_citations->open_text_style (style, false, is_embedded_marker);
                if (html_text_standard)
                  html_text_standard->open_text_style (style, false, is_embedded_marker);
                if (html_text_linked)
                  html_text_linked->open_text_style (style, false, is_embedded_marker);
              } else {
                if (odf_text_standard)
                  odf_text_standard->close_text_style (false, is_embedded_marker);
                if (odf_text_text_only)
                  odf_text_text_only->close_text_style (false, is_embedded_marker);
                if (odf_text_text_and_note_citations)
                  odf_text_text_and_note_citations->close_text_style (false, is_embedded_marker);
                if (html_text_standard)
                  html_text_standard->close_text_style (false, is_embedded_marker);
                if (html_text_linked)
                  html_text_linked->close_text_style (false, is_embedded_marker);
              }
              break;
            }
            case stylesv2::Type::page_break:
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
              if (odf_text_standard)
                odf_text_standard->new_page_break ();
              if (odf_text_text_only)
                odf_text_text_only->new_page_break ();
              if (odf_text_text_and_note_citations)
                odf_text_text_and_note_citations->new_page_break ();
              if (html_text_standard)
                html_text_standard->new_page_break ();
              if (html_text_linked)
                html_text_linked->new_page_break ();
              if (text_text)
                text_text->paragraph ();
              break;
            }
            case stylesv2::Type::figure:
            {
              if (is_opening_marker) {
                // Set a flag that the parser is going to be within figure markup and save the style.
                is_within_figure_markup = true;
                figure_marker = marker;
                // Create the style for the figure because it is used within the ODT generator.
                create_paragraph_style (style, false);
                // At the start of the \fig marker, close all text styles that might be open.
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
              } else {
                // Closing the \fig* markup.
                // Clear the flag since the parser is no longer within figure markup.
                is_within_figure_markup = false;
              }
              break;
            }
            case stylesv2::Type::word_list:
            {
              if (is_opening_marker) {
                add_to_word_list (marker);
              }
              break;
            }
            case stylesv2::Type::sidebar_begin:
            case stylesv2::Type::sidebar_end:
            {
              close_text_style_all();
              add_to_info (R"(Sidebar marker: \)" + marker, false);
              break;
            }
            case stylesv2::Type::peripheral:
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
              add_to_info(R"(Pheripheral markup: \)" + marker, true);
              // To start peripheral material on a new page.
              // https://ubsicap.github.io/usfm/peripherals/index.html
              if (odf_text_standard)
                odf_text_standard->new_page_break ();
              if (odf_text_text_only)
                odf_text_text_only->new_page_break ();
              if (odf_text_text_and_note_citations)
                odf_text_text_and_note_citations->new_page_break ();
              if (html_text_standard)
                html_text_standard->new_page_break ();
              if (html_text_linked)
                html_text_linked->new_page_break ();
              break;
            }
            case stylesv2::Type::milestone:
            {
              const std::string data = filter::usfm::remove_milestone (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
              if (!data.empty()) {
                add_to_info(R"(Milestone: )" + data, false);
              }
              break;
            }
            case stylesv2::Type::stopping_boundary:
            default:
              break;
          }
        }
        else {
          // Here is an unknown marker.
          // Add it to the fallout, plus any text that follows the marker.
          add_to_fallout (R"(Unknown marker \)" + marker + ", formatting error:", true);
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
          const std::string text_item {handle_tilde_and_double_slash(current_item)};
          if (odf_text_standard) odf_text_standard->add_text (text_item);
          if (odf_text_text_only) odf_text_text_only->add_text (text_item);
          if (odf_text_text_and_note_citations) odf_text_text_and_note_citations->add_text (text_item);
          if (html_text_standard) html_text_standard->add_text (text_item);
          if (html_text_linked) html_text_linked->add_text (text_item);
          if (onlinebible_text) onlinebible_text->add_text (text_item);
          if (esword_text) esword_text->add_text (text_item);
          if (text_text) text_text->addtext (text_item);
          if (headings_text_per_verse_active && heading_started) {
            int iverse = filter::strings::convert_to_int (m_current_verse_number);
            verses_headings [iverse].append (text_item);
          }
          if (headings_text_per_verse_active && text_started) {
            int iverse = filter::strings::convert_to_int (m_current_verse_number);
            if (m_verses_text.count (iverse) && !m_verses_text [iverse].empty ()) {
              m_verses_text [iverse].append (text_item);
              actual_verses_paragraph [iverse].append (text_item);
            } else {
              // The verse text straight after the \v starts with certain space type.
              // Replace it with a normal space.
              std::string item = filter::strings::replace (space_type_after_verse, " ", text_item);
              m_verses_text [iverse] = filter::strings::ltrim (item);
              actual_verses_paragraph [iverse] = filter::strings::ltrim (item);
            }
          }
          if (note_open_now) {
            notes_plain_text_buffer.append (text_item);
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
    const std::string current_item = chapter_usfm_markers_and_text[chapter_usfm_markers_and_text_pointer];
    if (filter::usfm::is_usfm_marker (current_item))
    {
      // Flags about the nature of the marker.
      bool is_opening_marker = filter::usfm::is_opening_marker (current_item);
      bool isEmbeddedMarker = filter::usfm::is_embedded_marker (current_item);
      // Clean up the marker, so we remain with the basic version, e.g. 'f'.
      const std::string marker = filter::usfm::get_marker (current_item);
      if (const stylesv2::Style* stylev2 {database::styles::get_marker_data (m_stylesheet, marker)}; stylev2)
      {
        switch (stylev2->type) {
          case stylesv2::Type::starting_boundary:
          case stylesv2::Type::none:
          case stylesv2::Type::book_id:
          case stylesv2::Type::usfm_version:
          case stylesv2::Type::file_encoding:
          case stylesv2::Type::remark:
          case stylesv2::Type::running_header:
          case stylesv2::Type::long_toc_text:
          case stylesv2::Type::short_toc_text:
          case stylesv2::Type::book_abbrev:
          case stylesv2::Type::introduction_end:
          case stylesv2::Type::title:
          case stylesv2::Type::heading:
          case stylesv2::Type::paragraph:
          case stylesv2::Type::chapter:
          case stylesv2::Type::chapter_label:
          case stylesv2::Type::published_chapter_marker:
          case stylesv2::Type::alternate_chapter_number:
            break;
          case stylesv2::Type::verse:
          {
            // Verse found. The note should have stopped here. Incorrect note markup.
            add_to_fallout ("The note did not close at the end of the verse. The text is not correct.", false);
            goto noteDone;
            break;
          }
          case stylesv2::Type::published_verse_marker:
          case stylesv2::Type::alternate_verse_marker:
          case stylesv2::Type::table_row:
          case stylesv2::Type::table_heading:
          case stylesv2::Type::table_cell:
            break;
          case stylesv2::Type::footnote_wrapper:
          {
            if (is_opening_marker) {
              const stylesv2::Style* ft_style = database::styles::get_marker_data(m_stylesheet, standard_content_marker_foot_end_note);
              ensure_note_paragraph_style (marker, ft_style);
              const std::string citation = get_note_citation (marker);
              if (odf_text_standard)
                odf_text_standard->add_note (citation, marker);
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
              if (odf_text_notes)
                odf_text_notes->add_text (citation + filter::strings::non_breaking_space_u00A0());
              // Open note in the web pages.
              if (html_text_standard)
                html_text_standard->add_note (citation, standard_content_marker_foot_end_note);
              if (html_text_linked)
                html_text_linked->add_note (citation, standard_content_marker_foot_end_note);
              // Online Bible. Footnotes do not seem to behave as they ought in the Online Bible compiler. Just leave them out.
              //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
              if (text_text)
                text_text->note ();
              // Handle opening notes in plain text.
              notes_plain_text_handler ();
              // Set flag.
              note_open_now = true;
            } else {
              goto noteDone;
            }
            break;
          }
          case stylesv2::Type::endnote_wrapper:
          {
            if (is_opening_marker) {
              const stylesv2::Style* ft_style = database::styles::get_marker_data(m_stylesheet, standard_content_marker_foot_end_note);
              ensure_note_paragraph_style (marker, ft_style);
              const std::string citation = get_note_citation (marker);
              if (odf_text_standard)
                odf_text_standard->add_note (citation, marker, true);
              // Note citation in superscript in the document with text and note citations.
              if (odf_text_text_and_note_citations) {
                std::vector <std::string> current_text_styles = odf_text_text_and_note_citations->m_current_text_style;
                odf_text_text_and_note_citations->m_current_text_style = {"superscript"};
                odf_text_text_and_note_citations->add_text (citation);
                odf_text_text_and_note_citations->m_current_text_style = current_text_styles;
              }
              // Open note in the web page.
              if (html_text_standard)
                html_text_standard->add_note (citation, standard_content_marker_foot_end_note, true);
              if (html_text_linked)
                html_text_linked->add_note (citation, standard_content_marker_foot_end_note, true);
              // Online Bible: Leave note out.
              //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
              if (text_text)
                text_text->note ();
              // Handle opening notes in plain text.
              notes_plain_text_handler ();
              // Set flag.
              note_open_now = true;
            } else {
              goto noteDone;
            }
            break;
          }
          case stylesv2::Type::note_standard_content:
          {
            // The style of the standard content is already used in the note's body.
            // If means that the text style should be cleared
            // in order to return to the correct style for the paragraph.
            if (odf_text_standard)
              odf_text_standard->close_text_style (true, false);
            if (odf_text_notes)
              odf_text_notes->close_text_style (false, false);
            if (html_text_standard)
              html_text_standard->close_text_style (true, false);
            if (html_text_linked)
              html_text_linked->close_text_style (true, false);
            break;
          }
          case stylesv2::Type::note_content:
          case stylesv2::Type::note_content_with_endmarker:
          {
            if (is_opening_marker) {
              if (odf_text_standard)
                odf_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (odf_text_notes)
                odf_text_notes->open_text_style (stylev2, false, isEmbeddedMarker);
              if (html_text_standard)
                html_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (html_text_linked)
                html_text_linked->open_text_style (stylev2, true, isEmbeddedMarker);
            } else {
              if (odf_text_standard)
                odf_text_standard->close_text_style (true, isEmbeddedMarker);
              if (odf_text_notes)
                odf_text_notes->close_text_style (false, isEmbeddedMarker);
              if (html_text_standard)
                html_text_standard->close_text_style (true, isEmbeddedMarker);
              if (html_text_linked)
                html_text_linked->close_text_style (true, isEmbeddedMarker);
            }
            break;
          }
          case stylesv2::Type::note_paragraph:
          {
            // The style of this is not yet implemented properly: It does not yet open a new paragraph.
            if (is_opening_marker) {
              if (odf_text_standard)
                odf_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (odf_text_notes)
                odf_text_notes->open_text_style (stylev2, false, isEmbeddedMarker);
              if (html_text_standard)
                html_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (html_text_linked)
                html_text_linked->open_text_style (stylev2, true, isEmbeddedMarker);
            } else {
              if (odf_text_standard)
                odf_text_standard->close_text_style (true, false);
              if (odf_text_notes)
                odf_text_notes->close_text_style (false, false);
              if (html_text_standard)
                html_text_standard->close_text_style (true, false);
              if (html_text_linked)
                html_text_linked->close_text_style (true, false);
//              if (text_text)
//                text_text->note ();
            }
            break;
          }
            
          case stylesv2::Type::crossreference_wrapper:
          {
            if (is_opening_marker) {
              const stylesv2::Style* xt_style = database::styles::get_marker_data(m_stylesheet, standard_content_marker_cross_reference);
              ensure_note_paragraph_style (marker, xt_style);
              std::string citation = get_note_citation (stylev2->marker);
              if (odf_text_standard)
                odf_text_standard->add_note (citation, marker);
              // Note citation in superscript in the document with text and note citations.
              if (odf_text_text_and_note_citations) {
                std::vector <std::string> current_text_styles = odf_text_text_and_note_citations->m_current_text_style;
                odf_text_text_and_note_citations->m_current_text_style = {"superscript"};
                odf_text_text_and_note_citations->add_text (citation);
                odf_text_text_and_note_citations->m_current_text_style = current_text_styles;
              }
              // Add a space if the paragraph has text already.
              if (odf_text_notes) {
                if (!odf_text_notes->m_current_paragraph_content.empty()) {
                  odf_text_notes->add_text (" ");
                }
              }
              // Add the note citation. And a no-break space (NBSP) after it.
              if (odf_text_notes)
                odf_text_notes->add_text (citation + filter::strings::non_breaking_space_u00A0());
              // Open note in the web page.
              ensure_note_paragraph_style (standard_content_marker_cross_reference, xt_style);
              if (html_text_standard)
                html_text_standard->add_note (citation, standard_content_marker_cross_reference);
              if (html_text_linked)
                html_text_linked->add_note (citation, standard_content_marker_cross_reference);
              // Online Bible: Skip notes.
              //if ($this->onlinebible_text) $this->onlinebible_text->addNote ();
              if (text_text)
                text_text->note ();
              // Handle opening notes in plain text.
              notes_plain_text_handler ();
              // Set flag.
              note_open_now = true;
            } else {
              goto noteDone;
            }
            break;
          }
          case stylesv2::Type::crossreference_standard_content:
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
          case stylesv2::Type::crossreference_content:
          case stylesv2::Type::crossreference_content_with_endmarker:
          {
            if (is_opening_marker) {
              if (odf_text_standard)
                odf_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (odf_text_notes)
                odf_text_notes->open_text_style (stylev2, false, isEmbeddedMarker);
              if (html_text_standard)
                html_text_standard->open_text_style (stylev2, true, isEmbeddedMarker);
              if (html_text_linked)
                html_text_linked->open_text_style (stylev2, true, isEmbeddedMarker);
            } else {
              if (odf_text_standard)
                odf_text_standard->close_text_style (true, isEmbeddedMarker);
              if (odf_text_notes)
                odf_text_notes->close_text_style (false, isEmbeddedMarker);
              if (html_text_standard)
                html_text_standard->close_text_style (true, isEmbeddedMarker);
              if (html_text_linked)
                html_text_linked->close_text_style (true, isEmbeddedMarker);
            }
            break;
          }
            
          case stylesv2::Type::character:
          case stylesv2::Type::page_break:
          case stylesv2::Type::figure:
          case stylesv2::Type::word_list:
          case stylesv2::Type::sidebar_begin:
          case stylesv2::Type::sidebar_end:
          case stylesv2::Type::peripheral:
          case stylesv2::Type::milestone:
          case stylesv2::Type::stopping_boundary:
          default:
            break;
        }
        
      }
      else {
        // Here is an unknown marker. Add the marker to fallout, plus any text that follows.
        add_to_fallout (R"(Unknown marker \)" + marker, true);
      }
    } else {
      // Here is no marker. Treat it as text.
      const std::string text_item {handle_tilde_and_double_slash(current_item)};
      if (odf_text_standard)
        odf_text_standard->add_note_text (text_item);
      if (odf_text_notes)
        odf_text_notes->add_text (text_item);
      if (html_text_standard)
        html_text_standard->add_note_text (text_item);
      if (html_text_linked)
        html_text_linked->add_note_text (text_item);
      if (text_text)
        text_text->addnotetext (text_item);
      if (note_open_now) {
        notes_plain_text_buffer.append (text_item);
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
void Filter_Text::produce_info_document (std::string path)
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
  for (const auto& item : chapter_labels) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }
  information.new_heading1 (translate("Publishing chapter markers"));
  for (const auto& item : published_chapter_markers) {
    const std::string line = database::books::get_english_from_id (static_cast<book_id>(item.m_book)) + " (USFM " + item.m_marker + ") => " + item.m_value;
    information.new_paragraph ();
    information.add_text (line);
  }

  // Output the word lists.
  for (const auto& word_list : word_lists) {
    const std::string& marker = word_list.first;
    information.new_heading1 (translate("Word list for marker") + " " + marker);
    const std::vector<std::string>& words = word_list.second;
    for (const auto& item : words) {
      information.new_paragraph ();
      information.add_text (item);
    }
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
std::string Filter_Text::get_current_passage_text ()
{
  return filter_passage_display (m_current_book_identifier, m_current_chapter_number, m_current_verse_number);
}



// This function adds a string to the Info array, prefixed by the current passage.
// $text: String to add to the Info array.
// $next: If true, it also adds the text following the marker to the info,
// and removes this text from the USFM input stream.
void Filter_Text::add_to_info (std::string text, bool next)
{
  text = get_current_passage_text() + " " + text;
  if (next) {
    text.append (" " + filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer));
  }
  info.push_back (text);
}



// This function adds a string to the Fallout array, prefixed by the current passage.
// $text: String to add to the Fallout array.
// $next: If true, it also adds the text following the marker to the fallout,
// and removes this text from the USFM input stream.
void Filter_Text::add_to_fallout (std::string text, bool next)
{
  text = get_current_passage_text () + " " + text;
  if (next) {
    text.append (" " + filter::usfm::get_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer));
  }
  fallout.push_back (text);
}



// This function adds something to a word list array, followed by the current passage.
// $marker: Which list to add the text to.
// The word is extracted from the input USFM. The USFM input pointer points to the current marker,
// and the text following that marker is added to the word list array.
void Filter_Text::add_to_word_list (const std::string& marker)
{
  std::string text = filter::usfm::peek_text_following_marker (chapter_usfm_markers_and_text, chapter_usfm_markers_and_text_pointer);
  text.append (" (");
  text.append (get_current_passage_text ());
  text.append (")");
  word_lists[marker].push_back (std::move(text));
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
void Filter_Text::create_paragraph_style (const stylesv2::Style* style, bool keep_with_next)
{
  const std::string marker = style->marker;
  if (find (created_styles.begin(), created_styles.end(), marker) == created_styles.end()) {
    const std::string font_name = database::config::bible::get_export_font (m_bible);
    if (style->paragraph) {
      const auto& paragraph = style->paragraph.value();
      const float font_size = paragraph.font_size;
      const stylesv2::TwoState italic = paragraph.italic;
      const stylesv2::TwoState bold = paragraph.bold;
      const stylesv2::TwoState underline = paragraph.underline;
      const stylesv2::TwoState smallcaps = paragraph.smallcaps;
      const stylesv2::TextAlignment text_alignment = paragraph.text_alignment;
      const float space_before = paragraph.space_before;
      const float space_after = paragraph.space_after;
      const float left_margin = paragraph.left_margin;
      const float right_margin = paragraph.right_margin;
      const float first_line_indent = paragraph.first_line_indent;
      // Columns are not implemented at present. Reason:
      // Copying and pasting sections with columns between documents in LibreOffice failed to work.
      // int spancolumns = style.spancolumns;
      constexpr const int drop_caps {0};
      if (odf_text_standard)
        odf_text_standard->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps);
      if (odf_text_text_only)
        odf_text_text_only->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps);
      if (odf_text_text_and_note_citations)
        odf_text_text_and_note_citations->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps);
    }
    created_styles.push_back (marker);
  }
}


// This function ensures that a certain paragraph style is in the OpenDocument,
// and then opens a paragraph with that style.
// $style: The style to use.
// $keep_with_next: Whether to keep this paragraph with the next one.
void Filter_Text::new_paragraph (const stylesv2::Style* style, bool keep_with_next)
{
  create_paragraph_style(style, keep_with_next);
  if (odf_text_standard)
    odf_text_standard->new_paragraph (style->marker);
  if (odf_text_text_only)
    odf_text_text_only->new_paragraph (style->marker);
  if (odf_text_text_and_note_citations)
    odf_text_text_and_note_citations->new_paragraph (style->marker);
  if (html_text_standard)
    html_text_standard->new_paragraph (style->marker);
  if (html_text_linked)
    html_text_linked->new_paragraph (style->marker);
  if (text_text)
    text_text->paragraph ();
}



// This applies the drop caps setting to the current paragraph style.
// This is for the chapter number to appear in drop caps in the OpenDocument.
// $dropCapsLength: Number of characters to put in drop caps.
void Filter_Text::apply_drop_caps_to_current_paragraph (int drop_caps_length)
{
  // To name a style according to the number of characters to put in drop caps,
  // e.g. a style name like p_c1 or p_c2 or p_c3.
  if (odf_text_standard) {
    std::string combined_style = odf_text_standard->m_current_paragraph_style + "_" + chapter_marker + std::to_string (drop_caps_length);
    if (find (created_styles.begin(), created_styles.end(), combined_style) == created_styles.end()) {
      const stylesv2::Style* style = database::styles::get_marker_data(m_stylesheet, odf_text_standard->m_current_paragraph_style);
      if (!style)
        return;
      if (!style->paragraph)
        return;
      const std::string font_name = database::config::bible::get_export_font (m_bible);
      const float font_size = style->paragraph.value().font_size;
      const auto italic = style->paragraph.value().italic;
      const auto bold = style->paragraph.value().bold;
      const auto underline = style->paragraph.value().underline;
      const auto smallcaps = style->paragraph.value().smallcaps;
      const auto text_alignment = style->paragraph.value().text_alignment;
      const float space_before = style->paragraph.value().space_before;
      const float space_after = style->paragraph.value().space_after;
      const float left_margin = style->paragraph.value().left_margin;
      const float right_margin = style->paragraph.value().right_margin;
      // First line that contains the chapter number in drop caps is not indented.
      constexpr float first_line_indent {0};
      //int spancolumns = style.spancolumns;
      constexpr bool keep_with_next {false};
      if (odf_text_standard)
        odf_text_standard->create_paragraph_style (combined_style, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps_length);
      if (odf_text_text_only)
        odf_text_text_only->create_paragraph_style (combined_style, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps_length);
      if (odf_text_text_and_note_citations)
        odf_text_text_and_note_citations->create_paragraph_style (combined_style, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, drop_caps_length);
      created_styles.push_back (combined_style);
    }
    if (odf_text_standard)
      odf_text_standard->update_current_paragraph_style (combined_style);
    if (odf_text_text_only)
      odf_text_text_only->update_current_paragraph_style (combined_style);
    if (odf_text_text_and_note_citations)
      odf_text_text_and_note_citations->update_current_paragraph_style (combined_style);
  }
}



// This puts the chapter number in a frame in the current paragraph.
// This is to put the chapter number in a frame so it looks like drop caps in the OpenDocument.
// $chapterText: The text of the chapter indicator to put.
void Filter_Text::put_chapter_number_in_frame (std::string chapter_text)
{
  // Get the chapter marker, that is \c.
  const stylesv2::Style* style {database::styles::get_marker_data (m_stylesheet, chapter_marker)};
  // In the unlikely case the chapter style is not valid, take defaults as fallback options.
  const float font_size = style->paragraph ? style->paragraph.value().font_size : 12;
  const auto italic = style->paragraph ? style->paragraph.value().italic : stylesv2::TwoState::off;
  const auto bold = style->paragraph ? style->paragraph.value().bold : stylesv2::TwoState::off;
  if (odf_text_standard)
    odf_text_standard->place_text_in_frame (chapter_text, chapter_marker, font_size, italic, bold);
  if (odf_text_text_only)
    odf_text_text_only->place_text_in_frame (chapter_text, chapter_marker, font_size, italic, bold);
  if (odf_text_text_and_note_citations)
    odf_text_text_and_note_citations->place_text_in_frame (chapter_text, chapter_marker, font_size, italic, bold);
}



// This gets the note citation.
// The first time that a xref is encountered, this function would return, e.g. 'a'.
// The second time, it would return 'b'. Then 'c', 'd', 'e', and so on, up to 'z'.
// Then it would restart with 'a'. And so on.
// The note citation is the character that is put in superscript in the main body of Bible text.
// $style: array with values for the note opening marker.
// Returns: The character for the note citation.
std::string Filter_Text::get_note_citation (const std::string& marker)
{
  const bool end_of_text_reached = (chapter_usfm_markers_and_text_pointer + 1) >= chapter_usfm_markers_and_text.size ();
  if (end_of_text_reached)
    return std::string();

  // Extract the raw note citation from the USFM. This could be, e.g. '+'.
  std::string next_text = chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer + 1];
  std::string citation = next_text.substr (0, 1);
  next_text = filter::strings::ltrim (next_text.substr (1));
  chapter_usfm_markers_and_text [chapter_usfm_markers_and_text_pointer + 1] = next_text;
  citation = filter::strings::trim (citation);
  
  // Get the rendered note citation.
  citation = note_citations.get(marker, citation);
  return citation;
}



// This function ensures that a certain paragraph style for a note is present in the OpenDocument.
// $marker: Which note, e.g. 'f' or 'x' or 'fe'.
// $style: The style to use.
void Filter_Text::ensure_note_paragraph_style (std::string marker, const stylesv2::Style* style)
{
  if (find (created_styles.begin(), created_styles.end(), marker) == created_styles.end()) {
    if (!style)
      return;
    if (!style->paragraph)
      return;
    const std::string font_name = database::config::bible::get_export_font (m_bible);
    const float font_size = style->paragraph.value().font_size;
    const auto italic = style->paragraph.value().italic;
    const auto bold = style->paragraph.value().bold;
    const auto underline = style->paragraph.value().underline;
    const auto smallcaps = style->paragraph.value().smallcaps;
    const auto text_alignment = style->paragraph.value().text_alignment;
    const float space_before = style->paragraph.value().space_before;
    const float space_after = style->paragraph.value().space_after;
    const float left_margin = style->paragraph.value().left_margin;
    const float right_margin = style->paragraph.value().right_margin;
    const float first_line_indent = style->paragraph.value().first_line_indent;
    //bool spancolumns = false;
    constexpr const bool keep_with_next = false;
    constexpr const int dropcaps = 0;
    if (odf_text_standard)
      odf_text_standard->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, dropcaps);
    if (odf_text_text_only)
      odf_text_text_only->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, dropcaps);
    if (odf_text_text_and_note_citations)
      odf_text_text_and_note_citations->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, space_before, space_after, left_margin, right_margin, first_line_indent, keep_with_next, dropcaps);
    if (odf_text_notes)
      odf_text_notes->create_paragraph_style (marker, font_name, font_size, italic, bold, underline, smallcaps, text_alignment, 0, 0, 0, 0, 0, keep_with_next, dropcaps);
    created_styles.push_back (marker);
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


std::string Filter_Text::handle_tilde_and_double_slash(std::string text)
{
  // According to https://ubsicap.github.io/usfm/characters/index.html#index-23,
  // change a tilde ( ~ ) to a non-breaking space.
  constexpr const char* tilde {"~"};
  if (text.find(tilde) != std::string::npos) {
    text = filter::strings::replace (tilde, filter::strings::non_breaking_space_u00A0(), std::move(text));
    add_to_info("A tilde was changed to a non-breaking space");
  }
  // According to https://ubsicap.github.io/usfm/characters/index.html#id3,
  // change a double forward slash ( // ) to a soft hyphen.
  constexpr const char* double_slash {"//"};
  if (text.find(double_slash) != std::string::npos) {
    text = filter::strings::replace (double_slash, filter::strings::soft_hyphen_u00AD(), std::move(text));
    add_to_info("A double forward slash was changed to a soft hyphen");
  }
  return text;
}

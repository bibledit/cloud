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


#include <export/web.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <database/bibleimages.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/css.h>
#include <filter/usfm.h>
#include <fonts/logic.h>
#include <html/text.h>
#include <html/header.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_web_book (std::string bible, int book, bool log)
{
  const std::string directory = export_logic::web_directory (bible);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  
  const std::string feedback_email = database::config::bible::get_export_feedback_email (bible);
  
  
  // Copy font to the output directory.
  const std::string font = fonts::logic::get_text_font (bible);
  if (!font.empty ()) {
    if (fonts::logic::font_exists (font)) {
      std::string fontpath = fonts::logic::get_font_path (font);
      const std::string contents = filter_url_file_get_contents (fontpath);
      fontpath = filter_url_create_path ({directory, font});
      filter_url_file_put_contents (fontpath, contents);
    }
  }
  
  
  const std::string backLinkPath {export_logic::web_back_link_directory (bible)};
  
  
  const std::string bibleBookText = bible + " " + database::books::get_english_from_id (static_cast<book_id>(book));
  
  
  // Web index file for the book.
  HtmlText html_text_rich_book_index (bibleBookText);
  Html_Header htmlHeader = Html_Header (html_text_rich_book_index);
  htmlHeader.search_back_link (backLinkPath + filter_url_html_file_name_bible ("", book), translate("Go back to") + " " + bibleBookText);
  htmlHeader.create ({
    std::pair (bible, filter_url_html_file_name_bible ()),
    std::pair (translate (database::books::get_english_from_id (static_cast<book_id>(book))), filter_url_html_file_name_bible ())
  });
  html_text_rich_book_index.new_paragraph ("navigationbar");
  html_text_rich_book_index.add_text ("|");
  
  
  // Go through the chapters of this book.
  const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
  for (size_t c = 0; c < chapters.size(); c++) {
    const int chapter = chapters [c];
    const bool is_first_chapter = (c == 0);
    const bool is_last_chapter = (c == chapters.size() - 1);
    
    // The text filter for this chapter.
    Filter_Text filter_text_chapter = Filter_Text (bible);
    
    // Get the USFM for the chapter.
    std::string usfm = database::bibles::get_chapter (bible, book, chapter);
    // Trim it.
    usfm = filter::strings::trim (usfm);
    // Use small chunks of USFM at a time for much better performance.
    filter_text_chapter.add_usfm_code (usfm);
    
    // Interlinked web data for one chapter.
    filter_text_chapter.html_text_linked = new HtmlText (translate("Bible"));
    filter_text_chapter.html_text_linked->custom_class = Filter_Css::getClass (bible);
    
    // Create breadcrumbs and navigator for the chapter.
    Html_Header html_header = Html_Header (*filter_text_chapter.html_text_linked);
    html_header.search_back_link (backLinkPath + filter_url_html_file_name_bible ("", book, chapter), translate("Go back to") + " " + bibleBookText + " " + std::to_string (chapter));
    std::vector <std::pair <std::string, std::string> > breadcrumbs_navigator;
    breadcrumbs_navigator.push_back (std::pair (bible, filter_url_html_file_name_bible ()));
    breadcrumbs_navigator.push_back (std::pair (translate (database::books::get_english_from_id (static_cast<book_id>(book))), filter_url_html_file_name_bible ()));
    if (!is_first_chapter) {
      breadcrumbs_navigator.push_back (std::pair ("«", filter_url_html_file_name_bible ("", book, chapter - 1)));
    }
    breadcrumbs_navigator.push_back (std::pair (std::to_string (chapter), filter_url_html_file_name_bible ("", book)));
    if (!is_last_chapter) {
      breadcrumbs_navigator.push_back (std::pair ("»", filter_url_html_file_name_bible ("", book, chapter + 1)));
    }
    // Optionally add a link for giving feedback by email.
    if (!feedback_email.empty ()) {
      breadcrumbs_navigator.push_back (std::pair ("|", ""));
      std::string subject = translate ("Comment on") + " " + bible + " " + database::books::get_english_from_id (static_cast<book_id>(book)) + " " + std::to_string (chapter);
      subject = filter::strings::replace (" ", "%20", subject);
      std::string link = "mailto:" + feedback_email + "?Subject=" + subject;
      breadcrumbs_navigator.push_back (std::pair (translate ("Feedback"), link));
    }
    html_header.create (breadcrumbs_navigator);
    
    // Create interlinked html for the chapter.
    filter_text_chapter.run (stylesheet);
    filter_text_chapter.html_text_linked->save (filter_url_html_file_name_bible (directory, book, chapter));
    
    html_text_rich_book_index.add_link (html_text_rich_book_index.current_p_node, filter_url_html_file_name_bible ("", book, chapter), "", std::to_string (chapter), "", " " + std::to_string (chapter) + " ");
    html_text_rich_book_index.add_text ("|");
    
    // Save any images that were included.
    for (auto src : filter_text_chapter.image_sources) {
      std::string contents = database::bible_images::get(src);
      std::string filename = filter_url_create_path ({directory, src});
      filter_url_file_put_contents(filename, contents);
    }
  }
  
  
  // Save the book index.
  html_text_rich_book_index.save (filter_url_html_file_name_bible (directory, book));
  
  
  // Clear the flag for this export.
  Database_State::clearExport (bible, book, export_logic::export_web);
  
  
  if (log) {
    Database_Logs::log (translate("Exported to web") + ": " + bible + " " + database::books::get_english_from_id (static_cast<book_id>(book)), Filter_Roles::translator ());
  }
}


void export_web_index (std::string bible, bool log)
{
  // Create folders for the web export.
  std::string directory = export_logic::web_directory (bible);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  // Filenames for the web file and stylesheet.
  std::string indexFile = filter_url_create_path ({directory, "index.html"});
  std::string index00 = filter_url_create_path ({directory, "00_index.html"});
  std::string filecss = filter_url_create_path ({directory, "stylesheet.css"});
  
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  
  // Create stylesheet.
  Styles_Sheets styles_sheets;
  styles_sheets.create (stylesheet, filecss, false, bible);
  
  
  std::string backLinkPath = export_logic::web_back_link_directory (bible);
  
  
  // Main index file.
  HtmlText html_text_rich_bible_index (bible);
  // On top are the breadcrumbs, starting with a clickable Bible name.
  Html_Header htmlHeader = Html_Header (html_text_rich_bible_index);
  htmlHeader.search_back_link (backLinkPath + filter_url_html_file_name_bible (), translate("Go back to Bible"));
  htmlHeader.create ({ std::pair (bible, filter_url_html_file_name_bible ())});
  
  
  // Prepare for the list of books in de html index file.
  html_text_rich_bible_index.new_paragraph ("navigationbar");
  html_text_rich_bible_index.add_text (" |");
  
  
  // Go through the Bible books.
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    // Add this book to the main web index.
    html_text_rich_bible_index.add_link (html_text_rich_bible_index.current_p_node,  filter_url_html_file_name_bible ("", book), "", translate (database::books::get_english_from_id (static_cast<book_id>(book))), "", " " + translate (database::books::get_english_from_id (static_cast<book_id>(book))) + " ");
    html_text_rich_bible_index.add_text ("|");
  }
  
  
  // Save index file for the interlinked web export.
  html_text_rich_bible_index.save (indexFile);
  html_text_rich_bible_index.save (index00);
  
  
  // Lens image supporting search.
  std::string lenspath = filter_url_create_root_path ({"webbb", "lens.png"});
  std::string contents = filter_url_file_get_contents (lenspath);
  lenspath = filter_url_create_path ({directory, "lens.png"});
  filter_url_file_put_contents (lenspath, contents);

  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, 0, export_logic::export_web_index);

  
  if (log) Database_Logs::log (translate("Exported to web") + ": " + bible + " Index", Filter_Roles::translator ());
}

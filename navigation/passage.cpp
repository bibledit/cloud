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


#include <navigation/passage.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/date.h>
#include <filter/passage.h>
#include <database/versifications.h>
#include <database/navigation.h>
#include <database/books.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <ipc/focus.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


/*
 On the iPad, Bibledit uses the UIWebView class as the web browser.
 https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIWebView_Class/
 Bibledit used to have a verse navigator that used overlays for selecting book, chapter, and verse.
 When selecting the verse, the UIWebView crashed with the following message:
 UIPopoverPresentationController should have a non-nil sourceView or barButtonItem set before the presentation occurs.
 This is a bug in the UIWebView.
 To circumvent that bug, and also for better operation on the iPad with regard to touch operation,
 Bibledit now uses a different verse navigator, one without the overlays.
*/


namespace navigation_passage {


constexpr const auto left_arrow {"ᐊ"};
constexpr const auto right_arrow {"ᐅ"};


static void add_selector_link (std::string& html, const std::string& id, const std::string& href, const std::string& text, const bool selected, const std::string& extra_class)
{
  // Add a space to cause wrapping between the books or chapters or verses.
  if (!html.empty ())
    html.append (" ");
  
  std::string class_expansion;
  if (selected) class_expansion.append (" active");
  if (!extra_class.empty()) {
    class_expansion.append (" ");
    class_expansion.append (extra_class);
  }
  
  // No wrapping of a book name consisting of more than one word.
  pugi::xml_document document;
  pugi::xml_node span_node = document.append_child("span");
  span_node.append_attribute("class") = ("selector" + class_expansion).c_str();
  {
    pugi::xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = (id + "apply").c_str();
    a_node.append_attribute("href") = href.c_str();
    a_node.text() = text.c_str();
  }
  std::stringstream output;
  document.print (output, "", pugi::format_raw);
  html.append(std::move(output).str());
}


static Passage get_next_book (const std::string& bible, int book)
{
  if (!bible.empty()) {
    const std::vector <int> books = database::bibles::get_books (bible);
    if (auto iter = std::find(books.cbegin(), books.cend(), book); iter != books.cend()) {
      iter++;
      if (iter != books.cend())
        book = *iter;
    }
  }
  return Passage (std::string(), book, 1, "1");
}


static Passage get_previous_book (const std::string& bible, int book)
{
  if (!bible.empty()) {
    const std::vector <int> books = database::bibles::get_books (bible);
    if (auto iter = std::find(books.cbegin(), books.cend(), book); iter != books.cend()) {
      if (iter != books.cbegin()) {
        iter--;
        book = *iter;
      }
    }
  }
  return Passage (std::string(), book, 1, "1");
}


static Passage get_next_chapter (const std::string& bible, const int book, int chapter)
{
  chapter++;
  if (!bible.empty()) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty())
        chapter = chapters.back ();
    }
  }
  return Passage (std::string(), book, chapter, "1");
}


static Passage get_previous_chapter (const std::string& bible, const int book, int chapter)
{
  chapter--;
  if (!bible.empty()) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty ())
        chapter = chapters.at(0);
    }
  }
  return Passage (std::string(), book, chapter, "1");
}


static Passage get_next_verse (const std::string& bible, const int book, const int chapter, int verse)
{
  verse++;
  if (!bible.empty()) {
    const std::vector <int> verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty())
        verse = verses.back ();
    }
  }
  return Passage (std::string(), book, chapter, std::to_string (verse));
}


static Passage get_previous_verse (const std::string& bible, const int book, const int chapter, int verse)
{
  verse--;
  if (!bible.empty()) {
    const std::vector <int> verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty ())
        verse = verses.at(0);
    }
  }
  return Passage (std::string(), book, chapter, std::to_string (verse));
}


std::string get_mouse_navigator (Webserver_Request& webserver_request, const std::string& bible)
{
  Database_Navigation database_navigation;
  
  const std::string& user = webserver_request.session_logic ()->get_username ();
  
  bool passage_clipped = false;
  
  const bool basic_mode = config::logic::basic_mode (webserver_request);
  
  const bool have_arrows = webserver_request.database_config_user()->get_show_navigation_arrows();
  
  pugi::xml_document document;
  
  // Links to go back and forward are available only when there's available history to go to.
  // In basic mode they were not there initially.
  // But later on it was decided to have them in basic mode too.
  // See reasons here: https://github.com/bibledit/cloud/issues/641
  // Using the <span> for detecting long press works well with the mouse.
  // But on Android the long press brings up the context menu.
  // It does not fire the long press event in Javascript.
  // It was tried whether changing the <span> to <button> would improve the situation on Android.
  // But it did not make a difference.
  {
    pugi::xml_node span_node = document.append_child("span");
    if (database_navigation.previous_exists (user)) {
      pugi::xml_node span_node_back = span_node.append_child("span");
      span_node_back.append_attribute("id") = "navigateback";
      const std::string title = translate("Go back or long-press to show history");
      span_node_back.append_attribute("title") = title.c_str();
      span_node_back.text() = "↶";
    }
  }
  {
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node pcdata = span_node.append_child (pugi::node_pcdata);
    pcdata.set_value(" ");
    if (database_navigation.next_exists (user)) {
      pugi::xml_node span_node_back = span_node.append_child("span");
      span_node_back.append_attribute("id") = "navigateforward";
      const std::string title = translate("Go forward or long-press to show history");
      span_node_back.append_attribute("title") = title.c_str();
      span_node_back.text() = "↷";
    }
  }
  
  int book = Ipc_Focus::getBook (webserver_request);
  
  bool prev_book_is_available = true;
  bool next_book_is_available = true;
  
  // The book should exist in the Bible.
  if (!bible.empty()) {
    const std::vector <int> books = database::bibles::get_books (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books.at(0);
      else book = 0;
      passage_clipped = true;
    }
    if (!books.empty ()) {
      prev_book_is_available = (book != books.front ());
      next_book_is_available = (book != books.back ());
    }
  }
  
  if (prev_book_is_available and have_arrows) {
    constexpr const auto previousbook {"previousbook"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = previousbook;
    }
    a_node.append_attribute("id") = previousbook;
    a_node.append_attribute("href") = previousbook;
    a_node.append_attribute("title") = translate("Go to previous book").c_str();
    a_node.text() = left_arrow;
  }
  
  {
    constexpr const auto selectbook {"selectbook"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = selectbook;
    a_node.append_attribute("href") = selectbook;
    a_node.append_attribute("title") = translate("Select book").c_str();
    const std::string book_name = translate (database::books::get_english_from_id (static_cast<book_id>(book)));
    a_node.text() = book_name.c_str();
  }
  
  if (next_book_is_available and have_arrows) {
    constexpr const auto nextbook {"nextbook"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = nextbook;
    }
    a_node.append_attribute("id") = nextbook;
    a_node.append_attribute("href") = nextbook;
    a_node.append_attribute("title") = translate("Go to next book").c_str();
    a_node.text() = right_arrow;
  }
  
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  bool next_chapter_is_available = true;
  
  // The chapter should exist in the book.
  if (!bible.empty()) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters.at(0);
      else chapter = 1;
      passage_clipped = true;
    }
    if (!chapters.empty ()) {
      if (chapter >= chapters.back ()) {
        next_chapter_is_available = false;
      }
    }
  }
  
  if (have_arrows) {
    constexpr const auto previouschapter {"previouschapter"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = previouschapter;
    }
    if (chapter) {
      a_node.append_attribute("id") = previouschapter;
      a_node.append_attribute("href") = previouschapter;
      a_node.append_attribute("title") = translate("Go to previous chapter").c_str();
    }
    a_node.text() = left_arrow;
  }
  
  {
    constexpr const auto selectchapter {"selectchapter"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = selectchapter;
    a_node.append_attribute("href") = selectchapter;
    a_node.append_attribute("title") = translate("Select chapter").c_str();
    a_node.text() = std::to_string (chapter).c_str();
  }
  
  if (next_chapter_is_available and have_arrows) {
    constexpr const auto nextchapter {"nextchapter"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = nextchapter;
    }
    a_node.append_attribute("id") = nextchapter;
    a_node.append_attribute("href") = nextchapter;
    a_node.append_attribute("title") = translate("Go to next chapter").c_str();
    a_node.text() = right_arrow;
  }
  
  int verse = Ipc_Focus::getVerse (webserver_request);
  
  bool next_verse_is_available = true;
  
  // The verse should exist in the chapter.
  if (!bible.empty()) {
    const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
    const std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses.at(0);
      else verse = 1;
      passage_clipped = true;
    }
    if (!verses.empty ()) {
      if (verse >= verses.back ()) {
        next_verse_is_available = false;
      }
    }
  }
  
  if (have_arrows) {
    constexpr const auto previousverse {"previousverse"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = previousverse;
    }
    if (verse) {
      a_node.append_attribute("id") = previousverse;
      a_node.append_attribute("href") = previousverse;
      a_node.append_attribute("title") = translate("Go to previous verse").c_str();
    }
    a_node.text() = left_arrow;
  }
  
  {
    constexpr const auto selectverse {"selectverse"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = selectverse;
    }
    a_node.append_attribute("id") = selectverse;
    a_node.append_attribute("href") = selectverse;
    a_node.append_attribute("title") = translate("Select verse").c_str();
    a_node.text() = std::to_string(verse).c_str();
  }
  
  if (next_verse_is_available and have_arrows) {
    constexpr const auto nextverse {"nextverse"};
    pugi::xml_node span_node = document.append_child("span");
    pugi::xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = nextverse;
    }
    a_node.append_attribute("id") = nextverse;
    a_node.append_attribute("href") = nextverse;
    a_node.append_attribute("title") = translate("Go to next verse").c_str();
    a_node.text() = right_arrow;
  }
  
  // Store book / chapter / verse if they were clipped.
  if (passage_clipped) {
    Ipc_Focus::set (webserver_request, book, chapter, verse);
  }
  
  // The result.
  std::stringstream output;
  document.print (output, "", pugi::format_raw);
  return output.str ();
}


std::string get_books_fragment (Webserver_Request& webserver_request, const std::string& bible)
{
  const book_id active_book = static_cast<book_id>(Ipc_Focus::getBook (webserver_request));
  // Take standard books in case of no Bible.
  std::vector <book_id> books;
  if (bible.empty()) {
    books = database::books::get_ids ();
  } else {
    const std::vector <int> book_numbers = filter_passage_get_ordered_books (bible);
    for (const auto book_number : book_numbers) books.push_back (static_cast<book_id>(book_number));
  }
  std::string html {};
  for (const auto book : books) {
    std::string book_name = database::books::get_english_from_id (book);
    book_name = translate (book_name);
    const bool selected = (book == active_book);
    const std::string book_type = database::books::book_type_to_string (database::books::get_type (book));
    add_selector_link (html, std::to_string (static_cast<int>(book)), "applybook", book_name, selected, book_type);
  }
  add_selector_link (html, "cancel", "applybook", "[" + translate ("cancel") + "]", false, "");
  html.insert (0, "<span id='applybook'>" + translate ("Select book") + ": ");
  html.append ("</span>");
  return html;
}


std::string get_chapters_fragment (const std::string& bible, const int book, const int chapter)
{
  std::vector <int> chapters;
  if (bible.empty ()) {
    Database_Versifications database_versifications;
    chapters = database_versifications.getChapters (filter::strings::english (), book, true);
  } else {
    chapters = database::bibles::get_chapters (bible, book);
  }
  std::string html;
  html.append (" ");
  for (const auto ch : chapters) {
    const bool selected = (ch == chapter);
    add_selector_link (html, std::to_string (ch), "applychapter", std::to_string (ch), selected, "");
  }
  add_selector_link (html, "cancel", "applychapter", "[" + translate ("cancel") + "]", false, "");
  
  html.insert (0, R"(<span id="applychapter">)" + translate ("Select chapter"));
  html.append ("</span>");
  
  return html;
}


std::string get_verses_fragment (const std::string& bible, const int book, const int chapter, const int verse)
{
  std::vector <int> verses;
  if (bible.empty()) {
    Database_Versifications database_versifications;
    verses = database_versifications.getVerses (filter::strings::english (), book, chapter);
  } else {
    verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, book, chapter));
  }
  std::string html;
  html.append (" ");
  for (const auto vs : verses) {
    const bool selected = (verse == vs);
    add_selector_link (html, std::to_string (vs), "applyverse", std::to_string (vs), selected, "");
  }
  add_selector_link (html, "cancel", "applyverse", "[" + translate ("cancel") + "]", false, "");
  
  html.insert (0, R"(<span id="applyverse">)" + translate ("Select verse"));
  html.append ("</span>");
  
  return html;
}


std::string code (const std::string& bible)
{
  std::string code;
  code.append(R"(<script type="text/javascript">)");
  code.append(R"(navigationBible = ")" + bible + R"(";)");
  code.append("</script>");
  code.append("\n");
  return code;
}


void set_book (Webserver_Request& webserver_request, const int book)
{
  Ipc_Focus::set (webserver_request, book, 1, 1);
  record_history (webserver_request, book, 1, 1);
}


void set_chapter (Webserver_Request& webserver_request, const int chapter)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, 1);
  record_history (webserver_request, book, chapter, 1);
}


void set_verse (Webserver_Request& webserver_request, const int verse)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, verse);
  record_history (webserver_request, book, chapter, verse);
}


void set_passage (Webserver_Request& webserver_request, const std::string& bible, std::string passage)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);
  passage = filter::strings::trim (std::move(passage));
  Passage passage_to_set;
  if ((passage.empty()) || (passage == "+")) {
    passage_to_set = get_next_verse (bible, book, chapter, verse);
  } else if (passage == "-") {
    passage_to_set = get_previous_verse (bible, book, chapter, verse);
  } else {
    const Passage inputpassage = Passage (std::string(), book, chapter, std::to_string (verse));
    passage_to_set = filter_passage_interpret_passage (inputpassage, passage);
  }
  if (passage_to_set.m_book) {
    Ipc_Focus::set (webserver_request, passage_to_set.m_book, passage_to_set.m_chapter, filter::strings::convert_to_int (passage_to_set.m_verse));
    record_history (webserver_request, passage_to_set.m_book, passage_to_set.m_chapter, filter::strings::convert_to_int (passage_to_set.m_verse));
  }
}


void goto_next_book (Webserver_Request& webserver_request, const std::string& bible)
{
  const int current_book = Ipc_Focus::getBook (webserver_request);
  Passage passage = get_next_book (bible, current_book);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void goto_previous_book (Webserver_Request& webserver_request, const std::string& bible)
{
  int current_book = Ipc_Focus::getBook (webserver_request);
  const Passage passage = get_previous_book (bible, current_book);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void goto_next_chapter (Webserver_Request& webserver_request, const std::string& bible)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const Passage passage = get_next_chapter (bible, book, chapter);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void goto_previous_chapter (Webserver_Request& webserver_request, const std::string& bible)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  const Passage passage = get_previous_chapter (bible, book, chapter);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void goto_next_verse (Webserver_Request& webserver_request, const std::string& bible)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);
  const Passage passage = get_next_verse (bible, book, chapter, verse);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void goto_previous_verse (Webserver_Request& webserver_request, const std::string& bible)
{
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);
  const Passage passage = get_previous_verse (bible, book, chapter, verse);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
    record_history (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void record_history (Webserver_Request& webserver_request, const int book, const int chapter, const int verse)
{
  const std::string& user = webserver_request.session_logic()->get_username();
  Database_Navigation database_navigation;
  database_navigation.record (filter::date::seconds_since_epoch (), user, book, chapter, verse);
}


void go_back (Webserver_Request& webserver_request)
{
  Database_Navigation database_navigation;
  const std::string& user = webserver_request.session_logic ()->get_username ();
  const Passage passage = database_navigation.get_previous (user);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


void go_forward (Webserver_Request& webserver_request)
{
  Database_Navigation database_navigation;
  const std::string& user = webserver_request.session_logic ()->get_username ();
  const Passage passage = database_navigation.get_next (user);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse));
  }
}


std::string get_keyboard_navigator (Webserver_Request& webserver_request, const std::string& bible)
{
  bool passage_clipped = false;
  
  std::string fragment;
  
  int book = Ipc_Focus::getBook (webserver_request);
  
  // The book should exist in the Bible.
  if (!bible.empty()) {
    const std::vector <int> books = database::bibles::get_books (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books.at(0);
      else book = 0;
      passage_clipped = true;
    }
  }
  
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  // The chapter should exist in the book.
  if (!bible.empty()) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters.at(0);
      else chapter = 1;
      passage_clipped = true;
    }
  }
  
  int verse = Ipc_Focus::getVerse (webserver_request);
  
  // The verse should exist in the chapter.
  if (!bible.empty()) {
    const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
    const std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses.at(0);
      else verse = 1;
      passage_clipped = true;
    }
  }
  
  const std::string current_passage = filter_passage_display (book, chapter, std::to_string (verse));
  fragment.append ("<span>");
  fragment.append (current_passage);
  fragment.append ("</span>");
  
  // Spacer.
  fragment.append ("<span> </span>");
  
  // Add some helpful information for the user what to do.
  fragment.append ("<span>" + translate ("Enter passage to go to") + ":</span>");
  
  // Add the input to the html fragment.
  fragment.append ("<span><input type='text' id='keyboard' href='keyboard' title='" + translate ("Enter passage to go to") + "'></span>");
  
  // Store book / chapter / verse if they were clipped.
  if (passage_clipped) {
    Ipc_Focus::set (webserver_request, book, chapter, verse);
  }
  
  // The result.
  return fragment;
}


void interpret_keyboard_navigator (Webserver_Request& webserver_request, std::string bible, std::string passage)
{
  int book = Ipc_Focus::getBook (webserver_request);
  
  // The book should exist in the Bible.
  if (!bible.empty()) {
    const std::vector <int> books = database::bibles::get_books (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books.at(0);
      else book = 0;
    }
  }
  
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  // The chapter should exist in the book.
  if (!bible.empty()) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters.at(0);
      else chapter = 1;
    }
  }
  
  int verse = Ipc_Focus::getVerse (webserver_request);
  
  // The verse should exist in the chapter.
  if (!bible.empty()) {
    const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
    const std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses.at(0);
      else verse = 1;
    }
  }
  
  // Determine the new passage based on the current one.
  const Passage current_passage (bible, book, chapter, std::to_string (verse));
  const Passage new_passage = filter_passage_interpret_passage (current_passage, passage);
  
  // Store book / chapter / verse.
  Ipc_Focus::set (webserver_request, new_passage.m_book, new_passage.m_chapter, filter::strings::convert_to_int (new_passage.m_verse));
  record_history (webserver_request, new_passage.m_book, new_passage.m_chapter, filter::strings::convert_to_int (new_passage.m_verse));
}


std::string get_history_back (Webserver_Request& webserver_request)
{
  // Get the whole history from the database.
  Database_Navigation database_navigation {};
  const std::string& user {webserver_request.session_logic ()->get_username ()};
  const std::vector<Passage> passages = database_navigation.get_history(user, -1);
  // Take the most recent nnn history items and render them.
  std::string html {};
  for (size_t i = 0; i < passages.size(); i++) {
    if (i >= 10) continue;
    const std::string rendering = filter_passage_display(passages[i].m_book, passages[i].m_chapter, passages[i].m_verse);
    const std::string book_type = database::books::book_type_to_string (database::books::get_type (static_cast <book_id> (passages[i].m_book)));
    add_selector_link (html, "b" + std::to_string (i), "applyhistory", rendering, false, book_type);
  }
  // Add a "cancel" link.
  add_selector_link (html, "cancel", "applyhistory", "[" + translate ("cancel") + "]", false, "");
  // Main html items.
  html.insert (0, "<span id='applyhistory'>" + translate ("History back") + ": ");
  html.append ("</span>");
  // Done.
  return html;
}


std::string get_history_forward (Webserver_Request& webserver_request)
{
  // Get the whole history from the database.
  Database_Navigation database_navigation;
  const std::string& user {webserver_request.session_logic ()->get_username ()};
  const std::vector<Passage> passages {database_navigation.get_history(user, 1)};
  // Take the most recent nnn history items and render them.
  std::string html {};
  for (size_t i = 0; i < passages.size(); i++) {
    if (i >= 10) continue;
    const std::string rendering = filter_passage_display(passages[i].m_book, passages[i].m_chapter, passages[i].m_verse);
    const std::string book_type = database::books::book_type_to_string (database::books::get_type (static_cast<book_id>(passages[i].m_book)));
    add_selector_link (html, "f" + std::to_string (i), "applyhistory", rendering, false, book_type);
  }
  // Add a "cancel" link.
  add_selector_link (html, "cancel", "applyhistory", "[" + translate ("cancel") + "]", false, "");
  // Main html items.
  html.insert (0, "<span id='applyhistory'>" + translate ("History forward") + ": ");
  html.append ("</span>");
  // Done.
  return html;
}


void go_history (Webserver_Request& webserver_request, std::string message)
{
  // Example messages:
  // * f0apply: The "f" means "go forward". The "0" means item 0, that is, the first item.
  // * b1apply: See above. It means to go back two steps.
  
  // Check that the fragment "apply" occurs in the message.
  // If so, remove it. If not, do nothing.
  const size_t pos = message.find("apply");
  if (pos == std::string::npos) return;
  message.erase (pos);
  
  // Check that the length of the remaining message is at least 2.
  // The remaining message could be "f1" or "b12" and so on.
  if (message.length() < 2) return;
  
  // Get the direction: forward or backward.
  int direction = 0;
  if (message[0] == 'f') direction = 1;
  if (message[0] == 'b') direction = -1;
  message.erase (0, 1);
  if (!direction) return;
  
  // Get the offset of the history item.
  const int offset = filter::strings::convert_to_int(message);
  
  // Go n times forward or backward.
  for (int i = 0; i <= offset; i++) {
    if (direction > 0) {
      go_forward(webserver_request);
    }
    if (direction < 0) {
      go_back(webserver_request);
    }
  }
}


}

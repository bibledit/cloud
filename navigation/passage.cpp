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
using namespace std;
using namespace pugi;


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


string Navigation_Passage::get_mouse_navigator (void * webserver_request, string bible)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  Database_Navigation database_navigation;
  
  string user = request->session_logic()->currentUser ();
  
  bool passage_clipped = false;
  
  bool basic_mode = config::logic::basic_mode (webserver_request);
  
  xml_document document;
  
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
    xml_node span_node = document.append_child("span");
    if (database_navigation.previous_exists (user)) {
      xml_node span_node_back = span_node.append_child("span");
      span_node_back.append_attribute("id") = "navigateback";
      string title = translate("Go back or long-press to show history");
      span_node_back.append_attribute("title") = title.c_str();
      span_node_back.text() = "↶";
    }
  }
  {
    xml_node span_node = document.append_child("span");
    xml_node pcdata = span_node.append_child (node_pcdata);
    pcdata.set_value(" ");
    if (database_navigation.next_exists (user)) {
      xml_node span_node_back = span_node.append_child("span");
      span_node_back.append_attribute("id") = "navigateforward";
      string title = translate("Go forward or long-press to show history");
      span_node_back.append_attribute("title") = title.c_str();
      span_node_back.text() = "↷";
    }
  }

  int book = Ipc_Focus::getBook (request);
  
  // The book should exist in the Bible.
  if (bible != "") {
    vector <int> books = request->database_bibles()->getBooks (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books [0];
      else book = 0;
      passage_clipped = true;
    }
  }
  
  string bookName = database::books::get_english_from_id (static_cast<book_id>(book));
  bookName = translate (bookName);

  {
    xml_node span_node = document.append_child("span");
    xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = "selectbook";
    a_node.append_attribute("href") = "selectbook";
    a_node.append_attribute("title") = translate("Select book").c_str();
    a_node.text() = bookName.c_str();
  }

  int chapter = Ipc_Focus::getChapter (request);
  
  // The chapter should exist in the book.
  if (bible != "") {
    vector <int> chapters = request->database_bibles()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters [0];
      else chapter = 1;
      passage_clipped = true;
    }
  }

  {
    xml_node span_node = document.append_child("span");
    xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = "selectchapter";
    a_node.append_attribute("href") = "selectchapter";
    a_node.append_attribute("title") = translate("Select chapter").c_str();
    a_node.text() = convert_to_string (chapter).c_str();
  }
  
  int verse = Ipc_Focus::getVerse (request);
  
  bool next_verse_is_available = true;
  
  // The verse should exist in the chapter.
  if (bible != "") {
    string usfm = request->database_bibles()->getChapter (bible, book, chapter);
    vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses [0];
      else verse = 1;
      passage_clipped = true;
    }
    if (!verses.empty ()) {
      if (verse >= verses.back ()) {
        next_verse_is_available = false;
      }
    }
  }

  {
    xml_node span_node = document.append_child("span");
    xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = "previousverse";
    }
    if (verse) {
      a_node.append_attribute("id") = "previousverse";
      a_node.append_attribute("href") = "previousverse";
      a_node.append_attribute("title") = translate("Go to previous verse").c_str();
    }
    a_node.text() = " ᐊ ";
  }

  {
    xml_node span_node = document.append_child("span");
    xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = "selectverse";
    }
    a_node.append_attribute("id") = "selectverse";
    a_node.append_attribute("href") = "selectverse";
    a_node.append_attribute("title") = translate("Select verse").c_str();
    a_node.text() = (" " + convert_to_string (verse) + " ").c_str();
  }

  if (next_verse_is_available) {
    xml_node span_node = document.append_child("span");
    xml_node a_node = span_node.append_child("a");
    if (!basic_mode) {
      a_node.append_attribute("class") = "nextverse";
    }
    a_node.append_attribute("id") = "nextverse";
    a_node.append_attribute("href") = "nextverse";
    a_node.append_attribute("title") = translate("Go to next verse").c_str();
    a_node.text() = " ᐅ ";
  }

  // Store book / chapter / verse if they were clipped.
  if (passage_clipped) {
    Ipc_Focus::set (request, book, chapter, verse);
  }

  // The result.
  stringstream output;
  document.print (output, "", format_raw);
  string fragment = output.str ();
  return fragment;
}


string Navigation_Passage::get_books_fragment (void * webserver_request, string bible)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  book_id active_book = static_cast<book_id>(Ipc_Focus::getBook (request));
  // Take standard books in case of no Bible.
  vector <book_id> books;
  if (bible.empty()) {
    books = database::books::get_ids ();
  } else {
    vector <int> book_numbers = filter_passage_get_ordered_books (bible);
    for (auto book_number : book_numbers) books.push_back (static_cast<book_id>(book_number));
  }
  string html {};
  for (auto book : books) {
    string book_name = database::books::get_english_from_id (book);
    book_name = translate (book_name);
    bool selected = (book == active_book);
    string book_type = database::books::book_type_to_string (database::books::get_type (book));
    add_selector_link (html, convert_to_string (static_cast<int>(book)), "applybook", book_name, selected, book_type);
  }
  add_selector_link (html, "cancel", "applybook", "[" + translate ("cancel") + "]", false, "");
  html.insert (0, "<span id='applybook'>" + translate ("Select book") + ": ");
  html.append ("</span>");
  return html;
}


string Navigation_Passage::get_chapters_fragment (void * webserver_request, string bible, int book, int chapter)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <int> chapters;
  if (bible.empty ()) {
    Database_Versifications database_versifications;
    chapters = database_versifications.getChapters (english (), book, true);
  } else {
    chapters = request->database_bibles()->getChapters (bible, book);
  }
  string html;
  html.append (" ");
  for (auto ch : chapters) {
    bool selected = (ch == chapter);
    add_selector_link (html, convert_to_string (ch), "applychapter", convert_to_string (ch), selected, "");
  }
  add_selector_link (html, "cancel", "applychapter", "[" + translate ("cancel") + "]", false, "");

  html.insert (0, R"(<span id="applychapter">)" + translate ("Select chapter"));
  html.append ("</span>");

  return html;
}


string Navigation_Passage::get_verses_fragment (void * webserver_request, string bible, int book, int chapter, int verse)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <int> verses;
  if (bible == "") {
    Database_Versifications database_versifications;
    verses = database_versifications.getVerses (english (), book, chapter);
  } else {
    verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
  }
  string html;
  html.append (" ");
  for (auto vs : verses) {
    bool selected = (verse == vs);
    add_selector_link (html, convert_to_string (vs), "applyverse", convert_to_string (vs), selected, "");
  }
  add_selector_link (html, "cancel", "applyverse", "[" + translate ("cancel") + "]", false, "");

  html.insert (0, R"(<span id="applyverse">)" + translate ("Select verse"));
  html.append ("</span>");

  return html;
}


string Navigation_Passage::code (string bible)
{
  string code;
  code += R"(<script type="text/javascript">)";
  code += R"(navigationBible = ")" + bible + R"(";)";
  code += "</script>";
  code += "\n";
  return code;
}


void Navigation_Passage::set_book (void * webserver_request, int book)
{
  Ipc_Focus::set (webserver_request, book, 1, 1);
  record_history (webserver_request, book, 1, 1);
}


void Navigation_Passage::set_chapter (void * webserver_request, int chapter)
{
  int book = Ipc_Focus::getBook (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, 1);
  record_history (webserver_request, book, chapter, 1);
}


void Navigation_Passage::set_verse (void * webserver_request, int verse)
{
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, verse);
  record_history (webserver_request, book, chapter, verse);
}


void Navigation_Passage::set_passage (void * webserver_request, string bible, string passage)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  passage = filter_string_trim (passage);
  Passage passage_to_set;
  if ((passage == "") || (passage == "+")) {
    passage_to_set = Navigation_Passage::get_next_verse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  } else if (passage == "-") {
    passage_to_set = Navigation_Passage::get_previous_verse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  } else {
    Passage inputpassage = Passage ("", currentBook, currentChapter, convert_to_string (currentVerse));
    passage_to_set = filter_passage_interpret_passage (inputpassage, passage);
  }
  if (passage_to_set.m_book != 0) {
    Ipc_Focus::set (webserver_request, passage_to_set.m_book, passage_to_set.m_chapter, convert_to_int (passage_to_set.m_verse));
    Navigation_Passage::record_history (webserver_request, passage_to_set.m_book, passage_to_set.m_chapter, convert_to_int (passage_to_set.m_verse));
  }
}


Passage Navigation_Passage::get_next_chapter (void * webserver_request, string bible, int book, int chapter)
{
  chapter++;
  if (bible != "") {
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
    vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters.back ();
    }
  }
  Passage passage = Passage ("", book, chapter, "1");
  return passage;
}


Passage Navigation_Passage::get_previous_chapter (void * webserver_request, string bible, int book, int chapter)
{
  chapter--;
  if (bible != "") {
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
    vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty ()) chapter = chapters [0];
    }
  }
  Passage passage = Passage ("", book, chapter, "1");
  return passage;
}


void Navigation_Passage::goto_next_chapter (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  Passage passage = Navigation_Passage::get_next_chapter (webserver_request, bible, currentBook, currentChapter);
  if (passage.m_book != 0) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
    Navigation_Passage::record_history (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


void Navigation_Passage::goto_previous_chapter (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  Passage passage = Navigation_Passage::get_previous_chapter (webserver_request, bible, currentBook, currentChapter);
  if (passage.m_book != 0) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
    Navigation_Passage::record_history (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


Passage Navigation_Passage::get_next_verse (void * webserver_request, string bible, int book, int chapter, int verse)
{
  verse++;
  if (bible != "") {
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
    vector <int> verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty()) verse = verses.back ();
    }
  }
  Passage passage = Passage ("", book, chapter, convert_to_string (verse));
  return passage;
}


Passage Navigation_Passage::get_previous_verse (void * webserver_request, string bible, int book, int chapter, int verse)
{
  verse--;
  if (bible != "") {
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
    vector <int> verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty ()) verse = verses [0];
    }
  }
  Passage passage = Passage ("", book, chapter, convert_to_string (verse));
  return passage;
}


void Navigation_Passage::goto_next_verse (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  Passage passage = Navigation_Passage::get_next_verse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  if (passage.m_book != 0) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
    Navigation_Passage::record_history (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


void Navigation_Passage::goto_previous_verse (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  Passage passage = Navigation_Passage::get_previous_verse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  if (passage.m_book != 0) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
    Navigation_Passage::record_history (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


void Navigation_Passage::record_history (void * webserver_request, int book, int chapter, int verse)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic()->currentUser ();
  Database_Navigation database_navigation;
  database_navigation.record (filter::date::seconds_since_epoch (), user, book, chapter, verse);
}


void Navigation_Passage::go_back (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Navigation database_navigation;
  string user = request->session_logic()->currentUser ();
  Passage passage = database_navigation.get_previous (user);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


void Navigation_Passage::go_forward (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Navigation database_navigation;
  string user = request->session_logic()->currentUser ();
  Passage passage = database_navigation.get_next (user);
  if (passage.m_book) {
    Ipc_Focus::set (webserver_request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
  }
}


void Navigation_Passage::add_selector_link (string& html, string id, string href, string text, bool selected, string extra_class)
{
  // Add a space to cause wrapping between the books or chapters or verses.
  if (!html.empty ()) html.append (" ");

  string class_expansion;
  if (selected) class_expansion.append (" active");
  if (!extra_class.empty()) {
    class_expansion.append (" ");
    class_expansion.append (extra_class);
  }
  
  // No wrapping of a book name consisting of more than one word.
  xml_document document;
  xml_node span_node = document.append_child("span");
  span_node.append_attribute("class") = ("selector" + class_expansion).c_str();
  {
    xml_node a_node = span_node.append_child("a");
    a_node.append_attribute("id") = (id + "apply").c_str();
    a_node.append_attribute("href") = href.c_str();
    a_node.text() = text.c_str();
  }
  stringstream output;
  document.print (output, "", format_raw);
  string fragment = output.str ();
  html.append(output.str());
}


string Navigation_Passage::get_keyboard_navigator (void * webserver_request, string bible)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string user = request->session_logic()->currentUser ();
  
  bool passage_clipped = false;
  
  string fragment;
  
  int book = Ipc_Focus::getBook (request);
  
  // The book should exist in the Bible.
  if (bible != "") {
    vector <int> books = request->database_bibles()->getBooks (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books [0];
      else book = 0;
      passage_clipped = true;
    }
  }
  
  int chapter = Ipc_Focus::getChapter (request);
  
  // The chapter should exist in the book.
  if (bible != "") {
    vector <int> chapters = request->database_bibles()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters [0];
      else chapter = 1;
      passage_clipped = true;
    }
  }

  int verse = Ipc_Focus::getVerse (request);
  
  // The verse should exist in the chapter.
  if (bible != "") {
    string usfm = request->database_bibles()->getChapter (bible, book, chapter);
    vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses [0];
      else verse = 1;
      passage_clipped = true;
    }
  }
  
  string current_passage = filter_passage_display (book, chapter, convert_to_string (verse));
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
    Ipc_Focus::set (request, book, chapter, verse);
  }
  
  // The result.
  return fragment;
}


void Navigation_Passage::interpret_keyboard_navigator (void * webserver_request, string bible, string passage)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string user = request->session_logic()->currentUser ();
  
  int book = Ipc_Focus::getBook (request);
  
  // The book should exist in the Bible.
  if (bible != "") {
    vector <int> books = request->database_bibles()->getBooks (bible);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books [0];
      else book = 0;
    }
  }
  
  int chapter = Ipc_Focus::getChapter (request);
  
  // The chapter should exist in the book.
  if (bible != "") {
    vector <int> chapters = request->database_bibles()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters [0];
      else chapter = 1;
    }
  }

  int verse = Ipc_Focus::getVerse (request);
  
  // The verse should exist in the chapter.
  if (bible != "") {
    string usfm = request->database_bibles()->getChapter (bible, book, chapter);
    vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    if (!in_array (verse, verses)) {
      if (!verses.empty()) verse = verses [0];
      else verse = 1;
    }
  }

  // Determine the new passage based on the current one.
  Passage current_passage (bible, book, chapter, convert_to_string (verse));
  Passage new_passage = filter_passage_interpret_passage (current_passage, passage);
  
  // Store book / chapter / verse.
  Ipc_Focus::set (request, new_passage.m_book, new_passage.m_chapter, convert_to_int (new_passage.m_verse));
  Navigation_Passage::record_history (request, new_passage.m_book, new_passage.m_chapter, convert_to_int (new_passage.m_verse));
}


string Navigation_Passage::get_history_back (void * webserver_request)
{
  // Get the whole history from the database.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Navigation database_navigation {};
  string user {request->session_logic()->currentUser ()};
  vector<Passage> passages = database_navigation.get_history(user, -1);
  // Take the most recent nnn history items and render them.
  string html {};
  for (size_t i = 0; i < passages.size(); i++) {
    if (i >= 10) continue;
    string rendering = filter_passage_display(passages[i].m_book, passages[i].m_chapter, passages[i].m_verse);
    string book_type = database::books::book_type_to_string (database::books::get_type (static_cast <book_id> (passages[i].m_book)));
    add_selector_link (html, "b" + convert_to_string (i), "applyhistory", rendering, false, book_type);
  }
  // Add a "cancel" link.
  add_selector_link (html, "cancel", "applyhistory", "[" + translate ("cancel") + "]", false, "");
  // Main html items.
  html.insert (0, "<span id='applyhistory'>" + translate ("History back") + ": ");
  html.append ("</span>");
  // Done.
  return html;
}


string Navigation_Passage::get_history_forward (void * webserver_request)
{
  // Get the whole history from the database.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Navigation database_navigation;
  string user {request->session_logic()->currentUser ()};
  vector<Passage> passages {database_navigation.get_history(user, 1)};
  // Take the most recent nnn history items and render them.
  string html {};
  for (size_t i = 0; i < passages.size(); i++) {
    if (i >= 10) continue;
    string rendering = filter_passage_display(passages[i].m_book, passages[i].m_chapter, passages[i].m_verse);
    string book_type = database::books::book_type_to_string (database::books::get_type (static_cast<book_id>(passages[i].m_book)));
    add_selector_link (html, "f" + convert_to_string (i), "applyhistory", rendering, false, book_type);
  }
  // Add a "cancel" link.
  add_selector_link (html, "cancel", "applyhistory", "[" + translate ("cancel") + "]", false, "");
  // Main html items.
  html.insert (0, "<span id='applyhistory'>" + translate ("History forward") + ": ");
  html.append ("</span>");
  // Done.
  return html;
}


void Navigation_Passage::go_history (void * webserver_request, string message)
{
  // Example messages:
  // * f0apply: The "f" means "go forward". The "0" means item 0, that is, the first item.
  // * b1apply: See above. It means to go back two steps.

  // Check that the fragment "apply" occurs in the message.
  // If so, remove it. If not, do nothing.
  size_t pos = message.find("apply");
  if (pos == string::npos) return;
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
  int offset = convert_to_int(message);
  
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



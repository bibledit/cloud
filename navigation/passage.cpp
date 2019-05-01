/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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
#include <database/versifications.h>
#include <database/navigation.h>
#include <database/books.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <ipc/focus.h>


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


string Navigation_Passage::getNavigator (void * webserver_request, string bible)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  Database_Navigation database_navigation;
  
  string user = request->session_logic()->currentUser ();
  
  bool passage_clipped = false;
  
  bool basic_mode = config_logic_basic_mode (webserver_request);
  
  string fragment;
  
  // Links to go back and forward are available only when there's available history to go to.
  // In basic mode they are not there.
  if (!basic_mode) {
    fragment.append ("<span>");
    if (database_navigation.previousExists (user)) {
      fragment.append ("<a id=\"navigateback\" href=\"navigateback\" title=\"" + translate("Back") + "\">↶</a>");
    }
    fragment.append ("</span>");
    fragment.append ("<span>");
    fragment.append (" ");
    if (database_navigation.nextExists (user)) {
      fragment.append ("<a id=\"navigateforward\" href=\"navigateforward\" title=\"" + translate("Forward") + "\">↷</a>");
    }
    fragment.append ("</span>");
    fragment.append ("\n");
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
  
  string bookName = Database_Books::getEnglishFromId (book);
  bookName = translate (bookName);

  fragment.append ("<span><a id='selectbook' href='selectbook' title='" + translate ("Select book") + "'>" + bookName + "</a></span>");
  
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

  fragment.append ("<span><a id=\"selectchapter\" href=\"selectchapter\" title=\"" + translate ("Select chapter") + "\"> " + convert_to_string (chapter) +  " </a></span>");
  
  int verse = Ipc_Focus::getVerse (request);
  
  bool next_verse_is_available = true;
  
  // The verse should exist in the chapter.
  if (bible != "") {
    string usfm = request->database_bibles()->getChapter (bible, book, chapter);
    vector <int> verses = usfm_get_verse_numbers (usfm);
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
  
  fragment.append ("<span><a");
  if (!basic_mode) {
    fragment.append (" class=\"previousverse\"");
  }
  if (verse) {
    // A previous verse (0) is assumed to be available.
    fragment.append (" id=\"previousverse\" href=\"previousverse\" title=\"" + translate ("Go to previous verse") + "\"");
  }
  fragment.append ("> « </a></span>");
  
  fragment.append ("<span><a");
  if (!basic_mode) fragment.append (" class=\"selectverse\"");
  fragment.append (" id=\"selectverse\" href=\"selectverse\" title=\"" + translate ("Select verse") + "\"> " + convert_to_string (verse) +  " </a></span>");

  if (next_verse_is_available) {
    fragment.append ("<span><a");
    if (!basic_mode) fragment.append (" class=\"nextverse\"");
    fragment.append (" id=\"nextverse\" href=\"nextverse\" title=\"" + translate ("Go to next verse") + "\"");
    fragment.append ("> » </a></span>");
  }

  // Store book / chapter / verse if they were clipped.
  if (passage_clipped) {
    Ipc_Focus::set (request, book, chapter, verse);
  }
  
  // The result.
  return fragment;
}


string Navigation_Passage::getBooksFragment (void * webserver_request, string bible)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int activeBook = Ipc_Focus::getBook (request);
  // Take standard books in case of no Bible.
  vector <int> books;
  if (bible == "") {
    books = Database_Books::getIDs ();
  } else {
    books = filter_passage_get_ordered_books (bible);
  }
  string html;
  for (auto book : books) {
    string bookName = Database_Books::getEnglishFromId (book);
    bookName = translate (bookName);
    bool selected = (book == activeBook);
    addSelectorLink (html, convert_to_string (book), "applybook", bookName, selected);
  }
  addSelectorLink (html, "cancel", "applybook", "[" + translate ("cancel") + "]", false);

  html.insert (0, "<span id='applybook'>" + translate ("Select book") + ": ");
  html.append ("</span>");

  return html;
}


string Navigation_Passage::getChaptersFragment (void * webserver_request, string bible, int book, int chapter)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
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
    addSelectorLink (html, convert_to_string (ch), "applychapter", convert_to_string (ch), selected);
  }
  addSelectorLink (html, "cancel", "applychapter", "[" + translate ("cancel") + "]", false);

  html.insert (0, "<span id=\"applychapter\">" + translate ("Select chapter"));
  html.append ("</span>");

  return html;
}


string Navigation_Passage::getVersesFragment (void * webserver_request, string bible, int book, int chapter, int verse)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <int> verses;
  if (bible == "") {
    Database_Versifications database_versifications;
    verses = database_versifications.getVerses (english (), book, chapter);
  } else {
    verses = usfm_get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
  }
  string html;
  html.append (" ");
  for (auto vs : verses) {
    bool selected = (verse == vs);
    addSelectorLink (html, convert_to_string (vs), "applyverse", convert_to_string (vs), selected);
  }
  addSelectorLink (html, "cancel", "applyverse", "[" + translate ("cancel") + "]", false);

  html.insert (0, "<span id=\"applyverse\">" + translate ("Select verse"));
  html.append ("</span>");

  return html;
}


string Navigation_Passage::code (string bible)
{
  string code;
  code += "<script type=\"text/javascript\">";
  code += "navigationBible = \"" + bible + "\";";
  code += "</script>";
  code += "\n";
  return code;
}


void Navigation_Passage::setBook (void * webserver_request, int book)
{
  Ipc_Focus::set (webserver_request, book, 1, 1);
  recordHistory (webserver_request, book, 1, 1);
}


void Navigation_Passage::setChapter (void * webserver_request, int chapter)
{
  int book = Ipc_Focus::getBook (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, 1);
  recordHistory (webserver_request, book, chapter, 1);
}


void Navigation_Passage::setVerse (void * webserver_request, int verse)
{
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  Ipc_Focus::set (webserver_request, book, chapter, verse);
  recordHistory (webserver_request, book, chapter, verse);
}


void Navigation_Passage::setPassage (void * webserver_request, string bible, string passage)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  passage = filter_string_trim (passage);
  Passage passage_to_set;
  if ((passage == "") || (passage == "+")) {
    passage_to_set = Navigation_Passage::getNextVerse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  } else if (passage == "-") {
    passage_to_set = Navigation_Passage::getPreviousVerse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  } else {
    Passage inputpassage = Passage ("", currentBook, currentChapter, convert_to_string (currentVerse));
    passage_to_set = filter_passage_interpret_passage (inputpassage, passage);
  }
  if (passage_to_set.book != 0) {
    Ipc_Focus::set (webserver_request, passage_to_set.book, passage_to_set.chapter, convert_to_int (passage_to_set.verse));
    Navigation_Passage::recordHistory (webserver_request, passage_to_set.book, passage_to_set.chapter, convert_to_int (passage_to_set.verse));
  }
}



Passage Navigation_Passage::getNextChapter (void * webserver_request, string bible, int book, int chapter)
{
  chapter++;
  if (bible != "") {
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty()) chapter = chapters.back ();
    }
  }
  Passage passage = Passage ("", book, chapter, "1");
  return passage;
}


Passage Navigation_Passage::getPreviousChapter (void * webserver_request, string bible, int book, int chapter)
{
  chapter--;
  if (bible != "") {
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
    if (find (chapters.begin(), chapters.end(), chapter) == chapters.end()) {
      if (!chapters.empty ()) chapter = chapters [0];
    }
  }
  Passage passage = Passage ("", book, chapter, "1");
  return passage;
}


void Navigation_Passage::gotoNextChapter (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  Passage passage = Navigation_Passage::getNextChapter (webserver_request, bible, currentBook, currentChapter);
  if (passage.book != 0) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
    Navigation_Passage::recordHistory (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


void Navigation_Passage::gotoPreviousChapter (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  Passage passage = Navigation_Passage::getPreviousChapter (webserver_request, bible, currentBook, currentChapter);
  if (passage.book != 0) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
    Navigation_Passage::recordHistory (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


Passage Navigation_Passage::getNextVerse (void * webserver_request, string bible, int book, int chapter, int verse)
{
  verse++;
  if (bible != "") {
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    vector <int> verses = usfm_get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty()) verse = verses.back ();
    }
  }
  Passage passage = Passage ("", book, chapter, convert_to_string (verse));
  return passage;
}


Passage Navigation_Passage::getPreviousVerse (void * webserver_request, string bible, int book, int chapter, int verse)
{
  verse--;
  if (bible != "") {
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    vector <int> verses = usfm_get_verse_numbers (request->database_bibles()->getChapter (bible, book, chapter));
    if (find (verses.begin(), verses.end(), verse) == verses.end()) {
      if (!verses.empty ()) verse = verses [0];
    }
  }
  Passage passage = Passage ("", book, chapter, convert_to_string (verse));
  return passage;
}


void Navigation_Passage::gotoNextVerse (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  Passage passage = Navigation_Passage::getNextVerse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  if (passage.book != 0) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
    Navigation_Passage::recordHistory (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


void Navigation_Passage::gotoPreviousVerse (void * webserver_request, string bible)
{
  int currentBook = Ipc_Focus::getBook (webserver_request);
  int currentChapter = Ipc_Focus::getChapter (webserver_request);
  int currentVerse = Ipc_Focus::getVerse (webserver_request);
  Passage passage = Navigation_Passage::getPreviousVerse (webserver_request, bible, currentBook, currentChapter, currentVerse);
  if (passage.book != 0) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
    Navigation_Passage::recordHistory (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


void Navigation_Passage::recordHistory (void * webserver_request, int book, int chapter, int verse)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic()->currentUser ();
  Database_Navigation database_navigation;
  database_navigation.record (filter_date_seconds_since_epoch (), user, book, chapter, verse);
}


void Navigation_Passage::goBack (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Navigation database_navigation;
  string user = request->session_logic()->currentUser ();
  Passage passage = database_navigation.getPrevious (user);
  if (passage.book) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


void Navigation_Passage::goForward (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Navigation database_navigation;
  string user = request->session_logic()->currentUser ();
  Passage passage = database_navigation.getNext (user);
  if (passage.book) {
    Ipc_Focus::set (webserver_request, passage.book, passage.chapter, convert_to_int (passage.verse));
  }
}


void Navigation_Passage::addSelectorLink (string& html, string id, string href, string text, bool selected)
{
  // Add bit to cause wrapping between the books or chapters or verses.
  if (!html.empty ()) html.append (" ");

  string isactive = "";
  if (selected) isactive  = " active";
  
  // No wrapping of a book name made of more than one word.
  html.append ("<span class='selector" + isactive + "'><a id='" + id + "apply' href='" + href + "'>");
  html.append (text);
  html.append ("</a></span>");
}

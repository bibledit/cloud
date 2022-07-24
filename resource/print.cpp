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


#include <resource/print.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/jobs.h>
#include <database/books.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <jobs/index.h>
#include <dialog/list.h>
#include <menu/logic.h>


string resource_print_url ()
{
  return "resource/print";
}


bool resource_print_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string resource_print (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Print"), request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;

  
  Database_Jobs database_jobs = Database_Jobs ();
  
  
  string bible = request->database_config_user()->getBible ();
  

  if (request->query.count ("add") || request->post.count ("add")) {
    string add = request->query["add"];
    if (add.empty ()) add = request->post ["add"];
    vector <string> resources = request->database_config_user()->getPrintResources ();
    resources.push_back (add);
    request->database_config_user()->setPrintResources (resources);
  }
  
  
  if (request->query.count ("generate")) {
    int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, Filter_Roles::consultant ());
    string username = request->session_logic()->currentUser ();
    tasks_logic_queue (PRINTRESOURCES, {convert_to_string (jobId), username, bible});
    redirect_browser (request, jobs_index_url () + "?id=" + convert_to_string (jobId));
    return "";
  }
  
  
  if (request->query.count ("remove")) {
    size_t offset = static_cast<size_t>(abs (convert_to_int (request->query["remove"])));
    vector <string> resources = request->database_config_user()->getPrintResources ();
    if (!resources.empty () && (offset < resources.size ())) {
      string remove = resources [offset];
      resources = filter_string_array_diff (resources, {remove});
      request->database_config_user()->setPrintResources (resources);
    }
  }
  
  
  string moveup = request->query ["moveup"];
  string movedown = request->query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = static_cast<size_t> (convert_to_int (moveup + movedown));
    vector <string> resources = request->database_config_user()->getPrintResources ();
    array_move_up_down (resources, move, !moveup.empty ());
    request->database_config_user()->setPrintResources (resources);
  }
  
  
  if (request->query.count ("frombook")) {
    string frombook = request->query["frombook"];
    if (frombook == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a book"), "", "");
      vector <int> books = request->database_bibles()->getBooks (bible);
      for (auto & book : books) {
        string bookname = Database_Books::getEnglishFromId (book);
        dialog_list.add_row (bookname, "frombook", convert_to_string (book));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set where to start from: Set book, chapter 1, verse 0.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      frompassage.m_book = convert_to_int (frombook);
      frompassage.m_chapter = 0;
      frompassage.m_verse = "0";
      request->database_config_user()->setPrintPassageFrom (frompassage);
      // Check if ending book matches.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set ending passage to a sensible value.
        topassage.m_book = convert_to_int (frombook);
        vector <int> chapters = request->database_bibles()->getChapters (bible, topassage.m_book);
        topassage.m_chapter = frompassage.m_chapter;
        if (!chapters.empty ()) topassage.m_chapter = chapters.back ();
        vector <int> verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = convert_to_string (verses.back ());
        request->database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (request->query.count ("fromchapter")) {
    string fromchapter = request->query["fromchapter"];
    if (fromchapter == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a chapter"), "", "");
      Passage passage = request->database_config_user()->getPrintPassageFrom ();
      vector <int> chapters = request->database_bibles()->getChapters (bible, passage.m_book);
      for (auto & chapter : chapters) {
        dialog_list.add_row (convert_to_string (chapter), "fromchapter", convert_to_string (chapter));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set which chapter to start from, and the verse also.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      frompassage.m_chapter = convert_to_int (fromchapter);
      frompassage.m_verse = "0";
      request->database_config_user()->setPrintPassageFrom (frompassage);
      // Check if ending passage is sensible.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set ending chapter / verse to sensible values.
        topassage.m_chapter = convert_to_int (fromchapter);
        vector <int> verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = convert_to_string (verses.back ());
        request->database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (request->query.count("fromverse")) {
    string fromverse = request->query["fromverse"];
    if (fromverse == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a verse"), "", "");
      Passage passage = request->database_config_user()->getPrintPassageFrom ();
      string usfm = request->database_bibles()->getChapter (bible, passage.m_book, passage.m_chapter);
      vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        dialog_list.add_row (convert_to_string (verse), "fromverse", convert_to_string (verse));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set verse.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      frompassage.m_verse = fromverse;
      request->database_config_user()->setPrintPassageFrom (frompassage);
      // Sensible matching ending verse.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        vector <int> verses = filter::usfm::get_verse_numbers (request->database_bibles()->getChapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = convert_to_string (verses.back ());
        request->database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (request->query.count ("tobook")) {
    string tobook = request->query["tobook"];
    if (tobook == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a book"), "", "");
      vector <int> books = request->database_bibles()->getBooks (bible);
      for (auto & book : books) {
        string bookname = Database_Books::getEnglishFromId (book);
        dialog_list.add_row (bookname, "tobook", convert_to_string (book));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set ending passage.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      topassage.m_book = convert_to_int (tobook);
      topassage.m_chapter = 1;
      topassage.m_verse = "0";
      request->database_config_user()->setPrintPassageTo (topassage);
      // Check on matching starting book.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = convert_to_int (tobook);
        frompassage.m_chapter = 0;
        frompassage.m_verse = "0";
        request->database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  if (request->query.count ("tochapter")) {
    string tochapter = request->query["tochapter"];
    if (tochapter == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a chapter"), "", "");
      Passage passage = request->database_config_user()->getPrintPassageTo ();
      vector <int> chapters = request->database_bibles()->getChapters (bible, passage.m_book);
      for (auto & chapter : chapters) {
        dialog_list.add_row (convert_to_string (chapter), "tochapter", convert_to_string (chapter));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set chapter.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      topassage.m_chapter = convert_to_int (tochapter);
      topassage.m_verse = "0";
      request->database_config_user()->setPrintPassageTo (topassage);
      // Match starting passage.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = topassage.m_book;
        frompassage.m_chapter = 0;
        frompassage.m_verse = "0";
        request->database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  if (request->query.count ("toverse")) {
    string toverse = request->query["toverse"];
    if (toverse == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a verse"), "", "");
      Passage passage = request->database_config_user()->getPrintPassageTo ();
      string usfm = request->database_bibles()->getChapter (bible, passage.m_book, passage.m_chapter);
      vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        dialog_list.add_row (convert_to_string (verse), "toverse", convert_to_string (verse));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set ending verse.
      Passage topassage = request->database_config_user()->getPrintPassageTo ();
      topassage.m_verse = toverse;
      request->database_config_user()->setPrintPassageTo (topassage);
      // Match starting verse.
      Passage frompassage = request->database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = topassage.m_book;
        frompassage.m_chapter = topassage.m_chapter;
        frompassage.m_verse = "0";
        request->database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  vector <string> resources = request->database_config_user()->getPrintResources ();
  for (size_t i = 0; i < resources.size (); i++) {
    string offset = convert_to_string (i);
    string name = resources[i];
    view.add_iteration ("resources", { pair ("offset", offset), pair ("name", name) } );
  }
  view.set_variable ("trash", emoji_wastebasket ());
  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());



  Passage passage = request->database_config_user()->getPrintPassageFrom ();
  view.set_variable ("from_book", Database_Books::getEnglishFromId (passage.m_book));
  view.set_variable ("from_chapter", convert_to_string (passage.m_chapter));
  view.set_variable ("from_verse", passage.m_verse);
  passage = request->database_config_user()->getPrintPassageTo ();
  view.set_variable ("to_book", Database_Books::getEnglishFromId (passage.m_book));
  view.set_variable ("to_chapter", convert_to_string (passage.m_chapter));
  view.set_variable ("to_verse", passage.m_verse);


  page += view.render ("resource", "print");
  page += Assets_Page::footer ();
  return page;
}


void resource_print_job (string jobId, string user, string bible)
{
  Webserver_Request request;
  request.session_logic ()->set_username (user);
  
  
  Database_Jobs database_jobs = Database_Jobs ();
  database_jobs.set_progress (convert_to_int (jobId), translate("The document is being created..."));
  
  
  vector <string> resources = request.database_config_user()->getPrintResourcesForUser (user);
  
  
  Passage from = request.database_config_user()->getPrintPassageFromForUser (user);
  int ifrom = filter_passage_to_integer (from);
  
  
  Passage to = request.database_config_user()->getPrintPassageToForUser (user);
  int ito = filter_passage_to_integer (to);
  
  
  vector <string> result;
  
  
  vector <int> books = request.database_bibles()->getBooks (bible);
  for (auto & book : books) {
    vector <int> chapters = request.database_bibles()->getChapters (bible, book);
    for (auto & chapter : chapters) {
      string usfm = request.database_bibles()->getChapter (bible, book, chapter);
      vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        int passage = filter_passage_to_integer (Passage ("", book, chapter, convert_to_string (verse)));
        if ((passage >= ifrom) && (passage <= ito)) {
          string passageText = filter_passage_display (book, chapter, convert_to_string (verse));
          database_jobs.set_progress (convert_to_int (jobId), passageText);
          result.push_back ("<div class=\"nextresource\">");
          result.push_back ("<p>" + passageText + "</p>");
          for (auto & resource : resources) {
            result.push_back ("<p>");
            result.push_back (resource);
            string html = resource_logic_get_html (&request, resource, book, chapter, verse, false);
            result.push_back (html);
            result.push_back ("</p>");
          }
          result.push_back ("</div>");
          result.push_back ("<br>");
        }
      }
    }
  }
  
  
  database_jobs.set_result (convert_to_int (jobId), filter_string_implode (result, "\n"));
}

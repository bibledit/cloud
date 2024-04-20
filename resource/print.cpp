/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


std::string resource_print_url ()
{
  return "resource/print";
}


bool resource_print_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string resource_print (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Print"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;

  
  Database_Jobs database_jobs = Database_Jobs ();
  
  
  std::string bible = webserver_request.database_config_user()->getBible ();
  

  if (webserver_request.query.count ("add") || webserver_request.post.count ("add")) {
    std::string add = webserver_request.query["add"];
    if (add.empty ()) add = webserver_request.post ["add"];
    std::vector <std::string> resources = webserver_request.database_config_user()->getPrintResources ();
    resources.push_back (add);
    webserver_request.database_config_user()->setPrintResources (resources);
  }
  
  
  if (webserver_request.query.count ("generate")) {
    int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, Filter_Roles::consultant ());
    std::string username = webserver_request.session_logic()->currentUser ();
    tasks_logic_queue (PRINTRESOURCES, {filter::strings::convert_to_string (jobId), username, bible});
    redirect_browser (webserver_request, jobs_index_url () + "?id=" + filter::strings::convert_to_string (jobId));
    return std::string();
  }
  
  
  if (webserver_request.query.count ("remove")) {
    size_t offset = static_cast<size_t>(abs (filter::strings::convert_to_int (webserver_request.query["remove"])));
    std::vector <std::string> resources = webserver_request.database_config_user()->getPrintResources ();
    if (!resources.empty () && (offset < resources.size ())) {
      std::string remove = resources [offset];
      resources = filter::strings::array_diff (resources, {remove});
      webserver_request.database_config_user()->setPrintResources (resources);
    }
  }
  
  
  std::string moveup = webserver_request.query ["moveup"];
  std::string movedown = webserver_request.query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = static_cast<size_t> (filter::strings::convert_to_int (moveup + movedown));
    std::vector <std::string> resources = webserver_request.database_config_user()->getPrintResources ();
    filter::strings::array_move_up_down (resources, move, !moveup.empty ());
    webserver_request.database_config_user()->setPrintResources (resources);
  }
  
  
  if (webserver_request.query.count ("frombook")) {
    std::string frombook = webserver_request.query["frombook"];
    if (frombook == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a book"), "", "");
      std::vector <int> books = webserver_request.database_bibles()->get_books (bible);
      for (auto & book : books) {
        std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
        dialog_list.add_row (bookname, "frombook", filter::strings::convert_to_string (book));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set where to start from: Set book, chapter 1, verse 0.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      frompassage.m_book = filter::strings::convert_to_int (frombook);
      frompassage.m_chapter = 0;
      frompassage.m_verse = "0";
      webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      // Check if ending book matches.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set ending passage to a sensible value.
        topassage.m_book = filter::strings::convert_to_int (frombook);
        std::vector <int> chapters = webserver_request.database_bibles()->get_chapters (bible, topassage.m_book);
        topassage.m_chapter = frompassage.m_chapter;
        if (!chapters.empty ()) topassage.m_chapter = chapters.back ();
        std::vector <int> verses = filter::usfm::get_verse_numbers (webserver_request.database_bibles()->get_chapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = filter::strings::convert_to_string (verses.back ());
        webserver_request.database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (webserver_request.query.count ("fromchapter")) {
    std::string fromchapter = webserver_request.query["fromchapter"];
    if (fromchapter == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a chapter"), "", "");
      Passage passage = webserver_request.database_config_user()->getPrintPassageFrom ();
      std::vector <int> chapters = webserver_request.database_bibles()->get_chapters (bible, passage.m_book);
      for (auto & chapter : chapters) {
        dialog_list.add_row (filter::strings::convert_to_string (chapter), "fromchapter", filter::strings::convert_to_string (chapter));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set which chapter to start from, and the verse also.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      frompassage.m_chapter = filter::strings::convert_to_int (fromchapter);
      frompassage.m_verse = "0";
      webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      // Check if ending passage is sensible.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set ending chapter / verse to sensible values.
        topassage.m_chapter = filter::strings::convert_to_int (fromchapter);
        std::vector <int> verses = filter::usfm::get_verse_numbers (webserver_request.database_bibles()->get_chapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = filter::strings::convert_to_string (verses.back ());
        webserver_request.database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (webserver_request.query.count("fromverse")) {
    std::string fromverse = webserver_request.query["fromverse"];
    if (fromverse == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a verse"), "", "");
      Passage passage = webserver_request.database_config_user()->getPrintPassageFrom ();
      std::string usfm = webserver_request.database_bibles()->get_chapter (bible, passage.m_book, passage.m_chapter);
      std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        dialog_list.add_row (filter::strings::convert_to_string (verse), "fromverse", filter::strings::convert_to_string (verse));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set verse.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      frompassage.m_verse = fromverse;
      webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      // Sensible matching ending verse.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        std::vector <int> verses = filter::usfm::get_verse_numbers (webserver_request.database_bibles()->get_chapter (bible, topassage.m_book, topassage.m_chapter));
        topassage.m_verse = frompassage.m_verse;
        if (!verses.empty ()) topassage.m_verse = filter::strings::convert_to_string (verses.back ());
        webserver_request.database_config_user()->setPrintPassageTo (topassage);
      }
    }
  }
  
  
  if (webserver_request.query.count ("tobook")) {
    std::string tobook = webserver_request.query["tobook"];
    if (tobook == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a book"), "", "");
      std::vector <int> books = webserver_request.database_bibles()->get_books (bible);
      for (auto & book : books) {
        std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
        dialog_list.add_row (bookname, "tobook", filter::strings::convert_to_string (book));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set ending passage.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      topassage.m_book = filter::strings::convert_to_int (tobook);
      topassage.m_chapter = 1;
      topassage.m_verse = "0";
      webserver_request.database_config_user()->setPrintPassageTo (topassage);
      // Check on matching starting book.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = filter::strings::convert_to_int (tobook);
        frompassage.m_chapter = 0;
        frompassage.m_verse = "0";
        webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  if (webserver_request.query.count ("tochapter")) {
    std::string tochapter = webserver_request.query["tochapter"];
    if (tochapter == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a chapter"), "", "");
      Passage passage = webserver_request.database_config_user()->getPrintPassageTo ();
      std::vector <int> chapters = webserver_request.database_bibles()->get_chapters (bible, passage.m_book);
      for (auto & chapter : chapters) {
        dialog_list.add_row (filter::strings::convert_to_string (chapter), "tochapter", filter::strings::convert_to_string (chapter));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set chapter.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      topassage.m_chapter = filter::strings::convert_to_int (tochapter);
      topassage.m_verse = "0";
      webserver_request.database_config_user()->setPrintPassageTo (topassage);
      // Match starting passage.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = topassage.m_book;
        frompassage.m_chapter = 0;
        frompassage.m_verse = "0";
        webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  if (webserver_request.query.count ("toverse")) {
    std::string toverse = webserver_request.query["toverse"];
    if (toverse == "") {
      Dialog_List dialog_list = Dialog_List ("print", translate("Select a verse"), "", "");
      Passage passage = webserver_request.database_config_user()->getPrintPassageTo ();
      std::string usfm = webserver_request.database_bibles()->get_chapter (bible, passage.m_book, passage.m_chapter);
      std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        dialog_list.add_row (filter::strings::convert_to_string (verse), "toverse", filter::strings::convert_to_string (verse));
      }
      page += dialog_list.run ();
      return page;
    } else {
      // Set ending verse.
      Passage topassage = webserver_request.database_config_user()->getPrintPassageTo ();
      topassage.m_verse = toverse;
      webserver_request.database_config_user()->setPrintPassageTo (topassage);
      // Match starting verse.
      Passage frompassage = webserver_request.database_config_user()->getPrintPassageFrom ();
      if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
        // Set starting passage to a sensible value.
        frompassage.m_book = topassage.m_book;
        frompassage.m_chapter = topassage.m_chapter;
        frompassage.m_verse = "0";
        webserver_request.database_config_user()->setPrintPassageFrom (frompassage);
      }
    }
  }
  
  
  std::vector <std::string> resources = webserver_request.database_config_user()->getPrintResources ();
  for (size_t i = 0; i < resources.size (); i++) {
    std::string offset = filter::strings::convert_to_string (i);
    std::string name = resources[i];
    view.add_iteration ("resources", { std::pair ("offset", offset), std::pair ("name", name) } );
  }
  view.set_variable ("trash", filter::strings::emoji_wastebasket ());
  view.set_variable ("uparrow", filter::strings::unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", filter::strings::unicode_black_down_pointing_triangle ());



  Passage passage = webserver_request.database_config_user()->getPrintPassageFrom ();
  view.set_variable ("from_book", database::books::get_english_from_id (static_cast<book_id>(passage.m_book)));
  view.set_variable ("from_chapter", filter::strings::convert_to_string (passage.m_chapter));
  view.set_variable ("from_verse", passage.m_verse);
  passage = webserver_request.database_config_user()->getPrintPassageTo ();
  view.set_variable ("to_book", database::books::get_english_from_id (static_cast<book_id>(passage.m_book)));
  view.set_variable ("to_chapter", filter::strings::convert_to_string (passage.m_chapter));
  view.set_variable ("to_verse", passage.m_verse);


  page += view.render ("resource", "print");
  page += assets_page::footer ();
  return page;
}


void resource_print_job (std::string jobId, std::string user, std::string bible)
{
  Webserver_Request webserver_request;
  webserver_request.session_logic ()->set_username (user);
  
  
  Database_Jobs database_jobs = Database_Jobs ();
  database_jobs.set_progress (filter::strings::convert_to_int (jobId), translate("The document is being created..."));
  
  
  std::vector <std::string> resources = webserver_request.database_config_user()->getPrintResourcesForUser (user);
  
  
  Passage from = webserver_request.database_config_user()->getPrintPassageFromForUser (user);
  int ifrom = filter_passage_to_integer (from);
  
  
  Passage to = webserver_request.database_config_user()->getPrintPassageToForUser (user);
  int ito = filter_passage_to_integer (to);
  
  
  std::vector <std::string> result;
  
  
  std::vector <int> books = webserver_request.database_bibles()->get_books (bible);
  for (auto & book : books) {
    std::vector <int> chapters = webserver_request.database_bibles()->get_chapters (bible, book);
    for (auto & chapter : chapters) {
      std::string usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
      std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        int passage = filter_passage_to_integer (Passage ("", book, chapter, filter::strings::convert_to_string (verse)));
        if ((passage >= ifrom) && (passage <= ito)) {
          std::string passageText = filter_passage_display (book, chapter, filter::strings::convert_to_string (verse));
          database_jobs.set_progress (filter::strings::convert_to_int (jobId), passageText);
          result.push_back ("<div class=\"nextresource\">");
          result.push_back ("<p>" + passageText + "</p>");
          for (auto & resource : resources) {
            result.push_back ("<p>");
            result.push_back (resource);
            std::string html = resource_logic_get_html (webserver_request, resource, book, chapter, verse, false);
            result.push_back (html);
            result.push_back ("</p>");
          }
          result.push_back ("</div>");
          result.push_back ("<br>");
        }
      }
    }
  }
  
  
  database_jobs.set_result (filter::strings::convert_to_int (jobId), filter::strings::implode (result, "\n"));
}

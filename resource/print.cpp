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
#include <dialog/select.h>
#include <menu/logic.h>


std::string resource_print_url ()
{
  return "resource/print";
}


bool resource_print_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string resource_print (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Print"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;

  
  Database_Jobs database_jobs = Database_Jobs ();
  
  
  std::string bible = webserver_request.database_config_user()->get_bible ();
  

  if (webserver_request.query.count ("add") || webserver_request.post.count ("add")) {
    std::string add = webserver_request.query["add"];
    if (add.empty ()) add = webserver_request.post ["add"];
    std::vector <std::string> resources = webserver_request.database_config_user()->get_print_resources ();
    resources.push_back (add);
    webserver_request.database_config_user()->set_print_resources (resources);
  }
  
  
  if (webserver_request.query.count ("generate")) {
    int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, roles::consultant);
    const std::string& username = webserver_request.session_logic ()->get_username ();
    tasks_logic_queue (task::print_resources, {std::to_string (jobId), username, bible});
    redirect_browser (webserver_request, jobs_index_url () + "?id=" + std::to_string (jobId));
    return std::string();
  }
  
  
  if (webserver_request.query.count ("remove")) {
    size_t offset = static_cast<size_t>(abs (filter::strings::convert_to_int (webserver_request.query["remove"])));
    std::vector <std::string> resources = webserver_request.database_config_user()->get_print_resources ();
    if (!resources.empty () && (offset < resources.size ())) {
      std::string remove = resources [offset];
      resources = filter::strings::array_diff (resources, {remove});
      webserver_request.database_config_user()->set_print_resources (resources);
    }
  }
  
  
  std::string moveup = webserver_request.query ["moveup"];
  std::string movedown = webserver_request.query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = static_cast<size_t> (filter::strings::convert_to_int (moveup + movedown));
    std::vector <std::string> resources = webserver_request.database_config_user()->get_print_resources ();
    filter::strings::array_move_up_down (resources, move, !moveup.empty ());
    webserver_request.database_config_user()->set_print_resources (resources);
  }

  
  constexpr const char* frombook    {"frombook"};
  constexpr const char* fromchapter {"fromchapter"};
  constexpr const char* fromverse   {"fromverse"};
  constexpr const char* tobook      {"tobook"};
  constexpr const char* tochapter   {"tochapter"};
  constexpr const char* toverse     {"toverse"};

  
  Passage frompassage = webserver_request.database_config_user()->get_print_passage_from ();
  Passage topassage = webserver_request.database_config_user()->get_print_passage_to ();


  if (webserver_request.post.count (frombook)) {
    // Set where to start from: Set book, chapter 1, verse 0.
    frompassage.m_book = filter::strings::convert_to_int (webserver_request.post.at(frombook));
    frompassage.m_chapter = 0;
    frompassage.m_verse = "0";
    // Check if ending book matches.
    if (topassage.m_book < frompassage.m_book) {
      // Set ending passage to a sensible value.
      topassage.m_book = frompassage.m_book;
      std::vector <int> chapters = database::bibles::get_chapters (bible, topassage.m_book);
      topassage.m_chapter = frompassage.m_chapter;
      if (!chapters.empty ()) topassage.m_chapter = chapters.back ();
      std::vector <int> verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, topassage.m_book, topassage.m_chapter));
      topassage.m_verse = frompassage.m_verse;
      if (!verses.empty ()) topassage.m_verse = std::to_string (verses.back ());
    }
  }
  

  if (webserver_request.post.count (fromchapter)) {
    // Set which chapter to start from, and the verse also.
    frompassage.m_chapter = filter::strings::convert_to_int (webserver_request.post.at(fromchapter));
    frompassage.m_verse = "0";
    // Check if ending passage is sensible.
    if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
      // Set ending chapter / verse to sensible values.
      topassage.m_chapter = frompassage.m_chapter;
      std::vector <int> verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, topassage.m_book, topassage.m_chapter));
      topassage.m_verse = frompassage.m_verse;
      if (!verses.empty ()) topassage.m_verse = std::to_string (verses.back ());
    }
  }
  
  
  if (webserver_request.post.count (fromverse)) {
    frompassage.m_verse = webserver_request.post.at(fromverse);
    // Sensible matching ending verse.
    if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
      std::vector <int> verses = filter::usfm::get_verse_numbers (database::bibles::get_chapter (bible, topassage.m_book, topassage.m_chapter));
      topassage.m_verse = frompassage.m_verse;
      if (!verses.empty ()) topassage.m_verse = std::to_string (verses.back ());
    }
  }

  
  if (webserver_request.post.count (tobook)) {
    // Set ending passage.
    topassage.m_book = filter::strings::convert_to_int (webserver_request.post.at(tobook));
    topassage.m_chapter = 1;
    topassage.m_verse = "0";
    // Check on matching starting book.
    if (topassage.m_book < frompassage.m_book) {
      // Set starting passage to a sensible value.
      frompassage.m_book = topassage.m_book;
      frompassage.m_chapter = 0;
      frompassage.m_verse = "0";
    }
  }
  
  
  if (webserver_request.post.count (tochapter)) {
    // Set chapter.
    topassage.m_chapter = filter::strings::convert_to_int(webserver_request.post.at(tochapter));
    topassage.m_verse = "0";
    // Match starting passage.
    if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
      // Set starting passage to a sensible value.
      frompassage.m_book = topassage.m_book;
      frompassage.m_chapter = 0;
      frompassage.m_verse = "0";
    }
  }
  
  
  if (webserver_request.post.count (toverse)) {
    // Set ending verse.
    topassage.m_verse = webserver_request.post.at(toverse);
    // Match starting verse.
    if (filter_passage_to_integer (topassage) < filter_passage_to_integer (frompassage)) {
      // Set starting passage to a sensible value.
      frompassage.m_book = topassage.m_book;
      frompassage.m_chapter = topassage.m_chapter;
      frompassage.m_verse = "0";
    }
  }


  // Save the passages. They could have been modified.
  webserver_request.database_config_user()->set_print_passage_from (frompassage);
  webserver_request.database_config_user()->set_print_passage_to (topassage);


  
  // Set the values in the UI after they have been refined above.
  {
    const std::vector<int>books = database::bibles::get_books (bible);
    std::vector<std::string> values, displayed;
    for (const auto book : books) {
      values.push_back(std::to_string(book));
      displayed.push_back(database::books::get_english_from_id (static_cast<book_id>(book)));
    }
    dialog::select::Settings settings {
      .identification = frombook,
      .values = std::move(values),
      .displayed = std::move(displayed),
      .selected = std::to_string(frompassage.m_book),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(frombook, dialog::select::form(settings, form));
  }
  {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, frompassage.m_book);
    std::vector<std::string> values;
    for (const auto chapter : chapters) {
      values.push_back(std::to_string(chapter));
    }
    dialog::select::Settings settings {
      .identification = fromchapter,
      .values = std::move(values),
      .selected = std::to_string(frompassage.m_chapter),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(fromchapter, dialog::select::form(settings, form));
  }
  {
    std::vector<std::string> values;
    const std::string usfm = database::bibles::get_chapter (bible, frompassage.m_book, frompassage.m_chapter);
    const std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    for (const auto verse : verses) {
      values.push_back(std::to_string (verse));
    }
    dialog::select::Settings settings {
      .identification = fromverse,
      .values = std::move(values),
      .selected = frompassage.m_verse,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(fromverse, dialog::select::form(settings, form));
  }
  {
    std::vector <int> books = database::bibles::get_books (bible);
    std::vector<std::string> values, displayed;
    for (const auto book : books) {
      values.push_back(std::to_string(book));
      displayed.push_back(database::books::get_english_from_id (static_cast<book_id>(book)));
    }
    dialog::select::Settings settings {
      .identification = tobook,
      .values = std::move(values),
      .displayed = std::move(displayed),
      .selected = std::to_string(topassage.m_book),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(tobook, dialog::select::form(settings, form));
  }
  {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, topassage.m_book);
    std::vector<std::string> values;
    for (const auto chapter : chapters) {
      values.push_back(std::to_string(chapter));
    }
    dialog::select::Settings settings {
      .identification = tochapter,
      .values = std::move(values),
      .selected = std::to_string(topassage.m_chapter),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(tochapter, dialog::select::form(settings, form));
  }
  {
    std::vector<std::string> values;
    const std::string usfm = database::bibles::get_chapter (bible, topassage.m_book, topassage.m_chapter);
    const std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
    for (const auto verse : verses) {
      values.push_back(std::to_string (verse));
    }
    dialog::select::Settings settings {
      .identification = toverse,
      .values = std::move(values),
      .selected = topassage.m_verse,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(toverse, dialog::select::form(settings, form));
  }

  
  std::vector <std::string> resources = webserver_request.database_config_user()->get_print_resources ();
  for (size_t i = 0; i < resources.size (); i++) {
    std::string offset = std::to_string (i);
    std::string name = resources[i];
    view.add_iteration ("resources", { std::pair ("offset", offset), std::pair ("name", name) } );
  }
  view.set_variable ("trash", filter::strings::emoji_wastebasket ());
  view.set_variable ("uparrow", filter::strings::unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", filter::strings::unicode_black_down_pointing_triangle ());



  Passage passage = webserver_request.database_config_user()->get_print_passage_from ();
  view.set_variable ("from_book", database::books::get_english_from_id (static_cast<book_id>(passage.m_book)));
  view.set_variable ("from_chapter", std::to_string (passage.m_chapter));
  view.set_variable ("from_verse", passage.m_verse);
  passage = webserver_request.database_config_user()->get_print_passage_to ();
  view.set_variable ("to_book", database::books::get_english_from_id (static_cast<book_id>(passage.m_book)));
  view.set_variable ("to_chapter", std::to_string (passage.m_chapter));
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
  
  
  std::vector <std::string> resources = webserver_request.database_config_user()->get_print_resources_for_user (user);
  
  
  Passage from = webserver_request.database_config_user()->get_print_passage_from_for_user (user);
  int ifrom = filter_passage_to_integer (from);
  
  
  Passage to = webserver_request.database_config_user()->get_print_passage_to_for_user (user);
  int ito = filter_passage_to_integer (to);
  
  
  std::vector <std::string> result;
  
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto & book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto & chapter : chapters) {
      std::string usfm = database::bibles::get_chapter (bible, book, chapter);
      std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
      for (auto & verse : verses) {
        int passage = filter_passage_to_integer (Passage ("", book, chapter, std::to_string (verse)));
        if ((passage >= ifrom) && (passage <= ito)) {
          std::string passageText = filter_passage_display (book, chapter, std::to_string (verse));
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

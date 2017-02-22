/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <bible/book.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/mappings.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <dialog/list.h>
#include <dialog/books.h>
#include <access/bible.h>
#include <book/create.h>
#include <bible/logic.h>
#include <client/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <bible/manage.h>


string bible_book_url ()
{
  return "bible/book";
}


bool bible_book_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string bible_book (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Book"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  string success_message;
  string error_message;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query["bible"]);
  view.set_variable ("bible", filter_string_sanitize_html (bible));
  
  // The book.
  int book = convert_to_int (request->query ["book"]);
  view.set_variable ("book", convert_to_string (book));
  string book_name = Database_Books::getEnglishFromId (book);
  view.set_variable ("book_name", filter_string_sanitize_html (book_name));
  
  // Whether the user has write access to this Bible book.
  bool write_access = access_bible_book_write (request, "", bible, book);
  if (write_access) view.enable_zone ("write_access");
  
  // Delete chapter.
  string deletechapter = request->query ["deletechapter"];
  if (deletechapter != "") {
    string confirm = request->query ["confirm"];
    if (confirm == "") {
      Dialog_Yes dialog_yes = Dialog_Yes ("book", translate("Would you like to delete this chapter?"));
      dialog_yes.add_query ("bible", bible);
      dialog_yes.add_query ("book", convert_to_string (book));
      dialog_yes.add_query ("deletechapter", deletechapter);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      if (write_access) bible_logic_delete_chapter (bible, book, convert_to_int (deletechapter));
    }
  }
  
  // Add chapter.
  if (request->query.count ("createchapter")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("book", translate("Please enter the number for the new chapter"), "", "createchapter", "");
    dialog_entry.add_query ("bible", bible);
    dialog_entry.add_query ("book", convert_to_string (book));
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("createchapter")) {
    int createchapter = convert_to_int (request->post ["entry"]);
    vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
    // Only create the chapters if it does not yet exist.
    if (find (chapters.begin(), chapters.end(), createchapter) == chapters.end()) {
      vector <string> feedback;
      bool result = true;
      if (write_access) result = book_create (bible, book, createchapter, feedback);
      string message = filter_string_implode (feedback, " ");
      if (result) success_message = message;
      else error_message = message;
    } else {
      error_message = translate ("This chapter already exists");
    }
  }
  
  // Available chapters.
  vector <int> chapters = request->database_bibles ()->getChapters (bible, book);
  string chapterblock;
  for (auto & chapter : chapters) {
    chapterblock.append ("<a href=\"chapter?bible=");
    chapterblock.append (bible);
    chapterblock.append ("&book=");
    chapterblock.append (convert_to_string (book));
    chapterblock.append ("&chapter=");
    chapterblock.append (convert_to_string (chapter));
    chapterblock.append ("\">");
    chapterblock.append (convert_to_string (chapter));
    chapterblock.append ("</a>\n");
  }
  view.set_variable ("chapterblock", chapterblock);
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  if (!client_logic_client_enabled ()) view.enable_zone ("server");

  page += view.render ("bible", "book");
  
  page += Assets_Page::footer ();
  
  return page;
}

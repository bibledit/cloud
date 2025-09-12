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


#include <notes/create.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <notes/index.h>
#include <database/modifications.h>
#include <menu/logic.h>
#include <access/logic.h>


std::string notes_create_url ()
{
  return "notes/create";
}


bool notes_create_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_create_comment_notes (webserver_request);
}


std::string notes_create (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Create note"), webserver_request);
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.add_bread_crumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;

  
  // Is is possible to pass a Bible to this script.
  // The note will then be created for this Bible.
  // If no Bible is passed, it takes the user's active Bible.
  std::string bible = webserver_request.post_get("bible");
  if (bible.empty ()) {
    bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  }
  
  
  int book;
  if (webserver_request.post_count("book")) book = filter::strings::convert_to_int (webserver_request.post_get("book"));
  else book = Ipc_Focus::getBook (webserver_request);
  int chapter;
  if (webserver_request.post_count("chapter")) chapter = filter::strings::convert_to_int (webserver_request.post_get("chapter"));
  else chapter = Ipc_Focus::getChapter (webserver_request);
  int verse;
  if (webserver_request.post_count("verse")) verse = filter::strings::convert_to_int (webserver_request.post_get("verse"));
  else verse = Ipc_Focus::getVerse (webserver_request);

  
  if (webserver_request.post_count("summary")) {
    std::string summary = filter::strings::trim (webserver_request.post_get("summary"));
    summary = filter_url_tag_to_plus (summary);
    std::string body = filter::strings::trim (webserver_request.post_get("body"));
    body = filter_url_tag_to_plus (body);
    notes_logic.createNote (bible, book, chapter, verse, summary, body, false);
    return std::string();
  }

  
  if (webserver_request.post_count("cancel")) {
    redirect_browser (webserver_request, notes_index_url ());
    return std::string();
  }
  

  // This script can be called from a change notification.
  // It will then create a note based on that change notification.
  if (webserver_request.query.count ("fromchange")) {
    int fromchange = filter::strings::convert_to_int (webserver_request.query ["fromchange"]);
    //string bible = database::modifications::getNotificationBible (fromchange);
    std::string summary = translate("Query about a change in the text");
    std::string contents = "<p>" + translate("Old text:") + "</p>";
    contents += database::modifications::getNotificationOldText (fromchange);
    contents += "<p>" +  translate("Change:") + "</p>";
    contents += "<p>" + database::modifications::getNotificationModification (fromchange) + "</p>";
    contents += "<p>" + translate("New text:") + "</p>";
    contents += database::modifications::getNotificationNewText (fromchange);
    view.set_variable ("summary", summary);
    view.set_variable ("contents", contents);
  }

  
  view.set_variable ("bible", bible);
  view.set_variable ("book", std::to_string (book));
  view.set_variable ("chapter", std::to_string (chapter));
  view.set_variable ("verse", std::to_string (verse));
  std::string passage = filter_passage_display (book, chapter, std::to_string (verse));
  view.set_variable ("passage", passage);
  if (webserver_request.database_config_user ()->get_show_verse_text_at_create_note ()) {
    std::string versetext;
    std::string chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
    std::string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    const std::string stylesheet = stylesv2::standard_sheet ();
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    versetext = filter_text.text_text->get ();
    view.set_variable ("versetext", versetext);
  }
                                                                                                      
  
  page += view.render ("notes", "create");

  page += assets_page::footer ();
  
  return page;
}

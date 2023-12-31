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
using namespace std;


string notes_create_url ()
{
  return "notes/create";
}


bool notes_create_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_create_comment_notes (std::addressof(webserver_request));
}


string notes_create (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (std::addressof(webserver_request));
  Notes_Logic notes_logic = Notes_Logic (std::addressof(webserver_request));
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Create note"), std::addressof(webserver_request));
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.add_bread_crumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;

  
  // Is is possible to pass a Bible to this script.
  // The note will then be created for this Bible.
  // If no Bible is passed, it takes the user's active Bible.
  string bible = webserver_request.post ["bible"];
  if (bible.empty ()) {
    bible = access_bible::clamp (std::addressof(webserver_request), webserver_request.database_config_user()->getBible ());
  }
  
  
  int book;
  if (webserver_request.post.count ("book")) book = filter::strings::convert_to_int (webserver_request.post ["book"]);
  else book = Ipc_Focus::getBook (std::addressof(webserver_request));
  int chapter;
  if (webserver_request.post.count ("chapter")) chapter = filter::strings::convert_to_int (webserver_request.post ["chapter"]);
  else chapter = Ipc_Focus::getChapter (std::addressof(webserver_request));
  int verse;
  if (webserver_request.post.count ("verse")) verse = filter::strings::convert_to_int (webserver_request.post ["verse"]);
  else verse = Ipc_Focus::getVerse (std::addressof(webserver_request));

  
  if (webserver_request.post.count ("summary")) {
    string summary = filter::strings::trim (webserver_request.post["summary"]);
    summary = filter_url_tag_to_plus (summary);
    string body = filter::strings::trim (webserver_request.post["body"]);
    body = filter_url_tag_to_plus (body);
    notes_logic.createNote (bible, book, chapter, verse, summary, body, false);
    return string();
  }

  
  if (webserver_request.post.count ("cancel")) {
    redirect_browser (std::addressof(webserver_request), notes_index_url ());
    return string();
  }
  

  // This script can be called from a change notification.
  // It will then create a note based on that change notification.
  if (webserver_request.query.count ("fromchange")) {
    int fromchange = filter::strings::convert_to_int (webserver_request.query ["fromchange"]);
    Database_Modifications database_modifications;
    //string bible = database_modifications.getNotificationBible (fromchange);
    string summary = translate("Query about a change in the text");
    string contents = "<p>" + translate("Old text:") + "</p>";
    contents += database_modifications.getNotificationOldText (fromchange);
    contents += "<p>" +  translate("Change:") + "</p>";
    contents += "<p>" + database_modifications.getNotificationModification (fromchange) + "</p>";
    contents += "<p>" + translate("New text:") + "</p>";
    contents += database_modifications.getNotificationNewText (fromchange);
    view.set_variable ("summary", summary);
    view.set_variable ("contents", contents);
  }

  
  view.set_variable ("bible", bible);
  view.set_variable ("book", filter::strings::convert_to_string (book));
  view.set_variable ("chapter", filter::strings::convert_to_string (chapter));
  view.set_variable ("verse", filter::strings::convert_to_string (verse));
  string passage = filter_passage_display (book, chapter, filter::strings::convert_to_string (verse));
  view.set_variable ("passage", passage);
  if (webserver_request.database_config_user ()->getShowVerseTextAtCreateNote ()) {
    string versetext;
    string chapter_usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
    string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    versetext = filter_text.text_text->get ();
    view.set_variable ("versetext", versetext);
  }
                                                                                                      
  
  // page += view.render ("notes", "create");
  page += view.render ("notes", "create");

  page += assets_page::footer ();
  
  return page;
}

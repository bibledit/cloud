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


#include <public/create.h>
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
#include <public/index.h>
#include <database/config/bible.h>
#include <read/index.h>
#include <config/logic.h>


std::string public_create_url ()
{
  return "public/create";
}


bool public_create_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string public_create (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Create note"), webserver_request);
  page += header.run ();
  Assets_View view;

  
  const std::string bible = webserver_request.database_config_user()->getBible ();
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);

  
  const std::string chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
  const std::string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.add_usfm_code (verse_usfm);
  filter_text.run (stylesheet);
  const std::string versetext = filter_text.html_text_standard->get_inner_html ();
  view.set_variable ("versetext", versetext);

 
  if (webserver_request.post.count ("submit")) {
    std::string summary = filter::strings::trim (webserver_request.post["summary"]);
    if (summary.empty ()) summary = translate ("Feedback");
    const std::string contents = "<p>" + versetext + "</p>" + filter::strings::trim (webserver_request.post["contents"]);
    const int identifier = notes_logic.createNote (bible, book, chapter, verse, summary, contents, false);
    // A note created by a public user is made public to all.
    database_notes.set_public (identifier, true);
    // Subscribe the user to the note.
    // Then the user receives email about any updates made on this note.
    database_notes.subscribe (identifier);
    // Go to the main public notes page.
    if (config::logic::default_bibledit_configuration ()) {
      redirect_browser (webserver_request, public_index_url ());
    }
    return std::string();
  }

  
  if (webserver_request.post.count ("cancel")) {
    if (config::logic::default_bibledit_configuration ()) {
      redirect_browser (webserver_request, public_index_url ());
    }
    return std::string();
  }
  
  
  const std::string passage = filter_passage_display (book, chapter, std::to_string (verse));
  view.set_variable ("passage", passage);
                                                                                                      
  
  page += view.render ("public", "create");
  page += assets_page::footer ();
  return page;
}

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


string public_create_url ()
{
  return "public/create";
}


bool public_create_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_create (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Create note"), request);
  page += header.run ();
  Assets_View view;

  
  string bible = request->database_config_user()->getBible ();
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);

  
  string chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  string verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new Html_Text (bible);
  filter_text.addUsfmCode (verse_usfm);
  filter_text.run (stylesheet);
  string versetext = filter_text.html_text_standard->getInnerHtml ();
  view.set_variable ("versetext", versetext);

 
  if (request->post.count ("submit")) {
    string summary = filter_string_trim (request->post["summary"]);
    if (summary.empty ()) summary = translate ("Feedback");
    string contents = "<p>" + versetext + "</p>" + filter_string_trim (request->post["contents"]);
    int identifier = notes_logic.createNote (bible, book, chapter, verse, summary, contents, false);
    // A note created by a public user is made public to all.
    database_notes.setPublic (identifier, true);
    // Subscribe the user to the note.
    // Then the user receives email about any updates made on this note.
    database_notes.subscribe (identifier);
    // Go to the main public notes page.
    redirect_browser (request, public_index_url ());
    return "";
  }

  
  if (request->post.count ("cancel")) {
    redirect_browser (request, public_index_url ());
    return "";
  }
  
  
  string passage = filter_passage_display (book, chapter, convert_to_string (verse));
  view.set_variable ("passage", passage);
                                                                                                      
  
  page += view.render ("public", "create");
  page += Assets_Page::footer ();
  return page;
}

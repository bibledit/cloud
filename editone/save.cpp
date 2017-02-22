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


#include <editone/save.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <database/git.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <editor/html2usfm.h>
#include <access/bible.h>
#include <bible/logic.h>
#include <editone/logic.h>
#include <developer/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


string editone_save_url ()
{
  return "editone/save";
}


bool editone_save_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editone_save (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  // Check on information about where to save the verse.
  bool save = (request->post.count ("bible") && request->post.count ("book") && request->post.count ("chapter") && request->post.count ("verse") && request->post.count ("html"));
  if (!save) {
    return translate("Don't know where to save");
  }

  
  string bible = request->post["bible"];
  int book = convert_to_int (request->post["book"]);
  int chapter = convert_to_int (request->post["chapter"]);
  int verse = convert_to_int (request->post["verse"]);
  string html = request->post["html"];
  string checksum = request->post["checksum"];

  
  // Checksum.
  if (Checksum_Logic::get (html) != checksum) {
    request->response_code = 409;
    return translate ("Checksum error");
  }

  
  // Decode html encoded in javascript.
  html = filter_url_tag_to_plus (html);

  
  // Check there's anything to save at all.
  html = filter_string_trim (html);
  if (html.empty ()) {
    return translate ("Nothing to save");
  }
  
  
  // Check on valid UTF-8.
  if (!unicode_string_is_valid (html)) {
    return translate ("Cannot save: Needs Unicode");
  }
  
  
  if (!access_bible_book_write (request, "", bible, book)) {
    return translate ("No write access");
  }

  
  string stylesheet = request->database_config_user()->getStylesheet();
 
  
  string usfm = editone_logic_html_to_usfm (stylesheet, html);

  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
#ifdef HAVE_CLOUD
  int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
#endif
  string oldText = request->database_bibles()->getChapter (bible, book, chapter);

  
  // Safely store the verse.
  string explanation;
  string message = usfm_safely_store_verse (request, bible, book, chapter, verse, usfm, explanation, true);
  bible_logic_unsafe_save_mail (message, explanation, username, usfm);
  if (message.empty ()) {
#ifdef HAVE_CLOUD
    // The Cloud stores details of the user's changes.
    int newID = request->database_bibles()->getChapterId (bible, book, chapter);
    string newText = request->database_bibles()->getChapter (bible, book, chapter);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave (username, bible, book, chapter, oldID, oldText, newID, newText);
    if (sendreceive_git_repository_linked (bible)) {
      Database_Git::store_chapter (username, bible, book, chapter, oldText, newText);
    }
    rss_logic_schedule_update (username, bible, book, chapter, oldText, newText);
#endif

    return locale_logic_text_saved ();
  }

  
  return message;
}

/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <edit2/save.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/merge.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <database/modifications.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/git.h>
#include <checksum/logic.h>
#include <editor/html2usfm.h>
#include <editor/usfm2html.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <edit2/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <quill/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


string edit2_save_url ()
{
  return "edit2/save";
}


bool edit2_save_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string edit2_save (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  bool post_complete = (request->post.count ("bible") && request->post.count ("book") && request->post.count ("chapter") && request->post.count ("html") && request->post.count ("checksum"));
  if (!post_complete) {
    return translate("Insufficient information");
  }

  string bible = request->post["bible"];
  int book = convert_to_int (request->post["book"]);
  int chapter = convert_to_int (request->post["chapter"]);
  string html = request->post["html"];
  string checksum = request->post["checksum"];
  string unique_id = request->post ["id"];

  if (Checksum_Logic::get (html) != checksum) {
    request->response_code = 409;
    return translate("Checksum error");
  }

  html = filter_url_tag_to_plus (html);
  html = filter_string_trim (html);

  if (html.empty ()) {
    Database_Logs::log (translate ("There was no text.") + " " + translate ("Nothing was saved.") + " " + translate ("The original text of the chapter was reloaded."));
    return translate("Nothing to save");
  }

  if (!unicode_string_is_valid (html)) {
    Database_Logs::log ("The text was not valid Unicode UTF-8. The chapter could not saved and has been reverted to the last good version.");
    return translate("Save failure");
  }
  
  if (!access_bible_book_write (request, "", bible, book)) {
    return translate("No write access");
  }

  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  Editor_Html2Usfm editor_export;
  editor_export.load (html);
  editor_export.stylesheet (stylesheet);
  editor_export.run ();
  string user_usfm = editor_export.get ();

  string ancestor_usfm = getLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);
  
  vector <BookChapterData> book_chapter_text = usfm_import (user_usfm, stylesheet);
  if (book_chapter_text.size () != 1) {
    Database_Logs::log (translate ("User tried to save something different from exactly one chapter."));
    return translate("Incorrect chapter");
  }
  
  int book_number = book_chapter_text[0].book;
  int chapter_number = book_chapter_text[0].chapter;
  user_usfm = book_chapter_text[0].data;
  bool chapter_ok = (((book_number == book) || (book_number == 0)) && (chapter_number == chapter));
  if (!chapter_ok) {
    return translate("Incorrect chapter") + " " + convert_to_string (chapter_number);
  }
  
  // Collect some data about the changes for this user
  // and for a possible merge of the user's data with the server's data.
  string username = request->session_logic()->currentUser ();
  int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
  string server_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  string newText = user_usfm;
  string oldText = ancestor_usfm;
  
  // Safekeep the USFM to save for later.
  string change = user_usfm;
  
  // Merge if the ancestor is there and differs from what's in the database.
  vector <Merge_Conflict> conflicts;
  if (!ancestor_usfm.empty ()) {
    if (server_usfm != ancestor_usfm) {
      // Prioritize the user's USFM.
      user_usfm = filter_merge_run (ancestor_usfm, server_usfm, user_usfm, true, conflicts);
      Database_Logs::log (translate ("Merging chapter."));
    }
  }
  
  // Check on the merge.
  bible_logic_merge_irregularity_mail ({username}, conflicts);
  
  // Check whether the USFM on disk has changed compared to the USFM that was loaded in the editor.
  // If there's a difference, email the user.
  // Although a merge was done, still, it's good to alert the user on this.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  if (ancestor_usfm != server_usfm) {
    bible_logic_recent_save_email (bible, book, chapter, 0, username, ancestor_usfm, server_usfm);
  }

  // Safely store the chapter.
  string explanation;
  string message = usfm_safely_store_chapter (request, bible, book, chapter, user_usfm, explanation);
  bible_logic_unsafe_save_mail (message, explanation, username, user_usfm);
  
  if (!message.empty ()) return message;

  // In server configuration, store details for the user's changes.
#ifndef HAVE_CLIENT
  int newID = request->database_bibles()->getChapterId (bible, book, chapter);
  Database_Modifications database_modifications;
  database_modifications.recordUserSave (username, bible, book, chapter, oldID, oldText, newID, newText);
  if (sendreceive_git_repository_linked (bible)) {
    Database_Git::store_chapter (username, bible, book, chapter, oldText, newText);
  }
  rss_logic_schedule_update (username, bible, book, chapter, oldText, newText);
#else
  (void) oldID;
#endif

  // Store a copy of the USFM loaded in the editor for later reference.
  storeLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);
  
  // Convert the stored USFM to html.
  // This converted html should be the same as the saved html.
  // If it differs, signal the browser to reload the chapter.
  // This also cares for renumbering and cleaning up any added or removed footnotes.
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (user_usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  string converted_html = editor_usfm2html.get ();
  // Convert to XML for comparison.
  // Remove spaces before comparing, so that entering a space in the editor does not cause a reload.
  html = html2xml (html);
  html = filter_string_str_replace (" ", "", html);
  html = filter_string_str_replace (unicode_non_breaking_space_entity (), "", html);
  filter_string_replace_between (html, "<", ">", "");
  converted_html = html2xml (converted_html);
  converted_html = filter_string_str_replace (" ", "", converted_html);
  converted_html = filter_string_str_replace (unicode_non_breaking_space_entity (), "", converted_html);
  filter_string_replace_between (converted_html, "<", ">", "");
  // If round trip conversion differs, send a known string to the browser,
  // to signal the browser to reload the reformatted chapter.
  if (html != converted_html) return locale_logic_text_reformat ();

  return locale_logic_text_saved ();
}

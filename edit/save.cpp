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


#include <edit/save.h>
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
#include <edit/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <filter/quill.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


std::string edit_save_url ()
{
  return "edit/save";
}


bool edit_save_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string edit_save (Webserver_Request& webserver_request)
{
  bool post_complete = (webserver_request.post_count("bible") && webserver_request.post_count("book") && webserver_request.post_count("chapter") && webserver_request.post_count("html") && webserver_request.post_count("checksum"));
  if (!post_complete) {
    return translate("Insufficient information");
  }

  const std::string bible = webserver_request.post_get("bible");
  const int book = filter::strings::convert_to_int (webserver_request.post_get("book"));
  const int chapter = filter::strings::convert_to_int (webserver_request.post_get("chapter"));
  std::string html = webserver_request.post_get("html");
  const std::string checksum = webserver_request.post_get("checksum");
  const std::string unique_id = webserver_request.post_get("id");

  if (checksum_logic::get (html) != checksum) {
    webserver_request.response_code = 409;
    return translate("Checksum error");
  }

  html = filter_url_tag_to_plus (std::move(html));
  html = filter::strings::trim (std::move(html));

  if (html.empty ()) {
    Database_Logs::log (translate ("There was no text.") + " " + translate ("Nothing was saved.") + " " + translate ("The original text of the chapter was reloaded."));
    return translate("Nothing to save");
  }

  if (!filter::strings::unicode_string_is_valid (html)) {
    Database_Logs::log ("The text was not valid Unicode UTF-8. The chapter could not saved and has been reverted to the last good version.");
    return translate("Save failure");
  }
  
  if (!access_bible::book_write (webserver_request, std::string(), bible, book)) {
    return translate("No write access");
  }

  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);

  Editor_Html2Usfm editor_export;
  editor_export.load (html);
  editor_export.stylesheet (stylesheet);
  editor_export.run ();
  std::string user_usfm = editor_export.get ();
  
  std::string ancestor_usfm = get_loaded_usfm (webserver_request, bible, book, chapter, unique_id);
  
  std::vector <filter::usfm::BookChapterData> book_chapter_text = filter::usfm::usfm_import (user_usfm, stylesheet);
  if (book_chapter_text.size () != 1) {
    Database_Logs::log (translate ("User tried to save something different from exactly one chapter."));
    return translate("Incorrect chapter");
  }
  
  int book_number = book_chapter_text[0].m_book;
  int chapter_number = book_chapter_text[0].m_chapter;
  user_usfm = book_chapter_text[0].m_data;
  bool chapter_ok = (((book_number == book) || (book_number == 0)) && (chapter_number == chapter));
  if (!chapter_ok) {
    return translate("Incorrect chapter") + " " + std::to_string (chapter_number);
  }
  
  // Collect some data about the changes for this user
  // and for a possible merge of the user's data with the server's data.
  const std::string& username = webserver_request.session_logic ()->get_username ();
  [[maybe_unused]] int oldID = database::bibles::get_chapter_id (bible, book, chapter);
  std::string server_usfm = database::bibles::get_chapter (bible, book, chapter);
  std::string newText = user_usfm;
  std::string oldText = ancestor_usfm;
  
  // Safekeep the USFM to save for later.
  std::string change = user_usfm;
  
  // Merge if the ancestor is there and differs from what's in the database.
  std::vector <Merge_Conflict> conflicts;
  if (!ancestor_usfm.empty ()) {
    if (server_usfm != ancestor_usfm) {
      // Prioritize the user's USFM.
      user_usfm = filter_merge_run (ancestor_usfm, server_usfm, user_usfm, true, conflicts);
      Database_Logs::log (translate ("Merging chapter."));
    }
  }
  
  // Check on the merge.
  filter_merge_add_book_chapter (conflicts, book, chapter);
  bible_logic::merge_irregularity_mail ({username}, conflicts);
  
  // Check whether the USFM on disk has changed compared to the USFM that was loaded in the editor.
  // If there's a difference, email the user.
  // Although a merge was done, still, it's good to alert the user on this.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  if (ancestor_usfm != server_usfm) {
    bible_logic::recent_save_email (bible, book, chapter, username, ancestor_usfm, server_usfm);
  }

  // Safely store the chapter.
  std::string explanation;
  std::string message = filter::usfm::safely_store_chapter (webserver_request, bible, book, chapter, user_usfm, explanation);
  bible_logic::unsafe_save_mail (message, explanation, username, user_usfm, book, chapter);

  // If an error message was given, then return that message to the browser.
  if (!message.empty ()) return message;

  // In server configuration, store details for the user's changes.
#ifdef HAVE_CLOUD
  int newID = database::bibles::get_chapter_id (bible, book, chapter);
  database::modifications::recordUserSave (username, bible, book, chapter, oldID, oldText, newID, newText);
  if (sendreceive_git_repository_linked (bible)) {
    database::git::store_chapter (username, bible, book, chapter, oldText, newText);
  }
  rss_logic_schedule_update (username, bible, book, chapter, oldText, newText);
#endif

  // Store a copy of the USFM loaded in the editor for later reference.
  store_loaded_usfm (webserver_request, bible, book, chapter, unique_id);

  // Convert the stored USFM to html.
  // This converted html should be the same as the saved html.
  // If it differs, signal the browser to reload the chapter.
  // This also cares for renumbering and cleaning up any added or removed footnotes.
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (user_usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  std::string converted_html = editor_usfm2html.get ();
  // Convert to XML for comparison.
  // Remove spaces before comparing.
  // Goal: Entering a space in the editor does not cause a reload.
  html = filter::strings::html2xml (html);
  html = filter::strings::replace (" ", "", html);
  html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), "", html);
  filter::strings::replace_between (html, "<", ">", "");
  converted_html = filter::strings::html2xml (converted_html);
  converted_html = filter::strings::replace (" ", "", converted_html);
  converted_html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), "", converted_html);
  filter::strings::replace_between (converted_html, "<", ">", "");
  // If round trip conversion differs, send a known string to the browser,
  // to signal the browser to reload the reformatted chapter.
  if (html != converted_html) return locale_logic_text_reformat ();

  return locale_logic_text_saved ();
}

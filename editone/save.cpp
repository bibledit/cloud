/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <filter/merge.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <database/modifications.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/git.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <editor/html2usfm.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <editone/logic.h>
#include <edit/logic.h>
#include <developer/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


std::string editone_save_url ()
{
  return "editone/save";
}


bool editone_save_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editone_save (Webserver_Request& webserver_request)
{
  // Check on information about where to save the verse.
  const bool save = (webserver_request.post_count("bible") && webserver_request.post_count("book") && webserver_request.post_count("chapter") && webserver_request.post_count("verse") && webserver_request.post_count("html"));
  if (!save) {
    return translate("Don't know where to save");
  }

  
  const std::string bible = webserver_request.post_get("bible");
  const int book = filter::string::convert_to_int (webserver_request.post_get("book"));
  const int chapter = filter::string::convert_to_int (webserver_request.post_get("chapter"));
  const int verse = filter::string::convert_to_int (webserver_request.post_get("verse"));
  std::string html = webserver_request.post_get("html");
  const std::string checksum = webserver_request.post_get("checksum");
  const std::string unique_id = webserver_request.post_get("id");

  
  // Checksum.
  if (checksum_logic::get (html) != checksum) {
    webserver_request.response_code = 409;
    return translate ("Checksum error");
  }

  
  // Decode html encoded in javascript.
  html = filter_url_tag_to_plus (std::move(html));

  
  // Check there's anything to save at all.
  html = filter::string::trim (std::move(html));
  if (html.empty ()) {
    return translate ("Nothing to save");
  }
  
  
  // Check on valid UTF-8.
  if (!filter::string::unicode_string_is_valid (html)) {
    return translate ("Cannot save: Needs Unicode");
  }
  
  
  if (!access_bible::book_write (webserver_request, std::string(), bible, book)) {
    return translate ("No write access");
  }

  
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  std::string verse_usfm = editone_logic_html_to_usfm (stylesheet, html);

  
  // Collect some data about the changes for this user.
  const std::string& username = webserver_request.session_logic ()->get_username ();
#ifdef HAVE_CLOUD
  const int old_id = database::bibles::get_chapter_id (bible, book, chapter);
#endif
  std::string old_chapter_usfm = database::bibles::get_chapter (bible, book, chapter);

  
  // If the most recent save operation on this chapter
  // caused the chapter to be different, email the user,
  // suggesting to check whether the user's edit came through.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  const std::string loaded_usfm = get_loaded_usfm (webserver_request, bible, book, chapter, unique_id);
  if (loaded_usfm != old_chapter_usfm) {
    bible_logic::recent_save_email (bible, book, chapter, username, loaded_usfm, old_chapter_usfm);
  }

  
  // Safely store the verse.
  std::string explanation;
  std::string message = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse, verse_usfm, explanation, true);
  bible_logic::unsafe_save_mail (message, explanation, username, verse_usfm, book, chapter);
  // If storing the verse worked out well, there's no message to display.
  if (message.empty ()) {
    // Get the chapter text now, that is, after the save operation completed.
    std::string new_chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
    // Check whether the text on disk was changed while the user worked with the older copy.
    if (!loaded_usfm.empty () && (loaded_usfm != old_chapter_usfm)) {
      // Do a merge for better editing reliability.
      std::vector <Merge_Conflict> conflicts;
      // Prioritize the USFM already in the chapter.
      new_chapter_usfm = filter_merge_run (loaded_usfm, new_chapter_usfm, old_chapter_usfm, true, conflicts);
      database::bibles::store_chapter (bible, book, chapter, new_chapter_usfm);
      Database_Logs::log (translate ("Merging chapter."));
    }
#ifdef HAVE_CLOUD
    // The Cloud stores details of the user's changes.
    const int new_id = database::bibles::get_chapter_id (bible, book, chapter);
    database::modifications::recordUserSave (username, bible, book, chapter, old_id, old_chapter_usfm, new_id, new_chapter_usfm);
    if (sendreceive_git_repository_linked (bible)) {
      database::git::store_chapter (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
    }
    rss_logic_schedule_update (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
#endif
    
    
    // Store a copy of the USFM now saved as identical to what's loaded in the editor for later reference.
    store_loaded_usfm (webserver_request, bible, book, chapter, unique_id);

    return locale_logic_text_saved ();
  }


  // The message contains information about save failure.
  // Send it to the browser for display to the user.
  return message;
}

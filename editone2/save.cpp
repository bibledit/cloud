/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <editone2/save.h>
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
#include <editone2/logic.h>
#include <edit/logic.h>
#include <developer/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>
#include <config/logic.h>


string editone2_save_url ()
{
  return "editone2/save";
}


bool editone2_save_acl (void * webserver_request)
{
  if (config_logic_indonesian_cloud_free ()) {
    return true;
  }
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = AccessBible::Any (webserver_request);
  return read;
}


string editone2_save (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
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
  string unique_id = request->post ["id"];

  
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
  
  
  if (!AccessBible::BookWrite (request, string(), bible, book)) {
    return translate ("No write access");
  }

  
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
 
  
  string verse_usfm = editone_logic_html_to_usfm (stylesheet, html);

  
  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
#ifdef HAVE_CLOUD
  int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
#endif
  string old_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);

  
  // If the most recent save operation on this chapter
  // caused the chapter to be different, email the user,
  // suggesting to check if the user's edit came through.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  string loaded_usfm = getLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);
  if (loaded_usfm != old_chapter_usfm) {
    bible_logic_recent_save_email (bible, book, chapter, verse, username, loaded_usfm, old_chapter_usfm);
  }

  
  // Safely store the verse.
  string explanation;
  string message = usfm_safely_store_verse (request, bible, book, chapter, verse, verse_usfm, explanation, true);
  bible_logic_unsafe_save_mail (message, explanation, username, verse_usfm, book, chapter);
  // If storing the verse worked out well, there's no message to display.
  if (message.empty ()) {
    // Get the chapter text now, that is, after the save operation completed.
    string new_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
    // Check whether the text on disk was changed while the user worked with the older copy.
    if (!loaded_usfm.empty () && (loaded_usfm != old_chapter_usfm)) {
      // Do a merge for better editing reliability.
      vector <Merge_Conflict> conflicts;
      // Prioritize the USFM already in the chapter.
      new_chapter_usfm = filter_merge_run (loaded_usfm, new_chapter_usfm, old_chapter_usfm, true, conflicts);
      request->database_bibles()->storeChapter (bible, book, chapter, new_chapter_usfm);
      Database_Logs::log (translate ("Merging chapter."));
    }
#ifdef HAVE_CLOUD
    // The Cloud stores details of the user's changes.
    int newID = request->database_bibles()->getChapterId (bible, book, chapter);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave (username, bible, book, chapter, oldID, old_chapter_usfm, newID, new_chapter_usfm);
    if (sendreceive_git_repository_linked (bible)) {
      Database_Git::store_chapter (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
    }
    rss_logic_schedule_update (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
#endif
    
    
    // Store a copy of the USFM now saved as identical to what's loaded in the editor for later reference.
    storeLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);

    return locale_logic_text_saved ();
  }


  // The message contains information about save failure.
  // Send it to the browser for display to the user.
  return message;
}

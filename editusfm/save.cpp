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


#include <editusfm/save.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/merge.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <database/modifications.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/git.h>
#include <checksum/logic.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <edit/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


std::string editusfm_save_url ()
{
  return "editusfm/save";
}


bool editusfm_save_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editusfm_save (Webserver_Request& webserver_request)
{
  std::string bible = webserver_request.post["bible"];
  int book = filter::strings::convert_to_int (webserver_request.post["book"]);
  int chapter = filter::strings::convert_to_int (webserver_request.post["chapter"]);
  std::string usfm = webserver_request.post["usfm"];
  std::string checksum = webserver_request.post["checksum"];
  std::string unique_id = webserver_request.post ["id"];

  
  if (webserver_request.post.count ("bible") && webserver_request.post.count ("book") && webserver_request.post.count ("chapter") && webserver_request.post.count ("usfm")) {
    if (checksum_logic::get (usfm) == checksum) {
      usfm = filter_url_tag_to_plus (usfm);
      usfm = filter::strings::trim (usfm);
      // Collapse multiple spaces in the USFM into one space.
      // https://github.com/bibledit/cloud/issues/711
      usfm = filter::strings::collapse_whitespace(usfm);
      if (!usfm.empty ()) {
        if (filter::strings::unicode_string_is_valid (usfm)) {
          const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
          std::vector <filter::usfm::BookChapterData> book_chapter_text = filter::usfm::usfm_import (usfm, stylesheet);
          if (!book_chapter_text.empty()) {
            filter::usfm::BookChapterData data = book_chapter_text[0];
            int book_number = data.m_book;
            int chapter_number = data.m_chapter;
            std::string chapter_data_to_save = data.m_data;
            if (((book_number == book) || (book_number == 0)) && (chapter_number == chapter)) {
              // The USFM loaded into the editor.
              std::string ancestor_usfm = get_loaded_usfm (webserver_request, bible, book, chapter, unique_id);
              // Collect some data about the changes for this user.
              const std::string& username = webserver_request.session_logic ()->get_username ();
              [[maybe_unused]] int oldID = database::bibles::get_chapter_id (bible, book, chapter);
              std::string oldText = ancestor_usfm;
              std::string newText = chapter_data_to_save;
              // Merge if the ancestor is there and differs from what's in the database.
              std::vector <Merge_Conflict> conflicts;
              // The USFM now on disk.
              std::string server_usfm = database::bibles::get_chapter (bible, book, chapter);
              if (!ancestor_usfm.empty ()) {
                if (server_usfm != ancestor_usfm) {
                  // Prioritize the USFM to save.
                  chapter_data_to_save = filter_merge_run (ancestor_usfm, server_usfm, chapter_data_to_save, true, conflicts);
                  Database_Logs::log (translate ("Merging and saving chapter."));
                }
              }
              
              // Check on the merge.
              filter_merge_add_book_chapter (conflicts, book, chapter);
              bible_logic::merge_irregularity_mail ({username}, conflicts);
              
              // If the USFM on disk is different from the USFM that was sent to the editor,
              // email the user,
              // suggesting to check if the user's edit came through.
              // The rationale is that if Bible text was saved through Send/receive,
              // or if another user saved Bible text,
              // it's worth to check on this.
              // Because the user's editor may not yet have loaded this updated Bible text.
              // https://github.com/bibledit/cloud/issues/340
              if (ancestor_usfm != server_usfm) {
                bible_logic::recent_save_email (bible, book, chapter, username, ancestor_usfm, server_usfm);
              }
              
             
              // Check on write access.
              if (access_bible::book_write (webserver_request, std::string(), bible, book)) {
                // Safely store the chapter.
                std::string explanation;
                std::string message = filter::usfm::safely_store_chapter (webserver_request, bible, book, chapter, chapter_data_to_save, explanation);
                bible_logic::unsafe_save_mail (message, explanation, username, chapter_data_to_save, book, chapter);
                if (message.empty()) {
#ifndef HAVE_CLIENT
                  // Server configuration: Store details for the user's changes.
                  int newID = database::bibles::get_chapter_id (bible, book, chapter);
                  database::modifications::recordUserSave (username, bible, book, chapter, oldID, oldText, newID, newText);
                  if (sendreceive_git_repository_linked (bible)) {
                    database::git::store_chapter (username, bible, book, chapter, oldText, newText);
                  }
                  rss_logic_schedule_update (username, bible, book, chapter, oldText, newText);
#endif
                  // Store a copy of the USFM loaded in the editor for later reference.
                  store_loaded_usfm (webserver_request, bible, book, chapter, unique_id);
                  return locale_logic_text_saved ();
                }
                return message;
              } else {
                return translate("No write access");
              }
            } else {
              Database_Logs::log ("The following data could not be saved and was discarded: " + chapter_data_to_save);
              return translate("Passage mismatch");
            }
          }
        } else {
          Database_Logs::log ("The text was not valid Unicode UTF-8. The chapter could not saved and has been reverted to the last good version.");
          return translate("Needs Unicode");
        }
      } else {
        Database_Logs::log ("There was no text. Nothing was saved. The original text of the chapter was reloaded.");
        return translate("Nothing to save");
      }
    } else {
      webserver_request.response_code = 409;
      return translate("Checksum error");
    }
  } else {
    return translate("Nothing to save");
  }
  return translate ("Confusing data");
}

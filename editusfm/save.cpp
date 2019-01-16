/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


string editusfm_save_url ()
{
  return "editusfm/save";
}


bool editusfm_save_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editusfm_save (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string bible = request->post["bible"];
  int book = convert_to_int (request->post["book"]);
  int chapter = convert_to_int (request->post["chapter"]);
  string usfm = request->post["usfm"];
  string checksum = request->post["checksum"];
  
  
  if (request->post.count ("bible") && request->post.count ("book") && request->post.count ("chapter") && request->post.count ("usfm")) {
    if (Checksum_Logic::get (usfm) == checksum) {
      usfm = filter_url_tag_to_plus (usfm);
      usfm = filter_string_trim (usfm);
      if (!usfm.empty ()) {
        if (unicode_string_is_valid (usfm)) {
          string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
          vector <BookChapterData> book_chapter_text = usfm_import (usfm, stylesheet);
          for (BookChapterData & data : book_chapter_text) {
            int book_number = data.book;
            int chapter_number = data.chapter;
            string chapter_data_to_save = data.data;
            if (((book_number == book) || (book_number == 0)) && (chapter_number == chapter)) {
              string ancestor_usfm = getLoadedUsfm (webserver_request, bible, book, chapter, "editusfm");
              // Collect some data about the changes for this user.
              string username = request->session_logic()->currentUser ();
              int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
              string oldText = ancestor_usfm;
              string newText = chapter_data_to_save;
              // Merge if the ancestor is there and differs from what's in the database.
              vector <tuple <string, string, string, string, string>> conflicts;
              string server_usfm = request->database_bibles ()->getChapter (bible, book, chapter);
              if (!ancestor_usfm.empty ()) {
                if (server_usfm != ancestor_usfm) {
                  // Prioritize the USFM to save.
                  chapter_data_to_save = filter_merge_run (ancestor_usfm, server_usfm, chapter_data_to_save, true, conflicts);
                  Database_Logs::log (translate ("Merging and saving chapter."));
                }
              }
              // Check on the merge.
              bible_logic_merge_irregularity_mail ({username}, conflicts);
              // Check on write access.
              if (access_bible_book_write (request, "", bible, book)) {
                // Safely store the chapter.
                string explanation;
                string message = usfm_safely_store_chapter (request, bible, book, chapter, chapter_data_to_save, explanation);
                bible_logic_unsafe_save_mail (message, explanation, username, chapter_data_to_save);
                if (message.empty()) {
#ifndef HAVE_CLIENT
                  // Server configuration: Store details for the user's changes.
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
                  storeLoadedUsfm (webserver_request, bible, book, chapter, "editusfm");
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
      request->response_code = 409;
      return translate("Checksum error");
    }
  } else {
    return translate("Nothing to save");
  }
  return translate ("Confusing data");
}

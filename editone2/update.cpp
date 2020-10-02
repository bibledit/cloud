/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#include <editone2/update.h>
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
#include <edit2/logic.h>
#include <developer/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


string editone2_update_url ()
{
  return "editone2/update";
}


bool editone2_update_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editone2_update (void * webserver_request) // Todo
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  // Check on information about where to update the verse text.
  bool parameters_ok = (request->post.count ("bible") && request->post.count ("book") && request->post.count ("chapter") && request->post.count ("verse") && request->post.count ("edited"));
  if (!parameters_ok) {
    return translate("Don't know what to update");
  }
  
  
  string bible = request->post["bible"];
  int book = convert_to_int (request->post["book"]);
  int chapter = convert_to_int (request->post["chapter"]);
  int verse = convert_to_int (request->post["verse"]);
  string loaded = request->post["loaded"];
  string edited = request->post["edited"];
  string checksum = request->post["checksum"];
  string unique_id = request->post ["id"];

  
  // Checksum of the edited html.
  if (Checksum_Logic::get (edited) != checksum) {
    request->response_code = 409;
    return translate ("Checksum error");
  }

  
  // Decode html encoded in javascript.
  loaded = filter_url_tag_to_plus (loaded);
  edited = filter_url_tag_to_plus (edited);

  
  // Check there's anything to update at all.
  loaded = filter_string_trim (loaded);
  edited = filter_string_trim (edited);
  if (loaded.empty () || edited.empty ()) {
    return translate ("Nothing to update");
  }

  
  // Check on valid UTF-8.
  if (!unicode_string_is_valid (loaded) || !unicode_string_is_valid (edited)) {
    return translate ("Cannot update: Needs Unicode");
  }

  
  if (!access_bible_book_write (request, "", bible, book)) {
    return translate ("No write access");
  }


  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);

  
  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
#ifdef HAVE_CLOUD
  int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
#endif
  string old_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);

  
  // Determine what (composed) version of USFM to save to the chapter.
  // Do a three-way merge to obtain that USFM.
  // This needs the loaded USFM as the ancestor,
  // the edited USFM as a change-set,
  // and the existing USFM as a prioritized change-set.
  string loaded_verse_usfm = editone2_logic_html_to_usfm (stylesheet, loaded);
  string edited_verse_usfm = editone2_logic_html_to_usfm (stylesheet, edited);
  string existing_verse_usfm = usfm_get_verse_text_quill (old_chapter_usfm, verse);
  existing_verse_usfm = filter_string_trim (existing_verse_usfm);


  // Handle the following situation:
  // 1. The text in the editor was not changed.
  // 2. The text on the server was changed.
  // The action to take is this:
  // To do a smart update of the text in the editor.
  // The result should be: The text in the editor matches the text on the server.
  if (loaded_verse_usfm == edited_verse_usfm) {
    
    
    // There are two relevant html text fragments:
    // 1. What the server has now.
    // 2. What the editor has now.
   
    // Server converts both the html slices to character/format pairs.
    
    // The server calculates the differences.
    
    // The AJAX call returns the differences.
    // See the text load for how to send multiple fragments to the editor.


    
    
  }
  
  
  /*
  Todo


  
  // If the most recent save operation on this chapter
  // caused the chapter to be different, email the user,
  // suggesting to check if the user's edit came through.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  // Todo fix changed behaviour: No loading at all, but updating existing text.
  string loaded_usfm = getLoadedUsfm2 (webserver_request, bible, book, chapter, unique_id);
  if (loaded_usfm != old_chapter_usfm) {
    bible_logic_recent_save_email (bible, book, chapter, verse, username, loaded_usfm, old_chapter_usfm);
  }

  // Safely store the verse.
  string explanation;
  string message = usfm_safely_store_verse (request, bible, book, chapter, verse, edited_verse_usfm, explanation, true);
  bible_logic_unsafe_save_mail (message, explanation, username, edited_verse_usfm);
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
  */

  
  // This is the format to send the changes in:
  // insert - position - text - format
  // delete - position - length
  
  string response;
  response.append ("insert");
  response.append ("#_be_#");
  response.append ("9");
  response.append ("#_be_#");
  response.append ("Lord");
  response.append ("#_be_#");
  response.append ("add");

  response.append ("#_be_#");
  response.append ("delete");
  response.append ("#_be_#");
  response.append ("13");
  response.append ("#_be_#");
  response.append ("6");
  
  string user = request->session_logic ()->currentUser ();
  bool write = access_bible_book_write (webserver_request, user, bible, book);
  response = Checksum_Logic::send (response, write);
  
  return response;
}

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


#include <edit2/update.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <filter/merge.h>
#include <filter/diff.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <database/modifications.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/git.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <editor/html2usfm.h>
#include <editor/usfm2html.h>
#include <editor/html2format.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <editone2/logic.h>
#include <edit2/logic.h>
#include <developer/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>


string edit2_update_url ()
{
  return "edit2/update";
}


bool edit2_update_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string edit2_update (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;


  // Whether the update is good to go.
  bool good2go = true;
  
  
  // The messages to return.
  vector <string> messages;

  
  // Check the relevant bits of information.
  if (good2go) {
    bool parameters_ok = true;
    if (!request->post.count ("bible")) parameters_ok = false;
    if (!request->post.count ("book")) parameters_ok = false;
    if (!request->post.count ("chapter")) parameters_ok = false;
    if (!request->post.count ("loaded")) parameters_ok = false;
    if (!request->post.count ("edited")) parameters_ok = false;
    if (!parameters_ok) {
      messages.push_back (translate("Don't know what to update"));
      good2go = false;
    }
  }

  
  // Get the relevant bits of information.
  string bible;
  int book = 0;
  int chapter = 0;
  string loaded_html;
  string edited_html;
  string checksum1;
  string checksum2;
  string unique_id;
  if (good2go) {
    bible = request->post["bible"];
    book = convert_to_int (request->post["book"]);
    chapter = convert_to_int (request->post["chapter"]);
    loaded_html = request->post["loaded"];
    edited_html = request->post["edited"];
    checksum1 = request->post["checksum1"];
    checksum2 = request->post["checksum2"];
    unique_id = request->post ["id"];
  }


  // Checksums of the loaded and edited html.
  if (good2go) {
    if (Checksum_Logic::get (loaded_html) != checksum1) {
      request->response_code = 409;
      messages.push_back (translate ("Checksum error"));
      good2go = false;
    }
  }
  if (good2go) {
    if (Checksum_Logic::get (edited_html) != checksum2) {
      request->response_code = 409;
      messages.push_back (translate ("Checksum error"));
      good2go = false;
    }
  }

  
  // Decode html encoded in javascript, and clean it.
  loaded_html = filter_url_tag_to_plus (loaded_html);
  edited_html = filter_url_tag_to_plus (edited_html);
  loaded_html = filter_string_trim (loaded_html);
  edited_html = filter_string_trim (edited_html);

  
  // Check on valid UTF-8.
  if (good2go) {
    if (!unicode_string_is_valid (loaded_html) || !unicode_string_is_valid (edited_html)) {
      messages.push_back (translate ("Cannot update: Needs Unicode"));
      good2go = false;
    }
  }


  bool bible_write_access = false;
  if (good2go) {
    bible_write_access = access_bible_book_write (request, "", bible, book);
  }


  string stylesheet;
  if (good2go) stylesheet = Database_Config_Bible::getEditorStylesheet (bible);

  
  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
#ifdef HAVE_CLOUD
  int oldID = 0;
  if (good2go) oldID = request->database_bibles()->getChapterId (bible, book, chapter);
#endif
  string old_chapter_usfm;
  if (good2go) old_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);

  
  // Determine what version of USFM to save to the chapter.
  // Later in the code, it will do a three-way merge, to obtain that USFM.
  // This needs the loaded USFM as the ancestor,
  // the edited USFM as a change-set,
  // and the existing USFM as a prioritized change-set.
  string loaded_chapter_usfm;
  if (good2go) {
    Editor_Html2Usfm editor_export;
    editor_export.load (loaded_html);
    editor_export.stylesheet (stylesheet);
    editor_export.run ();
    loaded_chapter_usfm = editor_export.get ();
  }
  string edited_chapter_usfm;
  if (good2go) {
    Editor_Html2Usfm editor_export;
    editor_export.load (edited_html);
    editor_export.stylesheet (stylesheet);
    editor_export.run ();
    edited_chapter_usfm = editor_export.get ();
  }
  string existing_chapter_usfm = filter_string_trim (old_chapter_usfm);


  // Check that the edited USFM contains no more than, and exactly like,
  // the book and chapter that was loaded in the editor.
  if (good2go && bible_write_access) {
    vector <BookChapterData> book_chapter_text = usfm_import (edited_chapter_usfm, stylesheet);
    if (book_chapter_text.size () != 1) {
      Database_Logs::log (translate ("A user tried to save something different from exactly one chapter"));
      messages.push_back (translate("Incorrect chapter"));
    }
    int book_number = book_chapter_text[0].book;
    int chapter_number = book_chapter_text[0].chapter;
    edited_chapter_usfm = book_chapter_text[0].data;
    bool chapter_ok = (((book_number == book) || (book_number == 0)) && (chapter_number == chapter));
    if (!chapter_ok) {
      messages.push_back (translate("Incorrect chapter") + " " + convert_to_string (chapter_number));
    }
  }


  // Set a flag if there is a reason to save the editor text, since it was edited.
  // This is important because the same routine is used for saving the editor text
  // as well as updating the editor text.
  // So if the text in the editor was not changed, it should not save it,
  // as saving the editor text would overwrite saves made by other(s).
  bool text_was_edited = (loaded_chapter_usfm != edited_chapter_usfm);

  
  // Do a three-way merge if needed.
  // There's a need for this if there were user-edits,
  // and if the USFM on the server differs from the USFM loaded in the editor.
  // The three-way merge reconciles those differences.
  if (good2go && bible_write_access && text_was_edited) {
    if (loaded_chapter_usfm != existing_chapter_usfm) {
      vector <Merge_Conflict> conflicts;
      // Do a merge while giving priority to the USFM already in the chapter.
      string merged_chapter_usfm = filter_merge_run (loaded_chapter_usfm, edited_chapter_usfm, existing_chapter_usfm, true, conflicts);
      // Mail the user if there is a merge anomaly.
      bible_logic_optional_merge_irregularity_email (bible, book, chapter, username, loaded_chapter_usfm, edited_chapter_usfm, merged_chapter_usfm);
      bible_logic_merge_irregularity_mail ({username}, conflicts);
      // Let the merged data now become the edited data (so it gets saved properly).
      edited_chapter_usfm = merged_chapter_usfm;
    }
  }
  

  // Check whether the USFM on disk has changed compared to the USFM that was loaded in the editor.
  // If there's a difference, email the user.
  // Although a merge was done, still, it's good to alert the user on this.
  // The rationale is that if Bible text was saved through Send/receive,
  // or if another user saved Bible text,
  // it's worth to check on this.
  // Because the user's editor may not yet have loaded this updated Bible text.
  // https://github.com/bibledit/cloud/issues/340
  // The above is no longer needed since the chapter editor does no longer "load" text.
  // Instead of loading text, the existing text gets updated.
  // The message below will no longer be given.
  // It might cause confusion more than it clarifies.
  //if (good2go && bible_write_access && text_was_edited) {
    //if (loaded_chapter_usfm != existing_chapter_usfm) {
      //bible_logic_recent_save_email (bible, book, chapter, 0, username, loaded_chapter_usfm, existing_chapter_usfm);
    //}
  //}

  
  // Safely store the chapter.
  string explanation;
  string message;
  if (good2go && bible_write_access && text_was_edited) {
    message = usfm_safely_store_chapter (request, bible, book, chapter, edited_chapter_usfm, explanation);
    bible_logic_unsafe_save_mail (message, explanation, username, edited_chapter_usfm);
    if (!message.empty ()) messages.push_back (message);
  }

  
  // The new chapter identifier and new chapter USFM.
  int newID = request->database_bibles()->getChapterId (bible, book, chapter);
  string new_chapter_usfm;
  if (good2go) {
    new_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  }

  
  if (good2go && bible_write_access && text_was_edited) {
    // If storing the chapter worked out well, there's no message to display.
    if (message.empty ()) {
#ifdef HAVE_CLOUD
      // The Cloud stores details of the user's changes.
      Database_Modifications database_modifications;
      database_modifications.recordUserSave (username, bible, book, chapter, oldID, old_chapter_usfm, newID, new_chapter_usfm);
      if (sendreceive_git_repository_linked (bible)) {
        Database_Git::store_chapter (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
      }
      rss_logic_schedule_update (username, bible, book, chapter, old_chapter_usfm, new_chapter_usfm);
#endif
      // Feedback to user.
      messages.push_back (locale_logic_text_saved ());
    } else {
      // Feedback about anomaly to user.
      messages.push_back (message);
    }
  }

  
  // If there's no message at all, return at least something to the editor.
  if (messages.empty ()) messages.push_back (locale_logic_text_saved ());
  
  
  // The response to send to back to the editor.
  string response;
  string separator = "#_be_#";
  // The response starts with the save message(s) if any.
  // The message(s) contain information about save success or failure.
  // Send it to the browser for display to the user.
  response.append (filter_string_implode (messages, " | "));

  
  // Add separator and the new chapter identifier to the response.
  response.append (separator);
  response.append (convert_to_string (newID));

  
  // The main purpose of the following block of code is this:
  // To send the differences between what the editor has now and what the server has now.
  // The purpose of sending the differences to the editor is this:
  // The editor can update its contents, so the editor will have what the server has.
  // This is the format to send the changes in:
  // insert - position - text - format
  // delete - position
  if (good2go) {
    // Determine the server's current chapter content, and the editor's current chapter content.
    string editor_html (edited_html);
    string server_html;
    {
      Editor_Usfm2Html editor_usfm2html;
      editor_usfm2html.load (new_chapter_usfm);
      editor_usfm2html.stylesheet (stylesheet);
      editor_usfm2html.run ();
      server_html = editor_usfm2html.get ();
    }
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    // Encode the condensed differences for the response to the Javascript editor.
    for (size_t i = 0; i < positions.size(); i++) {
      response.append ("#_be_#");
      response.append (convert_to_string (positions[i]));
      response.append ("#_be_#");
      string operation = operators[i];
      response.append (operation);
      if (operation == bible_logic_insert_operator ()) {
        string text = content[i];
        string character = unicode_string_substr (text, 0, 1);
        response.append ("#_be_#");
        response.append (character);
        size_t length = unicode_string_length (text);
        string format = unicode_string_substr (text, 1, length - 1);
        response.append ("#_be_#");
        response.append (format);
        // Also add the size of the character in UTF-16 format, 2-bytes or 4 bytes, as size 1 or 2.
        response.append ("#_be_#");
        response.append (convert_to_string (sizes[i]));
      }
      else if (operation == bible_logic_delete_operator ()) {
        // When deleting a UTF-16 character encoded in 4 bytes,
        // then the size in Quilljs is 2 instead of 1.
        // So always give the size when deleting a character.
        response.append ("#_be_#");
        response.append (convert_to_string (sizes[i]));
      }
      else if (operation == bible_logic_format_paragraph_operator ()) {
        response.append ("#_be_#");
        response.append (content[i]);
      }
      else if (operation == bible_logic_format_character_operator ()) {
        response.append ("#_be_#");
        response.append (content[i]);
      }
    }
  }
  
  // Things to try out in the C++ and Javacript update routines.

  // Test changing the format of the first paragraph.

  // Test changing the format of the second paragraph.
  
  // Test changing the format of the last paragraph.
  
  // Test deleting an entire paragraph.
  
  // Test adding a character to a formatted word, to see if the character format gets transferred properly.

  // Test that if this editor is the second or higher one in a workspace,
  // it is read-only. And updating the editor's contents does not grab focus.
  
  // Test inserting and updating 4-byte UTF-16 characters like the 😀.
  
  // Test continued typing during serious network latency.

  // Test using the Cloud together with client devices with send and receive.
  
  bool write = access_bible_book_write (webserver_request, username, bible, book);
  response = Checksum_Logic::send (response, write);

  // Ready.
  //this_thread::sleep_for(chrono::seconds(5));
  return response;
}

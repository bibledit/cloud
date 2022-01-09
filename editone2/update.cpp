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
#include <editone2/update.h>
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
#include <editor/html2format.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <editone2/logic.h>
#include <edit/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>
#include <config/logic.h>


string editone2_update_url ()
{
  return "editone2/update";
}


bool editone2_update_acl (void * webserver_request)
{
  if (config_logic_indonesian_cloud_free ()) {
    return true;
  }
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = AccessBible::Any (webserver_request);
  return read;
}


string editone2_update (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);


  // Whether the update is good to go.
  bool good2go = true;
  
  
  // The message(s) to return.
  vector <string> messages;

  
  // Check the relevant bits of information.
  if (good2go) {
    bool parameters_ok = true;
    if (!request->post.count ("bible")) parameters_ok = false;
    if (!request->post.count ("book")) parameters_ok = false;
    if (!request->post.count ("chapter")) parameters_ok = false;
    if (!request->post.count ("verse")) parameters_ok = false;
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
  int verse = 0;
  string loaded_html;
  string edited_html;
  string checksum1;
  string checksum2;
  string unique_id;
  if (good2go) {
    bible = request->post["bible"];
    book = convert_to_int (request->post["book"]);
    chapter = convert_to_int (request->post["chapter"]);
    verse = convert_to_int (request->post["verse"]);
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
    bible_write_access = AccessBible::BookWrite (request, string(), bible, book);
  }


  string stylesheet;
  if (good2go) {
    stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  }

  
  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
#ifdef HAVE_CLOUD
  int oldID = 0;
  if (good2go) oldID = request->database_bibles()->getChapterId (bible, book, chapter);
#endif
  string old_chapter_usfm;
  if (good2go) old_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);

  
  // Determine what (composed) version of USFM to save to the chapter.
  // Do a three-way merge to obtain that USFM.
  // This needs the loaded USFM as the ancestor,
  // the edited USFM as a change-set,
  // and the existing USFM as a prioritized change-set.
  string loaded_verse_usfm = editone_logic_html_to_usfm (stylesheet, loaded_html);
  string edited_verse_usfm = editone_logic_html_to_usfm (stylesheet, edited_html);
  string existing_verse_usfm = usfm_get_verse_text_quill (old_chapter_usfm, verse);
  existing_verse_usfm = filter_string_trim (existing_verse_usfm);

  
  // Set a flag if there is a reason to save the editor text, since it was edited.
  // This is important because the same routine is used for saving editor text
  // as well as updating the editor text.
  // So if the text in the editor was not changed, it should not save it,
  // as saving the editor text would overwrite saves made by other(s).
  bool text_was_edited = (loaded_verse_usfm != edited_verse_usfm);

  
  // Do a three-way merge if needed.
  // There's a need for this if there were user-edits,
  // and if the USFM on the server differs from the USFM loaded in the editor.
  // The three-way merge reconciles those differences.
  if (good2go && bible_write_access && text_was_edited) {
    if (loaded_verse_usfm != existing_verse_usfm) {
      vector <Merge_Conflict> conflicts;
      // Do a merge while giving priority to the USFM already in the chapter.
      string merged_verse_usfm = filter_merge_run (loaded_verse_usfm, edited_verse_usfm, existing_verse_usfm, true, conflicts);
      // Mail the user if there is a merge anomaly.
      filter_merge_add_book_chapter (conflicts, book, chapter);
      bible_logic_optional_merge_irregularity_email (bible, book, chapter, username, loaded_verse_usfm, edited_verse_usfm, merged_verse_usfm);
      // Let the merged data now become the edited data (so it gets saved properly).
      edited_verse_usfm = merged_verse_usfm;
    }
  }
  
  
  // Collapse any double spaces in the USFM to save.
  // https://github.com/bibledit/cloud/issues/711
  // If there's double spaces removed here,
  // then later in this code, the editor will load that text.
  if (good2go && bible_write_access && text_was_edited) {
    edited_verse_usfm = filter_string_collapse_whitespace(edited_verse_usfm);
  }

  
  // Safely store the verse.
  string explanation;
  string message;
  if (good2go && bible_write_access && text_was_edited) {
    message = usfm_safely_store_verse (request, bible, book, chapter, verse, edited_verse_usfm, explanation, true);
    bible_logic_unsafe_save_mail (message, explanation, username, edited_verse_usfm, book, chapter);
  }

  
  // The new chapter identifier and new chapter USFM.
  int newID = request->database_bibles()->getChapterId (bible, book, chapter);
  string new_chapter_usfm;
  if (good2go) {
    new_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  }

  
  if (good2go && bible_write_access && text_was_edited) {
    // If storing the verse worked out well, there's no message to display.
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
  if (messages.empty ()) messages.push_back (locale_logic_text_updated());


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
    // Determine the server's current verse content, and the editor's current verse content.
    string editor_html (edited_html);
    string server_html;
    {
      string verse_usfm = usfm_get_verse_text_quill (new_chapter_usfm, verse);
      editone_logic_editable_html (verse_usfm, stylesheet, server_html);
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
  
  bool write = AccessBible::BookWrite (webserver_request, username, bible, book);
  response = Checksum_Logic::send (response, write);

  // Ready.
  //this_thread::sleep_for(chrono::seconds(60));
  return response;
}

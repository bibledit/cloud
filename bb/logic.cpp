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


#include <bb/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/git.h>
#include <filter/diff.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/date.h>
#include <database/bibles.h>
#include <database/modifications.h>
#include <database/bibleactions.h>
#include <database/logs.h>
#include <database/versifications.h>
#include <database/books.h>
#include <database/config/general.h>
#include <database/cache.h>
#include <database/privileges.h>
#include <database/config/bible.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sync/resources.h>
#include <sword/logic.h>
#include <resource/logic.h>
#include <search/logic.h>
#include <locale/translate.h>
#include <email/send.h>
#include <developer/logic.h>
#include <locale/translate.h>
#include <editor/html2format.h>


void bible_logic_store_chapter (const string& bible, int book, int chapter, const string& usfm)
{
  Database_Bibles database_bibles;

  // Record data of the chapter to be stored prior to storing the new version.
  // Both client and cloud follow this order.

#ifdef HAVE_CLIENT

  // Client stores Bible action.
  string oldusfm = database_bibles.getChapter (bible, book, chapter);
  Database_BibleActions database_bibleactions;
  database_bibleactions.record (bible, book, chapter, oldusfm);
  
  // Kick the unsent-data timeout mechanism.
  bible_logic_kick_unsent_data_timer ();

#endif

#ifdef HAVE_CLOUD

  // Server stores diff data.
    Database_Modifications database_modifications;
    database_modifications.storeTeamDiff (bible, book, chapter);
  
#endif

  // Store the chapter in the database.
  database_bibles.storeChapter (bible, book, chapter, usfm);
}


void bible_logic_delete_chapter (const string& bible, int book, int chapter)
{
  Database_Bibles database_bibles;

  // Cloud and client record data of the chapter to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible action.
  string usfm = database_bibles.getChapter (bible, book, chapter);
  Database_BibleActions database_bibleactions;
  database_bibleactions.record (bible, book, chapter, usfm);
  
  // Kick the unsent-data timeout mechanism.
  bible_logic_kick_unsent_data_timer ();

#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiff (bible, book, chapter);
  
#endif

  // Delete the chapter from the database.
  database_bibles.deleteChapter (bible, book, chapter);
}


void bible_logic_delete_book (const string& bible, int book)
{
  Database_Bibles database_bibles;

  // Both client and cloud record data of the book to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible actions.
  Database_BibleActions database_bibleactions;
  vector <int> chapters = database_bibles.getChapters (bible, book);
  for (auto & chapter : chapters) {
    string usfm = database_bibles.getChapter (bible, book, chapter);
    database_bibleactions.record (bible, book, chapter, usfm);
  }
  
  // Kick the unsent-data timeout mechanism.
  bible_logic_kick_unsent_data_timer ();
  
#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiffBook (bible, book);
  
#endif
  
  // Delete the book from the database.
  database_bibles.deleteBook (bible, book);
}


void bible_logic_delete_bible (const string& bible)
{
  Database_Bibles database_bibles;

  // The client and the cloud record data of the Bible to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible actions.
  Database_BibleActions database_bibleactions;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string usfm = database_bibles.getChapter (bible, book, chapter);
      database_bibleactions.record (bible, book, chapter, usfm);
    }
  }
  
  // Kick the unsent-data timeout mechanism.
  bible_logic_kick_unsent_data_timer ();
  
#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiffBible (bible);

  // Possible git repository.
  string gitdirectory = filter_git_directory (bible);
  if (file_or_dir_exists (gitdirectory)) {
    filter_url_rmdir (gitdirectory);
  }

#endif
  
  // Delete the Bible from the database.
  database_bibles.deleteBible (bible);
  
  // Delete the search index.
  search_logic_delete_bible (bible);
  
  // Delete associated settings and privileges.
  Database_Privileges::removeBible (bible);
  Database_Config_Bible::remove (bible);
}


void bible_logic_import_resource (string bible, string resource)
{
  Database_Logs::log ("Starting to import resource " + resource + " into Bible " + bible);
  
  Database_Versifications database_versifications;
  Webserver_Request webserver_request;
  
  vector <int> books = database_versifications.getMaximumBooks ();
  for (auto & book : books) {
    
    string bookName = Database_Books::getEnglishFromId (book);

    vector <int> chapters = database_versifications.getMaximumChapters (book);
    for (auto & chapter : chapters) {

      string message = "Importing " + resource + " " + bookName + " chapter " + convert_to_string (chapter);
      Database_Logs::log (message, Filter_Roles::translator ());
      
      vector <string> usfm;
      
      if (chapter == 0) usfm.push_back ("\\id " + Database_Books::getUsfmFromId (book));
      
      if (chapter) {
        usfm.push_back ("\\c " + convert_to_string (chapter));
        usfm.push_back ("\\p");
      }

      vector <int> verses = database_versifications.getMaximumVerses (book, chapter);
      for (auto & verse : verses) {
        
        if (verse == 0) continue;

        // Fetch the text for the passage.
        bool server_is_installing_module = false;
        int wait_iterations = 0;
        string html;
        do {
          // Fetch this resource from the server.
          html = resource_logic_get_verse (&webserver_request, resource, book, chapter, verse);
          server_is_installing_module = (html == sword_logic_installing_module_text ());
          if (server_is_installing_module) {
            Database_Logs::log (translate ("Waiting while Bibledit Cloud installs the requested SWORD module"));
            this_thread::sleep_for (chrono::seconds (60));
            wait_iterations++;
          }
        } while (server_is_installing_module && (wait_iterations < 5));
        
        // Remove all html markup.
        html = filter_string_html2text (html);
        html = filter_string_str_replace ("\n", " ", html);

        // Add the verse to the USFM.
        usfm.push_back ("\\v " + convert_to_string (verse) + " " + filter_string_trim (html));
      }
      bible_logic_store_chapter (bible, book, chapter, filter_string_implode (usfm, "\n"));
    }
  }
  
  Database_Logs::log ("Completed importing resource " + resource + " into Bible " + bible);
}


// This logs the change in the Bible text.
// When $force is given, it records the change on clients also.
void bible_logic_log_change (const string& bible, int book, int chapter, const string& usfm, string user, const string & summary, bool force)
{
#ifdef HAVE_CLIENT
  if (!force) return;
#endif
  (void) force;
  
  Database_Bibles database_bibles;
  string existing_usfm = database_bibles.getChapter (bible, book, chapter);

  // It used to calculate the percentage difference, but this took a relatively long time.
  // In particular on low-power devices and on Windows, the time it took was excessive.

  string bookname = Database_Books::getEnglishFromId (book);
  string passage = bible + " " + bookname + " " + convert_to_string (chapter);
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  vector <int> existing_verse_numbers = usfm_get_verse_numbers (existing_usfm);
  vector <int> verse_numbers = usfm_get_verse_numbers (usfm);
  vector <int> verses = existing_verse_numbers;
  verses.insert (verses.end (), verse_numbers.begin (), verse_numbers.end ());
  verses = array_unique (verses);
  sort (verses.begin (), verses.end ());

  vector <string> body;
  
  body.push_back ("Changes:");
  
  for (auto verse : verses) {
    string existing_verse_usfm = usfm_get_verse_text (existing_usfm, verse);
    string verse_usfm = usfm_get_verse_text (usfm, verse);
    if (existing_verse_usfm != verse_usfm) {
      Filter_Text filter_text_old = Filter_Text (bible);
      Filter_Text filter_text_new = Filter_Text (bible);
      filter_text_old.text_text = new Text_Text ();
      filter_text_new.text_text = new Text_Text ();
      filter_text_old.addUsfmCode (existing_verse_usfm);
      filter_text_new.addUsfmCode (verse_usfm);
      filter_text_old.run (stylesheet);
      filter_text_new.run (stylesheet);
      string old_text = filter_text_old.text_text->get ();
      string new_text = filter_text_new.text_text->get ();
      if (old_text != new_text) {
        body.push_back ("");
        body.push_back (filter_passage_display (book, chapter, convert_to_string (verse)));
        body.push_back ("Old: " + old_text);
        body.push_back ("New: " + new_text);
      }
    }
  }

  body.push_back ("");
  body.push_back ("Old USFM:");
  body.push_back (existing_usfm);

  body.push_back ("");
  body.push_back ("New USFM:");
  body.push_back (usfm);
  
  if (!user.empty ()) user.append (" - ");
  Database_Logs::log (user + summary + " - " + passage, filter_string_implode (body, "\n"));
}


// This logs the change in the Bible text as a result of a merge operation.
// This is mostly for diagnostics as at times there's queries on how the merge was done.
void bible_logic_log_merge (string user, string bible, int book, int chapter,
                            string base, string change, string prioritized_change, string result)
{
  string bookname = Database_Books::getEnglishFromId (book);
  string passage = bible + " " + bookname + " " + convert_to_string (chapter);
  
  vector <string> body;

  body.push_back ("This is a record of a merge operation after receiving an update on a chapter from a client.");
  body.push_back ("");
  body.push_back ("Base:");
  body.push_back (base);
  
  body.push_back ("");
  body.push_back ("Change:");
  body.push_back (change);
  
  body.push_back ("");
  body.push_back ("Existing:");
  body.push_back (prioritized_change);
  
  body.push_back ("");
  body.push_back ("Result:");
  body.push_back (result);
  
  Database_Logs::log (user + " - merge record - " + passage, filter_string_implode (body, "\n"));
}


void bible_logic_kick_unsent_data_timer ()
{
  // The timer contains the oldest age of Bible data on a client not yet sent to the Cloud.
  // If the timer has been set already, bail out.
  if (Database_Config_General::getUnsentBibleDataTime () != 0) return;
  
  // Stamp with the current time.
  Database_Config_General::setUnsentBibleDataTime (filter_date_seconds_since_epoch ());
}


void bible_logic_kick_unreceived_data_timer ()
{
  Database_Config_General::setUnreceivedBibleDataTime (filter_date_seconds_since_epoch ());
}


// This returns a warning in case there's old Bible data not yet sent to the Cloud,
// or in case it has not received data from the Cloud for some days.
string bible_logic_unsent_unreceived_data_warning ()
{
  string warning;

#ifdef HAVE_CLIENT

  // Time-stamp for oldest unreceived Bible data.
  int data_time = Database_Config_General::getUnreceivedBibleDataTime ();
  // A value of 0 means that it is not relevant.
  if (data_time) {
    int now = filter_date_seconds_since_epoch ();
    // Unreceived data warning after four days.
    data_time += (4 * 24 * 3600);
    if (now > data_time) {
      warning.clear ();
      warning.append (translate ("It has been some time ago that changes in the Bible text were received from the Cloud."));
      warning.append (" ");
      warning.append (translate ("This will right itself in a little time if you are connected to the Internet."));
      warning.append (" ");
      warning.append (translate ("If not please do a Send/receive action."));
    }
  }
  
  // Time-stamp for oldest unsent Bible data.
  data_time = Database_Config_General::getUnsentBibleDataTime ();
  // A value of 0 means that there's no pending data.
  if (data_time) {
    int now = filter_date_seconds_since_epoch ();
    // Unsent data warning after four days.
    data_time += (4 * 24 * 3600);
    if (now > data_time) {
      warning.clear ();
      warning.append (translate ("There are pending changes in the Bible text that have not yet been sent to the Cloud for some time."));
      warning.append (" ");
      warning.append (translate ("This will right itself in a little time if you are connected to the Internet."));
      warning.append (" ");
      warning.append (translate ("If not please do a Send/receive action."));
    }
  }

#endif

  return warning;
}


void bible_logic_merge_irregularity_mail (vector <string> users, vector <Merge_Conflict> conflicts)
{
  if (conflicts.empty ()) return;
  
  for (auto & conflict : conflicts) {
    
    // Add the passage to the subject.
    string newsubject = conflict.subject;
    if (conflict.book) newsubject.append (" | " + filter_passage_display (conflict.book, conflict.chapter, ""));
    
    // Create the body of the email.
    xml_document document;
    xml_node node;
    node = document.append_child ("h3");
    node.text ().set (newsubject.c_str());

    // Storage of the changes the user sent, and the result that was saved, in raw USFM, per verse.
    vector <string> change_usfm;
    vector <string> result_usfm;

    // Go through all verses available in the USFM,
    // and make a record for each verse,
    // where the USFM differs between the change that the user made and the result that was saved.
    vector <int> verses = usfm_get_verse_numbers (conflict.result);
    for (auto verse : verses) {
      string change = usfm_get_verse_text (conflict.change, verse);
      string result = usfm_get_verse_text (conflict.result, verse);
      // When there's no change in the verse, skip it.
      if (change == result) continue;
      // Record the difference.
      change_usfm.push_back (change);
      result_usfm.push_back (result);
    }

    // Add some information for the user.
    node = document.append_child ("p");
    node.text ().set ("You sent changes to the Cloud. The changes were merged with other changes already in the Cloud. The crossed out parts below could not be merged. They were replaced with the bold text below. You may want to check the changes or resend them.");

    // Go over each verse where the change differs from the resulting text that was saved.
    // For each verse, outline the difference between the change and the result.
    // Clearly maark it up for the user.
    // The purpose is that the user immediately can see what happened,
    // and whether and how to correct it.
    for (size_t i = 0; i < change_usfm.size(); i++) {
      node = document.append_child ("p");
      string modification = filter_diff_diff (change_usfm[i], result_usfm[i]);
      // Add raw html to the email's text buffer.
      node.append_buffer (modification.c_str (), modification.size ());
    }
    
    // Add some information for the user.
    document.append_child ("hr");
    document.append_child ("br");
    xml_node div_node;
    div_node = document.append_child ("div");
    div_node.append_attribute ("style") = "font-size: 30%";

    node = div_node.append_child ("p");
    node.text ().set ("Full details follow below.");
    
    // Add the base text.
    div_node.append_child ("br");
    node = div_node.append_child ("p");
    node.text ().set ("Base text loaded in your editor");
    node = div_node.append_child ("pre");
    node.text ().set (conflict.base.c_str ());
    
    // Add the changed text.
    div_node.append_child ("br");
    node = div_node.append_child ("p");
    node.text ().set ("Changed text by you");
    node = div_node.append_child ("pre");
    node.text ().set (conflict.change.c_str ());
    
    // Add the existing text.
    div_node.append_child ("br");
    node = div_node.append_child ("p");
    node.text ().set ("Existing text in the Cloud");
    node = div_node.append_child ("pre");
    node.text ().set (conflict.prioritized_change.c_str ());
    
    // Add the merge result.
    div_node.append_child ("br");
    node = div_node.append_child ("p");
    node.text ().set ("The text that was actually saved to the chapter in the Cloud");
    node = div_node.append_child ("pre");
    node.text ().set (conflict.result.c_str ());
    
    // Convert the document to a string.
    stringstream output;
    document.print (output, "", format_raw);
    string html = output.str ();
    
    // Schedule the mail for sending to the user(s).
    for (auto user : users) {
      email_schedule (user, newsubject, html);
    }
  }
}


void bible_logic_unsafe_save_mail (const string & message, const string & explanation, const string & user, const string & usfm)
{
  if (message.empty ()) return;
  
  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (message.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  node.text ().set (explanation.c_str ());
  node = document.append_child ("p");
  node.text ().set ("Failed to save the text below.");
  
  // Add the base text.
  document.append_child ("br");
  node = document.append_child ("pre");
  node.text ().set (usfm.c_str ());
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, message, html);
}


// This function sends an email
// if the USFM received from the client
// does not match the USFM that gets stored on the server.
void bible_logic_client_receive_merge_mail (const string & bible, int book, int chapter,
                                            const string & user,
                                            const string & client_old,
                                            const string & client_new,
                                            const string & server)
{
  // No difference: Done.
  if (client_old == server) return;
  
  vector <string> client_diff, server_diff;
  
  // Go through all verses from the client,
  // and make a record for each verse,
  // where the USFM differs between client and server.
  vector <int> verses = usfm_get_verse_numbers (client_old);
  for (auto verse : verses) {
    string client_old_verse = usfm_get_verse_text (client_old, verse);
    string client_new_verse = usfm_get_verse_text (client_new, verse);
    // When there's no change in the verse as sent by the client, skip further checks.
    if (client_old_verse == client_new_verse) continue;
    // Check whether the client's change made it to the server.
    string server_verse = usfm_get_verse_text (server, verse);
    if (client_new_verse == server_verse) continue;
    // Record the difference.
    client_diff.push_back (client_new_verse);
    server_diff.push_back (server_verse);
  }

  // No differences found: Done.
  if (client_diff.empty ()) return;
  
  string location = bible + " " + filter_passage_display (book, chapter, "");
  string subject = "Saved Bible text was merged " + location;
  
  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  string information;
  information.append (translate ("The Cloud already had some changes in this chapter."));
  information.append (" ");
  information.append (translate ("You sent some changes for this chapter too."));
  information.append (" ");
  information.append (translate ("The Cloud merged the two."));
  information.append (" ");
  information.append (translate ("Most likely this is okay."));
  information.append (" ");
  information.append (translate ("You may want to check the result of the merge, whether it is correct."));
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());

  for (unsigned int i = 0; i < client_diff.size(); i++) {

    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("You sent:");
    node = document.append_child ("p");
    node.text ().set (client_diff[i].c_str ());
    node = document.append_child ("p");
    node.text ().set ("The Cloud now has:");
    node = document.append_child ("p");
    node.text ().set (server_diff[i].c_str ());
    node = document.append_child ("p");
    node.text ().set ("The difference is:");
    node = document.append_child ("p");
    string difference = filter_diff_diff (client_diff[i], server_diff[i]);
    node.append_buffer (difference.c_str (), difference.size ());
  }
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


// This emails pending Bible updates to the user.
void bible_logic_client_mail_pending_bible_updates (string user)
{
  // Iterate over all the actions stored for all Bible data ready for sending to the Cloud.
  Database_BibleActions database_bibleactions;
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibleactions.getBibles ();
  for (string bible : bibles) {
    // Skip the Sample Bible, for less clutter.
    if (bible == demo_sample_bible_name ()) continue;
    vector <int> books = database_bibleactions.getBooks (bible);
    for (int book : books) {
      vector <int> chapters = database_bibleactions.getChapters (bible, book);
      for (int chapter : chapters) {
        
        // Get old and new USFM for this chapter.
        string oldusfm = database_bibleactions.getUsfm (bible, book, chapter);
        string newusfm = database_bibles.getChapter (bible, book, chapter);
        // If old USFM and new USFM are the same, or the new USFM is empty, skip it.
        if (newusfm == oldusfm) continue;
        if (newusfm.empty ()) continue;

        string location = bible + " " + filter_passage_display (book, chapter, "");
        string subject = "Discarded text update " + location;
        
        // Create the body of the email.
        xml_document document;
        xml_node node;
        node = document.append_child ("h3");
        node.text ().set (subject.c_str());
        
        // Add some information for the user.
        node = document.append_child ("p");
        string information;
        information.append (translate ("You have now connected to Bibledit Cloud."));
        information.append (" ");
        information.append (translate ("While you were disconnected from Bibledit Cloud, you made some changes in the Bible text on your device."));
        information.append (" ");
        information.append (translate ("These changes will not be saved to Bibledit Cloud."));
        information.append (" ");
        information.append (translate ("The unsaved text is below."));
        node.text ().set (information.c_str());
        
        // Add the passage.
        node = document.append_child ("p");
        node.text ().set (location.c_str ());
        
        // Add the text.
        document.append_child ("br");
        node = document.append_child ("pre");
        node.text ().set (newusfm.c_str ());
        
        // Convert the document to a string.
        stringstream output;
        document.print (output, "", format_raw);
        string html = output.str ();
        
        // Schedule the mail for sending to the user.
        email_schedule (user, subject, html);
      }
    }
  }
}


void bible_logic_client_no_write_access_mail (const string & bible, int book, int chapter, const string & user,
                                              const string & oldusfm, const string & newusfm)
{
  // No difference: Done.
  if (oldusfm == newusfm) return;
  
  vector <string> client_new_diff, client_old_diff;
  
  // Go through all verses from the client,
  // and make a record for each verse,
  // where the USFM differs between client and server.
  vector <int> verses = usfm_get_verse_numbers (oldusfm);
  for (auto verse : verses) {
    string client_old_verse = usfm_get_verse_text (oldusfm, verse);
    string client_new_verse = usfm_get_verse_text (newusfm, verse);
    // When there's no change in the verse as sent by the client, skip further checks.
    if (client_old_verse == client_new_verse) continue;
    // Record the difference.
    client_new_diff.push_back (client_new_verse);
    client_old_diff.push_back (client_old_verse);
  }
  
  // No differences found: Done.
  if (client_new_diff.empty ()) return;
  
  string subject = "No write access while sending Bible text";
  
  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  string information;
  information.append (translate ("While sending Bible text to Bibledit Cloud, you did not have write access to this chapter."));
  information.append (" ");
  information.append (translate ("You may want to check whether this is correct."));
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());
  
  for (unsigned int i = 0; i < client_new_diff.size(); i++) {
    
    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("You sent:");
    node = document.append_child ("p");
    node.text ().set (client_new_diff[i].c_str ());
    node = document.append_child ("p");
    node.text ().set ("The difference is:");
    node = document.append_child ("p");
    string difference = filter_diff_diff (client_old_diff[i], client_new_diff[i]);
    node.append_buffer (difference.c_str (), difference.size ());
  }
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


void bible_logic_recent_save_email (const string & bible, int book, int chapter, int verse,
                                    const string & user,
                                    const string & old_usfm, const string & new_usfm)
{
  (void) verse;
  
  vector <string> old_verses;
  vector <string> new_verses;

  // Go through all verses available in the USFM,
  // and make a record for each verse,
  // where the USFM differs between the old and the new USFM.
  vector <int> verses = usfm_get_verse_numbers (new_usfm);
  for (auto verse : verses) {
    string old_verse = usfm_get_verse_text (old_usfm, verse);
    string new_verse = usfm_get_verse_text (new_usfm, verse);
    // When there's no change in the verse, skip further checks.
    if (old_verse == new_verse) continue;
    // Record the difference.
    old_verses.push_back (old_verse);
    new_verses.push_back (new_verse);
  }

  // No differences found: Done.
  if (new_verses.empty ()) return;

  string subject = translate ("Check whether Bible text was saved");
  
  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  string information;
  information.append (translate ("Bibledit saved the Bible text below."));
  information.append (" ");
  information.append (translate ("But Bibledit is not entirely sure that all went well."));
  information.append (" ");
  information.append (translate ("You may want to check whether the Bible text was saved correctly."));
  information.append (" ");
  information.append (translate ("The text in bold was added."));
  information.append (" ");
  information.append (translate ("The text in strikethrough was removed."));
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());

  bool differences_found = false;
  for (unsigned int i = 0; i < new_verses.size(); i++) {
    Filter_Text filter_text_old = Filter_Text (bible);
    Filter_Text filter_text_new = Filter_Text (bible);
    filter_text_old.html_text_standard = new Html_Text (translate("Bible"));
    filter_text_new.html_text_standard = new Html_Text (translate("Bible"));
    filter_text_old.text_text = new Text_Text ();
    filter_text_new.text_text = new Text_Text ();
    filter_text_old.addUsfmCode (old_verses[i]);
    filter_text_new.addUsfmCode (new_verses[i]);
    filter_text_old.run (styles_logic_standard_sheet());
    filter_text_new.run (styles_logic_standard_sheet());
    string old_text = filter_text_old.text_text->get ();
    string new_text = filter_text_new.text_text->get ();
    if (old_text != new_text) {
      node = document.append_child ("p");
      string modification = filter_diff_diff (old_text, new_text);
      string fragment = /* convert_to_string (verse) + " " + */ modification;
      node.append_buffer (fragment.c_str (), fragment.size ());
      differences_found = true;
    }
  }
  // If no differences were found, bail out.
  // This also handles differences in spacing.
  // If the differences consist of whitespace only, bail out here.
  // See issue https://github.com/bibledit/cloud/issues/413
  if (!differences_found) return;
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  string html = output.str ();

  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


void bible_logic_optional_merge_irregularity_email (const string & bible, int book, int chapter,
                                                    const string & user,
                                                    const string & ancestor_usfm,
                                                    const string & edited_usfm,
                                                    const string & merged_usfm)
{
  // If the merged edited USFM is the same as the edited USFM,
  // that means that the user's changes will get saved to the chapter.
  if (edited_usfm == merged_usfm) return;
  // But if the merged edited USFM differs from the original edited USFM,
  // more checks need to be done to be sure that the user's edits made it.

  string subject = translate ("Check whether Bible text was saved");

  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  string information;
  information.append (translate ("Bibledit saved the Bible text below."));
  information.append (" ");
  information.append (translate ("But Bibledit is not entirely sure that all went well."));
  information.append (" ");
  information.append (translate ("You may want to check whether the Bible text was saved correctly."));
  information.append (" ");
  information.append (translate ("The text in bold was added."));
  information.append (" ");
  information.append (translate ("The text in strikethrough was removed."));
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());

  bool anomalies_found = false;

  // Go through all verses available in the USFM,
  // and check the differences for each verse.
  vector <int> verses = usfm_get_verse_numbers (merged_usfm);
  for (auto verse : verses) {
    string ancestor_verse_usfm = usfm_get_verse_text (ancestor_usfm, verse);
    string edited_verse_usfm = usfm_get_verse_text (edited_usfm, verse);
    string merged_verse_usfm = usfm_get_verse_text (merged_usfm, verse);
    // There's going to be a check to find out that all the changes the user made,
    // are available among the changes resulting from the merge.
    // If all the changes are there, all is good.
    // If not, then email the user about this.
    bool anomaly_found = false;
    vector <string> user_removals, user_additions, merged_removals, merged_additions;
    filter_diff_diff (ancestor_verse_usfm, edited_verse_usfm, &user_removals, &user_additions);
    filter_diff_diff (ancestor_verse_usfm, merged_verse_usfm, &merged_removals, &merged_additions);
    //vector <string> missed_removals, missed_additions;
    for (auto user_removal : user_removals) {
      //if (!in_array (user_removal, merged_removals)) missed_removals.push_back (user_removal);
      if (!in_array (user_removal, merged_removals)) anomaly_found = true;
    }
    for (auto user_addition : user_additions) {
      //if (!in_array (user_addition, merged_additions)) missed_additions.push_back (user_addition);
      if (!in_array (user_addition, merged_additions)) anomaly_found = true;
    }
    if (!anomaly_found) continue;
    anomalies_found = true;
    Filter_Text filter_text_ancestor = Filter_Text (bible);
    Filter_Text filter_text_edited = Filter_Text (bible);
    Filter_Text filter_text_merged = Filter_Text (bible);
    filter_text_ancestor.html_text_standard = new Html_Text (translate("Bible"));
    filter_text_edited.html_text_standard = new Html_Text (translate("Bible"));
    filter_text_merged.html_text_standard = new Html_Text (translate("Bible"));
    filter_text_ancestor.text_text = new Text_Text ();
    filter_text_edited.text_text = new Text_Text ();
    filter_text_merged.text_text = new Text_Text ();
    filter_text_ancestor.addUsfmCode (ancestor_verse_usfm);
    filter_text_edited.addUsfmCode (edited_verse_usfm);
    filter_text_merged.addUsfmCode (merged_verse_usfm);
    filter_text_ancestor.run (styles_logic_standard_sheet());
    filter_text_edited.run (styles_logic_standard_sheet());
    filter_text_merged.run (styles_logic_standard_sheet());
    string ancestor_text = filter_text_ancestor.text_text->get ();
    string edited_text = filter_text_edited.text_text->get ();
    string merged_text = filter_text_merged.text_text->get ();
    string modification;
    node = document.append_child ("p");
    modification = translate ("You edited:") + " " + filter_diff_diff (ancestor_text, edited_text);
    node.append_buffer (modification.c_str (), modification.size ());
    node = document.append_child ("p");
    modification = translate ("Bibledit saved:") + " " + filter_diff_diff (ancestor_text, merged_text);
    node.append_buffer (modification.c_str (), modification.size ());
  }

  // If no differences were found, bail out.
  // This also handles differences in spacing.
  // If the differences consist of whitespace only, bail out here.
  // See issue https://github.com/bibledit/cloud/issues/413
  if (!anomalies_found) return;
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  string html = output.str ();

  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


const char * bible_logic_insert_operator ()
{
  return "i";
}
const char * bible_logic_delete_operator ()
{
  return "d";
}
const char * bible_logic_format_paragraph_operator ()
{
  return "p";
}
const char * bible_logic_format_character_operator ()
{
  return "c";
}

// There are three containers with updating information.
// The function condenses this updating information.
// This condensed information works better for the Quill editor.
void bible_logic_condense_editor_updates (const vector <int> & positions_in,
                                          const vector <int> & sizes_in,
                                          const vector <bool> & additions_in,
                                          const vector <string> & content_in,
                                          vector <int> & positions_out,
                                          vector <int> & sizes_out,
                                          vector <string> & operators_out,
                                          vector <string> & content_out)
{
  positions_out.clear();
  sizes_out.clear();
  operators_out.clear();
  content_out.clear();
  
  int previous_position = numeric_limits<int>::min();
  bool previous_addition = false;
  string previous_character = string();
  for (size_t i = 0; i < positions_in.size(); i++) {
    int position     = positions_in[i];
    int size         = sizes_in[i];
    bool addition    = additions_in[i];
    string character = content_in[i].substr (0, 1);
    string format = content_in[i].substr (1);

    // The following situation occurs when changing the style of a paragraph.
    // Like for example changing a paragraph style from "p" to "s".
    // The current sequence for this change is:
    // 1. Delete a new line at a known position.
    // 2. Insert a new line at the same position, with a given format.
    // Condense this as follows:
    // 1. Apply the given paragraph format to the given position.
    // Without condensing this, the following would happen:
    // 1. Delete the new line before the second line.
    // Result: The first line gets the format of the second line.
    // 2. Insert the new line before the second line again.
    // 3. Appy formatting to the second line.
    // The net result is that the first line remains with the paragraph format of the second line.
    bool newlineflag = addition && !previous_addition && (character == "\n") && (character == previous_character) && (position == previous_position);
    if (newlineflag) {
      // Remove the previous "delete new line".
      positions_out.pop_back();
      sizes_out.pop_back();
      operators_out.pop_back();
      content_out.pop_back();
      // Add the paragraph format operation data.
      positions_out.push_back(position);
      sizes_out.push_back(size);
      operators_out.push_back(bible_logic_format_paragraph_operator());
      content_out.push_back(format);
    } else {
      positions_out.push_back(position);
      sizes_out.push_back(size);
      if (addition) operators_out.push_back(bible_logic_insert_operator());
      else operators_out.push_back(bible_logic_delete_operator());
      content_out.push_back(character + format);
    }

    // Store data for the next iteration.
    previous_position = position;
    previous_addition = addition;
    previous_character = character;
  }
  
}


void bible_logic_html_to_editor_updates (const string & editor_html,
                                         const string & server_html,
                                         vector <int> & positions,
                                         vector <int> & sizes,
                                         vector <string> & operators,
                                         vector <string> & content)
{
  // Clear outputs.
  positions.clear();
  sizes.clear();
  operators.clear();
  content.clear();
  
  // Convert the html to formatted text.
  Editor_Html2Format editor_format;
  Editor_Html2Format server_format;
  editor_format.load (editor_html);
  server_format.load (server_html);
  editor_format.run ();
  server_format.run ();

  // Convert the formatted text fragments to formatted UTF-8 characters.
  vector <string> editor_formatted_character_content;
  for (size_t i = 0; i < editor_format.texts.size(); i++) {
    string text = editor_format.texts[i];
    string format = editor_format.formats[i];
    size_t length = unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      string utf8_character = unicode_string_substr (text, pos, 1);
      editor_formatted_character_content.push_back (utf8_character + format);
    }
  }
  vector <string> server_formatted_character_content;
  vector <string> server_utf8_characters;
  for (size_t i = 0; i < server_format.texts.size(); i++) {
    string text = server_format.texts[i];
    string format = server_format.formats[i];
    size_t length = unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      string utf8_character = unicode_string_substr (text, pos, 1);
      server_formatted_character_content.push_back (utf8_character + format);
      server_utf8_characters.push_back(utf8_character);
    }
  }

  // Find the differences between the two sets of content.
  vector <int> positions_diff;
  vector <int> sizes_diff;
  vector <bool> additions_diff;
  vector <string> content_diff;
  int new_line_diff_count;
  filter_diff_diff_utf16 (editor_formatted_character_content, server_formatted_character_content,
                          positions_diff, sizes_diff, additions_diff, content_diff, new_line_diff_count);

  // Condense the differences a bit and render them to another format.
  bible_logic_condense_editor_updates (positions_diff, sizes_diff, additions_diff, content_diff,
                                       positions, sizes, operators, content);

  // Problem description:
  // User action: Remove the new line at the end of the current paragraph.
  // Result: The current paragraph takes the style of the next paragraph.
  // User actions: While removing notes, this goes wrong.
  // Solution:
  // If there's new line(s) added or removed, apply all paragraph styles again.
  if (new_line_diff_count) {
    int position = 0;
    for (size_t i = 0; i < server_utf8_characters.size(); i++) {
      int size = (int)convert_to_u16string (server_utf8_characters[i]).length();
      if (server_utf8_characters[i] == "\n") {
        positions.push_back(position);
        sizes.push_back(size);
        operators.push_back(bible_logic_format_paragraph_operator());
        content.push_back(server_formatted_character_content[i].substr (1));
      }
      position += size;
    }
  }
  
}

/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <bible/logic.h>
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


void bible_logic_store_chapter (const string& bible, int book, int chapter, const string& usfm)
{
  Database_Bibles database_bibles;

  // Record data of the chapter to be stored prior to storing the new version.
  if (client_logic_client_enabled ()) {

    // Client stores Bible action.
    string oldusfm = database_bibles.getChapter (bible, book, chapter);
    Database_BibleActions database_bibleactions;
    database_bibleactions.record (bible, book, chapter, oldusfm);
    
    // Kick the unsent-data timeout mechanism.
    bible_logic_kick_unsent_data_timer ();

  } else {

    // Server stores diff data.
    Database_Modifications database_modifications;
    database_modifications.storeTeamDiff (bible, book, chapter);

  }

  // Store the chapter in the database.
  database_bibles.storeChapter (bible, book, chapter, usfm);
}


void bible_logic_delete_chapter (const string& bible, int book, int chapter)
{
  Database_Bibles database_bibles;

  // Record data of the chapter to be deleted prior to deletion.
  if (client_logic_client_enabled ()) {

    // Client stores Bible action.
    string usfm = database_bibles.getChapter (bible, book, chapter);
    Database_BibleActions database_bibleactions;
    database_bibleactions.record (bible, book, chapter, usfm);

    // Kick the unsent-data timeout mechanism.
    bible_logic_kick_unsent_data_timer ();

  } else {

    // Server stores diff data.
    Database_Modifications database_modifications;
    database_modifications.storeTeamDiff (bible, book, chapter);

  }

  // Delete the chapter from the database.
  database_bibles.deleteChapter (bible, book, chapter);
}


void bible_logic_delete_book (const string& bible, int book)
{
  Database_Bibles database_bibles;

  // Record data of the book to be deleted prior to deletion.
  if (client_logic_client_enabled ()) {

    // Client stores Bible actions.
    Database_BibleActions database_bibleactions;
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto & chapter : chapters) {
      string usfm = database_bibles.getChapter (bible, book, chapter);
      database_bibleactions.record (bible, book, chapter, usfm);
    }

    // Kick the unsent-data timeout mechanism.
    bible_logic_kick_unsent_data_timer ();

  } else {

    // Server stores diff data.
    Database_Modifications database_modifications;
    database_modifications.storeTeamDiffBook (bible, book);

  }

  // Delete the book from the database.
  database_bibles.deleteBook (bible, book);
}


void bible_logic_delete_bible (const string& bible)
{
  Database_Bibles database_bibles;

  // Record data of the Bible to be deleted prior to deletion.
  if (client_logic_client_enabled ()) {

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

  } else {

    // Server stores diff data.
    Database_Modifications database_modifications;
    database_modifications.storeTeamDiffBible (bible);
    
#ifdef HAVE_CLOUD
    // Possible git repository.
    string gitdirectory = filter_git_directory (bible);
    if (file_or_dir_exists (gitdirectory)) {
      filter_url_rmdir (gitdirectory);
    }
#endif

  }

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
            Database_Logs::log ("Waiting while Bibledit Cloud installs the requested SWORD module");
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
// $extended: Whether to give an extended warning.
string bible_logic_unsent_unreceived_data_warning (bool extended)
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
      if (extended) {
        warning.append (translate ("It has been some time ago that changes in the Bible text were received from the Cloud."));
        warning.append (" ");
        warning.append (translate ("Please do a Send/receive action to receive them from the Cloud."));
      } else {
        warning.append (translate ("Please do a Send/receive to the Cloud"));
      }
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
      if (extended) {
        warning.append (translate ("There are pending changes in the Bible text that have not yet been sent to the Cloud for some time."));
        warning.append (" ");
        warning.append (translate ("Please do a Send/receive action to send them to the Cloud."));
      } else {
        warning.append (translate ("Please do a Send/receive to the Cloud"));
      }
    }
  }

#endif
  
  (void) extended;

  return warning;
}


void bible_logic_merge_irregularity_mail (vector <string> users,
                                          vector <tuple <string, string, string, string, string>> conflicts)
{
  if (conflicts.empty ()) return;
  
  for (auto & conflict : conflicts) {
    
    string base = get<0>(conflict);
    string change = get<1>(conflict);
    string prioritized_change = get<2>(conflict);
    string result = get<3>(conflict);;
    string subject = get<4>(conflict);
    
    // Create the body of the email.
    xml_document document;
    xml_node node;
    node = document.append_child ("h3");
    node.text ().set (subject.c_str());
    
    // Add some information for the user.
    node = document.append_child ("p");
    node.text ().set ("While saving Bible text, Bibledit detected something unusual.");
    
    // Add the base text.
    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("Base text");
    node = document.append_child ("pre");
    node.text ().set (base.c_str ());
    
    // Add the changed text.
    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("Changed text");
    node = document.append_child ("pre");
    node.text ().set (change.c_str ());
    
    // Add the existing text.
    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("Existing text");
    node = document.append_child ("pre");
    node.text ().set (prioritized_change.c_str ());
    
    // Add the merge result.
    document.append_child ("br");
    node = document.append_child ("p");
    node.text ().set ("The text that was actually saved to the chapter");
    node = document.append_child ("pre");
    node.text ().set (result.c_str ());
    
    // Convert the document to a string.
    stringstream output;
    document.print (output, "", format_raw);
    string html = output.str ();
    
    // Schedule the mail for sending to the user(s).
    for (auto user : users) {
      email_schedule (user, subject, html);
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
  node.text ().set ("Bibledit failed to save the Bible text below.");
  
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


// This function is sends an email
// if the USFM received from the client
// does not match the USFM that gets stored on the server.
void bible_logic_client_receive_merge_mail (const string & bible, int book, int chapter, const string & user,
                                            const string & client_old, const string & client_new, const string & server)
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
  
  string subject = "Saved Bible text was merged";
  
  // Create the body of the email.
  xml_document document;
  xml_node node;
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  node.text ().set ("The Bible text you sent to the Cloud was not saved exactly as you sent it. It was merged with changes already avaible in the Cloud.");
  node = document.append_child ("p");
  string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
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

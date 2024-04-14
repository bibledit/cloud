/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <book/create.h>


void bible_logic::store_chapter (const std::string& bible, int book, int chapter, const std::string& usfm)
{
  Database_Bibles database_bibles {};

  // Record data of the chapter to be stored prior to storing the new version.
  // Both client and cloud follow this order.

#ifdef HAVE_CLIENT

  // Client stores Bible action.
  const std::string oldusfm = database_bibles.get_chapter (bible, book, chapter);
  Database_BibleActions database_bibleactions;
  database_bibleactions.record (bible, book, chapter, oldusfm);
  
  // Kick the unsent-data timeout mechanism.
  bible_logic::kick_unsent_data_timer ();

#endif

#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiff (bible, book, chapter);
  
#endif

  // Store the chapter in the database.
  database_bibles.store_chapter (bible, book, chapter, usfm);
}


void bible_logic::delete_chapter (const std::string& bible, int book, int chapter)
{
  Database_Bibles database_bibles {};

  // Cloud and client record data of the chapter to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible action.
  const std::string usfm = database_bibles.get_chapter (bible, book, chapter);
  Database_BibleActions database_bibleactions;
  database_bibleactions.record (bible, book, chapter, usfm);
  
  // Kick the unsent-data timeout mechanism.
  bible_logic::kick_unsent_data_timer ();

#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiff (bible, book, chapter);
  
#endif

  // Delete the chapter from the database.
  database_bibles.delete_chapter (bible, book, chapter);
}


void bible_logic::delete_book (const std::string& bible, int book)
{
  Database_Bibles database_bibles {};

  // Both client and cloud record data of the book to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible actions.
  Database_BibleActions database_bibleactions;
  const std::vector <int> chapters = database_bibles.get_chapters (bible, book);
  for (const auto& chapter : chapters) {
    const std::string usfm = database_bibles.get_chapter (bible, book, chapter);
    database_bibleactions.record (bible, book, chapter, usfm);
  }
  
  // Kick the unsent-data timeout mechanism.
  bible_logic::kick_unsent_data_timer ();
  
#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications;
  database_modifications.storeTeamDiffBook (bible, book);
  
#endif
  
  // Delete the book from the database.
  database_bibles.delete_book (bible, book);
}


void bible_logic::delete_bible (const std::string& bible)
{
  Database_Bibles database_bibles {};

  // The client and the cloud record data of the Bible to be deleted prior to deletion.
  
#ifdef HAVE_CLIENT

  // Client stores Bible actions.
  Database_BibleActions database_bibleactions {};
  const std::vector <int> books = database_bibles.get_books (bible);
  for (const auto book : books) {
    const std::vector <int> chapters = database_bibles.get_chapters (bible, book);
    for (const auto chapter : chapters) {
      const std::string usfm = database_bibles.get_chapter (bible, book, chapter);
      database_bibleactions.record (bible, book, chapter, usfm);
    }
  }
  
  // Kick the unsent-data timeout mechanism.
  bible_logic::kick_unsent_data_timer ();
  
#endif
  
#ifdef HAVE_CLOUD

  // Server stores diff data.
  Database_Modifications database_modifications {};
  database_modifications.storeTeamDiffBible (bible);

  // Possible git repository.
  const std::string gitdirectory = filter_git_directory (bible);
  if (file_or_dir_exists (gitdirectory)) {
    filter_url_rmdir (gitdirectory);
  }

#endif
  
  // Delete the Bible from the database.
  database_bibles.delete_bible (bible);
  
  // Delete the search index.
  search_logic_delete_bible (bible);
  
  // Delete associated settings and privileges.
  DatabasePrivileges::remove_bible (bible);
  Database_Config_Bible::remove (bible);
}


void bible_logic::import_resource (std::string bible, std::string resource)
{
  Database_Logs::log ("Starting to import resource " + resource + " into Bible " + bible);
  
  Database_Versifications database_versifications {};
  Webserver_Request webserver_request {};
  
  const std::vector <int> books = database_versifications.getMaximumBooks ();
  for (const auto book : books) {
    
    const std::string bookName = database::books::get_english_from_id (static_cast<book_id>(book));

    const std::vector <int> chapters = database_versifications.getMaximumChapters (book);
    for (const auto chapter : chapters) {

      const std::string message = "Importing " + resource + " " + bookName + " chapter " + filter::strings::convert_to_string (chapter);
      Database_Logs::log (message, Filter_Roles::translator ());
      
      std::vector <std::string> usfm {};
      
      if (chapter == 0) usfm.push_back ("\\id " + database::books::get_usfm_from_id (static_cast<book_id>(book)));
      
      if (chapter) {
        usfm.push_back ("\\c " + filter::strings::convert_to_string (chapter));
        usfm.push_back ("\\p");
      }

      const std::vector <int> verses = database_versifications.getMaximumVerses (book, chapter);
      for (const auto verse : verses) {
        
        if (verse == 0) continue;

        // Fetch the text for the passage.
        bool server_is_installing_module = false;
        int wait_iterations = 0;
        std::string html;
        do {
          // Fetch this resource from the server.
          html = resource_logic_get_verse (webserver_request, resource, book, chapter, verse);
          server_is_installing_module = (html == sword_logic_installing_module_text ());
          if (server_is_installing_module) {
            Database_Logs::log (translate ("Waiting while Bibledit Cloud installs the requested SWORD module"));
            std::this_thread::sleep_for (std::chrono::seconds (60));
            wait_iterations++;
          }
        } while (server_is_installing_module && (wait_iterations < 5));
        
        // Remove all html markup.
        html = filter::strings::html2text (html);
        html = filter::strings::replace ("\n", " ", html);

        // Add the verse to the USFM.
        usfm.push_back ("\\v " + filter::strings::convert_to_string (verse) + " " + filter::strings::trim (html));
      }
      bible_logic::store_chapter (bible, book, chapter, filter::strings::implode (usfm, "\n"));
    }
  }
  
  Database_Logs::log ("Completed importing resource " + resource + " into Bible " + bible);
}


// This logs the change in the Bible text.
// When $force is given, it records the change on clients also.
void bible_logic::log_change (const std::string& bible,
                              int book, int chapter,
                              const std::string& usfm,
                              std::string user,
                              const std::string& summary,
                              [[maybe_unused]] bool force)
{
#ifdef HAVE_CLIENT
  if (!force) return;
#endif
  
  Database_Bibles database_bibles;
  const std::string existing_usfm = database_bibles.get_chapter (bible, book, chapter);

  // It used to calculate the percentage difference, but this took a relatively long time.
  // In particular on low-power devices and on Windows, the time it took was excessive.

  const std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  const std::string passage = bible + " " + bookname + " " + filter::strings::convert_to_string (chapter);
  
  const std::string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  const std::vector <int> existing_verse_numbers = filter::usfm::get_verse_numbers (existing_usfm);
  const std::vector <int> verse_numbers = filter::usfm::get_verse_numbers (usfm);
  std::vector <int> verses = existing_verse_numbers;
  verses.insert (verses.end (), verse_numbers.begin (), verse_numbers.end ());
  verses = filter::strings::array_unique (verses);
  sort (verses.begin (), verses.end ());

  std::vector <std::string> body;
  
  body.push_back ("Changes:");
  
  for (const auto verse : verses) {
    const std::string existing_verse_usfm = filter::usfm::get_verse_text (existing_usfm, verse);
    const std::string verse_usfm = filter::usfm::get_verse_text (usfm, verse);
    if (existing_verse_usfm != verse_usfm) {
      Filter_Text filter_text_old = Filter_Text (bible);
      Filter_Text filter_text_new = Filter_Text (bible);
      filter_text_old.text_text = new Text_Text ();
      filter_text_new.text_text = new Text_Text ();
      filter_text_old.add_usfm_code (existing_verse_usfm);
      filter_text_new.add_usfm_code (verse_usfm);
      filter_text_old.run (stylesheet);
      filter_text_new.run (stylesheet);
      const std::string old_text = filter_text_old.text_text->get ();
      const std::string new_text = filter_text_new.text_text->get ();
      if (old_text != new_text) {
        body.push_back (std::string());
        body.push_back (filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)));
        body.push_back ("Old: " + old_text);
        body.push_back ("New: " + new_text);
      }
    }
  }

  body.push_back (std::string());
  body.push_back ("Old USFM:");
  body.push_back (existing_usfm);

  body.push_back (std::string());
  body.push_back ("New USFM:");
  body.push_back (usfm);
  
  if (!user.empty ()) user.append (" - ");
  Database_Logs::log (user + summary + " - " + passage, filter::strings::implode (body, "\n"));
}


// This logs the change in the Bible text as a result of a merge operation.
// This is mostly for diagnostics as at times there's queries on how the merge was done.
void bible_logic::log_merge (const std::string& user, const std::string& bible, int book, int chapter,
                             const std::string& base, const std::string& change,
                             const std::string& prioritized_change, const std::string& result)
{
  const std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  const std::string passage = bible + " " + bookname + " " + filter::strings::convert_to_string (chapter);
  
  std::vector <std::string> body {};

  body.push_back ("This is a record of a merge operation after receiving an update on a chapter from a client.");
  body.push_back (std::string());
  body.push_back ("Base:");
  body.push_back (base);
  
  body.push_back (std::string());
  body.push_back ("Change:");
  body.push_back (change);
  
  body.push_back (std::string());
  body.push_back ("Existing:");
  body.push_back (prioritized_change);
  
  body.push_back (std::string());
  body.push_back ("Result:");
  body.push_back (result);
  
  Database_Logs::log (user + " - merge record - " + passage, filter::strings::implode (body, "\n"));
}


void bible_logic::kick_unsent_data_timer ()
{
  // The timer contains the oldest age of Bible data on a client not yet sent to the Cloud.
  // If the timer has been set already, bail out.
  if (Database_Config_General::getUnsentBibleDataTime () != 0) return;
  
  // Stamp with the current time.
  Database_Config_General::setUnsentBibleDataTime (filter::date::seconds_since_epoch ());
}


void bible_logic::kick_unreceived_data_timer ()
{
  Database_Config_General::setUnreceivedBibleDataTime (filter::date::seconds_since_epoch ());
}


// This returns a warning in case there's old Bible data not yet sent to the Cloud,
// or in case it has not received data from the Cloud for some days.
std::string bible_logic::unsent_unreceived_data_warning ()
{
  std::string warning {};

#ifdef HAVE_CLIENT

  // Time-stamp for oldest unreceived Bible data.
  int data_time = Database_Config_General::getUnreceivedBibleDataTime ();
  // A value of 0 means that it is not relevant.
  if (data_time) {
    const int now = filter::date::seconds_since_epoch ();
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
    const int now = filter::date::seconds_since_epoch ();
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


void bible_logic::merge_irregularity_mail (const std::vector <std::string>& users,
                                           const std::vector <Merge_Conflict>& conflicts)
{
  if (conflicts.empty ()) return;
  
  for (const auto& conflict : conflicts) {
    
    // Add the passage to the subject.
    std::string newsubject = conflict.subject;
    if (conflict.book) newsubject.append (" | " + filter_passage_display (conflict.book, conflict.chapter, std::string()));
    
    // Create the body of the email.
    pugi::xml_document document {};
    pugi::xml_node node = document.append_child ("h3");
    node.text ().set (newsubject.c_str());

    // Storage of the changes the user sent, and the result that was saved, in raw USFM, per verse.
    std::vector <std::string> change_usfm {};
    std::vector <std::string> result_usfm {};

    // Go through all verses available in the USFM,
    // and make a record for each verse,
    // where the USFM differs between the change that the user made and the result that was saved.
    const std::vector <int> verses = filter::usfm::get_verse_numbers (conflict.result);
    for (const auto verse : verses) {
      const std::string change = filter::usfm::get_verse_text (conflict.change, verse);
      const std::string result = filter::usfm::get_verse_text (conflict.result, verse);
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
      const std::string modification = filter_diff_diff (change_usfm[i], result_usfm[i]);
      // Add raw html to the email's text buffer.
      node.append_buffer (modification.c_str (), modification.size ());
    }
    
    // Add some information for the user.
    document.append_child ("hr");
    document.append_child ("br");
    pugi::xml_node div_node {};
    div_node = document.append_child ("div");
    // Disabled: https://github.com/bibledit/cloud/issues/401
    // div_node.append_attribute ("style") = "font-size: 30%";

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
    std::stringstream output {};
    document.print (output, "", pugi::format_raw);
    const std::string html = output.str ();
    
    // Schedule the mail for sending to the user(s).
    for (const auto & user : users) {
      email_schedule (user, newsubject, html);
    }
  }
}


void bible_logic::unsafe_save_mail (std::string subject, const std::string& explanation,
                                    const std::string& user,
                                    const std::string& usfm,
                                    int book, int chapter)
{
  if (subject.empty ()) return;

  // Add the passage to the subject for extra clarity.
  // https://github.com/bibledit/cloud/issues/676
  subject.append (" | " + filter_passage_display (book, chapter, std::string()));

  // Create the body of the email.
  pugi::xml_document document {};
  pugi::xml_node node {};
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
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
  std::stringstream output {};
  document.print (output, "", pugi::format_raw);
  const std::string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


// This function sends an email
// if the USFM received from the client
// does not match the USFM that gets stored on the server.
void bible_logic::client_receive_merge_mail (const std::string& bible, int book, int chapter,
                                             const std::string& user,
                                             const std::string& client_old,
                                             const std::string& client_new,
                                             const std::string& server)
{
  // No difference: Done.
  if (client_old == server) return;
  
  std::vector <std::string> client_diff {};
  std::vector <std::string> server_diff {};
  
  // Go through all verses from the client,
  // and make a record for each verse,
  // where the USFM differs between client and server.
  const std::vector <int> verses = filter::usfm::get_verse_numbers (client_old);
  for (const auto verse : verses) {
    const std::string client_old_verse = filter::usfm::get_verse_text (client_old, verse);
    const std::string client_new_verse = filter::usfm::get_verse_text (client_new, verse);
    // When there's no change in the verse as sent by the client, skip further checks.
    if (client_old_verse == client_new_verse) continue;
    // Check whether the client's change made it to the server.
    const std::string server_verse = filter::usfm::get_verse_text (server, verse);
    if (client_new_verse == server_verse) continue;
    // Record the difference.
    client_diff.push_back (client_new_verse);
    server_diff.push_back (server_verse);
  }

  // No differences found: Done.
  if (client_diff.empty ()) return;
  
  // Add the passage to the subject.
  std::string subject = "Saved Bible text was merged";
  subject.append (" | " + filter_passage_display (book, chapter, std::string()));

  // Create the body of the email.
  pugi::xml_document document {};
  pugi::xml_node node {};
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  std::string information {};
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
  std::string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
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
    const std::string difference = filter_diff_diff (client_diff[i], server_diff[i]);
    node.append_buffer (difference.c_str (), difference.size ());
  }
  
  // Convert the document to a string.
  std::stringstream output {};
  document.print (output, "", pugi::format_raw);
  const std::string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


// This emails pending Bible updates to the user.
void bible_logic::client_mail_pending_bible_updates (const std::string& user)
{
  // Iterate over all the actions stored for all Bible data ready for sending to the Cloud.
  Database_BibleActions database_bibleactions {};
  Database_Bibles database_bibles {};
  const std::vector <std::string> bibles = database_bibleactions.getBibles ();
  for (const auto& bible : bibles) {
    // Skip the Sample Bible, for less clutter.
    if (bible == demo_sample_bible_name ()) continue;
    const std::vector <int> books = database_bibleactions.getBooks (bible);
    for (const int book : books) {
      const std::vector <int> chapters = database_bibleactions.getChapters (bible, book);
      for (const int chapter : chapters) {
        
        // Get old and new USFM for this chapter.
        const std::string oldusfm = database_bibleactions.getUsfm (bible, book, chapter);
        const std::string newusfm = database_bibles.get_chapter (bible, book, chapter);
        // If old USFM and new USFM are the same, or the new USFM is empty, skip it.
        if (newusfm == oldusfm) continue;
        if (newusfm.empty ()) continue;

        // Add the passage to the subject.
        std::string subject = "Discarded text update";
        subject.append (" | " + filter_passage_display (book, chapter, std::string()));
        
        // Create the body of the email.
        pugi::xml_document document {};
        pugi::xml_node node {};
        node = document.append_child ("h3");
        node.text ().set (subject.c_str());
        
        // Add some information for the user.
        node = document.append_child ("p");
        std::string information {};
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
        std::string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
        node.text ().set (location.c_str ());
        
        // Add the text.
        document.append_child ("br");
        node = document.append_child ("pre");
        node.text ().set (newusfm.c_str ());
        
        // Convert the document to a string.
        std::stringstream output {};
        document.print (output, "", pugi::format_raw);
        const std::string html = output.str ();
        
        // Schedule the mail for sending to the user.
        email_schedule (user, subject, html);
      }
    }
  }
}


void bible_logic::client_no_write_access_mail (const std::string& bible, int book, int chapter,
                                               const std::string& user,
                                               const std::string& oldusfm, const std::string& newusfm)
{
  // No difference: Done.
  if (oldusfm == newusfm) return;
  
  std::vector <std::string> client_new_diff {};
  std::vector <std::string> client_old_diff {};
  
  // Go through all verses from the client,
  // and make a record for each verse,
  // where the USFM differs between client and server.
  const std::vector <int> verses = filter::usfm::get_verse_numbers (oldusfm);
  for (const auto verse : verses) {
    const std::string client_old_verse = filter::usfm::get_verse_text (oldusfm, verse);
    const std::string client_new_verse = filter::usfm::get_verse_text (newusfm, verse);
    // When there's no change in the verse as sent by the client, skip further checks.
    if (client_old_verse == client_new_verse) continue;
    // Record the difference.
    client_new_diff.push_back (client_new_verse);
    client_old_diff.push_back (client_old_verse);
  }
  
  // No differences found: Done.
  if (client_new_diff.empty ()) return;
  
  // Add the passage to the subject.
  std::string subject = "No write access while sending Bible text";
  subject.append (" | " + filter_passage_display (book, chapter, ""));

  // Create the body of the email.
  pugi::xml_document document {};
  pugi::xml_node node {};
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  std::string information {};
  information.append (translate ("While sending Bible text to Bibledit Cloud, you did not have write access to this chapter."));
  information.append (" ");
  information.append (translate ("You may want to check whether this is correct."));
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  const std::string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
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
    const std::string difference = filter_diff_diff (client_old_diff[i], client_new_diff[i]);
    node.append_buffer (difference.c_str (), difference.size ());
  }
  
  // Convert the document to a string.
  std::stringstream output {};
  document.print (output, "", pugi::format_raw);
  const std::string html = output.str ();
  
  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


void bible_logic::recent_save_email (const std::string& bible,
                                     int book, int chapter,
                                     const std::string& user,
                                     const std::string& old_usfm, const std::string& new_usfm)
{
  std::vector <std::string> old_verses {};
  std::vector <std::string> new_verses {};

  // Go through all verses available in the USFM,
  // and make a record for each verse,
  // where the USFM differs between the old and the new USFM.
  const std::vector <int> verses = filter::usfm::get_verse_numbers (new_usfm);
  for (const auto verse : verses) {
    const std::string old_verse = filter::usfm::get_verse_text (old_usfm, verse);
    const std::string new_verse = filter::usfm::get_verse_text (new_usfm, verse);
    // When there's no change in the verse, skip further checks.
    if (old_verse == new_verse) continue;
    // Record the difference.
    old_verses.push_back (old_verse);
    new_verses.push_back (new_verse);
  }

  // No differences found: Done.
  if (new_verses.empty ()) return;

  // Add the passage to the subject.
  std::string subject = translate ("Check whether Bible text was saved");
  subject.append (" | " + filter_passage_display (book, chapter, ""));

  // Create the body of the email.
  pugi::xml_document document {};
  pugi::xml_node node {};
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  std::string information {};
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
  const std::string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());

  bool differences_found {false};
  for (unsigned int i = 0; i < new_verses.size(); i++) {
    Filter_Text filter_text_old = Filter_Text (bible);
    Filter_Text filter_text_new = Filter_Text (bible);
    filter_text_old.html_text_standard = new HtmlText (translate("Bible"));
    filter_text_new.html_text_standard = new HtmlText (translate("Bible"));
    filter_text_old.text_text = new Text_Text ();
    filter_text_new.text_text = new Text_Text ();
    filter_text_old.add_usfm_code (old_verses[i]);
    filter_text_new.add_usfm_code (new_verses[i]);
    filter_text_old.run (styles_logic_standard_sheet());
    filter_text_new.run (styles_logic_standard_sheet());
    const std::string old_text = filter_text_old.text_text->get ();
    const std::string new_text = filter_text_new.text_text->get ();
    if (old_text != new_text) {
      node = document.append_child ("p");
      const std::string modification = filter_diff_diff (old_text, new_text);
      const std::string fragment = /* filter::strings::convert_to_string (verse) + " " + */ modification;
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
  std:: stringstream output {};
  document.print (output, "", pugi::format_raw);
  const std::string html = output.str ();

  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


void bible_logic::optional_merge_irregularity_email (const std::string& bible, int book, int chapter,
                                                     const std::string& user,
                                                     const std::string& ancestor_usfm,
                                                     const std::string& edited_usfm,
                                                     const std::string& merged_usfm)
{
  // If the merged edited USFM is the same as the edited USFM,
  // that means that the user's changes will get saved to the chapter.
  if (edited_usfm == merged_usfm) return;
  // But if the merged edited USFM differs from the original edited USFM,
  // more checks need to be done to be sure that the user's edits made it.

  // Add the passage to the subject.
  std::string subject = translate ("Check whether Bible text was saved");
  subject.append (" | " + filter_passage_display (book, chapter, ""));

  // Create the body of the email.
  pugi::xml_document document {};
  pugi::xml_node node {};
  node = document.append_child ("h3");
  node.text ().set (subject.c_str());
  
  // Add some information for the user.
  node = document.append_child ("p");
  std::string information {};
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
  const std::string location = bible + " " + filter_passage_display (book, chapter, "") +  ".";
  node.text ().set (location.c_str ());

  bool anomalies_found {false};

  // Go through all verses available in the USFM,
  // and check the differences for each verse.
  const std::vector <int> verses = filter::usfm::get_verse_numbers (merged_usfm);
  for (const auto verse : verses) {
    const std::string ancestor_verse_usfm = filter::usfm::get_verse_text (ancestor_usfm, verse);
    const std::string edited_verse_usfm = filter::usfm::get_verse_text (edited_usfm, verse);
    const std::string merged_verse_usfm = filter::usfm::get_verse_text (merged_usfm, verse);
    // There's going to be a check to find out that all the changes the user made,
    // are available among the changes resulting from the merge.
    // If all the changes are there, all is good.
    // If not, then email the user about this.
    bool anomaly_found {false};
    std::vector <std::string> user_removals, user_additions, merged_removals, merged_additions;
    filter_diff_diff (ancestor_verse_usfm, edited_verse_usfm, &user_removals, &user_additions);
    filter_diff_diff (ancestor_verse_usfm, merged_verse_usfm, &merged_removals, &merged_additions);
    for (const auto& user_removal : user_removals) {
      if (!in_array (user_removal, merged_removals)) anomaly_found = true;
    }
    for (const auto& user_addition : user_additions) {
      if (!in_array (user_addition, merged_additions)) anomaly_found = true;
    }
    if (!anomaly_found) continue;
    Filter_Text filter_text_ancestor = Filter_Text (bible);
    Filter_Text filter_text_edited = Filter_Text (bible);
    Filter_Text filter_text_merged = Filter_Text (bible);
    filter_text_ancestor.html_text_standard = new HtmlText (translate("Bible"));
    filter_text_edited.html_text_standard = new HtmlText (translate("Bible"));
    filter_text_merged.html_text_standard = new HtmlText (translate("Bible"));
    filter_text_ancestor.text_text = new Text_Text ();
    filter_text_edited.text_text = new Text_Text ();
    filter_text_merged.text_text = new Text_Text ();
    filter_text_ancestor.add_usfm_code (ancestor_verse_usfm);
    filter_text_edited.add_usfm_code (edited_verse_usfm);
    filter_text_merged.add_usfm_code (merged_verse_usfm);
    filter_text_ancestor.run (styles_logic_standard_sheet());
    filter_text_edited.run (styles_logic_standard_sheet());
    filter_text_merged.run (styles_logic_standard_sheet());
    const std::string ancestor_text = filter_text_ancestor.text_text->get ();
    const std::string edited_text = filter_text_edited.text_text->get ();
    const std::string merged_text = filter_text_merged.text_text->get ();
    std::string modification_edited = filter_diff_diff (ancestor_text, edited_text);
    std::string modification_saved = filter_diff_diff (ancestor_text, merged_text);
    // If the edited and saved modifications are the same, do not email this.
    // This keeps changes in footnotes and cross references out from consideration.
    if (modification_saved == modification_edited) continue;
    // Add labels to the modifications.
    modification_edited.insert(0, translate ("You edited:") + " ");
    modification_saved.insert(0, translate ("Bibledit saved:") + " ");
    // Add the modifications to the email body.
    node = document.append_child ("p");
    node.append_buffer (modification_edited.c_str (), modification_edited.size ());
    node = document.append_child ("p");
    node.append_buffer (modification_saved.c_str (), modification_saved.size ());
    anomalies_found = true;
  }

  // If no differences were found, bail out.
  // This also handles differences in spacing.
  // If the differences consist of whitespace only, bail out here.
  // See issue https://github.com/bibledit/cloud/issues/413
  if (!anomalies_found) return;
  
  // Convert the document to a string.
  std::stringstream output {};
  document.print (output, "", pugi::format_raw);
  const std::string html = output.str ();

  // Schedule the mail for sending to the user.
  email_schedule (user, subject, html);
}


const char * bible_logic::insert_operator ()
{
  return "i";
}
const char * bible_logic::delete_operator ()
{
  return "d";
}
const char * bible_logic::format_paragraph_operator ()
{
  return "p";
}
const char * bible_logic::format_character_operator ()
{
  return "c";
}

// There are three containers with updating information.
// The function condenses this updating information.
// This condensed information works better for the Quill editor.
void bible_logic::condense_editor_updates (const std::vector <int>& positions_in,
                                           const std::vector <int>& sizes_in,
                                           const std::vector <bool>& additions_in,
                                           const std::vector <std::string>& content_in,
                                           std::vector <int>& positions_out,
                                           std::vector <int>& sizes_out,
                                           std::vector <std::string>& operators_out,
                                           std::vector <std::string>& content_out)
{
  positions_out.clear();
  sizes_out.clear();
  operators_out.clear();
  content_out.clear();
  
  // https://stackoverflow.com/questions/40492414/why-does-stdnumeric-limitslong-longmax-fail
  int previous_position = (std::numeric_limits<int>::min)();
  bool previous_addition {false};
  std::string previous_character {};
  for (size_t i = 0; i < positions_in.size(); i++) {
    const int position = positions_in[i];
    const int size = sizes_in[i];
    const bool addition = additions_in[i];
    const std::string character = content_in[i].substr (0, 1);
    const std::string format = content_in[i].substr (1);

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
    const bool newlineflag = addition && !previous_addition && (character == "\n") && (character == previous_character) && (position == previous_position);
    if (newlineflag) {
      // Remove the previous "delete new line".
      positions_out.pop_back();
      sizes_out.pop_back();
      operators_out.pop_back();
      content_out.pop_back();
      // Add the paragraph format operation data.
      positions_out.push_back(position);
      sizes_out.push_back(size);
      operators_out.push_back(bible_logic::format_paragraph_operator());
      content_out.push_back(format);
    } else {
      positions_out.push_back(position);
      sizes_out.push_back(size);
      if (addition) operators_out.push_back(bible_logic::insert_operator());
      else operators_out.push_back(bible_logic::delete_operator());
      content_out.push_back(character + format);
    }

    // Store data for the next iteration.
    previous_position = position;
    previous_addition = addition;
    previous_character = character;
  }

}


void bible_logic::html_to_editor_updates (const std::string& editor_html,
                                          const std::string& server_html,
                                          std::vector <int>& positions,
                                          std::vector <int>& sizes,
                                          std::vector <std::string>& operators,
                                          std::vector <std::string>& content)
{
  // Clear outputs.
  positions.clear();
  sizes.clear();
  operators.clear();
  content.clear();
  
  // Convert the html to formatted text.
  Editor_Html2Format editor_format {};
  Editor_Html2Format server_format {};
  editor_format.load (editor_html);
  server_format.load (server_html);
  editor_format.run ();
  server_format.run ();

  // Convert the formatted text fragments to formatted UTF-8 characters.
  std::vector <std::string> editor_formatted_character_content;
  for (size_t i = 0; i < editor_format.texts.size(); i++) {
    const std::string& text = editor_format.texts[i];
    const std::string& format = editor_format.formats[i];
    const size_t length = filter::strings::unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      const std::string utf8_character = filter::strings::unicode_string_substr (text, pos, 1);
      editor_formatted_character_content.push_back (utf8_character + format);
    }
  }
  std::vector <std::string> server_formatted_character_content {};
  std::vector <std::string> server_utf8_characters {};
  for (size_t i = 0; i < server_format.texts.size(); i++) {
    const std::string& text = server_format.texts[i];
    const std::string& format = server_format.formats[i];
    const size_t length = filter::strings::unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      const std::string utf8_character = filter::strings::unicode_string_substr (text, pos, 1);
      server_formatted_character_content.push_back (utf8_character + format);
      server_utf8_characters.push_back(utf8_character);
    }
  }

  // Find the differences between the two sets of content.
  std::vector <int> positions_diff {};
  std::vector <int> sizes_diff {};
  std::vector <bool> additions_diff {};
  std::vector <std::string> content_diff {};
  int new_line_diff_count {};
  filter_diff_diff_utf16 (editor_formatted_character_content, server_formatted_character_content,
                          positions_diff, sizes_diff, additions_diff, content_diff, new_line_diff_count);

  // Condense the differences a bit and render them to another format.
  bible_logic::condense_editor_updates (positions_diff, sizes_diff, additions_diff, content_diff,
                                        positions, sizes, operators, content);

  // Problem description:
  // User action: Remove the new line at the end of the current paragraph.
  // Result: The current paragraph takes the style of the next paragraph.
  // User actions: While removing notes, this goes wrong.
  // Solution:
  // If there's new line(s) added or removed, apply all paragraph styles again.
  if (new_line_diff_count) {
    int position {0};
    for (size_t i = 0; i < server_utf8_characters.size(); i++) {
      const int size = static_cast<int>(filter::strings::convert_to_u16string (server_utf8_characters[i]).length());
      if (server_utf8_characters[i] == "\n") {
        positions.push_back(position);
        sizes.push_back(size);
        operators.push_back(bible_logic::format_paragraph_operator());
        content.push_back(server_formatted_character_content[i].substr (1));
      }
      position += size;
    }
  }
}


void bible_logic::create_empty_bible (const std::string& name)
{
  Database_Logs::log (translate("Creating Bible") + " " + name);
  
  // Remove and create the empty Bible.
  Database_Bibles database_bibles {};
  database_bibles.delete_bible (name);
  database_bibles.create_bible (name);
  
  // Remove index for the Bible.
  search_logic_delete_bible (name);

  // Create books with blank verses for the OT and NT.
  std::vector <book_id> books = database::books::get_ids ();
  for (const auto book : books) {
    const book_type type = database::books::get_type (book);
    if ((type == book_type::old_testament) || (type == book_type::new_testament)) {
      std::vector <std::string> feedback {};
      book_create (name, book, -1, feedback);
    }
  }
  
  Database_Logs::log (translate("Created:") + " " + name);
}

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


#include <sendreceive/bibles.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/privileges.h>
#include <database/bibleactions.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/bibles.h>
#include <checksum/logic.h>
#include <bb/logic.h>
#include <demo/logic.h>


int sendreceive_bibles_watchdog = 0;


void send_receive_bibles_done ()
{
  sendreceive_bibles_watchdog = 0;
  config_globals_syncing_bibles = false;
}


string sendreceive_bibles_text ()
{
  return translate("Bibles") + ": ";
}


string sendreceive_bibles_sendreceive_text ()
{
  return sendreceive_bibles_text () + translate ("Send/receive");
}


string sendreceive_bibles_up_to_date_text ()
{
  return sendreceive_bibles_text () + translate ("Up to date");
}


void sendreceive_bibles ()
{
  if (sendreceive_bibles_watchdog) {
    int time = filter_date_seconds_since_epoch ();
    if (time < (sendreceive_bibles_watchdog + 900)) {
      Database_Logs::log (sendreceive_bibles_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (sendreceive_bibles_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }
  sendreceive_bibles_kick_watchdog ();
  config_globals_syncing_bibles = true;
  
  
  Database_Logs::log (sendreceive_bibles_sendreceive_text (), Filter_Roles::translator ());
  
  
  Webserver_Request request;
  Sync_Logic sync_logic = Sync_Logic (&request);
  
  
  string response = client_logic_connection_setup ("", "");
  int iresponse = convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (sendreceive_bibles_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    send_receive_bibles_done ();
    return;
  }
  
  
  // Set the correct user in the session: The sole user on the Client.
  vector <string> users = request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), Filter_Roles::translator ());
    send_receive_bibles_done ();
    return;
  }
  string user = users [0];
  request.session_logic ()->setUsername (user);
  string password = request.database_users ()->get_md5 (user);
  
  
  // The basic request to be POSTed to the server.
  // It contains the user's credentials.
  map <string, string> post;
  post ["u"] = bin2hex (user);
  post ["p"] = password;
  post ["l"] = convert_to_string (request.database_users ()->get_level (user));

  
  // Error variable.
  string error;
  
  
  // Server URL to call.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_bibles_url ());
  

  // Check on communication errors to be careful that there will be no loss of Bible data on the client.
  bool communication_errors = false;

  
  // Go through the Bibles / books / chapters that have actions recorded for them.
  Database_BibleActions database_bibleactions;
  vector <string> bibles = database_bibleactions.getBibles ();
  for (string bible : bibles) {
    vector <int> books = database_bibleactions.getBooks (bible);
    for (int book : books) {
      vector <int> chapters = database_bibleactions.getChapters (bible, book);
      for (int chapter : chapters) {
        
        sendreceive_bibles_kick_watchdog ();

        string bookname = Database_Books::getEnglishFromId (book);
        Database_Logs::log (sendreceive_bibles_text () + translate("Sending to server") + ": " + bible + " " + bookname + " " + convert_to_string (chapter), Filter_Roles::translator ());
        
        // Get old and new USFM for this chapter.
        string oldusfm = database_bibleactions.getUsfm (bible, book, chapter);
        string newusfm = request.database_bibles()->getChapter (bible, book, chapter);
        
        // Straightaway clear the Bible action for this chapter.
        // This atomic operation enables new edits from the user in this chapter to be recorded straightaway,
        // even during the time that this chapter is still being sent.
        // In the face of a slow network at times, this does occur in practise.
        // Examples have been seen.
        database_bibleactions.erase (bible, book, chapter);
        
        // If old USFM and new USFM differ, and the new USFM is not empty, send it to the server.
        if ((newusfm != oldusfm) && (newusfm != "")) {
          
          string checksum = Checksum_Logic::get (oldusfm + newusfm);
          
          // Generate a POST request.
          map <string, string> sendpost = post;
          sendpost ["a"]  = convert_to_string (Sync_Logic::bibles_send_chapter);
          sendpost ["b"]  = bible;
          sendpost ["bk"] = convert_to_string (book);
          sendpost ["c"]  = convert_to_string (chapter);
          // Safeguard the + signs that the server otherwise would remove.
          sendpost ["o"]  = filter_url_plus_to_tag (oldusfm);
          sendpost ["n"]  = filter_url_plus_to_tag (newusfm);
          sendpost ["s"]  = checksum;
          
          string error;
          response = sync_logic.post (sendpost, url, error);
          
          if (!error.empty ()) {
            
            // Communication error.
            communication_errors = true;
            Database_Logs::log (sendreceive_bibles_text () + "Failure sending chapter: " + error, Filter_Roles::translator ());
            // Restore the Bible action for this chapter.
            database_bibleactions.erase (bible, book, chapter);
            database_bibleactions.record (bible, book, chapter, oldusfm);
            
          } else if (response == checksum) {
            
            // Cloud has confirmed receipt: OK.
          
          } else {
            
            // The Cloud sends a response to the client in case of an error.
            communication_errors = true;
            Database_Logs::log (sendreceive_bibles_text () + translate ("Failure sending chapter") + ", " + translate ("Bibledit Cloud says:") + " " + response, Filter_Roles::translator ());
            // Restore the Bible action for this chapter.
            database_bibleactions.erase (bible, book, chapter);
            database_bibleactions.record (bible, book, chapter, oldusfm);
            
          }
        }
      }
    }
  }
  
  
  // Test the following:
  // * Slight edits on client: Transfer to the server.
  // * Edits on client and on server: Merge both versions.
  // * Big edits on client: Transfer to server.
  // * Empty data on server: The client version is stored on the server.
  // * Client has no access: It displays message in the journal.
  // * Merge conflict: Takes the server's copy, and the server emails the user.
  // * Network latency: Test it by adding delays here and there.
  
  
  // After successfully sending all changes to the Cloud, clear the unsent-data timeout warning data.
  if (!communication_errors) {
    Database_Config_General::setUnsentBibleDataTime (0);
  }
  
  
  // After all Bible actions have been sent to the server, and the chapters updated on the client,
  // the client will now sync its Bibles with the server's Bibles.
  // But if there was a communications error during sending a change to the server,
  // then the client should not download changes from the server,
  // because downloading them would overwrite the changes on the client.
  // The client only downloads changes from the server after it has sent all local edits successfully.
  if (communication_errors) {
    Database_Logs::log (sendreceive_bibles_text () + "Not downloading changes from the server because of communication errors", Filter_Roles::translator ());
    send_receive_bibles_done ();
    return;
  }

  
  // Whether this is the first synchronize action after the user connected to the Cloud.
  bool first_sync_after_connect = Database_Config_General::getJustConnectedToCloud ();
  
  
  // Calculate the total checksum of all chapters in all books in all local Bibles.
  // Send the credentials to the server,
  // to enable the server to calculate a similar checksum for all Bibles on the server
  // that the client username has access to via its credentials.
  // The client compares the two checksums.
  // If they match, it means everything is in sync.
  bibles = request.database_bibles()->getBibles ();
  string client_checksum = Checksum_Logic::getBibles (&request, bibles);
  post ["a"] = convert_to_string (Sync_Logic::bibles_get_total_checksum);
  string server_checksum = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting total checksum") + ": " + error, Filter_Roles::translator ());
    send_receive_bibles_done ();
    return;
  }
  if (client_checksum == server_checksum) {
    Database_Logs::log (sendreceive_bibles_up_to_date_text (), Filter_Roles::translator ());
    send_receive_bibles_done ();
    bible_logic_kick_unreceived_data_timer ();
    return;
  }
  

  // Send the user credentials to the server and request the Bibles.
  // The server responds with the Bibles this user has access to.
  // The client stores this list for later use.
  post ["a"] = convert_to_string (Sync_Logic::bibles_get_bibles);
  string server_bibles = sync_logic.post (post, url, error);
  vector <string> v_server_bibles = filter_string_explode (server_bibles, '\n');
  if (!error.empty () || v_server_bibles.empty ()) {
    Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting list of Bibles"), Filter_Roles::translator ());
    send_receive_bibles_done ();
    return;
  }
  // Verify the checksum of the list of Bibles to be sure that the data is valid.
  // This is important, because there have been cases that the list of Bible was corrupted,
  // so the client deleted all its local Bibles.
  // Checksumming prevents this behaviour.
  server_checksum = v_server_bibles [0];
  v_server_bibles.erase (v_server_bibles.begin());
  string message_checksum = Checksum_Logic::get (v_server_bibles);
  if (server_checksum != message_checksum) {
    Database_Logs::log (sendreceive_bibles_text () + translate("Checksum error while receiving list of Bibles"), Filter_Roles::translator ());
    send_receive_bibles_done ();
    bible_logic_kick_unreceived_data_timer ();
    return;
  }
  Database_Logs::log (sendreceive_bibles_text () + filter_string_implode (v_server_bibles, ", "), Filter_Roles::translator ());
  
  
  // The client now has a list of Bibles the user has access to on the server.
  bibles = request.database_bibles()->getBibles ();
  bibles = filter_string_array_diff (bibles, v_server_bibles);
  if (first_sync_after_connect) {
    // At the first synchronize action after the user connected to the Cloud,
    // any local Bible not available from the server,
    // the client schedules them for upload to the Cloud.
    // If a new client connects to the Cloud, this new client would upload the sample Bible.
    // This would be undesired behaviour. Skip it.
    for (auto bible : bibles) {
      if (bible == demo_sample_bible_name ()) continue;
      vector <int> books = request.database_bibles ()->getBooks (bible);
      for (auto book : books) {
        vector <int> chapters = request.database_bibles ()->getChapters (bible, book);
        for (auto chapter : chapters) {
          database_bibleactions.record (bible, book, chapter, "");
        }
      }
    }
  } else {
    // The client deletes any local Bible not available from the server.
    // It does not record change Bible actions for this operation.
    for (string bible : bibles) {
      request.database_bibles()->deleteBible (bible);
      Database_Privileges::removeBible (bible);
      Database_Config_Bible::remove (bible);
      Database_Logs::log (sendreceive_bibles_text () + translate("Deleting Bible because the server did not grant access to it") + ": " + bible, Filter_Roles::translator ());
    }
  }
  
  
  // The client goes through all the Bibles, and deals with each of them.
  for (string bible : v_server_bibles) {
    
    
    // Compare the checksum of the whole Bible on client and server to see if this Bible is in sync.
    string client_checksum = Checksum_Logic::getBible (&request, bible);
    post ["a"] = convert_to_string (Sync_Logic::bibles_get_bible_checksum);
    post ["b"] = bible;
    string server_checksum = sync_logic.post (post, url, error);
    if (!error.empty () || client_checksum.empty ()) {
      Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting Bible checksum") + ": " + error, Filter_Roles::translator ());
      communication_errors = true;
      continue;
    }
    if (client_checksum == server_checksum) {
      Database_Logs::log (sendreceive_bibles_text () + translate("Bible up to date") + ": " + bible, Filter_Roles::translator ());
      continue;
    }
    
    
    // Request all books in the $bible on the server.
    vector <int> client_books = request.database_bibles()->getBooks (bible);
    post ["a"] = convert_to_string (Sync_Logic::bibles_get_books);
    string server_books = sync_logic.post (post, url, error);
    if (!error.empty () || server_books.empty ()) {
      Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting books") + ": " + error, Filter_Roles::translator ());
      communication_errors = true;
      continue;
    }
    // Do checksumming on the book list to be sure the data is valid.
    // Invalid data may cause books to be added or deleted on the client.
    vector <string> v_server_books = filter_string_explode (server_books, '\n');
    server_checksum = v_server_books [0];
    v_server_books.erase (v_server_books.begin());
    string message_checksum = Checksum_Logic::get (v_server_books);
    if (server_checksum != message_checksum) {
      Database_Logs::log (sendreceive_bibles_text () + translate("Checksum error while receiving list of books from server"), Filter_Roles::translator ());
      communication_errors = true;
      continue;
    }
    vector <int> i_server_books;
    for (auto & book : v_server_books) i_server_books.push_back (convert_to_int (book));


    // Find the books on the client which are not on the server.
    client_books = filter_string_array_diff (client_books, i_server_books);
    if (first_sync_after_connect) {
      // The first sync action after connecting to the Cloud,
      // any books on the client and not on the server,
      // schedule them for upload to the Cloud.
      for (auto book : client_books) {
        vector <int> chapters = request.database_bibles ()->getChapters (bible, book);
        for (auto & chapter : chapters) {
          database_bibleactions.record (bible, book, chapter, "");
        }
      }
    } else {
      // Any books not on the server, delete them from the client as well.
      // But for more robustness while connected to a very bad network, the client will remove only one book at a time.
      if (!client_books.empty ()) {
        int book = client_books [0];
        request.database_bibles()->deleteBook (bible, book);
        string book_name = Database_Books::getEnglishFromId (book);
        Database_Logs::log (sendreceive_bibles_text () + translate("Deleting book because the server does not have it") + ": " + bible + " " + book_name , Filter_Roles::translator ());
      }
    }
    
    
    // The client goes through all the books as on the server, and deals with each of them.
    for (auto & book : i_server_books) {
      
  
      string book_name = Database_Books::getEnglishFromId (book);
      
      
      // Compare the checksum for the whole book on the client with the same on the server to see if this book is in sync.
      string client_checksum = Checksum_Logic::getBook (&request, bible, book);
      post ["a"] = convert_to_string (Sync_Logic::bibles_get_book_checksum);
      post ["bk"] = convert_to_string (book);
      string server_checksum = sync_logic.post (post, url, error);
      if (!error.empty ()) {
        Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting book checksum") + ": " + error, Filter_Roles::translator ());
        communication_errors = true;
        continue;
      }
      if (client_checksum == server_checksum) {
        continue;
      }


      // The client requests all chapters per book from the server.
      vector <int> client_chapters = request.database_bibles()->getChapters (bible, book);
      post ["a"] = convert_to_string (Sync_Logic::bibles_get_chapters);
      string server_chapters = sync_logic.post (post, url, error);
      if (!error.empty () || server_chapters.empty ()) {
        Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting list of chapters:") + " " + bible + " " + book_name, Filter_Roles::translator ());
        communication_errors = true;
        continue;
      }
      vector <string> v_server_chapters = filter_string_explode (server_chapters, '\n');
      // Do checksumming on the data to be sure the data is valid.
      // Invalid data may cause chapters to be added or deleted on the client.
      server_checksum = v_server_chapters [0];
      v_server_chapters.erase (v_server_chapters.begin());
      string message_checksum = Checksum_Logic::get (v_server_chapters);
      if (server_checksum != message_checksum) {
        Database_Logs::log (sendreceive_bibles_text () + translate("Checksum error while receiving list of chapters"), Filter_Roles::translator ());
        communication_errors = true;
        continue;
      }
      vector <int> i_server_chapters;
      for (auto & chapter : v_server_chapters) i_server_chapters.push_back (convert_to_int (chapter));
      
      
      // The client now should remove any local chapters not on the server.
      client_chapters = filter_string_array_diff (client_chapters, i_server_chapters);
      if (first_sync_after_connect) {
        // First sync after connecting to Cloud:
        // It does not delete any chapters, but rather sends them to the Cloud.
        for (auto chapter : client_chapters) {
          database_bibleactions.record (bible, book, chapter, "");
        }
      } else {
        // For better robustness, while connected to a very bad network, the client only removes one local chapter.
        // If necessary it will delete another one during next sync operation.
        if (!client_chapters.empty ()) {
          int chapter = client_chapters [0];
          request.database_bibles()->deleteChapter (bible, book, chapter);
          Database_Logs::log (sendreceive_bibles_text () + translate("Deleting chapter because the server does not have it") + ": " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
        }
      }
      
      
      // The client goes through all chapters in the book.
      // It compares the checksum of that chapter locally with the same checksum on the server.
      // If the checksums differ, then the client requests the updated chapter from the server and stores it locally.
      // As the user on the client may continue editing the Bible while the sync operations runs,
      // the edits made by the user should not be overwritten by downloading the newest chapter from the server,
      // because in this case the newest chapter is not on the server, but on the client.
      // The solution is this:
      // When the Bible actions database contains an action for this particular chapter,
      // it means that chapter is being edited,
      // and therefore the new chapter from the server should be merged with what is on the client.
      for (auto & chapter : i_server_chapters) {

        
        // Get checksum for the chapter on client and on server.
        // If both are the same, it means the USFM in both is the same, and we're done.
        string client_checksum = Checksum_Logic::getChapter (&request, bible, book, chapter);
        post ["a"] = convert_to_string (Sync_Logic::bibles_get_chapter_checksum);
        post ["c"] = convert_to_string (chapter);
        string server_checksum = sync_logic.post (post, url, error);
        if (!error.empty () || server_checksum.empty ()) {
          Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting checksum:") + " " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
          communication_errors = true;
          continue;
        }
        if (client_checksum == server_checksum) {
          continue;
        }
        
        
        sendreceive_bibles_kick_watchdog ();


        // Different checksums: Get the USFM for the chapter as it is on the server.
        Database_Logs::log (sendreceive_bibles_text () + translate("Getting chapter:") + " " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
        post ["a"] = convert_to_string (Sync_Logic::bibles_get_chapter);
        string server_usfm = sync_logic.post (post, url, error);
        if (!error.empty () || server_usfm.empty ()) {
          Database_Logs::log (sendreceive_bibles_text () + translate("Failure getting chapter:") + " " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
          communication_errors = true;
          continue;
        }

        
        // Verify the checksum of the chapter on the server, to be sure there's no corruption during transmission.
        vector <string> v_server_usfm = filter_string_explode (server_usfm, '\n');
        string checksum = v_server_usfm [0];
        v_server_usfm.erase (v_server_usfm.begin());
        server_usfm = filter_string_implode (v_server_usfm, "\n");
        if (Checksum_Logic::get (server_usfm) != checksum) {
          Database_Logs::log (sendreceive_bibles_text () + translate("Checksum error while receiving chapter from server:") + " " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
          communication_errors = true;
          continue;
        }
        

        // Check whether the user on the client has made changes in this chapter since the edits were sent to the server.
        // If there are none, then the client stores the chapter as it gets it from the server, and is done.
        string old_usfm = database_bibleactions.getUsfm (bible, book, chapter);
        if (old_usfm.empty ()) {
          request.database_bibles()->storeChapter (bible, book, chapter, server_usfm);
          continue;
        }

        
        // At this stage we're in a situation where there's changes on both the client and the server.
        // Merge them.
        // It uses a 3-way merge, taking the chapter from the bible actions as the basis,
        // and the other two (client and server) to be merged.
        // Store the merged data on the client.
        // It stores through the Bible logic so the changes get staged to be sent.
        // The changes will be sent to the server during the next synchronize action.
        vector <Merge_Conflict> conflicts;
        Database_Logs::log (sendreceive_bibles_text () + translate("Merging changes on server and client") + " " + bible + " " + book_name + " " + convert_to_string (chapter), Filter_Roles::translator ());
        string client_usfm = request.database_bibles()->getChapter (bible, book, chapter);
        string merged_usfm = filter_merge_run (old_usfm, client_usfm, server_usfm, true, conflicts);
        bible_logic_merge_irregularity_mail ( { user }, conflicts);
        bible_logic_store_chapter (bible, book, chapter, merged_usfm);
      }
    }
  }

  
  // It has completed one synchronization action here.
  // Clear the first-run flag in case there's no communication errors.
  if (!communication_errors) {
    if (first_sync_after_connect) {
      Database_Config_General::setJustConnectedToCloud (false);
    }
  }

  
  // Done.
  Database_Logs::log (sendreceive_bibles_text () + "Ready", Filter_Roles::translator ());
  bible_logic_kick_unreceived_data_timer ();
  send_receive_bibles_done ();
}


void sendreceive_bibles_kick_watchdog ()
{
  sendreceive_bibles_watchdog = filter_date_seconds_since_epoch ();
}

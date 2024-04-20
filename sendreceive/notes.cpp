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


#include <sendreceive/notes.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <filter/date.h>
#include <filter/archive.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/notes.h>
#include <database/noteactions.h>
#include <database/logs.h>
#include <database/state.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/notes.h>
#include <checksum/logic.h>
#include <bb/logic.h>
#include <notes/logic.h>


int sendreceive_notes_watchdog = 0;


std::string sendreceive_notes_text ()
{
  return translate("Notes") + ": ";
}


std::string sendreceive_notes_sendreceive_text ()
{
  return sendreceive_notes_text () + translate ("Send/receive");
}


std::string sendreceive_notes_up_to_date_text ()
{
  return sendreceive_notes_text () + translate ("Up to date");
}


void sendreceive_notes ()
{
  if (sendreceive_notes_watchdog) {
    int time = filter::date::seconds_since_epoch ();
    if (time < (sendreceive_notes_watchdog + 900)) {
      Database_Logs::log (sendreceive_notes_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (sendreceive_notes_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }

  sendreceive_notes_kick_watchdog ();
  config_globals_syncing_notes = true;
  

  bool success = sendreceive_notes_upload ();
  

  // After all note actions have been sent to the server, and the notes updated on the client,
  // the client will now sync its notes with the server's notes.
  if (success) {
    success = sendreceive_notes_download (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  }

  if (success) Database_Logs::log (sendreceive_notes_up_to_date_text (), Filter_Roles::translator ());

  sendreceive_notes_watchdog = 0;
  config_globals_syncing_notes = false;
}


// Upload all note actions to the server.
bool sendreceive_notes_upload ()
{
  Webserver_Request webserver_request;
  Sync_Logic sync_logic (webserver_request);
  Database_Notes database_notes (webserver_request);
  Database_NoteActions database_noteactions = Database_NoteActions ();
  
  
  Database_Logs::log (sendreceive_notes_sendreceive_text (), Filter_Roles::translator ());
  
  
  std::string response = client_logic_connection_setup ("", "");
  int iresponse = filter::strings::convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (sendreceive_notes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    return false;
  }
  
  
  // Set the correct user in the session: The sole user on the Client.
  std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (sendreceive_notes_text () + translate("No local user found"), Filter_Roles::translator ());
    return false;
  }
  std::string user = users [0];
  webserver_request.session_logic ()->set_username (user);
  
  
  // The basic request to be POSTed to the server.
  // It contains the user's credentials.
  std::map <std::string, std::string> post;
  post ["u"] = filter::strings::bin2hex (user);
  post ["p"] = webserver_request.database_users ()->get_md5 (user);
  post ["l"] = filter::strings::convert_to_string (webserver_request.database_users ()->get_level (user));
  
  
  // Error variable.
  std::string error;
  
  
  // Server URL to call.
  std::string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  std::string url = client_logic_url (address, port, sync_notes_url ());
  
  
  // Check on communication errors to be careful that there will be no loss of notes data on the client.
  // While sending updates for a certai note identifier,
  // make sure to take the most recent update if there's more than one in the database for that identifier.
  
  
  // Go through all notes which have actions recorded for them.
  // While sending note actions, the database method to retrieve them
  // keeps the sequence of the actions as they occurred,
  // as later updates can undo or affect earlier updates.
  std::vector <int> notes = database_noteactions.getNotes ();
  for (auto identifier : notes) {
    

    std::string summary = database_notes.get_summary (identifier);
    if (summary.empty ()) summary = "<deleted>";
    Database_Logs::log (sendreceive_notes_text () + translate("Sending note to server") + ": " + summary, Filter_Roles::translator ());
    
    
    // Get all the actions for the current note.
    std::vector <Database_Note_Action> note_actions = database_noteactions.getNoteData (identifier);
    
    
    // Due to some updates sent out here, record final actions to get the updated note from the server.
    std::map <int, bool> final_get_actions;
    
    
    // Deal with the note actions for this note.
    for (auto note_action : note_actions) {
      
      int rowid = note_action.rowid;
      std::string username = note_action.username;
      //int timestamp = note_action.timestamp;
      int action = note_action.action;
      std::string content = note_action.content;
      
      
      // Generate a POST request.
      post ["i"] = filter::strings::convert_to_string (identifier);
      post ["a"] = filter::strings::convert_to_string (action);
      switch (action) {
        case Sync_Logic::notes_put_create_initiate: break;
        case Sync_Logic::notes_put_create_complete: break;
        case Sync_Logic::notes_put_summary:
        {
          content = database_notes.get_summary (identifier);
          break;
        }
        case Sync_Logic::notes_put_contents: break;
        case Sync_Logic::notes_put_comment: break;
        case Sync_Logic::notes_put_subscribe: break;
        case Sync_Logic::notes_put_unsubscribe: break;
        case Sync_Logic::notes_put_assign: break;
        case Sync_Logic::notes_put_unassign: break;
        case Sync_Logic::notes_put_status:
        {
          content = database_notes.get_raw_status (identifier);
          break;
        }
        case Sync_Logic::notes_put_passages:
        {
          content = database_notes.decode_passage (identifier);
          break;
        }
        case Sync_Logic::notes_put_severity:
        {
          content = filter::strings::convert_to_string (database_notes.get_raw_severity (identifier));
          break;
        }
        case Sync_Logic::notes_put_bible:
        {
          content = database_notes.get_bible (identifier);
          break;
        }
        case Sync_Logic::notes_put_mark_delete: break;
        case Sync_Logic::notes_put_unmark_delete: break;
        case Sync_Logic::notes_put_delete: break;
        default: break;
      }
      post ["c"] = content;
      
      
      // Send the request off and receive the response.
      sendreceive_notes_kick_watchdog ();
      response = sync_logic.post (post, url, error);
      if (!error.empty ()) {
        Database_Logs::log (sendreceive_notes_text () + "Failure sending note: " + error, Filter_Roles::translator ());
        return false;
      }
      
      
      // Delete this note action because it has been dealt with.
      database_noteactions.erase (rowid);
      
      
      // Add final note actions depending on what was updated.
      switch (action) {
        case Sync_Logic::notes_put_create_initiate: break;
        case Sync_Logic::notes_put_create_complete:
        {
          int i = Sync_Logic::notes_get_contents;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_subscribers;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_assignees;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_modified;
          final_get_actions [i] = true;
          break;
        }
        case Sync_Logic::notes_put_summary: break;
        case Sync_Logic::notes_put_contents: break;
        case Sync_Logic::notes_put_comment:
        {
          int i = Sync_Logic::notes_get_contents;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_subscribers;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_assignees;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_modified;
          final_get_actions [i] = true;
          break;
        }
        case Sync_Logic::notes_put_subscribe: break;
        case Sync_Logic::notes_put_unsubscribe: break;
        case Sync_Logic::notes_put_assign: break;
        case Sync_Logic::notes_put_unassign: break;
        case Sync_Logic::notes_put_status: break;
        case Sync_Logic::notes_put_passages: break;
        case Sync_Logic::notes_put_severity: break;
        case Sync_Logic::notes_put_bible: break;
        case Sync_Logic::notes_put_mark_delete: break;
        case Sync_Logic::notes_put_unmark_delete: break;
        case Sync_Logic::notes_put_delete: break;
        default: break;
      }
    }
    
    
    // Deal with the extra, added, note actions.
    for (int action = Sync_Logic::notes_get_total; action <= Sync_Logic::notes_get_modified; action++) {
      std::map <std::string, std::string> post2;
      post2 ["u"] = filter::strings::bin2hex (user);
      post2 ["i"] = filter::strings::convert_to_string (identifier);
      post2 ["a"] = filter::strings::convert_to_string (action);
      sendreceive_notes_kick_watchdog ();
      response = sync_logic.post (post2, url, error);
      if (error.empty ()) {
        if (action == Sync_Logic::notes_get_contents) {
          if (response != database_notes.get_contents (identifier)) {
            database_notes.set_contents (identifier, response);
          }
        }
        if (action == Sync_Logic::notes_get_subscribers) {
          std::vector <std::string> subscribers = filter::strings::explode (response, '\n');
          database_notes.set_subscribers (identifier, subscribers);
        }
        if (action == Sync_Logic::notes_get_assignees) {
          std::vector <std::string> assignees = filter::strings::explode (response, '\n');
          database_notes.set_assignees (identifier, assignees);
        }
        if (action == Sync_Logic::notes_get_modified) {
          database_notes.set_modified (identifier, filter::strings::convert_to_int (response));
        }
      }
    }
  }
  
  return true;
}


// The function is called with the first and last note identifier to deal with.
bool sendreceive_notes_download (int lowId, int highId)
{
  Webserver_Request webserver_request;
  Database_Notes database_notes (webserver_request);
  Database_NoteActions database_noteactions = Database_NoteActions ();
  Sync_Logic sync_logic (webserver_request);
  
  
  std::string response = client_logic_connection_setup ("", "");
  int iresponse = filter::strings::convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (sendreceive_notes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    return false;
  }
  
  
  // The client is supposed to have one user.
  // Get this username, plus the md5 hash of the password,
  // in preparation for sending it to the server in in the script below.
  // The server will use this user to find out the Bibles this user has access to,
  // so the server can select the correct notes for this user.
  // The client selects all available notes on the system.
  std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (sendreceive_notes_text () + translate("No local user found"), Filter_Roles::translator ());
    return false;
  }
  std::string user = users [0];
  webserver_request.session_logic ()->set_username (user);
  std::string password = webserver_request.database_users ()->get_md5 (user);

  
  // Check for the health of the notes databases and take action if needed.
  bool healthy = true;
  if (!database_notes.healthy ()) healthy = false;
  if (!database_notes.checksums_healthy ()) healthy = false;
  if (!healthy) {
    Database_Logs::log (sendreceive_notes_text () + "Abort receive just now because of database problems", Filter_Roles::translator ());
    return false;
  }
  
  
  // Server URL to call.
  std::string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  std::string url = client_logic_url (address, port, sync_notes_url ());
  
  
  // The basic request to be POSTed to the server.
  std::map <std::string, std::string> post;
  post ["u"] = filter::strings::bin2hex (user);
  post ["l"] = filter::strings::convert_to_string (lowId);
  post ["h"] = filter::strings::convert_to_string (highId);

  
  // Error variable.
  std::string error;
  
  
  // The script requests the total note count from the server, and their total checksum.
  // It compares this with the local notes.
  // If it matches, that means that the local notes match the notes on the server.
  // The script is then ready.
  post ["a"] = filter::strings::convert_to_string (Sync_Logic::notes_get_total);
  // Response comes after a relatively long delay: Enable burst mode timing.
  sendreceive_notes_kick_watchdog ();
  response = sync_logic.post (post, url, error, true);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_notes_text () + "Failure requesting totals: " + error, Filter_Roles::translator ());
    return false;
  }
  std::vector <std::string> vresponse = filter::strings::explode (response, '\n');
  int server_total = 0;
  if (vresponse.size () >= 1) server_total = filter::strings::convert_to_int (vresponse [0]);
  std::string server_checksum;
  if (vresponse.size () >= 2) server_checksum = vresponse [1];
  std::vector <int> identifiers = database_notes.get_notes_in_range_for_bibles (lowId, highId, {}, true);
  int client_total = static_cast<int>(identifiers.size());
  // Checksum cache to speed things up in case of thousands of notes.
  std::string client_checksum = Database_State::getNotesChecksum (lowId, highId);
  if (client_checksum.empty ()) {
    client_checksum = database_notes.get_multiple_checksum (identifiers);
    Database_State::putNotesChecksum (lowId, highId, client_checksum);
  }
  if (server_total == client_total) {
    if (server_checksum == client_checksum) {
      return true;
    }
  }
  
  
  // At this stage the total note count and/or their checksum on the client differs
  // from the total note count and/or their checksum on the server.
  // This applies to a certain range of notes for certain Bibles.


  // We know the total number of notes.
  // If the total note count is too high, divide the range of notes into smaller ranges,
  // and then deal with each range.
  if (server_total > 20) {
     std::vector <Sync_Logic_Range> ranges = sync_logic.create_range (lowId, highId);
    for (auto range : ranges) {
      sendreceive_notes_download (range.low, range.high);
    }
    return true;
  }
  
  
  // At this stage the total note count is small enough that the function can
  // start to deal with the individual notes.
  // It requests the note identifiers, their checksums,
  // and compares them with the local notes.
  // If needed, it downloads the notes from the server.
  // If needed, the client removes local notes no longer available on the server.
  
  
  // Get note identifiers and checksums as on the server.
  post ["a"] = filter::strings::convert_to_string (Sync_Logic::notes_get_identifiers);
  sendreceive_notes_kick_watchdog ();
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_notes_text () + "Failure requesting identifiers: " + error, Filter_Roles::translator ());
    return false;
  }
  std::vector <int> server_identifiers;
  std::vector <std::string> server_checksums;
  vresponse = filter::strings::explode (response, '\n');
  for (size_t i = 0; i < vresponse.size (); i++) {
    if (i % 2 == 0) {
      int identifier = filter::strings::convert_to_int (vresponse [i]);
      if (identifier > 0) server_identifiers.push_back (identifier);
    }
    else server_checksums.push_back (vresponse [i]);
  }
  

  // Get note identifiers as locally on the client.
  std::vector <int> client_identifiers = database_notes.get_notes_in_range_for_bibles (lowId, highId, {}, true);
  
  
  // The client deletes notes no longer on the server.
  // But it skips the notes that have actions recorded for them,
  // as these notes are scheduled to be sent to the server first.
  identifiers = filter::strings::array_diff (client_identifiers, server_identifiers);
  int delete_counter = 0;
  for (auto identifier : identifiers) {
    if (database_noteactions.exists (identifier)) continue;
    // It has been seen that a client started to delete all notes, thousands of them, for an unknown reason.
    // The reason may have been a miscommunication between client and server.
    // And then, next send/receive, it started to re-download all thousands of them from the server.
    // Therefore limit the number of notes a client can delete in one go.
    delete_counter++;
    if (delete_counter > 15) continue;
    std::string summary = database_notes.get_summary (identifier);
    database_notes.erase (identifier);
    Database_Logs::log (sendreceive_notes_text () + "Deleting because it is not on the server: " + summary, Filter_Roles::translator ());
  }
  

  // Check whether the local notes on the client match the ones on the server.
  // If needed, download the note from the server.
  std::vector <std::string> identifiers_bulk_download;
  for (size_t i = 0; i < server_identifiers.size (); i++) {

    if (i >= server_checksums.size ()) continue;
    
    int identifier = server_identifiers [i];
    
    // Skip note if it is still to be sent off to the server.
    if (database_noteactions.exists (identifier)) continue;
    
    std::string server_checksum_note = server_checksums [i];
    std::string client_checksum_note = database_notes.get_checksum (identifier);
    if (client_checksum_note == server_checksum_note) continue;
    
    // Store the identifier to be downloaded as part of a bulk download.
    identifiers_bulk_download.push_back (filter::strings::convert_to_string (identifier));
  }
  
  // Download the notes in bulk from the Cloud, for faster transfer.
  if (!identifiers_bulk_download.empty ()) {
    sendreceive_notes_kick_watchdog ();
    if (identifiers_bulk_download.size () >= 3) {
      Database_Logs::log (sendreceive_notes_text () + "Receiving multiple notes: " + filter::strings::convert_to_string (identifiers_bulk_download.size ()), Filter_Roles::manager ());
    }
    // Request the JSON from the Cloud: It will contain the requested notes.
    post.clear ();
    post ["a"] = filter::strings::convert_to_string (Sync_Logic::notes_get_bulk);
    std::string bulk_identifiers = filter::strings::implode (identifiers_bulk_download, "\n");
    post ["b"] = bulk_identifiers;
    std::string json = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (sendreceive_notes_text () + "Failure requesting multiple notes: " + error, Filter_Roles::consultant ());
      return false;
    }
    // Store the notes in the file system.
    std::vector <std::string> summaries = database_notes.set_bulk (json);
    // More specific feedback in case it downloaded only a few notes, rather than notes in bulk.
    if (identifiers_bulk_download.size () < 3) {
      for (auto & summary : summaries) {
        Database_Logs::log (sendreceive_notes_text () + "Receiving: " + summary, Filter_Roles::manager ());
      }
    }
  }
  
  return true;
}


void sendreceive_notes_kick_watchdog ()
{
  sendreceive_notes_watchdog = filter::date::seconds_since_epoch ();
}

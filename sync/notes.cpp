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


#include <sync/notes.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <filter/archive.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/notes.h>
#include <database/modifications.h>
#include <database/state.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <notes/logic.h>


std::string sync_notes_url ()
{
  return "sync/notes";
}


std::string sync_notes (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);

  
  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }

  
  // Bail out if the notes databases are not available or not in good shape.
  bool available = true;
  if (!database_notes.healthy ()) available = false;
  if (!database_notes.checksums_healthy ()) available = false;
  if (!database_notes.available ()) available = false;
  if (!available) {
    webserver_request.response_code = 503;
    return std::string();
  }


  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();

  
  // What action does the client request from us?
  int action = filter::strings::convert_to_int (webserver_request.post_get("a"));

  
  // Check on the credentials when the clients sends data to the server to be stored there.
  if ((action >= Sync_Logic::notes_put_create_initiate) && (action != Sync_Logic::notes_get_bulk)) {
    if (!sync_logic.credentials_okay ()) return std::string();
  }


  // Check on username only, without password or level.
  std::string user = filter::strings::hex2bin (webserver_request.post_get("u"));
  if ((action == Sync_Logic::notes_get_total) || (action == Sync_Logic::notes_get_identifiers)) {
    if (!webserver_request.database_users ()->usernameExists (user)) {
      Database_Logs::log ("A client passes a non-existing user " + user, roles::manager);
      return std::string();
    }
  }
  webserver_request.session_logic ()->set_username (user);
  
  
  // Note lower and upper limits.
  int lowId = filter::strings::convert_to_int (webserver_request.post_get("l"));
  int highId = filter::strings::convert_to_int (webserver_request.post_get("h"));

  
  int identifier = filter::strings::convert_to_int (webserver_request.post_get("i"));
  std::string content = webserver_request.post_get("c");
  
  switch (action) {
    case Sync_Logic::notes_get_total:
    {
      std::vector <std::string> bibles = access_bible::bibles (webserver_request, user);
      std::vector <int> identifiers = database_notes.get_notes_in_range_for_bibles (lowId, highId, bibles, false);
      // Checksum cache to speed things up in case of thousands of notes.
      // Else the server would run at 100% CPU usage for some time to get the total checksums of notes.
      std::string checksum = Database_State::getNotesChecksum (lowId, highId);
      if (checksum.empty ()) {
        checksum = database_notes.get_multiple_checksum (identifiers);
        Database_State::putNotesChecksum (lowId, highId, checksum);
      }
      std::string response = std::to_string (identifiers.size ()) + "\n" + checksum;
      return response;
    }
    case Sync_Logic::notes_get_identifiers:
    {
      std::vector <std::string> bibles = access_bible::bibles (webserver_request, user);
      std::vector <int> identifiers = database_notes.get_notes_in_range_for_bibles (lowId, highId, bibles, false);
      std::string response;
      for (auto id : identifiers) {
        if (!response.empty ()) response.append ("\n");
        response.append (std::to_string (id));
        response.append ("\n");
        response.append (database_notes.get_checksum (id));
      }
      return response;
    }
    case Sync_Logic::notes_get_summary:
    {
      // Update search and checksum when the client requests the summary of a note,
      // because this is the first thing a client does when it requests a full note.
      // The client requests the notes in bits and pieces.
      database_notes.update_search_fields (identifier);
      database_notes.update_checksum (identifier);
      // Return summary.
      std::string summary = database_notes.get_summary (identifier);
      return summary;
    }
    case Sync_Logic::notes_get_contents:
    {
      return database_notes.get_contents (identifier);
    }
    case Sync_Logic::notes_get_subscribers:
    {
      std::vector <std::string> subscribers = database_notes.get_subscribers (identifier);
      return filter::strings::implode (subscribers, "\n");
    }
    case Sync_Logic::notes_get_assignees:
    {
      std::vector <std::string> assignees = database_notes.get_assignees (identifier);
      return filter::strings::implode (assignees, "\n");
    }
    case Sync_Logic::notes_get_status:
    {
      return database_notes.get_raw_status (identifier);
    }
    case Sync_Logic::notes_get_passages:
    {
      // Send the raw passage contents to the client, see the client code for the reason why.
      return database_notes.decode_passage (identifier);
    }
    case Sync_Logic::notes_get_severity:
    {
      return std::to_string (database_notes.get_raw_severity (identifier));
    }
    case Sync_Logic::notes_get_bible:
    {
      return database_notes.get_bible (identifier);
    }
    case Sync_Logic::notes_get_modified:
    {
      return std::to_string (database_notes.get_modified (identifier));
    }
    case Sync_Logic::notes_put_create_initiate:
    {
      // Create the note on the server.
      Database_Notes::NewNote new_note {
        .bible = "",
        .book = 1,
        .chapter = 1,
        .verse = 1,
        .summary = "<empty>",
        .contents = "<empty>",
      };
      int server_id = database_notes.store_new_note (new_note);
      // Update the note identifier on the server to be same as on the client.
      database_notes.set_identifier (server_id, identifier);
      // Update search field.
      database_notes.update_search_fields (identifier);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_create_complete:
    {
      // Do notifications.
      notes_logic.handlerNewNote (identifier);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_summary:
    {
      // Set the summary of the note on the server.
      notes_logic.set_summary (identifier, content);
      // Update search field.
      database_notes.update_search_fields (identifier);
      // Info.
      Database_Logs::log ("Client created or updated a note on the server: " + content, roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_contents:
    {
      // Set the note's contents on the server.
      database_notes.set_contents (identifier, content);
      // Update search field.
      database_notes.update_search_fields (identifier);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_comment:
    {
      // Add the comment to the note on the server.
      notes_logic.addComment (identifier, content);
      // Update search field.
      database_notes.update_search_fields (identifier);
      // Info.
      Database_Logs::log ("Client added comment to note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Notifications.
      notes_logic.handlerAddComment (identifier);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_subscribe:
    {
      // Subscribe to the note on the server.
      database_notes.subscribe_user (identifier, user);
      // Info.
      Database_Logs::log ("Client subscribed to note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_unsubscribe:
    {
      // Unsubscribe from the note on the server.
      database_notes.unsubscribe_user (identifier, user);
      // Info.
      Database_Logs::log ("Client unsubscribed from note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_assign:
    {
      // Assign user to the note on the server.
      notes_logic.assignUser (identifier, content);
      // Info
      Database_Logs::log ("Client assigned the note to a user on server: " + database_notes.get_summary (identifier), roles::manager);
      // Notifications.
      notes_logic.handlerAssignNote (identifier, content);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_unassign:
    {
      // Unassign the user from the note on the server.
      notes_logic.unassignUser (identifier, content);
      // Info.
      Database_Logs::log ("Client unassigned a user from the note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_status:
    {
      // Set the status for a note on the server.
      notes_logic.setStatus (identifier, content);
      // Info.
      Database_Logs::log ("Client set the note status on server: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_passages:
    {
      // Set the passage(s) for a note on the server.
      database_notes.set_raw_passage (identifier, content);
      database_notes.index_raw_passage (identifier, content);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_severity:
    {
      // Set the severity for a note on the server.
      notes_logic.setRawSeverity (identifier, filter::strings::convert_to_int (content));
      // Info
      Database_Logs::log ("Client set the severity for a note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_bible:
    {
      // Set the Bible for a note on the server.
      notes_logic.setBible (identifier, content);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_mark_delete:
    {
      // Mark note on server for deletion.
      notes_logic.markForDeletion (identifier);
      // Info.
      Database_Logs::log ("Client marked a note on server for deletion: " + database_notes.get_summary (identifier), roles::manager);
      // Notifications.
      notes_logic.handlerMarkNoteForDeletion (identifier);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_unmark_delete:
    {
      // Unmark note on server for deletion.
      notes_logic.unmarkForDeletion (identifier);
      // Info.
      Database_Logs::log ("Client unmarked a note on server for deletion: " + database_notes.get_summary (identifier), roles::manager);
      // Done.
      return std::string();
    }
    case Sync_Logic::notes_put_delete:
    {
      // Info to be given before the note is deleted, else the info is lost.
      Database_Logs::log ("Client deleted a note on server: " + database_notes.get_summary (identifier), roles::manager);
      // Notifications.
      notes_logic.handlerDeleteNote (identifier);
      // Delete note on server.
      notes_logic.erase (identifier);
      // Done.
      return std::string();
    }
    // This method of bulk download was implemented as of September 2016.
    // After a year or so, the logic for the replaced download methods can probably be removed from the Cloud.
    case Sync_Logic::notes_get_bulk:
    {
      // Get the note identifiers the client requests.
      std::vector <std::string> notes = filter::strings::explode (webserver_request.post_get("b"), '\n');
      std::vector <int> identifiers;
      for (auto note : notes) identifiers.push_back (filter::strings::convert_to_int (note));
      // Return the JSON that contains all the requested notes.
      std::string json = database_notes.get_bulk (identifiers);
      return json;
    }
    default: {};
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  std::this_thread::sleep_for (std::chrono::seconds (1));
  webserver_request.response_code = 400;
  return std::string();
}

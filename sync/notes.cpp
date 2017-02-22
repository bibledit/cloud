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
#include <bible/logic.h>
#include <notes/logic.h>


string sync_notes_url ()
{
  return "sync/notes";
}


string sync_notes (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Sync_Logic sync_logic = Sync_Logic (webserver_request);
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);

  
  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }

  
  // Bail out if the notes databases are not available or in good shape.
  bool available = true;
  if (!database_notes.healthy ()) available = false;
  if (!database_notes.checksums_healthy ()) available = false;
  if (!database_notes.available ()) available = false;
  if (!available) {
    request->response_code = 503;
    return "";
  }


  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();

  
  // What action does the client request from us?
  int action = convert_to_int (request->post ["a"]);

  
  // Check on the credentials when the clients sends data to the server to be stored there.
  if ((action >= Sync_Logic::notes_put_create_initiate) && (action != Sync_Logic::notes_get_bulk)) {
    if (!sync_logic.credentials_okay ()) return "";
  }


  // Check on username only, without password or level.
  string user = hex2bin (request->post ["u"]);
  if ((action == Sync_Logic::notes_get_total) || (action == Sync_Logic::notes_get_identifiers)) {
    if (!request->database_users ()->usernameExists (user)) {
      Database_Logs::log ("A client passes a non-existing user " + user, Filter_Roles::manager ());
      return "";
    }
  }
  request->session_logic ()->setUsername (user);
  
  
  // Note lower and upper limits.
  int lowId = convert_to_int (request->post ["l"]);
  int highId = convert_to_int (request->post ["h"]);

  
  int identifier = convert_to_int (request->post ["i"]);
  string content = request->post ["c"];
  
  switch (action) {
    case Sync_Logic::notes_get_total:
    {
      vector <string> bibles = access_bible_bibles (webserver_request, user);
      vector <int> identifiers = database_notes.getNotesInRangeForBibles (lowId, highId, bibles, false);
      // Checksum cache to speed things up in case of thousands of notes.
      // Else the server would run at 100% cpu for some time to get the total checksums of notes.
      string checksum = Database_State::getNotesChecksum (lowId, highId);
      if (checksum.empty ()) {
        checksum = database_notes.getMultipleChecksum (identifiers);
        Database_State::putNotesChecksum (lowId, highId, checksum);
      }
      string response = convert_to_string (identifiers.size ()) + "\n" + checksum;
      return response;
    }
    case Sync_Logic::notes_get_identifiers:
    {
      vector <string> bibles = access_bible_bibles (webserver_request, user);
      vector <int> identifiers = database_notes.getNotesInRangeForBibles (lowId, highId, bibles, false);
      string response;
      for (auto identifier : identifiers) {
        if (!response.empty ()) response.append ("\n");
        response.append (convert_to_string (identifier));
        response.append ("\n");
        response.append (database_notes.getChecksum (identifier));
      }
      return response;
    }
    case Sync_Logic::notes_get_summary:
    {
      // Update search and checksum when the client requests the summary of a note,
      // because this is the first thing a client does when it requests a full note.
      // The client requests the notes in bits and pieces.
      database_notes.updateSearchFields (identifier);
      database_notes.updateChecksum (identifier);
      // Return summary.
      string summary = database_notes.getSummary (identifier);
      return summary;
    }
    case Sync_Logic::notes_get_contents:
    {
      return database_notes.getContents (identifier);
    }
    case Sync_Logic::notes_get_subscribers:
    {
      vector <string> subscribers = database_notes.getSubscribers (identifier);
      return filter_string_implode (subscribers, "\n");
    }
    case Sync_Logic::notes_get_assignees:
    {
      vector <string> assignees = database_notes.getAssignees (identifier);
      return filter_string_implode (assignees, "\n");
    }
    case Sync_Logic::notes_get_status:
    {
      return database_notes.getRawStatus (identifier);
    }
    case Sync_Logic::notes_get_passages:
    {
      // Send the raw passage contents to the client, see the client code for the reason why.
      return database_notes.getRawPassage (identifier);
    }
    case Sync_Logic::notes_get_severity:
    {
      return convert_to_string (database_notes.getRawSeverity (identifier));
    }
    case Sync_Logic::notes_get_bible:
    {
      return database_notes.getBible (identifier);
    }
    case Sync_Logic::notes_get_modified:
    {
      return convert_to_string (database_notes.getModified (identifier));
    }
    case Sync_Logic::notes_put_create_initiate:
    {
      // Create the note on the server.
      int server_id = database_notes.storeNewNote ("", 1, 1, 1, "<empty>", "<empty>", false);
      // Update the note identifier on the server to be same as on the client.
      database_notes.setIdentifier (server_id, identifier);
      // Update search field.
      database_notes.updateSearchFields (identifier);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_create_complete:
    {
      // Do notifications.
      notes_logic.handlerNewNote (identifier);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_summary:
    {
      // Set the summary of the note on the server.
      notes_logic.setSummary (identifier, content);
      // Update search field.
      database_notes.updateSearchFields (identifier);
      // Info.
      Database_Logs::log ("Client created or updated a note on the server: " + content, Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_contents:
    {
      // Set the note's contents on the server.
      database_notes.setContents (identifier, content);
      // Update search field.
      database_notes.updateSearchFields (identifier);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_comment:
    {
      // Add the comment to the note on the server.
      notes_logic.addComment (identifier, content);
      // Update search field.
      database_notes.updateSearchFields (identifier);
      // Info.
      Database_Logs::log ("Client added comment to note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Notifications.
      notes_logic.handlerAddComment (identifier);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_subscribe:
    {
      // Subscribe to the note on the server.
      database_notes.subscribeUser (identifier, user);
      // Info.
      Database_Logs::log ("Client subscribed to note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_unsubscribe:
    {
      // Unsubscribe from the note on the server.
      database_notes.unsubscribeUser (identifier, user);
      // Info.
      Database_Logs::log ("Client unsubscribed from note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_assign:
    {
      // Assign user to the note on the server.
      notes_logic.assignUser (identifier, content);
      // Info
      Database_Logs::log ("Client assigned the note to a user on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Notifications.
      notes_logic.handlerAssignNote (identifier, content);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_unassign:
    {
      // Unassign the user from the note on the server.
      notes_logic.unassignUser (identifier, content);
      // Info.
      Database_Logs::log ("Client unassigned a user from the note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_status:
    {
      // Set the status for a note on the server.
      notes_logic.setStatus (identifier, content);
      // Info.
      Database_Logs::log ("Client set the note status on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_passages:
    {
      // Set the passage(s) for a note on the server.
      database_notes.setRawPassage (identifier, content);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_severity:
    {
      // Set the severity for a note on the server.
      notes_logic.setRawSeverity (identifier, convert_to_int (content));
      // Info
      Database_Logs::log ("Client set the severity for a note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_bible:
    {
      // Set the Bible for a note on the server.
      notes_logic.setBible (identifier, content);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_mark_delete:
    {
      // Mark note on server for deletion.
      notes_logic.markForDeletion (identifier);
      // Info.
      Database_Logs::log ("Client marked a note on server for deletion: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Notifications.
      notes_logic.handlerMarkNoteForDeletion (identifier);
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_unmark_delete:
    {
      // Unmark note on server for deletion.
      notes_logic.unmarkForDeletion (identifier);
      // Info.
      Database_Logs::log ("Client unmarked a note on server for deletion: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Done.
      return "";
    }
    case Sync_Logic::notes_put_delete:
    {
      // Info to be given before the note is deleted, else the info is lost.
      Database_Logs::log ("Client deleted a note on server: " + database_notes.getSummary (identifier), Filter_Roles::manager ());
      // Notifications.
      notes_logic.handlerDeleteNote (identifier);
      // Delete note on server.
      notes_logic.erase (identifier);
      // Done.
      return "";
    }
    // This method of bulk download was implemented as of September 2016.
    // After a year or so, the logic for the replaced download methods can probably be removed from the Cloud.
    case Sync_Logic::notes_get_bulk:
    {
      // Get the note identifiers the client requests.
      vector <string> notes = filter_string_explode (request->post ["b"], '\n');
      vector <int> identifiers;
      for (auto note : notes) identifiers.push_back (convert_to_int (note));
      // Return the JSON that contains all the requested notes.
      string json = database_notes.getBulk (identifiers);
      return json;
    }
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}

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


#include <database/ipc.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <webserver/request.h>


// Database resilience: Stored in plain file system.


Database_Ipc_Message::Database_Ipc_Message ()
{
  // Empty message has an identifier of 0.
  id = 0;
}


Database_Ipc::Database_Ipc (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


void Database_Ipc::trim ()
{
  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    if (record.user == "") {
      deleteMessage (record.rowid);
    }
  }
}


void Database_Ipc::storeMessage (string user, string channel, string command, string message)
{
  // Load entire database into memory.
  vector <Database_Ipc_Item> data = readData ();

  // Gather information about records to delete.
  vector <int> deletes;
  if (channel == "") {
    for (auto & record : data) {
      if ((record.user == user) && (record.channel == channel) && (record.command == command)) {
        deletes.push_back (record.rowid);
      }
    }
  }
  
  // Write new information.
  int rowid = getNextId (data);
  writeRecord (rowid, user, channel, command, message);
  
  // Actually delete the records.
  // Do this after writing new data, to be sure there is always some data on disk.
  for (auto& erase : deletes) {
    deleteMessage (erase);
  }
}


// Retrieves a message if there is any.
// Returns an object with the data.
// The rowid is 0 if there was nothing,
// Else the object's properties are set properly.
Database_Ipc_Message Database_Ipc::retrieveMessage (int id, string user, string channel, string command)
{
  int highestId = 0;
  string hitChannel = "";
  string hitCommand = "";
  string hitMessage = "";
  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    // Selection condition 1: The database record has a message identifier younger than the calling identifier.
    int recordid = record.rowid;
    if (recordid > id) {
      // Selection condition 2: Channel matches calling channel, or empty channel.
      string recordchannel = record.channel;
      if ((recordchannel == channel) || (recordchannel == "")) {
        // Selection condition 3: Record user matches calling user, or empty user.
        string recorduser = record.user;
        if ((recorduser == user) || (recorduser == "")) {
          // Selection condition 4: Matching command.
          string recordcommand = record.command;
          if (recordcommand == command) {
            if (recordid > highestId) {
              highestId = recordid;
              hitChannel = recordchannel;
              hitCommand = recordcommand;
              hitMessage = filter_url_file_get_contents (file (record.file));
            }
          }
        }
      }
    }
  }
  Database_Ipc_Message message = Database_Ipc_Message ();
  if (highestId) {
    message.id = highestId;
    message.channel = hitChannel;
    message.command = hitCommand;
    message.message = hitMessage;
  }
  return message;
}


void Database_Ipc::deleteMessage (int id)
{
  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    if (record.rowid == id) {
      filter_url_unlink (file (record.file));
    }
  }
}


string Database_Ipc::getFocus ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic ()->currentUser ();

  int highestId = 0;
  string hitMessage = "";
  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    int recordid = record.rowid;
    // Conditions: Command is "focus", and matching user.
    if (record.command == "focus") {
      if (record.user == user) {
        if (recordid > highestId) {
          highestId = recordid;
          hitMessage = filter_url_file_get_contents (file (record.file));
        }
      }
    }
  }

  if (highestId) return hitMessage;

  // No focus found: Return Genesis 1:1.
  return "1.1.1";
}


Database_Ipc_Message Database_Ipc::getNote ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic ()->currentUser ();

  int highestId = 0;
  string hitMessage = "";

  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    int recordid = record.rowid;
    // Conditions: Command is "opennote", and matching user.
    if (record.command == "opennote") {
      if (record.user == user) {
        if (recordid > highestId) {
          highestId = recordid;
          hitMessage = filter_url_file_get_contents (file (record.file));
        }
      }
    }
  }

  Database_Ipc_Message note = Database_Ipc_Message ();
  if (highestId) {
    note.id = highestId;
    note.message = hitMessage;
  }

  return note;
}


bool Database_Ipc::getNotesAlive ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic ()->currentUser ();

  int highestId = 0;
  string hitMessage = "";

  vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    int recordid = record.rowid;
    // Conditions: Command is "notesalive", and matching user.
    if (record.command == "notesalive") {
      if (record.user == user) {
        if (recordid > highestId) {
          highestId = recordid;
          hitMessage = filter_url_file_get_contents (file (record.file));
        }
      }
    }
  }

  if (highestId) return convert_to_bool (hitMessage);

  return false;
}


string Database_Ipc::folder ()
{
  return filter_url_create_root_path ("databases", "ipc");
}


string Database_Ipc::file (string file)
{
  return filter_url_create_path (folder (), file);
}


// Reads most fields of all data in this database.
// Returns an array containing the above.
// The fields are those that can be obtained from the name of the files.
// One file is one entry in the database.
// The filename looks like this: rowid__user__channel__command
vector <Database_Ipc_Item> Database_Ipc::readData ()
{
  vector <Database_Ipc_Item> data;
  vector <string> files = filter_url_scandir (folder ());
  for (string file : files) {
    vector <string> explosion = filter_string_explode (file, '_');
    if (explosion.size () == 7) {
      Database_Ipc_Item item = Database_Ipc_Item ();
      item.file = file;
      item.rowid = convert_to_int (explosion [0]);
      item.user = explosion [2];
      item.channel = explosion [4];
      item.command = explosion [6];
      data.push_back (item);
    }
  }
  return data;
}
  

void Database_Ipc::writeRecord (int rowid, string user, string channel, string command, string message)
{
  string filename = convert_to_string (rowid) + "__" + user + "__" +  channel + "__" + command;
  filename = file (filename);
  filter_url_file_put_contents (filename, message);
}


// Returns the next available row identifier.
int Database_Ipc::getNextId (const vector <Database_Ipc_Item> & data)
{
  int id = 0;
  for (auto & record : data) {
    if (record.rowid > id) id = record.rowid;
  }
  id++;
  return id;
}



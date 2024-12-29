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


#include <database/ipc.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <database/logic.h>


// Database resilience: Stored in plain file system.


Database_Ipc::Database_Ipc (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
}


void Database_Ipc::trim ()
{
  std::vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    if (record.user == "") {
      deleteMessage (record.rowid);
    }
  }
  int now = filter::date::seconds_since_epoch ();
  std::vector <std::string> files = filter_url_scandir (folder ());
  for (std::string item : files) {
    std::string path = file(item);
    int time = filter_url_file_modification_time (path);
    int age_seconds = now - time;
    if (age_seconds > 3600) {
      filter_url_unlink (path);
    }
  }
}


void Database_Ipc::storeMessage (std::string user, std::string channel, std::string command, std::string message)
{
  // Load entire database into memory.
  std::vector <Database_Ipc_Item> data = readData ();

  // Gather information about records to delete.
  std::vector <int> deletes;
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
Database_Ipc_Message Database_Ipc::retrieveMessage (int id, std::string user, std::string channel, std::string command)
{
  int highestId = 0;
  std::string hitChannel = "";
  std::string hitCommand = "";
  std::string hitMessage = "";
  std::vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    // Selection condition 1: The database record has a message identifier younger than the calling identifier.
    int recordid = record.rowid;
    if (recordid > id) {
      // Selection condition 2: Channel matches calling channel, or empty channel.
      std::string recordchannel = record.channel;
      if ((recordchannel == channel) || (recordchannel == "")) {
        // Selection condition 3: Record user matches calling user, or empty user.
        std::string recorduser = record.user;
        if ((recorduser == user) || (recorduser == "")) {
          // Selection condition 4: Matching command.
          std::string recordcommand = record.command;
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
  std::vector <Database_Ipc_Item> data = readData ();
  for (auto & record : data) {
    if (record.rowid == id) {
      filter_url_unlink (file (record.file));
    }
  }
}


std::string Database_Ipc::getFocus ()
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();

  int highestId = 0;
  std::string hitMessage = "";
  std::vector <Database_Ipc_Item> data = readData ();
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
  const std::string& user = m_webserver_request.session_logic ()->get_username ();

  int highestId = 0;
  std::string hitMessage = "";

  std::vector <Database_Ipc_Item> data = readData ();
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
  const std::string& user = m_webserver_request.session_logic ()->get_username ();

  int highestId = 0;
  std::string hitMessage = "";

  std::vector <Database_Ipc_Item> data = readData ();
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

  if (highestId) return filter::strings::convert_to_bool (hitMessage);

  return false;
}


std::string Database_Ipc::folder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "ipc"});
}


std::string Database_Ipc::file (std::string file)
{
  return filter_url_create_path ({folder (), file});
}


// Reads most fields of all data in this database.
// Returns an array containing the above.
// The fields are those that can be obtained from the name of the files.
// One file is one entry in the database.
// The filename looks like this: rowid__user__channel__command
std::vector <Database_Ipc_Item> Database_Ipc::readData ()
{
  std::vector <Database_Ipc_Item> data;
  std::vector <std::string> files = filter_url_scandir (folder ());
  for (std::string file : files) {
    std::vector <std::string> explosion = filter::strings::explode (file, '_');
    if (explosion.size () == 7) {
      Database_Ipc_Item item = Database_Ipc_Item ();
      item.file = file;
      item.rowid = filter::strings::convert_to_int (explosion [0]);
      item.user = explosion [2];
      item.channel = explosion [4];
      item.command = explosion [6];
      data.push_back (item);
    }
  }
  return data;
}
  

void Database_Ipc::writeRecord (int rowid, std::string user, std::string channel, std::string command, std::string message)
{
  std::string filename = std::to_string (rowid) + "__" + user + "__" +  channel + "__" + command;
  filename = file (filename);
  filter_url_file_put_contents (filename, message);
}


// Returns the next available row identifier.
int Database_Ipc::getNextId (const std::vector <Database_Ipc_Item> & data)
{
  int id = 0;
  for (auto & record : data) {
    if (record.rowid > id) id = record.rowid;
  }
  id++;
  return id;
}



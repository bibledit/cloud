/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <unittests/ipc.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <ipc/notes.h>
#include <filter/string.h>
using namespace std;


void test_ipc ()
{
  trace_unit_tests (__func__);
  
  // Initialize.
  refresh_sandbox (true);
  Webserver_Request request;
  request.database_users ()->create ();
  request.session_logic ()->set_username ("phpunit");
  
  // There should be no note identifier.
  int identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 0, identifier);
  
  // Test opening note.
  Ipc_Notes::open (&request, 123456789);
  identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 123456789, identifier);
  
  // Test trimming.
  request.database_ipc()->trim ();
  identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 123456789, identifier);
  
  // Test deleting note once.
  Ipc_Notes::open (&request, 123456789);
  Ipc_Notes::erase (&request);
  identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 0, identifier);
  
  // Test deleting two notes.
  Ipc_Notes::open (&request, 123456789);
  Ipc_Notes::open (&request, 123456789);
  Ipc_Notes::erase (&request);
  identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 0, identifier);
  Ipc_Notes::erase (&request);
  identifier = Ipc_Notes::get (&request);
  evaluate (__LINE__, __func__, 0, identifier);
}


void test_database_ipc ()
{
  trace_unit_tests (__func__);
  
  // Test trim.
  {
    refresh_sandbox (true);
    Database_Ipc database_ipc = Database_Ipc (nullptr);
    database_ipc.trim ();
  }
  // Test store retrieve
  {
    refresh_sandbox (true);
    Webserver_Request request;
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    int id = 1;
    string user = "phpunit";
    string channel = "channel";
    string command = "command";
    string message = "message";
    
    database_ipc.storeMessage (user, channel, command, message);
    
    Database_Ipc_Message data = database_ipc.retrieveMessage (id, user, channel, command);
    evaluate (__LINE__, __func__, 0, data.id);
    
    database_ipc.storeMessage (user, channel, command, message);
    
    data = database_ipc.retrieveMessage (id, user, channel, command);
    evaluate (__LINE__, __func__, 2, data.id);
    evaluate (__LINE__, __func__, message, data.message);
  }
  // Test delete
  {
    refresh_sandbox (true);
    Webserver_Request request;
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    int id = 1;
    string user = "phpunit";
    string channel = "channel";
    string command = "command";
    string message = "message";
    
    database_ipc.storeMessage (user, channel, command, message);
    
    Database_Ipc_Message data = database_ipc.retrieveMessage (0, user, channel, command);
    evaluate (__LINE__, __func__, id, data.id);
    
    database_ipc.deleteMessage (id);
    
    data = database_ipc.retrieveMessage (0, user, channel, command);
    evaluate (__LINE__, __func__, 0, data.id);
  }
  // Test get focus
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    string user = "phpunit";
    request.session_logic ()->set_username (user);
    string channel = "channel";
    string command = "focus";
    
    string passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, "1.1.1", passage);
    
    string message = "2.3.4";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, message, passage);
    
    message = "5.6.7";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, message, passage);
  }
  // Test get note.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    string user = "phpunit";
    request.session_logic ()->set_username (user);
    string channel = "channel";
    string command = "opennote";
    
    Database_Ipc_Message note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, 0, note.id);
    
    string message = "12345";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, message, note.message);
    
    message = "54321";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, message, note.message);
  }
  // Test notes alive.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    string user = "phpunit";
    request.session_logic ()->set_username (user);
    string channel = "channel";
    string command = "notesalive";
    
    bool alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, false, alive);
    
    string message = "1";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, filter::strings::convert_to_bool (message), alive);
    
    message = "0";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, filter::strings::convert_to_bool (message), alive);
  }
}

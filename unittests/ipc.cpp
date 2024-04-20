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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <ipc/notes.h>
#include <filter/string.h>


TEST (ipc, basic)
{
  // Initialize.
  refresh_sandbox (false);
  Webserver_Request webserver_request;
  webserver_request.database_users ()->create ();
  webserver_request.session_logic ()->set_username ("phpunit");
  
  // There should be no note identifier.
  int identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (0, identifier);
  
  // Test opening note.
  Ipc_Notes::open (webserver_request, 123456789);
  identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (123456789, identifier);
  
  // Test trimming.
  webserver_request.database_ipc()->trim ();
  identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (123456789, identifier);
  
  // Test deleting note once.
  Ipc_Notes::open (webserver_request, 123456789);
  Ipc_Notes::erase (webserver_request);
  identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (0, identifier);
  
  // Test deleting two notes.
  Ipc_Notes::open (webserver_request, 123456789);
  Ipc_Notes::open (webserver_request, 123456789);
  Ipc_Notes::erase (webserver_request);
  identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (0, identifier);
  Ipc_Notes::erase (webserver_request);
  identifier = Ipc_Notes::get (webserver_request);
  EXPECT_EQ (0, identifier);
}


TEST (database, ipc)
{
  // Test trim.
  {
    refresh_sandbox (false);
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    database_ipc.trim ();
  }

  // Test store retrieve
  {
    refresh_sandbox (true);
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    
    int id = 1;
    std::string user = "phpunit";
    std::string channel = "channel";
    std::string command = "command";
    std::string message = "message";
    
    database_ipc.storeMessage (user, channel, command, message);
    
    Database_Ipc_Message data = database_ipc.retrieveMessage (id, user, channel, command);
    EXPECT_EQ (0, data.id);
    
    database_ipc.storeMessage (user, channel, command, message);
    
    data = database_ipc.retrieveMessage (id, user, channel, command);
    EXPECT_EQ (2, data.id);
    EXPECT_EQ (message, data.message);
  }

  // Test delete
  {
    refresh_sandbox (true);
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    
    int id = 1;
    std::string user = "phpunit";
    std::string channel = "channel";
    std::string command = "command";
    std::string message = "message";
    
    database_ipc.storeMessage (user, channel, command, message);
    
    Database_Ipc_Message data = database_ipc.retrieveMessage (0, user, channel, command);
    EXPECT_EQ (id, data.id);
    
    database_ipc.deleteMessage (id);
    
    data = database_ipc.retrieveMessage (0, user, channel, command);
    EXPECT_EQ (0, data.id);
  }

  // Test get focus
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    
    std::string user = "phpunit";
    webserver_request.session_logic ()->set_username (user);
    std::string channel = "channel";
    std::string command = "focus";
    
    std::string passage = database_ipc.getFocus ();
    EXPECT_EQ ("1.1.1", passage);
    
    std::string message = "2.3.4";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    EXPECT_EQ (message, passage);
    
    message = "5.6.7";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    EXPECT_EQ (message, passage);
  }

  // Test get note.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    
    std::string user = "phpunit";
    webserver_request.session_logic ()->set_username (user);
    std::string channel = "channel";
    std::string command = "opennote";
    
    Database_Ipc_Message note = database_ipc.getNote ();
    EXPECT_EQ (0, note.id);
    
    std::string message = "12345";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    EXPECT_EQ (message, note.message);
    
    message = "54321";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    EXPECT_EQ (message, note.message);
  }

  // Test notes alive.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Ipc database_ipc (webserver_request);
    
    std::string user = "phpunit";
    webserver_request.session_logic ()->set_username (user);
    std::string channel = "channel";
    std::string command = "notesalive";
    
    bool alive = database_ipc.getNotesAlive ();
    EXPECT_EQ (false, alive);
    
    std::string message = "1";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    EXPECT_EQ (filter::strings::convert_to_bool (message), alive);
    
    message = "0";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    EXPECT_EQ (filter::strings::convert_to_bool (message), alive);
  }
}


#endif

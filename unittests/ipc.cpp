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


#include <unittests/ipc.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <ipc/notes.h>


void test_ipc ()
{
  trace_unit_tests (__func__);
  
  // Initialize.
  refresh_sandbox (true);
  Webserver_Request request;
  request.database_users ()->create ();
  request.session_logic ()->setUsername ("phpunit");
  
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

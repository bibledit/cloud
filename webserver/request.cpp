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


#include <webserver/request.h>


Webserver_Request::Webserver_Request ()
{
  secure = false;
  get = "/index";
  is_post = false;
  user_agent.assign ("Browser/1.0");
  accept_language = "en-US";
  content_length = 0;
  response_code = 200;
  resend_cookie = false;
}


Webserver_Request::~Webserver_Request ()
{
  if (session_logic_instance) delete session_logic_instance;
  if (database_config_user_instance) delete database_config_user_instance;
  if (database_users_instance) delete database_users_instance;
  if (database_styles_instance) delete database_styles_instance;
  if (database_bibles_instance) delete database_bibles_instance;
  if (database_check_instance) delete database_check_instance;
  if (database_ipc_instance) delete database_ipc_instance;
}


// Returns a pointer to a live Session_Logic object.
Session_Logic * Webserver_Request::session_logic ()
{
  // Single live object during the entire web request.
  if (!session_logic_instance) session_logic_instance = new Session_Logic (this);
  return session_logic_instance;
}


// Returns a pointer to a live Database_Config_User object.
Database_Config_User * Webserver_Request::database_config_user ()
{
  // Single live object during the entire web request.
  if (!database_config_user_instance) database_config_user_instance = new Database_Config_User (this);
  return database_config_user_instance;
}


// Returns a pointer to a live Database_Users object.
Database_Users * Webserver_Request::database_users ()
{
  // Single live object during the entire web request.
  if (!database_users_instance) database_users_instance = new Database_Users ();
  return database_users_instance;
}


Database_Styles * Webserver_Request::database_styles ()
{
  if (!database_styles_instance) database_styles_instance = new Database_Styles ();
  return database_styles_instance;
}


Database_Bibles * Webserver_Request::database_bibles ()
{
  if (!database_bibles_instance) database_bibles_instance = new Database_Bibles ();
  return database_bibles_instance;
}


Database_Check * Webserver_Request::database_check ()
{
  if (!database_check_instance) database_check_instance = new Database_Check ();
  return database_check_instance;
}


Database_Ipc * Webserver_Request::database_ipc ()
{
  if (!database_ipc_instance) database_ipc_instance = new Database_Ipc (this);
  return database_ipc_instance;
}



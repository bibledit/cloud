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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

std::string session_admin_credentials ();

class Session_Logic
{
public:
  Session_Logic (Webserver_Request& webserver_request);
  Session_Logic(const Session_Logic&) = delete;
  Session_Logic operator=(const Session_Logic&) = delete;
  void set_username (const std::string& name);
  bool attempt_login (std::string user_or_email, const std::string& password, bool touch_enabled,
                     bool skip_checks = false);
  bool get_logged_in ();
  const std::string& get_username () const;
  bool get_touch_enabled ();
  int get_level (bool force = false);
  void logout ();
  void switch_user (std::string new_user);
private:
  int m_level {0};  // The level of the user.
  bool m_logged_in {false}; // Whether user is logged in.
  std::string m_username {}; // The username.
  bool m_touch_enabled {false}; // Whether user works from a touch-enabled device.
  Webserver_Request& m_webserver_request;  // Referencee to instance of Webserver_Request.
  void open ();
  bool open_access ();
  bool client_access ();
};

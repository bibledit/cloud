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


#ifndef INCLUDED_SESSION_LOGIC_H
#define INCLUDED_SESSION_LOGIC_H


#include <config/libraries.h>


string session_admin_credentials ();


class Session_Logic
{
public:
  Session_Logic (void * webserver_request_in);
  void setUsername (string name);
  bool attemptLogin (string user_or_email, string password, bool touch_enabled);
  bool loggedIn ();
  string currentUser ();
  bool touchEnabled ();
  int currentLevel (bool force = false);
  string remoteAddress ();
  void logout ();
  void switchUser (string username);
private:
  int level = 0;               // The level of the user.
  int check_ip_blocks = 3;     // How many numbers from IP use in fingerprint?
  bool logged_in;              // Whether user is logged in.
  string username;             // The username.
  bool touch_enabled;          // Whether user works from a touch-enabled device.
  void * webserver_request;    // Pointer to instance of Webserver_Request.
  void open ();
  bool openAccess ();
  string fingerprint ();
  bool clientAccess ();
};


#endif

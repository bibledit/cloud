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
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <config/globals.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <user/logic.h>
#include <filter/date.h>


// Test for class Session_Logic.
TEST (session, logic1)
{
  refresh_sandbox (false);
  
  // The session logic depends on users in the database.
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  // In a demo installation, a client is always logged in as user admin, even after logging out.
  config_globals_open_installation = true;
  Webserver_Request request;
  EXPECT_EQ (true, request.session_logic ()->get_logged_in ());
  EXPECT_EQ (session_admin_credentials (), request.session_logic ()->get_username ());
  EXPECT_EQ (Filter_Roles::admin (), request.session_logic ()->get_level ());
  request.session_logic ()->logout ();
  EXPECT_EQ (true, request.session_logic ()->get_logged_in ());
  EXPECT_EQ (session_admin_credentials (), request.session_logic ()->get_username ());
  EXPECT_EQ (Filter_Roles::admin (), request.session_logic ()->get_level ());
  
  // Test function to set the username.
  std::string username = "ঃইঝম";
  request.session_logic ()->set_username (username);
  EXPECT_EQ (username, request.session_logic ()->get_username ());
  config_globals_open_installation = false;
}


TEST (session, logic2)
{
  // In a client installation, a client is logged in as admin when there's no user in the database.
  refresh_sandbox (false);
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  config_globals_client_prepared = true;
  Webserver_Request request;
  EXPECT_TRUE (request.session_logic ()->get_logged_in ());
  EXPECT_EQ (session_admin_credentials (), request.session_logic ()->get_username ());
  EXPECT_EQ (Filter_Roles::admin (), request.session_logic ()->get_level ());
  config_globals_client_prepared = false;
}

TEST (session, logic3)
{
  // In a client installation, a client is logged in as the first user in the database.
  refresh_sandbox (false);
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  const std::string username = "ঃইঝম";
  const int level = 10;
  database_users.add_user (username, "password", level, "email");
  config_globals_client_prepared = true;
  Webserver_Request request;
  EXPECT_TRUE (request.session_logic ()->get_logged_in ());
  EXPECT_EQ (username, request.session_logic ()->get_username ());
  EXPECT_EQ (level, request.session_logic ()->get_level ());
  config_globals_client_prepared = false;
}


TEST (session, logic4)
{
  refresh_sandbox (false);
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  Webserver_Request request1;
  
  // Enter a user into the database.
  const std::string username = "ঃইঝম";
  const std::string password = "ᨃᨄᨔᨕ";
  const std::string email = "email@website";
  const int level = 10;
  database_users.add_user (username, password, level, email);
  const std::string session = "abcdefgh";
  
  // Log in by providing username and password.
  request1.session_identifier = session;
  EXPECT_FALSE (request1.session_logic ()->attempt_login (username, "incorrect", true));
  user_logic_login_failure_clear ();
  EXPECT_TRUE (request1.session_logic ()->attempt_login (username, password, true));
  EXPECT_TRUE (request1.session_logic ()->get_logged_in ());
  
  // Check whether logged in also from another session.
  Webserver_Request request2;
  request2.session_identifier = session;
  EXPECT_TRUE (request2.session_logic ()->get_logged_in ());
  EXPECT_EQ (username, request2.session_logic ()->get_username ());
  EXPECT_EQ (level, request2.session_logic ()->get_level ());
  
  // Logout in another session, and check it in a subsequent session.
  Webserver_Request request3;
  request3.session_identifier = session;
  request3.session_logic ()->logout ();
  Webserver_Request request4;
  request4.session_identifier = session;
  EXPECT_FALSE (request4.session_logic ()->get_logged_in ());
  EXPECT_EQ (std::string(), request4.session_logic ()->get_username ());
  EXPECT_EQ (Filter_Roles::guest(), request4.session_logic ()->get_level ());
  
  // Login. Then vary the browser's signature for subsequent sessions.
  Webserver_Request request5;
  request5.session_identifier = session;
  EXPECT_TRUE (request5.session_logic ()->attempt_login (username, password, true));
  EXPECT_TRUE (request5.session_logic ()->get_logged_in ());
  const std::string remote_address = request5.remote_address;
  const std::string user_agent = request5.user_agent;
  const std::string accept_language = request5.accept_language;
  Webserver_Request request6;
  request6.session_identifier = session;
  request6.remote_address = "1.2.3.4";
  EXPECT_TRUE (request6.session_logic ()->get_logged_in ());
  Webserver_Request request7;
  request7.session_identifier = session;
  request7.remote_address = remote_address;
  EXPECT_TRUE (request7.session_logic ()->get_logged_in ());
  Webserver_Request request8;
  request8.session_identifier = session;
  request8.user_agent = "User's Agent";
  EXPECT_TRUE (request8.session_logic ()->get_logged_in ());
  Webserver_Request request9;
  request9.session_identifier = session;
  request9.user_agent = user_agent;
  EXPECT_TRUE (request9.session_logic ()->get_logged_in ());
  Webserver_Request request10;
  request10.session_identifier = session;
  request10.accept_language = "xy_ZA";
  EXPECT_TRUE (request10.session_logic ()->get_logged_in ());
  Webserver_Request request11;
  request11.session_identifier = session;
  request11.accept_language = accept_language;
  EXPECT_TRUE (request11.session_logic ()->get_logged_in ());
}

TEST (session, logic5)
{
  // Detection and mitigation of brute force login attack.
  refresh_sandbox (false);
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  // Enter a user into the database.
  const std::string username = "username";
  const std::string password = "password";
  const std::string email = "info@bibledit.org";
  constexpr int level = 5;
  database_users.add_user (username, password, level, email);
  Webserver_Request request1;
  const std::string session = "abcdefgh";
  
  // To get stable results from the unit tests, run them right from the start of a new second.
  // The issue is that the failed logins are recorded per second.
  int now = filter::date::seconds_since_epoch ();
  while (now == filter::date::seconds_since_epoch ()) std::this_thread::sleep_for (std::chrono::milliseconds (10));
  
  // Test the brute force detection mechanism.
  EXPECT_TRUE (user_logic_login_failure_check_okay ());
  user_logic_login_failure_register ();
  EXPECT_FALSE (user_logic_login_failure_check_okay ());
  user_logic_login_failure_clear ();
  EXPECT_TRUE (user_logic_login_failure_check_okay ());
  user_logic_login_failure_clear ();
  
  // Properly login.
  Webserver_Request request2;
  request2.session_identifier = session;
  EXPECT_TRUE (request2.session_logic ()->attempt_login (username, password, true));
  EXPECT_TRUE (request2.session_logic ()->get_logged_in ());
  request2.session_logic ()->logout ();
  
  // Login with wrong password.
  Webserver_Request request3;
  request3.session_identifier = session;
  EXPECT_FALSE (request3.session_logic ()->attempt_login (username, password + "wrong", true));
  EXPECT_FALSE (request3.session_logic ()->get_logged_in ());
  
  // Detector kicks in: A proper login now fails also.
  Webserver_Request request4;
  request4.session_identifier = session;
  EXPECT_FALSE (request4.session_logic ()->attempt_login (username, password, true));
  EXPECT_FALSE (request4.session_logic ()->get_logged_in ());
  
  // Wait till the next second.
  now = filter::date::seconds_since_epoch ();
  while (now == filter::date::seconds_since_epoch ()) std::this_thread::sleep_for (std::chrono::milliseconds (10));
  
  // After a second, a proper login works again.
  Webserver_Request request5;
  request5.session_identifier = session;
  EXPECT_EQ (true, request5.session_logic ()->attempt_login (username, password, true));
  EXPECT_EQ (true, request5.session_logic ()->get_logged_in ());
  request5.session_logic ()->logout ();
}


// Checks on login session without proper check, as in a confirmation through a link.
TEST (session, logic6)
{
  refresh_sandbox (false);
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  Webserver_Request request;
  
  // Enter a user into the database.
  const std::string username = "ঃইঝম";
  const std::string password = "ᨃᨄᨔᨕ";
  const std::string email = "email@website";
  const int level = 10;
  database_users.add_user (username, password, level, email);
  const std::string session = "abcdefgh";
  
  // Log in by providing email and incorrect password, but skip checks.
  request.session_identifier = session;
  EXPECT_TRUE (request.session_logic ()->attempt_login (email, "incorrect", true, true));
  EXPECT_TRUE (request.session_logic ()->get_logged_in ());
  
  // Log out and log in again with username, and skip checks, but this should fail.
  request.session_logic()->logout();
  EXPECT_FALSE (request.session_logic ()->attempt_login (username, "wrong", true, true));
  user_logic_login_failure_clear ();
}


#endif


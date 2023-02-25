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


#include <unittests/session.h>
#include <unittests/utilities.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <config/globals.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <user/logic.h>
#include <filter/date.h>
using namespace std;


void test_session ()
{
  trace_unit_tests (__func__);
  
  // Test for class Session_Logic.
  {
    refresh_sandbox (true);
    
    // The session logic depends on users in the database.
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // In a demo installation, a client is always logged in as user admin, even after logging out.
    config_globals_open_installation = true;
    Webserver_Request request;
    evaluate (__LINE__, __func__, true, request.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, session_admin_credentials (), request.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, Filter_Roles::admin (), request.session_logic ()->currentLevel ());
    request.session_logic ()->logout ();
    evaluate (__LINE__, __func__, true, request.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, session_admin_credentials (), request.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, Filter_Roles::admin (), request.session_logic ()->currentLevel ());
    
    // Test function to set the username.
    string username = "ঃইঝম";
    request.session_logic ()->set_username (username);
    evaluate (__LINE__, __func__, username, request.session_logic ()->currentUser ());
    config_globals_open_installation = false;
  }

  {
    // In a client installation, a client is logged in as admin when there's no user in the database.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    config_globals_client_prepared = true;
    Webserver_Request request;
    evaluate (__LINE__, __func__, true, request.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, session_admin_credentials (), request.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, Filter_Roles::admin (), request.session_logic ()->currentLevel ());
    config_globals_client_prepared = false;
  }

  {
    // In a client installation, a client is logged in as the first user in the database.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    string username = "ঃইঝম";
    int level = 10;
    database_users.add_user (username, "password", level, "email");
    config_globals_client_prepared = true;
    Webserver_Request request;
    evaluate (__LINE__, __func__, true, request.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, username, request.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, level, request.session_logic ()->currentLevel ());
    config_globals_client_prepared = false;
  }

  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    Webserver_Request request1;

    // Enter a user into the database.
    string username = "ঃইঝম";
    string password = "ᨃᨄᨔᨕ";
    string email = "email@website";
    int level = 10;
    database_users.add_user (username, password, level, email);
    string session = "abcdefgh";
    
    // Log in by providing username and password.
    request1.session_identifier = session;
    evaluate (__LINE__, __func__, false, request1.session_logic ()->attempt_login (username, "incorrect", true));
    user_logic_login_failure_clear ();
    evaluate (__LINE__, __func__, true, request1.session_logic ()->attempt_login (username, password, true));
    evaluate (__LINE__, __func__, true, request1.session_logic ()->loggedIn ());

    // Check whether logged in also from another session.
    Webserver_Request request2;
    request2.session_identifier = session;
    evaluate (__LINE__, __func__, true, request2.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, username, request2.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, level, request2.session_logic ()->currentLevel ());
    
    // Logout in another session, and check it in a subsequent session.
    Webserver_Request request3;
    request3.session_identifier = session;
    request3.session_logic ()->logout ();
    Webserver_Request request4;
    request4.session_identifier = session;
    evaluate (__LINE__, __func__, false, request4.session_logic ()->loggedIn ());
    evaluate (__LINE__, __func__, "", request4.session_logic ()->currentUser ());
    evaluate (__LINE__, __func__, Filter_Roles::guest(), request4.session_logic ()->currentLevel ());
    
    // Login. Then vary the browser's signature for subsequent sessions.
    Webserver_Request request5;
    request5.session_identifier = session;
    evaluate (__LINE__, __func__, true, request5.session_logic ()->attempt_login (username, password, true));
    evaluate (__LINE__, __func__, true, request5.session_logic ()->loggedIn ());
    string remote_address = request5.remote_address;
    string user_agent = request5.user_agent;
    string accept_language = request5.accept_language;
    Webserver_Request request6;
    request6.session_identifier = session;
    request6.remote_address = "1.2.3.4";
    evaluate (__LINE__, __func__, true, request6.session_logic ()->loggedIn ());
    Webserver_Request request7;
    request7.session_identifier = session;
    request7.remote_address = remote_address;
    evaluate (__LINE__, __func__, true, request7.session_logic ()->loggedIn ());
    Webserver_Request request8;
    request8.session_identifier = session;
    request8.user_agent = "User's Agent";
    evaluate (__LINE__, __func__, true, request8.session_logic ()->loggedIn ());
    Webserver_Request request9;
    request9.session_identifier = session;
    request9.user_agent = user_agent;
    evaluate (__LINE__, __func__, true, request9.session_logic ()->loggedIn ());
    Webserver_Request request10;
    request10.session_identifier = session;
    request10.accept_language = "xy_ZA";
    evaluate (__LINE__, __func__, true, request10.session_logic ()->loggedIn ());
    Webserver_Request request11;
    request11.session_identifier = session;
    request11.accept_language = accept_language;
    evaluate (__LINE__, __func__, true, request11.session_logic ()->loggedIn ());
  }

  {
    // Detection and mitigation of brute force login attack.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // Enter a user into the database.
    string username = "username";
    string password = "password";
    string email = "info@bibledit.org";
    int level = 5;
    database_users.add_user (username, password, level, email);
    Webserver_Request request1;
    string session = "abcdefgh";
    
    // To get stable results from the unit tests, run them right from the start of a new second.
    // The issue is that the failed logins are recorded per second.
    int now = filter::date::seconds_since_epoch ();
    while (now == filter::date::seconds_since_epoch ()) this_thread::sleep_for (chrono::milliseconds (10));
    
    // Test the brute force detection mechanism.
    evaluate (__LINE__, __func__, true, user_logic_login_failure_check_okay ());
    user_logic_login_failure_register ();
    evaluate (__LINE__, __func__, false, user_logic_login_failure_check_okay ());
    user_logic_login_failure_clear ();
    evaluate (__LINE__, __func__, true, user_logic_login_failure_check_okay ());
    user_logic_login_failure_clear ();
    
    // Properly login.
    Webserver_Request request2;
    request2.session_identifier = session;
    evaluate (__LINE__, __func__, true, request2.session_logic ()->attempt_login (username, password, true));
    evaluate (__LINE__, __func__, true, request2.session_logic ()->loggedIn ());
    request2.session_logic ()->logout ();
    
    // Login with wrong password.
    Webserver_Request request3;
    request3.session_identifier = session;
    evaluate (__LINE__, __func__, false, request3.session_logic ()->attempt_login (username, password + "wrong", true));
    evaluate (__LINE__, __func__, false, request3.session_logic ()->loggedIn ());
    
    // Detector kicks in: A proper login now fails also.
    Webserver_Request request4;
    request4.session_identifier = session;
    evaluate (__LINE__, __func__, false, request4.session_logic ()->attempt_login (username, password, true));
    evaluate (__LINE__, __func__, false, request4.session_logic ()->loggedIn ());
    
    // Wait till the next second.
    now = filter::date::seconds_since_epoch ();
    while (now == filter::date::seconds_since_epoch ()) this_thread::sleep_for (chrono::milliseconds (10));
    
    // After a second, a proper login works again.
    Webserver_Request request5;
    request5.session_identifier = session;
    evaluate (__LINE__, __func__, true, request5.session_logic ()->attempt_login (username, password, true));
    evaluate (__LINE__, __func__, true, request5.session_logic ()->loggedIn ());
    request5.session_logic ()->logout ();
  }
  
  // Checks on login session without proper check, as in a confirmation through a link.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    Webserver_Request request;

    // Enter a user into the database.
    string username = "ঃইঝম";
    string password = "ᨃᨄᨔᨕ";
    string email = "email@website";
    int level = 10;
    database_users.add_user (username, password, level, email);
    string session = "abcdefgh";
    
    // Log in by providing email and incorrect password, but skip checks.
    request.session_identifier = session;
    evaluate (__LINE__, __func__, true, request.session_logic ()->attempt_login (email, "incorrect", true, true));
    evaluate (__LINE__, __func__, true, request.session_logic ()->loggedIn ());
    
    // Log out and log in again with username, and skip checks, but this should fail.
    request.session_logic()->logout();
    evaluate (__LINE__, __func__, false, request.session_logic ()->attempt_login (username, "wrong", true, true));
    user_logic_login_failure_clear ();
  }
  
}

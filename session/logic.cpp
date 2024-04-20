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


#include <session/logic.h>
#include <database/sqlite.h>
#include <database/users.h>
#include <database/login.h>
#include <webserver/request.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <config/globals.h>
#include <user/logic.h>


// The username and password for a demo installation, and for a disconnected client installation
std::string session_admin_credentials ()
{
  return "admin";
}


/*

The PHP persistent storage of sessions normally works well.
On shared hosts, the hosting provider may influence how PHP sessions work.
In such a case, the session mechanism does not always work as desired.
This may result in frequently being locked out.
A possible solution would be to work around this behaviour.

Bibledit then found another solution:
It had its own persistent session storage mechanism.
This mechanism was independent of any hosting provider or any PHP session mechanism.

That mechanism made the following situation possible, although not likely:
* One user logs in.
* Another user, with exactly the same signature, is automatically logged in too.
The above is undesired.
The behaviour can be prevented when one of the users sets another user agent.

To improve the persistent login mechanism, the following has been done:
1. Bibledit generates a cookie with unique session identifier and other parameters.
2. When the user logs in, the browser will send this new cookie.
3. Bibledit stores the cookie details as it receives the cookie during login.
4. Next time when the user requests any page page, the browser will again send this cookie.
   Bibledit checks the details from the submitted cookie with its database.
   If the details match, the user is logged in.
It sets the cookie to expire after a certain time.

*/


Session_Logic::Session_Logic (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
  touch_enabled = false;
  open ();
}


// Call this when logging in.
void Session_Logic::open ()
{
  if (openAccess ()) return;
  if (clientAccess ()) return;

  // Work around a weird bug where the user_agent's size is 140735294083184 leading to a crash.
  if (m_webserver_request.user_agent.size () > 10'000) return;

  // Discard empty cookies right-away.
  // Don't regard this as something that triggers the brute force attach mitigation mechanism.
  std::string cookie = m_webserver_request.session_identifier;
  if (cookie.empty ()) {
    set_username ("");
    logged_in = false;
    return;
  }
  
  bool daily;
  std::string username_from_cookie = Database_Login::getUsername (cookie, daily);
  if (!username_from_cookie.empty ()) {
    set_username (username_from_cookie);
    logged_in = true;
    if (daily) m_webserver_request.resend_cookie = true;
    touch_enabled = Database_Login::getTouchEnabled (cookie);
  } else {
    set_username (std::string());
    logged_in = false;
  }
}


void Session_Logic::set_username (std::string name)
{
  username = name;
}


bool Session_Logic::openAccess ()
{
  // Open access if it is flagged as such.
  if (config_globals_open_installation) {
    set_username (session_admin_credentials ());
    level = Filter_Roles::admin ();
    logged_in = true;
    return true;
  }
  return false;
}


// Returns IP blocks of remote address.
std::string Session_Logic::remoteAddress ()
{
  std::vector <std::string> blocks = filter::strings::explode (m_webserver_request.remote_address, '.');
  std::string address;
  size_t num_blocks = static_cast<size_t> (abs (check_ip_blocks));
  if (num_blocks > blocks.size ()) num_blocks = blocks.size ();
  for (unsigned int i = 0; i < num_blocks; i++) {
    address += blocks [i] + ".";
  }
  return address;
}


// Returns a fingerprint from the user's browser.
std::string Session_Logic::fingerprint ()
{
  std::string fingerprint = "";
  // fingerprint += $_SERVER ['HTTP_CONNECTION']; Unstable fingerprint. No use for persistent login.
  // fingerprint += $_SERVER ['HTTP_ACCEPT_ENCODING']; Unstable fingerprint. No use for persistent login.
  fingerprint += m_webserver_request.accept_language;
  return fingerprint;
}


// Attempts to log into the system.
// Records whether the user logged in from a touch-enabled device.
// Returns boolean success.
bool Session_Logic::attempt_login (std::string user_or_email, std::string password,
                                   bool touch_enabled_in, bool skip_checks)
{
  // Brute force attack mitigation.
  if (!user_logic_login_failure_check_okay ()) {
    return false;
  }

  Database_Users database_users = Database_Users ();
  bool login_okay = false;

  // Match username and email.
  if (database_users.matchUserPassword (user_or_email, password)) {
    login_okay = true;
  }

  // Match password and email.
  if (database_users.matchEmailPassword (user_or_email, password)) {
    login_okay = true;
    // Fetch username that belongs to the email address that was used to login.
    user_or_email = database_users.getEmailToUser (user_or_email);
  }
  
  // The routine can skip the checks, as in the case of a confirmation link.
  if (skip_checks) {
    login_okay = true;
    // When skipping checks, the email is passed, so fetch the username from that.
    user_or_email = database_users.getEmailToUser (user_or_email);
    if (user_or_email.empty()) login_okay = false;
  }
  
  if (login_okay) {
    open ();
    set_username (user_or_email);
    logged_in = true;
    std::string cookie = m_webserver_request.session_identifier;
    Database_Login::setTokens (user_or_email, "", "", "", cookie, touch_enabled_in);
    currentLevel (true);
    return true;
  } else {
    user_logic_login_failure_register ();
  }
  
  return false;
}


// Returns true if the user is logged in.
bool Session_Logic::loggedIn ()
{
  // The logged-in status is stored in the object, so that if it is requested twice,
  // the session system is queried only once. It has been seen on some sites that if the php session
  // system was queried more than once, it did not behave consistently.
  // Buffering the status in the object resolved this.
  // After the session system was dropped, the above comment is no longer relevant.
  // The information this comment contains remains relevant for the future.
  if (openAccess ()) return true;
  return logged_in;
}


std::string Session_Logic::currentUser ()
{
  return username;
}


bool Session_Logic::touchEnabled ()
{
  // Deal with the global variable for touch-enabled.
  // The variable, if zero, does nothing.
  // Else it either sets or clears the touch-enabled state.
  // This 'doing nothing' is needed for a situation where a server has clients
  // with and without a touch-screen, so one global variable does not affect a local state.
  // The global variable is for a client application, where there's only one user per app.
  if (config_globals_touch_enabled > 0) touch_enabled = true;
  if (config_globals_touch_enabled < 0) touch_enabled = false;
  // Give the result, either set globally, or else through prior reading from the database.
  return touch_enabled;
}


// Returns the current level of the session as an integer.
int Session_Logic::currentLevel (bool force)
{
  if (openAccess ()) return level;
  if ((level == 0) || force) {
    if (loggedIn()) {
      Database_Users database = Database_Users();
      level = database.get_level (currentUser());
    } else {
      level = Filter_Roles::guest ();
    }
  }
  return level;
}


void Session_Logic::logout ()
{
  const std::string cookie = m_webserver_request.session_identifier;
  Database_Login::removeTokens (currentUser (), cookie);
  set_username (std::string());
  level = Filter_Roles::guest();
}


bool Session_Logic::clientAccess ()
{
  // If client mode is prepared, 
  // log in as the first username in the user database,
  // or as the admin in case no user has been set up yet.
  if (config_globals_client_prepared) {
    Database_Users database_users;
    std::vector <std::string> users = database_users.get_users ();
    std::string user;
    if (users.empty ()) {
      user = session_admin_credentials ();
      level = Filter_Roles::admin ();
    } else {
      user = users [0];
      level = database_users.get_level (user);
    }
    set_username (user);
    logged_in = true;
    return true;
  }
  return false;
}


void Session_Logic::switch_user (std::string new_user)
{
  std::string cookie = m_webserver_request.session_identifier;
  Database_Login::removeTokens (new_user, cookie);
  Database_Login::renameTokens (currentUser (), new_user, cookie);
}

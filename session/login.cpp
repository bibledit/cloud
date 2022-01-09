/*
Copyright (©) 2003-2022 Teus Benschop.

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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <session/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <index/index.h>
#include <ldap/logic.h>
#include <user/logic.h>
#include <config/logic.h>


const char * session_login_url ()
{
  return "session/login";
}


bool session_login_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string session_login (void * webserver_request)
{
  /*
  This script can have several functions:
  
  1. Display login form.
  The user is not logged in. 
  The standard form is displayed.
  
  2. Incorrect credentials entered.
  The user did not enter correct credentials.
  The login form is displayed, with an error description.
  
  3. Forward to URL.
  The script is called with a query for where to forward the user to.
  */

  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string page;

  Assets_View view;

  // Form submission handler.
  if (request->post["submit"] != "") {
    bool form_is_valid = true;
    string user = request->post["user"];
    string pass = request->post["pass"];
    // During login it determines whether the device is a touch enabled device.
    // Research shows that most desktop users move with their mouse over the screen before they click,
    // so we can detect those mouse movements through javascript,
    // and store that information with the user and device.
    // There is also wurfl.io that detects a mobile device in javascript,
    // but this library is of no immediate use at the server side.
    bool touch_enabled = convert_to_bool (request->post["touch"]);
    if (user.length () < 2) {
      form_is_valid = false;
      view.set_variable ("username_invalid", translate ("Username should be at least two characters long"));
    }
    if (pass.length() < 4) {
      form_is_valid = false;
      view.set_variable ("password_invalid", translate ("Password should be at least four characters long"));
    }
    if (form_is_valid) {
      // Optionally query the LDAP server and log the response.
      user_logic_optional_ldap_authentication (webserver_request, user, pass);
      // Authenticate against local database.
      if (request->session_logic()->attemptLogin (user, pass, touch_enabled)) {
        // Log the login.
        Database_Logs::log (request->session_logic()->currentUser () + " logged in");
        // Store web site's base URL.
        string siteUrl = get_base_url (request);
        Database_Config_General::setSiteURL (siteUrl);
      } else {
        view.set_variable ("error_message", translate ("Username or email address or password are not correct"));
        request->session_logic()->logout();
        // Log the login failure for the Administrator(s) only.
        // Others with lower roles should not be able to reverse engineer a user's password
        // based on the failure information.
        Database_Logs::log ("Failed login attempt for user " + user + " with password " + pass, Filter_Roles::admin ());
      }
    }
  }
  
  view.set_variable ("VERSION", config_logic_version ());
  
  if (ldap_logic_is_on ()) {
    view.enable_zone ("ldap");
  } else {
    view.enable_zone ("local");
  }


  string forward = request->query ["request"];
  
  if (request->session_logic ()->loggedIn ()) {
    if (forward != "") {
      // After login, the user is forwarded to the originally requested URL, if any.
      redirect_browser (request, forward);
      return "";
    }
    // After login, go to the main page.
    redirect_browser (request, index_index_url ());
    return "";
  } else {
    page += session_login_display_header (webserver_request);
    view.set_variable ("forward", forward);
    view.enable_zone ("logging_in");
    page += view.render ("session", "login");
  }

  page += Assets_Page::footer ();

  return page;
}


string session_login_display_header (void * webserver_request)
{
  /*
  Postpone displaying the header for two reasons:
  1. The logged-in state is likely to change during this script.
     Therefore the header should wait till the new state is known.
  2. The script may forward the user to another page.
     Therefore no output should be sent so the forward headers work.
  */
  Assets_Header header = Assets_Header (translate ("Login"), webserver_request);
  header.touchCSSOn ();
  return header.run ();
}

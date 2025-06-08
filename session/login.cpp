/*
Copyright (©) 2003-2025 Teus Benschop.

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


const char * session_login_url ()
{
  return "session/login";
}


bool session_login_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string session_login (Webserver_Request& webserver_request)
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

  std::string page{};

  Assets_View view{};

  // Form submission handler.
  if (webserver_request.post["submit"] != "") {
    bool form_is_valid = true;
    const std::string user = webserver_request.post["user"];
    const std::string pass = webserver_request.post["pass"];
    // During login it determines whether the device is a touch enabled device.
    // Research shows that most desktop users move with their mouse over the screen before they click,
    // so we can detect those mouse movements through javascript,
    // and store that information with the user and device.
    // There is also wurfl.io that detects a mobile device in javascript,
    // but this library is of no immediate use at the server side.
    const bool touch_enabled = filter::strings::convert_to_bool (webserver_request.post["touch"]);
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
      if (webserver_request.session_logic()->attempt_login (user, pass, touch_enabled)) {
        // Log the login.
        Database_Logs::log (webserver_request.session_logic ()->get_username () + " logged in");
        // Store web site's base URL.
        const std::string site_url = get_base_url (webserver_request);
        database::config::general::set_site_url (site_url);
      } else {
        view.set_variable ("error_message", translate ("Username or email address or password are not correct"));
        webserver_request.session_logic()->logout();
        // Log the login failure for the Administrator(s) only.
        // Others with lower roles should not be able to reverse engineer a user's password
        // based on the failure information.
        Database_Logs::log ("Failed login attempt for user " + user + " with password " + pass, roles::admin);
      }
    }
  }
  
  view.set_variable ("VERSION", config::logic::version ());
  
  if (ldap_logic_is_on ()) {
    view.enable_zone ("ldap");
  } else {
    view.enable_zone ("local");
  }

  if (!config::logic::create_no_accounts()) {
    view.enable_zone("create_accounts");
  }


  const std::string forward = webserver_request.query ["request"];
  
  if (webserver_request.session_logic ()->get_logged_in ()) {
    if (!forward.empty()) {
      // After login, the user is forwarded to the originally requested URL, if any.
      redirect_browser (webserver_request, forward);
      return std::string();
    }
    // After login, go to the main page.
    redirect_browser (webserver_request, index_index_url ());
    return std::string();
  } else {
    page += session_login_display_header (webserver_request);
    view.set_variable ("forward", forward);
    view.enable_zone ("logging_in");
    page += view.render ("session", "login");
  }

  page += assets_page::footer ();

  return page;
}


std::string session_login_display_header (Webserver_Request& webserver_request)
{
  /*
  Postpone displaying the header for two reasons:
  1. The logged-in state is likely to change during this script.
     Therefore the header should wait till the new state is known.
  2. The script may forward the user to another page.
     Therefore no output should be sent so the forward headers work.
  */
  Assets_Header header = Assets_Header (translate ("Login"), webserver_request);
  header.touch_css_on ();
  return header.run ();
}

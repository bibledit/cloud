/*
Copyright (©) 2003-2021 Teus Benschop.

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
#include <public/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <public/index.h>


const char * public_login_url ()
{
  return "public/login";
}


bool public_login_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_login (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate ("Public login"), request);
  header.touchCSSOn ();
  page = header.run ();
  Assets_View view;

  
  // Form submission handler.
  if (!request->post["submit"].empty ()) {

    bool form_is_valid = true;
    string name = request->post["name"];
    string email = request->post["email"];
    
    // During login it determines whether the device is a touch enabled device.
    // Research shows that most desktop users move with their mouse over the screen before they click,
    // so we can detect those mouse movements through javascript,
    // and store that information with the user and device.
    bool touch_enabled = convert_to_bool (request->post["touch"]);
    
    if (name.length () < 2) {
      form_is_valid = false;
      view.set_variable ("name_invalid", translate ("The name should be at least two characters long"));
    }
    
    if (!filter_url_email_is_valid (email)) {
      form_is_valid = false;
      view.set_variable ("email_invalid", translate("The email address is not valid"));
    }
    
    string other_login = translate ("This account needs a password.") + " " + translate("Please login the other way, through Menu / Login.");
    
    // If the username exists with a level higher than guest, that would not be right.
    if (form_is_valid) {
      int level = request->database_users ()->get_level (name);
      if (level > Filter_Roles::guest ()) {
        form_is_valid = false;
        view.set_variable ("error", other_login);
      }
    }
    
    // If the email address exists with a level higher than guest, that would not be right.
    if (form_is_valid) {
      if (request->database_users ()->emailExists (email)) {
        string username = request->database_users ()->getEmailToUser (email);
        int level = request->database_users ()->get_level (username);
        if (level > Filter_Roles::guest ()) {
          form_is_valid = false;
          view.set_variable ("error", other_login);
        }
      }
    }

    // If the email address exists with a guest role,
    // update the username to be matching with this email address.
    if (form_is_valid) {
      if (request->database_users ()->emailExists (email)) {
        string username = request->database_users ()->getEmailToUser (email);
        int level = request->database_users ()->get_level (username);
        if (level == Filter_Roles::guest ()) {
          name = username;
        }
      }
    }

    if (form_is_valid) {
      // For public login, the password is taken to be the same as the username.
      if (request->session_logic()->attemptLogin (name, name, touch_enabled)) {
        // Log the login.
        Database_Logs::log ("User " + request->session_logic()->currentUser () + " logged in");
      } else {
        // Add a new user and login.
        request->database_users ()->add_user(name, name, Filter_Roles::guest (), email);
        request->session_logic()->attemptLogin (name, name, touch_enabled);
        Database_Logs::log ("Public account created for user " + request->session_logic()->currentUser () + " with email " + email);
      }
    }
  }


  if (request->session_logic ()->loggedIn ()) {
    redirect_browser (request, public_index_url ());
    return "";
  }

  
  page += view.render ("public", "login");
  page += Assets_Page::footer ();

  
  return page;
}



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
#include <public/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <public/index.h>
#include <config/logic.h>
#include <config/logic.h>


const char * public_login_url ()
{
  return "public/login";
}


bool public_login_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


std::string public_login (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate ("Public login"), webserver_request);
  header.touch_css_on ();
  page = header.run ();
  Assets_View view;

  
  // Form submission handler.
  if (!webserver_request.post["submit"].empty ()) {

    bool form_is_valid = true;
    std::string name = webserver_request.post["name"];
    const std::string email = webserver_request.post["email"];
    
    // During login it determines whether the device is a touch enabled device.
    // Research shows that most desktop users move with their mouse over the screen before they click,
    // so we can detect those mouse movements through javascript,
    // and store that information with the user and device.
    const bool touch_enabled = filter::strings::convert_to_bool (webserver_request.post["touch"]);
    
    if (name.length () < 2) {
      form_is_valid = false;
      view.set_variable ("name_invalid", translate ("The name should be at least two characters long"));
    }
    
    if (!filter_url_email_is_valid (email)) {
      form_is_valid = false;
      view.set_variable ("email_invalid", translate("The email address is not valid"));
    }
    
    std::string other_login = translate ("This account needs a password.") + " " + translate("Please login the other way, through Menu / Login.");
    
    // If the username exists with a level higher than guest, that would not be right.
    if (form_is_valid) {
      const int level = webserver_request.database_users ()->get_level (name);
      if (level > Filter_Roles::guest ()) {
        form_is_valid = false;
        view.set_variable ("error", other_login);
      }
    }
    
    // If the email address exists with a level higher than guest, that would not be right.
    if (form_is_valid) {
      if (webserver_request.database_users ()->emailExists (email)) {
        const std::string username = webserver_request.database_users ()->getEmailToUser (email);
        const  int level = webserver_request.database_users ()->get_level (username);
        if (level > Filter_Roles::guest ()) {
          form_is_valid = false;
          view.set_variable ("error", other_login);
        }
      }
    }

    // If the email address exists with a guest role,
    // update the username to be matching with this email address.
    if (form_is_valid) {
      if (webserver_request.database_users ()->emailExists (email)) {
        const std::string username = webserver_request.database_users ()->getEmailToUser (email);
        const int level = webserver_request.database_users ()->get_level (username);
        if (level == Filter_Roles::guest ()) {
          name = username;
        }
      }
    }

    if (form_is_valid) {
      // For public login, the password is taken to be the same as the username.
      if (webserver_request.session_logic()->attempt_login (name, name, touch_enabled)) {
        // Log the login.
        Database_Logs::log ("User " + webserver_request.session_logic ()->get_username () + " logged in");
      } else {
        // Add a new user and login.
        webserver_request.database_users ()->add_user(name, name, Filter_Roles::guest (), email);
        webserver_request.session_logic()->attempt_login (name, name, touch_enabled);
        Database_Logs::log ("Public account created for user " + webserver_request.session_logic ()->get_username () + " with email " + email);
      }
    }
  }


  if (webserver_request.session_logic ()->get_logged_in ()) {
    redirect_browser (webserver_request, public_index_url ());
    return std::string();
  }

  
  page += view.render ("public", "login");
  page += assets_page::footer ();

  
  return page;
}

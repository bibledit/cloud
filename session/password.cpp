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


#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <session/password.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <email/send.h>


const char * session_password_url ()
{
  return "session/password";
}


bool session_password_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string session_password (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;

  Assets_Header header = Assets_Header (translate ("Password"), webserver_request);
  header.touchCSSOn ();
  page += header.run ();

  Assets_View view;

  // Form submission handler.
  if (request->post["submit"] != "") {
    bool form_is_valid = true;
    string user = request->post["user"];
    if (user.length () < 4) {
      view.set_variable ("error_message", translate("Username or email address is too short"));
      form_is_valid = false;
    }
    string email;
    Database_Users database_users;
    if (form_is_valid) {
      form_is_valid = false;
      email = database_users.get_email (user);
      if (email != "") {
        form_is_valid = true;
      }
      if (!form_is_valid) {
        if (database_users.emailExists (user)) {
          form_is_valid = true;
          email = user;
        }
      }
    }
    if (form_is_valid) {
      // Generate and store a new password.
      string generated_password = md5 (convert_to_string (filter_string_rand (0, 1000000)));
      generated_password = generated_password.substr (0, 15);
      string username = database_users.getEmailToUser (email);
      database_users.set_password (username, generated_password);
      // Send the new password to the user.
      string subject = translate("Account changed");
      string body = translate("Somebody requested a new password for your account.");
      body += "\n\n";
      body += translate("Here is the new password:");
      body += "\n\n";
      body += generated_password;
      body += "\n\n";
      body += translate("It is recommended to log into your account with this new password, and then change it.");
      email_schedule (username, subject, body);
      view.set_variable ("success_message", translate("A message was sent to the email address belonging to this account to help you getting the password"));
    } else {
      view.set_variable ("error_message", translate("Username or email address cannot be found"));
    }
  }
  
  page += view.render ("session", "password");

  page += Assets_Page::footer ();

  return page;
}

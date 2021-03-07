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


#include <user/account.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <confirm/worker.h>
#include <locale/translate.h>
#include <assets/header.h>
#include <menu/logic.h>


string user_account_url ()
{
  return "user/account";
}


bool user_account_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


string user_account (void * webserver_request)
{
  (void) webserver_request;
  
  string page;

#ifdef HAVE_CLOUD

  Webserver_Request * request = (Webserver_Request *) webserver_request;

  Assets_Header header = Assets_Header (translate("Account"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  Assets_View view;

  string username = request->session_logic()->currentUser ();
  string email = request->database_users()->get_email (username);

  bool actions_taken = false;
  vector <string> success_messages;

  // Form submission handler.
  if (request->post.count ("submit")) {
    bool form_is_valid = true;
    string currentpassword = request->post ["currentpassword"];
    string newpassword     = request->post ["newpassword"];
    string newpassword2    = request->post ["newpassword2"];
    string newemail        = request->post ["newemail"];
  
    if ((newpassword != "") || (newpassword2 != "")) {
      if (newpassword.length () < 4) {
        form_is_valid = false;
        view.set_variable ("new_password_invalid_message", translate("Password should be at least four characters long"));
      }
      if (newpassword2.length () < 4) {
        form_is_valid = false;
        view.set_variable ("new_password2_invalid_message", translate("Password should be at least four characters long"));
      }
      if (newpassword2 != newpassword) {
        form_is_valid = false;
        view.set_variable ("new_password2_invalid_message", translate("Passwords do not match"));
      }
      if (!request->database_users()->matchUserPassword (username, currentpassword)) {
        form_is_valid = false;
        view.set_variable ("current_password_invalid_message", translate("Current password is not valid"));
      }
      if (form_is_valid) {
        request->database_users()->set_password (username, newpassword);
        actions_taken = true;
        success_messages.push_back (translate("The new password was saved"));
      }
    }
  
    if (newemail != "") {
      if (!filter_url_email_is_valid (newemail)) {
        form_is_valid = false;
        view.set_variable ("new_email_invalid_message", translate("Email address is not valid"));
      }
      if (!request->database_users()->matchUserPassword (username, currentpassword)) {
        form_is_valid = false;
        view.set_variable ("current_password_invalid_message", translate("Current password is not valid"));
      }
      if (form_is_valid) {
        Confirm_Worker confirm_worker = Confirm_Worker (webserver_request);
        string initial_subject = translate("Email address verification");
        string initial_body = translate("Somebody requested to change the email address that belongs to your account.");
        string query = request->database_users()->updateEmailQuery (username, newemail);
        string subsequent_subject = translate("Email address change");
        string subsequent_body = translate("The email address that belongs to your account has been changed successfully.");
        confirm_worker.setup (newemail, initial_subject, initial_body, query, subsequent_subject, subsequent_body);
        actions_taken = true;
        success_messages.push_back (translate("A verification email was sent to") + " " + newemail);
      }
    }
  
    if (!actions_taken) {
      success_messages.push_back (translate("No changes were made"));
    }
  
  }

  view.set_variable ("username", escape_special_xml_characters (username));
  view.set_variable ("email", escape_special_xml_characters (email));
  string success_message = filter_string_implode (success_messages, "\n");
  view.set_variable ("success_messages", success_message);
  if (!actions_taken) view.enable_zone ("no_actions_taken");

  page += view.render ("user", "account");

  page += Assets_Page::footer ();

#endif

  return page;
}

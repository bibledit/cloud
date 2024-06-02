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


#include <email/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/general.h>
#include <webserver/request.h>
#include <email/send.h>
#include <email/receive.h>
#include <locale/translate.h>
#include <assets/header.h>
#include <menu/logic.h>


std::string email_index_url ()
{
  return "email/index";
}


bool email_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::admin ());
}


std::string email_index (Webserver_Request& webserver_request)
{
  std::string page;

  Assets_Header header = Assets_Header (translate("Mail"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  Assets_View view;

  // Site name and email.
  if (webserver_request.post ["email"] != "") {
    bool form_is_valid = true;
    std::string sitename = webserver_request.post ["sitename"];
    std::string sitemail = webserver_request.post ["sitemail"];
    if (sitemail.length () > 0) {
      if (!filter_url_email_is_valid (sitemail)) {
        form_is_valid = false;
        view.set_variable ("site_name_error", translate("The email address does not appear to be valid"));
      }
    }
    if (form_is_valid) {
      database::config::general::set_site_mail_name (sitename);
      database::config::general::set_site_mail_address (sitemail);
      view.set_variable ("site_name_success", translate("The name and email address were saved"));
    }
  }
  view.set_variable ("sitename", database::config::general::get_site_mail_name ());
  view.set_variable ("sitemail", database::config::general::get_site_mail_address ());

  // Email retrieval.
  if (webserver_request.post ["retrieve"] != "") {
    std::string storagehost = webserver_request.post ["storagehost"];
    std::string storageusername = webserver_request.post ["storageusername"];
    std::string storagepassword = webserver_request.post ["storagepassword"];
    std::string storagesecurity = webserver_request.post ["storagesecurity"];
    std::string storageport = webserver_request.post ["storageport"];
    database::config::general::set_mail_storage_host (storagehost);
    database::config::general::set_mail_storage_username (storageusername);
    database::config::general::set_mail_storage_password (storagepassword);
    database::config::general::set_mail_storage_protocol (storagesecurity);
    database::config::general::set_mail_storage_port (storageport);
    std::string storage_success = translate("The details were saved.");
    std::string storage_error;
    int mailcount = email_receive_count (storage_error, true);
    if (storage_error.empty ()) {
      storage_success.append (" ");
      storage_success.append (translate("The account was accessed successfully."));
      storage_success.append (" ");
      storage_success.append (translate ("Messages on server:"));
      storage_success.append (" ");
      storage_success.append (std::to_string (mailcount));
      storage_success.append (".");
    }
    view.set_variable ("storage_success", storage_success);
    view.set_variable ("storage_error", storage_error);
  }
  view.set_variable ("storagehost", database::config::general::get_mail_storage_host ());
  view.set_variable ("storageusername", database::config::general::get_mail_storage_username ());
  view.set_variable ("storagepassword", database::config::general::get_mail_storage_password ());
  if (database::config::general::get_mail_storage_protocol () == "POP3S") view.set_variable ("storagepop3s", R"(selected="selected")");
  view.set_variable ("storageport", database::config::general::get_mail_storage_port ());
  
  // Sending email.
  if (webserver_request.post ["send"] != "") {
    std::string sendhost = webserver_request.post ["sendhost"];
    std::string sendauthentication = webserver_request.post ["sendauthentication"];
    std::string sendusername = webserver_request.post ["sendusername"];
    std::string sendpassword = webserver_request.post ["sendpassword"];
    std::string sendsecurity = webserver_request.post ["sendsecurity"];
    std::string sendport  = webserver_request.post ["sendport"];
    database::config::general::set_mail_send_host (sendhost);
    database::config::general::set_mail_send_username (sendusername);
    database::config::general::set_mail_send_password (sendpassword);
    database::config::general::set_mail_send_port (sendport);
    std::string send_success  = translate("The details were saved.");
    std::string send_error;
    std::string send_debug;
    std::string result = email_send (database::config::general::get_site_mail_address(), database::config::general::get_site_mail_name(), "Test", "This is to check sending email.", true);
    if (result.empty()) {
      send_success.append (" ");
      send_success.append ("For checking sending email, a test email was sent out to the account above:");
      send_success.append (" ");
      send_success.append (database::config::general::get_site_mail_address());
    } else {
      send_error = result;
    }
    view.set_variable ("send_success", send_success);;
    view.set_variable ("send_error", send_error);
    view.set_variable ("send_debug", send_debug);
  }
  view.set_variable ("sendhost", database::config::general::get_mail_send_host ());
  view.set_variable ("sendusername", database::config::general::get_mail_send_username ());
  view.set_variable ("sendpassword", database::config::general::get_mail_send_password ());
  view.set_variable ("sendport", database::config::general::get_mail_send_port ());

  page += view.render ("email", "index");

  page += assets_page::footer ();

  return page;
}


/*

The more recent versions of the VMime library can access Gmail in a more secure manner than libcurl can.
The VMime library also can extract the plain text message easily, 
in case the message consists of various parts.
If necessary, the Cloud version of Bibledit can use this VMime library 
for sending, receiving and processing email.

*/

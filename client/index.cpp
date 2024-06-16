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


#include <client/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/noteactions.h>
#include <database/bibleactions.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <assets/external.h>
#include <bb/logic.h>


std::string client_index_url ()
{
  return "client/index";
}


bool client_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


void client_index_remove_all_users (Webserver_Request& webserver_request)
{
  const std::vector <std::string> existing_users {webserver_request.database_users()->get_users ()};
  for (const auto& existing_user : existing_users) {
    webserver_request.database_users()->removeUser (existing_user);
  }
}


void client_index_enable_client (Webserver_Request& webserver_request, const std::string& username, const std::string& password, const int level)
{
  // Enable client mode upon a successful connection.
  client_logic_enable_client (true);
  
  // Remove all users from the database, and add the current one.
  client_index_remove_all_users (webserver_request);
  webserver_request.database_users ()->add_user (username, password, level, std::string());
  
  // Update the username and the level in the current session.
  webserver_request.session_logic ()->set_username (username);
  webserver_request.session_logic ()->get_level (true);
  
  // If there's pending Bible updates, send them off to the user.
  bible_logic::client_mail_pending_bible_updates (username);
  
  // Clear all pending note actions and Bible actions and settings updates.
  Database_NoteActions database_noteactions;
  database_noteactions.clear ();
  database_noteactions.create ();
  database::bible_actions::clear ();
  database::bible_actions::create ();
  webserver_request.session_logic ()->set_username (username);
  webserver_request.database_config_user()->setUpdatedSettings ({});
  database::config::general::set_unsent_bible_data_time (0);
  database::config::general::set_unreceived_bible_data_time (filter::date::seconds_since_epoch ());
  
  // Set flag for first run after connecting.
  database::config::general::set_just_connected_to_cloud (true);
  
  // Set it to repeat sync every so often.
  if (database::config::general::get_repeat_send_receive () == 0) {
    database::config::general::set_repeat_send_receive (2);
  }
  
  // Schedule a sync operation straightaway.
  sendreceive_queue_sync (-1, 0);
}


std::string client_index (Webserver_Request& webserver_request)
{
  Assets_View view {};
  
  if (webserver_request.query.count ("disable")) {
    client_logic_enable_client (false);
    client_index_remove_all_users (webserver_request);
    database::config::general::set_repeat_send_receive (0);
    database::config::general::set_unsent_bible_data_time (0);
    database::config::general::set_unreceived_bible_data_time (0);
    database::config::general::set_just_connected_to_cloud (false);
  }
  
  bool connect = webserver_request.post.count ("connect");
  bool demo = webserver_request.query.count ("demo");
  if (connect || demo) {

    bool proceed {true};
    
    std::string address {};
    if (proceed) address = webserver_request.post ["address"];
    if (demo) address = demo_address ();
    // If there's not something like "http" in the server address, then add it.
    if (address.find ("http") == std::string::npos)
      address = filter_url_set_scheme (address, false);
    if (proceed) {
      // Get schema, host and port.
      std::string scheme {};
      std::string host {};
      int port {0};
      filter_url_get_scheme_host_port (address, scheme, host, port);
      // If no address given, then that's an error.
      if (proceed) if (host.empty()) {
        view.set_variable ("error", translate ("Supply an internet address"));
        proceed = false;
      }
      // If the user entered a port number here too, then that's an error.
      if (proceed) if (port > 0) {
        view.set_variable ("error", translate ("Remove the port number from the internet address"));
        proceed = false;
      }
    }
    // Store the address.
    database::config::general::set_server_address (address);
    
    int port = filter::strings::convert_to_int (config::logic::http_network_port ());
    if (proceed) port = filter::strings::convert_to_int (webserver_request.post ["port"]);
    if (demo) port = demo_port ();
    if (proceed) if (port == 0) {
      view.set_variable ("error", translate ("Supply a port number"));
      proceed = false;
    }
    database::config::general::set_server_port (port);
    
    std::string user {};
    if (proceed) user = webserver_request.post ["user"];
    if (demo) user = session_admin_credentials ();
    if (proceed) if (user.empty()) {
      view.set_variable ("error", translate ("Supply a username"));
      proceed = false;
    }
    
    std::string pass {};
    if (proceed) pass = webserver_request.post ["pass"];
    if (demo) pass = session_admin_credentials ();
    if (proceed) if (pass.empty()) {
      view.set_variable ("error", translate ("Supply a password"));
      proceed = false;
    }

    if (proceed) {
      const std::string response = client_logic_connection_setup (user, md5 (pass));
      const int iresponse = filter::strings::convert_to_int (response);
      if ((iresponse >= Filter_Roles::guest ()) && (iresponse <= Filter_Roles::admin ())) {
        // Enable client mode upon a successful connection.
        client_index_enable_client (webserver_request, user, pass, iresponse);
        // Feedback.
        view.set_variable ("success", translate("Connection is okay."));
      } else {
        view.set_variable ("error", translate ("Could not create a connection with Bibledit Cloud") + ": " + response);
      }
    }
  }

  if (client_logic_client_enabled ()) view.enable_zone ("clienton");
  else view.enable_zone ("clientoff");
  
  const std::string address {database::config::general::get_server_address ()};
  view.set_variable ("address", address);
  
  const int port {database::config::general::get_server_port ()};
  view.set_variable ("port", std::to_string (port));
  
  view.set_variable ("url", client_logic_link_to_cloud ("", ""));
  
  const std::vector <std::string> users {webserver_request.database_users ()->get_users ()};
  for (const auto& user : users) {
    const int level = webserver_request.database_users()->get_level (user);
    view.set_variable ("role", Filter_Roles::text (level));
  }
  
  view.set_variable ("demo", demo_client_warning ());

  view.set_variable ("external", assets_external_logic_link_addon ());

  if (webserver_request.query.count ("info")) {
    view.enable_zone ("info");
  }
  
  const bool basic_mode {config::logic::basic_mode (webserver_request)};
  if (basic_mode) view.enable_zone("basicmode");
  
  std::string page {};

  // Since the role of the user may change after a successful connection to the server,
  // the menu generation in the header should be postponed till when the actual role is known.
  page = assets_page::header (translate ("Server"), webserver_request);
  
  page += view.render ("client", "index");
  
  page += assets_page::footer ();
  
  return page;
}

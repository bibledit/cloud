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


#include <manage/accounts.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <dialog/entry.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/privileges.h>
#include <database/login.h>
#include <database/noteassignment.h>
#include <access/user.h>
#include <locale/translate.h>
#include <notes/logic.h>
#include <menu/logic.h>
#include <session/switch.h>
#include <user/logic.h>


std::string manage_accounts_url ()
{
  return "manage/accounts";
}


bool manage_accounts_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string manage_accounts (Webserver_Request& webserver_request)
{
  bool user_updated = false;
  bool privileges_updated = false;
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Accounts"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  // The user to act on.
  std::string objectUsername = webserver_request.query["user"];
  int user_level = webserver_request.database_users ()->get_level (objectUsername);
  
  // Delete a user.
  if (webserver_request.query.count ("delete")) {
    std::string role = roles::text (user_level);
    std::string email = webserver_request.database_users ()->get_email (objectUsername);
    std::vector <std::string> users = webserver_request.database_users ()->get_users ();
    std::vector <std::string> administrators = webserver_request.database_users ()->getAdministrators ();
    if (users.size () == 1) {
      page += assets_page::error (translate("Cannot remove the last user"));
    } else if ((user_level >= roles::admin) && (administrators.size () == 1)) {
      page += assets_page::error (translate("Cannot remove the last administrator"));
    } else {
      std::string message;
      user_logic_delete_account (objectUsername, role, email, message);
      user_updated = true;
      message.append (" ");
      message.append ("See the Journal for progress");
      page += assets_page::success (message);
    }
  }
  
  // Get the account creation times.
  std::map <std::string, int> account_creation_times;
  {
    std::vector <std::string> lines = database::config::general::get_account_creation_times ();
    for (auto line : lines) {
      std::vector <std::string> bits = filter::strings::explode(line, '|');
      if (bits.size() != 2) continue;
      int seconds = filter::strings::convert_to_int(bits[0]);
      std::string user = bits[1];
      account_creation_times [user] = seconds;
    }
  }
  
  // Retrieve assigned users.
  const std::vector <std::string> users = access_user::assignees (webserver_request);
  for (const auto& username : users) {
    
    // Gather details for this user account.
    user_level = webserver_request.database_users ()->get_level (username);
    const std::string role = roles::text (user_level);
    const std::string email = webserver_request.database_users ()->get_email (username);
    const int seconds = filter::date::seconds_since_epoch() - account_creation_times[username];
    const std::string days = std::to_string (seconds / (3600 * 24));
    
    // Pass information about this user to the flate engine for display.
    view.add_iteration ("tbody", {
      std::pair ("user", username),
      std::pair ("days", days),
      std::pair ("role", role),
      std::pair ("email", email),
    });
  }
  
  page += view.render ("manage", "accounts");

  page += assets_page::footer ();
  
  if (user_updated) notes_logic_maintain_note_assignees (true);
  if (privileges_updated) database_privileges_client_create (objectUsername, true);

  return page;
}

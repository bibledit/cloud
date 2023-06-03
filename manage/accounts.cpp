/*
Copyright (©) 2003-2023 Teus Benschop.

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
#include <dialog/list.h>
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
using namespace std;


string manage_accounts_url ()
{
  return "manage/accounts";
}


bool manage_accounts_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string manage_accounts (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  bool user_updated = false;
  bool privileges_updated = false;
  
  string page;
  Assets_Header header = Assets_Header (translate("Accounts"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  // The user to act on.
  string objectUsername = request->query["user"];
  int user_level = request->database_users ()->get_level (objectUsername);
  
  // Delete a user.
  if (request->query.count ("delete")) {
    string role = Filter_Roles::text (user_level);
    string email = request->database_users ()->get_email (objectUsername);
    vector <string> users = request->database_users ()->get_users ();
    vector <string> administrators = request->database_users ()->getAdministrators ();
    if (users.size () == 1) {
      page += assets_page::error (translate("Cannot remove the last user"));
    } else if ((user_level >= Filter_Roles::admin ()) && (administrators.size () == 1)) {
      page += assets_page::error (translate("Cannot remove the last administrator"));
    } else {
      string message;
      user_logic_delete_account (objectUsername, role, email, message);
      user_updated = true;
      message.append (" ");
      message.append ("See the Journal for progress");
      page += assets_page::success (message);
    }
  }
  
  // Get the account creation times.
  map <string, int> account_creation_times;
  {
    vector <string> lines = Database_Config_General::getAccountCreationTimes ();
    for (auto line : lines) {
      vector <string> bits = filter::strings::explode(line, '|');
      if (bits.size() != 2) continue;
      int seconds = filter::strings::convert_to_int(bits[0]);
      string user = bits[1];
      account_creation_times [user] = seconds;
    }
  }
  
  // Retrieve assigned users.
  vector <string> users = access_user::assignees (webserver_request);
  for (auto & username : users) {
    
    // Gather details for this user account.
    user_level = request->database_users ()->get_level (username);
    string role = Filter_Roles::text (user_level);
    string email = request->database_users ()->get_email (username);
    int seconds = filter::date::seconds_since_epoch() - account_creation_times[username];
    string days = filter::strings::convert_to_string (seconds / (3600 * 24));
    
    // Pass information about this user to the flate engine for display.
    view.add_iteration ("tbody", {
      pair ("user", username),
      pair ("days", days),
      pair ("role", role),
      pair ("email", email),
    });
  }
  
  page += view.render ("manage", "accounts");

  page += assets_page::footer ();
  
  if (user_updated) notes_logic_maintain_note_assignees (true);
  if (privileges_updated) database_privileges_client_create (objectUsername, true);

  return page;
}

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


#include <manage/accounts.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
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
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  bool user_updated = false;
  bool privileges_updated = false;
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Accounts"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;


  int myLevel = request->session_logic ()->currentLevel ();
  
  
  // New user creation. Todo not needed.
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("users", translate("Please enter a name for the new user"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("new")) {
    string user = request->post["entry"];
    if (request->database_users ()->usernameExists (user)) {
      page += Assets_Page::error (translate("User already exists"));
    } else {
      request->database_users ()->add_user(user, user, Filter_Roles::member (), "");
      user_updated = true;
      page += Assets_Page::success (translate("User created"));
    }
  }
  
  
  // The user to act on.
  string objectUsername = request->query["user"];
  int user_level = request->database_users ()->get_level (objectUsername);
  
  
  // Delete a user. Todo implement
  if (request->query.count ("delete")) {
    string role = Filter_Roles::text (user_level);
    string email = request->database_users ()->get_email (objectUsername);
    vector <string> users = request->database_users ()->getUsers ();
    vector <string> administrators = request->database_users ()->getAdministrators ();
    if (users.size () == 1) {
      page += Assets_Page::error (translate("Cannot remove the last user"));
    } else if ((user_level >= Filter_Roles::admin ()) && (administrators.size () == 1)) {
      page += Assets_Page::error (translate("Cannot remove the last administrator"));
    } else {
      string message = "Deleted user " + objectUsername + " with role " + role + " and email " + email;
      Database_Logs::log (message, Filter_Roles::admin ());
      request->database_users ()->removeUser (objectUsername);
      user_updated = true;
      database_privileges_client_remove (objectUsername);
      page += Assets_Page::success (message);
      // Also remove any privileges for this user.
      // In particular for the Bible privileges this is necessary,
      // beause if old users remain in the privileges storage,
      // then a situation where no user has any privileges to any Bible,
      // and thus all relevant users have all privileges,
      // can never be achieved again.
      Database_Privileges::removeUser (objectUsername);
      // Remove any login tokens the user might have had: Just to clean things up.
      Database_Login::removeTokens (objectUsername);
      // Remove any settings for the user.
      // The advantage of this is that when a user is removed, all settings are gone,
      // so when the same user would be created again, all settings will go back to their defaults.
      request->database_config_user ()->remove (objectUsername);
      // Remove note assignments for clients for this user.
      Database_NoteAssignment database_noteassignment;
      database_noteassignment.remove (objectUsername);
    }
  }
  
  
  // The user's role. Todo not needed.
  if (request->query.count ("level")) {
    string level = request->query ["level"];
    if (level == "") {
      Dialog_List dialog_list = Dialog_List ("users", translate("Select a role for") + " " + objectUsername, "", "");
      dialog_list.add_query ("user", objectUsername);
      for (int i = Filter_Roles::lowest (); i <= Filter_Roles::highest (); i++) {
        if (i <= myLevel) {
          dialog_list.add_row (Filter_Roles::text (i), "level", convert_to_string (i));
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_users ()->set_level (objectUsername, convert_to_int (level));
      user_updated = true;
    }
  }
  
  
  // User's email address. Todo view only.
  if (request->query.count ("email")) {
    string email = request->query ["email"];
    if (email == "") {
      string question = translate("Please enter an email address for") + " " + objectUsername;
      string value = request->database_users ()->get_email (objectUsername);
      Dialog_Entry dialog_entry = Dialog_Entry ("users", question, value, "email", "");
      dialog_entry.add_query ("user", objectUsername);
      page += dialog_entry.run ();
      return page;
    }
  }
  if (request->post.count ("email")) {
    string email = request->post["entry"];
    if (filter_url_email_is_valid (email)) {
      page += Assets_Page::success (translate("Email address was updated"));
      request->database_users ()->updateUserEmail (objectUsername, email);
      user_updated = true;
    } else {
      page += Assets_Page::error (translate("The email address is not valid"));
    }
  }
  
  
  // Fetch all available Bibles. Todo not needed
  vector <string> allbibles = request->database_bibles ()->getBibles ();
  
  
  // Add Bible to user account. Todo not needed.
  if (request->query.count ("addbible")) {
    string addbible = request->query["addbible"];
    if (addbible == "") {
      Dialog_List dialog_list = Dialog_List ("users", translate("Would you like to grant the user access to a Bible?"), "", "");
      dialog_list.add_query ("user", objectUsername);
      for (auto bible : allbibles) {
        dialog_list.add_row (bible, "addbible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      Assets_Page::success (translate("The user has been granted access to this Bible"));
      // Write access depends on whether it's a translator role or higher.
      bool write = (user_level >= Filter_Roles::translator ());
      Database_Privileges::setBible (objectUsername, addbible, write);
      user_updated = true;
      privileges_updated = true;
    }
  }
  
  
  // Remove Bible from user. Todo goes out.
  if (request->query.count ("removebible")) {
    string removebible = request->query ["removebible"];
    Database_Privileges::removeBibleBook (objectUsername, removebible, 0);
    user_updated = true;
    privileges_updated = true;
    Assets_Page::success (translate("The user no longer has access to this Bible"));
  }
  
  
  // Enable or disable a user account. // Todo not needed.
  if (request->query.count ("enable")) {
    request->database_users ()->set_enabled (objectUsername, true);
    Assets_Page::success (translate("The user account was enabled"));
  }
  if (request->query.count ("disable")) {
    // Disable the user in the database.
    request->database_users ()->set_enabled (objectUsername, false);
    // Remove all login tokens (cookies) for this user, so the user no longer is logged in.
    Database_Login::removeTokens (objectUsername);
    // Feedback.
    Assets_Page::success (translate("The user account was disabled"));
  }
  
  
  // Login on behalf of another user. // Todo out.
  if (request->query.count ("login")) {
    request->session_logic ()->switchUser (objectUsername);
    redirect_browser (request, session_switch_url ());
    return "";
  }
  
  
  // Retrieve assigned users.
  vector <string> users = access_user_assignees (webserver_request);
  for (auto & username : users) {
    
    // Gather details for this user account.
    user_level = request->database_users ()->get_level (username);
    string role = Filter_Roles::text (user_level);
    string email = request->database_users ()->get_email (username);
    
    // In the Indonesian free Cloud,
    // the free guest accounts have a role of Consultant.
    // So only displays roles of Consultant or lower.
    // Skip any higher roles.
#ifdef HAVE_INDONESIANCLOUDFREE
    if (user_level > Filter_Roles::consultant()) continue;
#endif

    // Pass information about this user to the flate engine for display.
    view.add_iteration ("tbody", {
      make_pair ("user", username),
      make_pair ("role", role),
      make_pair ("email", email),
    });
  }
  
  page += view.render ("manage", "accounts");

  page += Assets_Page::footer ();
  
  if (user_updated) notes_logic_maintain_note_assignees (true);
  if (privileges_updated) database_privileges_client_create (objectUsername, true);

  return page;
}

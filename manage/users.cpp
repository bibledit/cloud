/*
Copyright (©) 2003-2022 Teus Benschop.

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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <manage/users.h>
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
#include <ldap/logic.h>
#include <user/logic.h>
#include <config/logic.h>
#include <sstream>
#include <iomanip>


string manage_users_url ()
{
  return "manage/users";
}


bool manage_users_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string manage_users (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  bool user_updated = false;
  bool privileges_updated = false;
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Users"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;


  int myLevel = request->session_logic ()->currentLevel ();
  
  
  // New user creation.
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
      user_logic_store_account_creation (user);
      user_updated = true;
      page += Assets_Page::success (translate("User created"));
    }
  }
  
  
  // The user to act on.
  string objectUsername = request->query["user"];
  int objectUserLevel = request->database_users ()->get_level (objectUsername);
  
  
  // Delete a user.
  if (request->query.count ("delete")) {
    string role = Filter_Roles::text (objectUserLevel);
    string email = request->database_users ()->get_email (objectUsername);
    vector <string> users = request->database_users ()->get_users ();
    vector <string> administrators = request->database_users ()->getAdministrators ();
    if (users.size () == 1) {
      page += Assets_Page::error (translate("Cannot remove the last user"));
    } else if ((objectUserLevel >= Filter_Roles::admin ()) && (administrators.size () == 1)) {
      page += Assets_Page::error (translate("Cannot remove the last administrator"));
    } else if (config_logic_demo_enabled () && (objectUsername ==  session_admin_credentials ())) {
      page += Assets_Page::error (translate("Cannot remove the demo admin"));
    } else {
      string message;
      user_logic_delete_account (objectUsername, role, email, message);
      user_updated = true;
      page += Assets_Page::success (message);
    }
  }
  
  
  // The user's role.
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
  
  
  // User's email address.
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
  
  
  // Fetch all available Bibles.
  vector <string> allbibles = request->database_bibles ()->getBibles ();
  
  
  // Add Bible to user account.
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
      bool write = (objectUserLevel >= Filter_Roles::translator ());
      Database_Privileges::setBible (objectUsername, addbible, write);
      user_updated = true;
      privileges_updated = true;
    }
  }
  
  
  // Remove Bible from user.
  if (request->query.count ("removebible")) {
    string removebible = request->query ["removebible"];
    Database_Privileges::removeBibleBook (objectUsername, removebible, 0);
    user_updated = true;
    privileges_updated = true;
    Assets_Page::success (translate("The user no longer has access to this Bible"));
  }
  
  
  // Enable or disable a user account.
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
  
  
  // Login on behalf of another user.
  if (request->query.count ("login")) {
    request->session_logic ()->switchUser (objectUsername);
    redirect_browser (request, session_switch_url ());
    return "";
  }
  
  
  // User accounts to display.
  stringstream tbody;
  bool ldap_on = ldap_logic_is_on ();
  // Retrieve assigned users.
  vector <string> users = access_user_assignees (webserver_request);
  for (auto & username : users) {
    
    // Gather details for this user account.
    objectUserLevel = request->database_users ()->get_level (username);
    string namedrole = Filter_Roles::text (objectUserLevel);
    string email = request->database_users ()->get_email (username);
    if (email == "") email = "--";
    bool enabled = request->database_users ()->get_enabled (username);
    
    // New row in table.
    tbody << "<tr>";
    
    // Display emoji to delete this account.
    tbody << "<td>";
    tbody << "<a href=" << quoted("?user=" + username + "&delete") << ">" << emoji_wastebasket () << "</a> " << username;
    tbody << "</td>";

    // Divider.
    tbody << "<td>│</td>";
    
    // The user's role is displayed when the account is enabled.
    // Normally the role can be changed, but when an LDAP server is enabled, it cannot be changed here.
    tbody << "<td>";
    if (enabled) {
      if (!ldap_on) tbody << "<a href=" << quoted ("?user=" + username + "&level") << ">";
      tbody << namedrole << "</a>";
      if (!ldap_on) tbody << "</a>";
    }
    tbody << "</td>";
    
    // Divider.
    tbody << "<td>│</td>";
    
    // The user's email address will be displayed when the account is enabled.
    // Normally the email address can be changed here,
    // but in case of authentication via an LDAP server, it cannot be changed here.
    tbody << "<td>";
    if (enabled) {
      if (!ldap_on) tbody << "<a href=" << quoted ("?user=" + username + "&email") << ">";
      tbody << email;
      if (!ldap_on) tbody << "</a>";
    }
    tbody << "</td>";
    
    // Divider.
    tbody << "<td>│</td>";
    
    // Assigned Bibles.
    tbody << "<td>";
    if (enabled) {
      if (objectUserLevel < Filter_Roles::manager ()) {
        for (auto & bible : allbibles) {
          bool exists = Database_Privileges::getBibleBookExists (username, bible, 0);
          if (exists) {
            auto [ read, write ] = Database_Privileges::getBible (username, bible);
            if  (objectUserLevel >= Filter_Roles::translator ()) write = true;
            tbody << "<a href=" << quoted ("?user=" + username + "&removebible=" + bible) << ">" << emoji_wastebasket () << "</a>";
            tbody << "<a href=" << quoted("/bible/settings?bible=" + bible) << ">" << bible << "</a>";
            tbody << "<a href=" << quoted("write?user=" + username + "&bible=" + bible) << ">";
            int readwritebooks = 0;
            vector <int> books = request->database_bibles ()->getBooks (bible);
            for (auto book : books) {
              Database_Privileges::getBibleBook (username, bible, book, read, write);
              if (write) readwritebooks++;
            }
            tbody << "(" << readwritebooks << "/" << books.size () << ")";
            tbody << "</a>";
            tbody << "|";
          }
        }
      }
      if (objectUserLevel >= Filter_Roles::manager ()) {
        // Managers and higher roles have access to all Bibles.
        tbody << "(" << translate ("all") << ")";
      } else {
        tbody << "<a href=" << quoted("?user=" + username + "&addbible=") << ">" << emoji_heavy_plus_sign () << "</a>";
      }
    }
    tbody << "</td>";

    // Divider.
    tbody << "<td>│</td>";

    // Assigned privileges to the user.
    tbody << "<td>";
    if (enabled) {
      if (objectUserLevel >= Filter_Roles::manager ()) {
        // Managers and higher roles have all privileges.
        tbody << "(" << translate ("all") << ")";
      } else {
        tbody << "<a href=" << quoted("privileges?user=" + username) << ">" << translate ("edit") << "</a>";
      }
    }
    tbody << "</td>";
    
    // Disable or enable the account.
    if (myLevel >= Filter_Roles::manager ()) {
      if (myLevel > objectUserLevel) {
        tbody << "<td>│</td>";
        tbody << "<td>";
        bool enabled = request->database_users ()->get_enabled (username);
        if (enabled) {
          tbody << "<a href=" << quoted("?user=" + username + "&disable") << ">" << translate ("Disable") << "</a>";
        } else {
          tbody << "<a href=" << quoted("?user=" + username + "&enable") << ">" << translate ("Enable") << "</a>";
        }
        tbody << "</td>";
      }
    }

    // Login on behalf of another user.
    if (enabled) {
      if (myLevel > objectUserLevel) {
        tbody << "<td>│</td>";
        tbody << "<td>";
        tbody << "<a href=" << quoted ("?user=" + username + "&login") << ">" << translate ("Login") << "</a>";
        tbody << "</td>";
      }
    }
    
    // Done with the row.
    tbody << "</tr>";
  }

  view.set_variable ("tbody", tbody.str());

  if (!ldap_on) {
    view.enable_zone ("local");
  }

  if (config_logic_indonesian_cloud_free ()) {
    view.enable_zone("accounts");
  }

  page += view.render ("manage", "users");

  page += Assets_Page::footer ();
  
  if (user_updated) notes_logic_maintain_note_assignees (true);
  if (privileges_updated) database_privileges_client_create (objectUsername, true);

  return page;
}

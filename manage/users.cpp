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


#include <manage/users.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <dialog/entry.h>
#include <dialog/select.h>
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
#include <access/logic.h>
#include <locale/translate.h>
#include <notes/logic.h>
#include <menu/logic.h>
#include <session/switch.h>
#include <ldap/logic.h>
#include <user/logic.h>


std::string manage_users_url ()
{
  return "manage/users";
}


bool manage_users_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string manage_users (Webserver_Request& webserver_request)
{
  bool user_updated = false;
  bool privileges_updated = false;
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Users"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;


  const int my_level = webserver_request.session_logic ()->get_level();
  const std::string my_user = webserver_request.session_logic()->get_username();

  
  constexpr const char* level_identification {"level"};


  // The default new user role.
  {
    constexpr const char* identification {"defaultacl"};
    if (webserver_request.post_count(identification)) {
      const std::string value {webserver_request.post_get(identification)};
      const int defaultacl = filter::strings::convert_to_int (value);
      database::config::general::set_default_new_user_access_level(defaultacl);
      return std::string();
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = { std::to_string(roles::guest), std::to_string(roles::member) },
      .displayed = { roles::english(roles::guest), roles::english(roles::member) },
      .selected = std::to_string (database::config::general::get_default_new_user_access_level()),
    };
    view.set_variable(identification, dialog::select::ajax(settings));
  }

  
  // New user creation.
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("users", translate("Please enter a name for the new user"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post_count("new")) {
    std::string user = webserver_request.post_get("entry");
    if (webserver_request.database_users ()->usernameExists (user)) {
      page += assets_page::error (translate("User already exists"));
    } else {

      // Set the role of the new created user, it is set as member if no
      // default has been set by an administrator.
      int role = database::config::general::get_default_new_user_access_level ();
      webserver_request.database_users ()->add_user(user, user, role, "");

      // Set default privileges on new created user.
      std::set <std::string> defusers = access_logic::default_privilege_usernames ();
      std::vector <int> privileges = {PRIVILEGE_VIEW_RESOURCES, PRIVILEGE_VIEW_NOTES, PRIVILEGE_CREATE_COMMENT_NOTES};
      auto default_username = next(defusers.begin(), static_cast<long>(role + 1));
      for (auto & privilege : privileges) {
        bool state = DatabasePrivileges::get_feature (*default_username, privilege);
        DatabasePrivileges::set_feature (user, privilege, state);
      }

      const bool deletenotes = webserver_request.database_config_user ()->get_privilege_delete_consultation_notes_for_user (*default_username);
      const bool useadvancedmode = webserver_request.database_config_user ()->get_privilege_use_advanced_mode_for_user (*default_username);
      const bool editstylesheets = webserver_request.database_config_user ()->get_privilege_set_stylesheets_for_user (*default_username);

      if (deletenotes)
        webserver_request.database_config_user ()->set_privilege_delete_consultation_notes_for_user (user, 1);
      if (useadvancedmode)
        webserver_request.database_config_user ()->set_privilege_use_advanced_mode_for_user (user, 1);
      if (editstylesheets)
        webserver_request.database_config_user ()->set_privilege_set_stylesheets_for_user (user, 1);

      page += assets_page::error (*default_username);

      user_logic_store_account_creation (user);
      user_updated = true;
      page += assets_page::success (translate("User created"));
    }
  }
  
  
  // The user to act on.
  const std::string object_username = webserver_request.query["user"];
  int object_user_level = webserver_request.database_users ()->get_level (object_username);
  
  
  // Delete a user.
  if (webserver_request.query.count ("delete")) {
    std::string role = roles::text (object_user_level);
    std::string email = webserver_request.database_users ()->get_email (object_username);
    std::vector <std::string> users = webserver_request.database_users ()->get_users ();
    std::vector <std::string> administrators = webserver_request.database_users ()->getAdministrators ();
    if (users.size () == 1) {
      page += assets_page::error (translate("Cannot remove the last user"));
    } else if ((object_user_level >= roles::admin) && (administrators.size () == 1)) {
      page += assets_page::error (translate("Cannot remove the last administrator"));
    } else if (config::logic::demo_enabled () && (object_username ==  session_admin_credentials ())) {
      page += assets_page::error (translate("Cannot remove the demo admin"));
    } else {
      std::string message;
      user_logic_delete_account (object_username, role, email, message);
      user_updated = true;
      page += assets_page::success (message);
    }
  }
  
  
  // The user's role.
  {
    const std::vector <std::string> users = access_user::assignees (webserver_request);
    for (unsigned int u {0}; u < users.size(); u++) {
      const std::string identification = level_identification + std::to_string(u);
      if (webserver_request.post_count(identification)) {
        const std::string value = webserver_request.post_get(identification);
        webserver_request.database_users ()->set_level (object_username, filter::strings::convert_to_int (value));
      }
    }
  }


  // User's email address.
  if (webserver_request.query.count ("email")) {
    std::string email = webserver_request.query ["email"];
    if (email == "") {
      std::string question = translate("Please enter an email address for") + " " + object_username;
      std::string value = webserver_request.database_users ()->get_email (object_username);
      Dialog_Entry dialog_entry = Dialog_Entry ("users", question, value, "email", "");
      dialog_entry.add_query ("user", object_username);
      page += dialog_entry.run ();
      return page;
    }
  }
  if (webserver_request.post_count("email")) {
    std::string email = webserver_request.post_get("entry");
    if (filter_url_email_is_valid (email)) {
      page += assets_page::success (translate("Email address was updated"));
      webserver_request.database_users ()->updateUserEmail (object_username, email);
      user_updated = true;
    } else {
      page += assets_page::error (translate("The email address is not valid"));
    }
  }
  
  
  // Fetch all available Bibles.
  const std::vector <std::string> allbibles = database::bibles::get_bibles ();
  
  
  // Enable or disable a user account.
  if (webserver_request.query.count ("enable")) {
    webserver_request.database_users ()->set_enabled (object_username, true);
    assets_page::success (translate("The user account was enabled"));
  }
  if (webserver_request.query.count ("disable")) {
    // Disable the user in the database.
    webserver_request.database_users ()->set_enabled (object_username, false);
    // Remove all login tokens (cookies) for this user, so the user no longer is logged in.
    Database_Login::removeTokens (object_username);
    // Feedback.
    assets_page::success (translate("The user account was disabled"));
  }
  
  
  // Login on behalf of another user.
  if (webserver_request.query.count ("login")) {
    webserver_request.session_logic ()->switch_user (object_username);
    redirect_browser (webserver_request, session_switch_url ());
    return std::string();
  }
  
  
  // User accounts to display.
  std::stringstream tbody;
  bool ldap_on = ldap_logic_is_on ();
  // Retrieve the assigned users.
  const std::vector <std::string> users = access_user::assignees (webserver_request);
  // The offset within the users list is needed below, hence this way of iterating the users container.
  for (unsigned int u {0}; u < users.size(); u++) {
    const auto& username = users.at(u);
    
    // Gather details for this user account.
    object_user_level = webserver_request.database_users()->get_level (username);
    std::string namedrole = roles::text (object_user_level);
    std::string email = webserver_request.database_users()->get_email (username);
    if (email.empty())
      email = "--";
    bool enabled = webserver_request.database_users()->get_enabled (username);
    
    // Start a new table row.
    tbody << "<tr>";
    
    // Display emoji to delete this account.
    tbody << "<td>";
    tbody << "<a href=" << std::quoted("?user=" + username + "&delete") << ">" << filter::strings::emoji_wastebasket () << "</a> " << username;
    tbody << "</td>";

    // Divider.
    tbody << "<td>│</td>";
    
    // The user's role is displayed when the account is enabled.
    // Normally the role can be changed, but when an LDAP server is enabled, it cannot be changed here.
    tbody << "<td>";
    if (enabled) {
      std::vector<std::string> values;
      std::vector<std::string> displayed;
      for (int i = roles::lowest; i <= roles::highest; i++) {
        if (i <= my_level) {
          values.push_back(std::to_string(i));
          displayed.push_back(roles::text(i));
        }
      }
      // The "id" of elements should be unique, create that here.
      const std::string identification = level_identification + std::to_string(u);
      dialog::select::Settings settings {
        .identification = identification.c_str(),
        .values = std::move(values),
        .displayed = std::move(displayed),
        .selected = std::to_string(object_user_level),
        .parameters = { {"user", username} },
        // Handle LDAP users. A user cannot change his own role to prevent locking himself out of an appropriate role.
        .disabled = ldap_on or (username == my_user),
        .tooltip = translate("Select a role"),
      };
      dialog::select::Form form { .auto_submit = true };
      tbody << dialog::select::form(settings, form);
    }
    tbody << "</td>";
    
    // Divider.
    tbody << "<td>│</td>";
    
    // The user's email address will be displayed when the account is enabled.
    // Normally the email address can be changed here,
    // but in case of authentication via an LDAP server, it cannot be changed here.
    tbody << "<td>";
    if (enabled) {
      if (!ldap_on) tbody << "<a href=" << std::quoted ("?user=" + username + "&email") << ">";
      tbody << email;
      if (!ldap_on) tbody << "</a>";
    }
    tbody << "</td>";
    
    // Divider.
    tbody << "<td>│</td>";
    
    // Assigned Bibles.
    tbody << "<td>";
    if (enabled) {
      if (object_user_level >= roles::manager) {
        // Managers and higher roles have access to all Bibles.
        tbody << "(" << translate ("all") << ")";
      } else {
        tbody << "<a href=" << std::quoted("bibles?user=" + username) << ">" << translate ("edit") << "</a>";
      }
    }
    tbody << "</td>";

    // Divider.
    tbody << "<td>│</td>";

    // Assigned privileges to the user.
    tbody << "<td>";
    if (enabled) {
      if (object_user_level >= roles::manager) {
        // Managers and higher roles have all privileges.
        tbody << "(" << translate ("all") << ")";
      } else {
        tbody << "<a href=" << std::quoted("privileges?user=" + username) << ">" << translate ("edit") << "</a>";
      }
    }
    tbody << "</td>";
    
    // Disable or enable the account.
    if (my_level >= roles::manager) {
      if (my_level > object_user_level) {
        tbody << "<td>│</td>";
        tbody << "<td>";
        bool account_enabled = webserver_request.database_users ()->get_enabled (username);
        if (account_enabled) {
          tbody << "<a href=" << std::quoted("?user=" + username + "&disable") << ">" << translate ("Disable") << "</a>";
        } else {
          tbody << "<a href=" << std::quoted("?user=" + username + "&enable") << ">" << translate ("Enable") << "</a>";
        }
        tbody << "</td>";
      }
    }

    // Login on behalf of another user.
    if (enabled) {
      if (my_level > object_user_level) {
        tbody << "<td>│</td>";
        tbody << "<td>";
        tbody << "<a href=" << std::quoted ("?user=" + username + "&login") << ">" << translate ("Login") << "</a>";
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

  if (webserver_request.session_logic()->get_level () == roles::highest) view.enable_zone ("admin_settings");

  page.append(view.render("manage", "users"));

  page.append(assets_page::footer());
  
  if (user_updated)
    notes_logic_maintain_note_assignees (true);
  if (privileges_updated)
    database_privileges_client_create (object_username, true);

  return page;
}

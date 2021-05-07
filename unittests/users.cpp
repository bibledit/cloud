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


#include <unittests/users.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/md5.h>


void test_users ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  // Tests for a user's identifier.
  {
    Webserver_Request request;
    request.database_users ()->create ();
    request.session_logic()->setUsername ("phpunit");
    evaluate (__LINE__, __func__, 13683715, filter_string_user_identifier (&request));
    request.session_logic()->setUsername ("phpunit2");
    evaluate (__LINE__, __func__, 13767813, filter_string_user_identifier (&request));
  }
  refresh_sandbox (true);
}


void test_database_users ()
{
  trace_unit_tests (__func__);
  
  // Tests for Database_Users.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    string username = "unit test";
    string password = "pazz";
    int level = 10;
    string email = "email@site.nl";
    
    database_users.optimize ();
    database_users.trim ();
    
    database_users.add_user (username, password, level, email);
    
    evaluate (__LINE__, __func__, true, database_users.matchUserPassword (username, password));
    evaluate (__LINE__, __func__, false, database_users.matchUserPassword (username, "wrong password"));
    
    evaluate (__LINE__, __func__, true, database_users.matchEmailPassword (email, password));
    evaluate (__LINE__, __func__, false, database_users.matchEmailPassword (email, "wrong password"));
    
    // No matches for a disabled account.
    database_users.set_enabled (username, false);
    evaluate (__LINE__, __func__, false, database_users.matchUserPassword (username, password));
    evaluate (__LINE__, __func__, false, database_users.matchEmailPassword (email, password));
    
    string ref = "INSERT INTO users (username, password, level, email) VALUES ('unit test', '014877e71841e82d44ce524d66dcc732', 10, 'email@site.nl');";
    string act = database_users.add_userQuery (username, password, level, email);
    evaluate (__LINE__, __func__, ref, act);
    
    evaluate (__LINE__, __func__, username, database_users.getEmailToUser (email));
    evaluate (__LINE__, __func__, "", database_users.getEmailToUser ("wrong email"));
    
    evaluate (__LINE__, __func__, email, database_users.get_email (username));
    evaluate (__LINE__, __func__, "", database_users.get_email ("wrong username"));
    
    evaluate (__LINE__, __func__, true, database_users.usernameExists (username));
    evaluate (__LINE__, __func__, false, database_users.usernameExists ("invalid username"));
    
    evaluate (__LINE__, __func__, true, database_users.emailExists (email));
    evaluate (__LINE__, __func__, false, database_users.emailExists ("invalid email"));
    
    evaluate (__LINE__, __func__, level, database_users.get_level (username));
    evaluate (__LINE__, __func__, Filter_Roles::guest (), database_users.get_level ("invalid username"));
    
    level = 7;
    database_users.set_level (username, level);
    evaluate (__LINE__, __func__, level, database_users.get_level (username));
    
    database_users.removeUser (username);
    evaluate (__LINE__, __func__, false, database_users.usernameExists (username));
    
    evaluate (__LINE__, __func__, " UPDATE users SET email =  'email@site.nl'  WHERE username =  'unit test'  ; ", database_users.updateEmailQuery (username, email));
  }
  
  // Test administrators and updating email.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // Test data for two admins.
    string username1 = "unit test1";
    string username2 = "unit test2";
    string password = "pazz";
    int level = Filter_Roles::admin();
    string email = "email@site";
    
    database_users.add_user (username1, password, level, email);
    vector <string> admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 1, (int)admins.size());
    if (!admins.empty()) evaluate (__LINE__, __func__, username1, admins [0]);
    
    database_users.add_user (username2, password, level, email);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 2, (int)admins.size());
    
    // Check that a disabled admin account is not included in the number of administrators.
    database_users.set_enabled (username1, false);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 1, (int)admins.size());
    
    // Check that once an account is enabled, it is included again in the number of administrators.
    database_users.set_enabled (username1, true);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 2, (int)admins.size());
    
    email = "new@email.address";
    database_users.updateUserEmail (username1, email);
    evaluate (__LINE__, __func__, email, database_users.get_email (username1));
    
    vector <string> users = database_users.get_users ();
    evaluate (__LINE__, __func__, 2, (int)users.size());
    
    evaluate (__LINE__, __func__, md5 (password), database_users.get_md5 (username1));
  }
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // LDAP should be off initially.
    string user = "unittest";
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
    database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
    
    // Test LDAP on.
    database_users.set_ldap (user + "x", true);
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
    database_users.set_ldap (user, true);
    evaluate (__LINE__, __func__, true, database_users.get_ldap (user));
    
    // Test LDAP off.
    database_users.set_ldap (user, false);
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
  }
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // Non-existing account is disabled.
    string user = "unittest";
    evaluate (__LINE__, __func__, false, database_users.get_enabled (user));
    
    // Account should be enabled initially.
    database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
    
    // Test disable account of other user.
    database_users.set_enabled (user + "x", false);
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
    
    // Test disable account.
    database_users.set_enabled (user, false);
    evaluate (__LINE__, __func__, false, database_users.get_enabled (user));
    
    // Test enable account.
    database_users.set_enabled (user, true);
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
  }
}

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/users.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/md5.h>


// Tests for a user's identifier.
TEST (session, users)
{
  refresh_sandbox (true);
  Webserver_Request request;
  request.database_users ()->create ();
  request.session_logic()->set_username ("phpunit");
  EXPECT_EQ (13683715, filter::strings::user_identifier (&request));
  request.session_logic()->set_username ("phpunit2");
  EXPECT_EQ (13767813, filter::strings::user_identifier (&request));
  refresh_sandbox (true);
}


// Tests for Database_Users.
TEST (database, users1)
{
  refresh_sandbox (true);
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  std::string username = "unit test";
  std::string password = "pazz";
  int level = 10;
  std::string email = "email@site.nl";
  
  database_users.optimize ();
  database_users.trim ();
  
  database_users.add_user (username, password, level, email);
  
  EXPECT_TRUE (database_users.matchUserPassword (username, password));
  EXPECT_FALSE (database_users.matchUserPassword (username, "wrong password"));
  
  EXPECT_TRUE (database_users.matchEmailPassword (email, password));
  EXPECT_FALSE (database_users.matchEmailPassword (email, "wrong password"));
  
  // No matches for a disabled account.
  database_users.set_enabled (username, false);
  EXPECT_FALSE (database_users.matchUserPassword (username, password));
  EXPECT_FALSE (database_users.matchEmailPassword (email, password));
  
  std::string ref = "INSERT INTO users (username, password, level, email) VALUES ('unit test', '014877e71841e82d44ce524d66dcc732', 10, 'email@site.nl');";
  std::string act = database_users.add_userQuery (username, password, level, email);
  EXPECT_EQ (ref, act);
  
  EXPECT_EQ (username, database_users.getEmailToUser (email));
  EXPECT_EQ (std::string(), database_users.getEmailToUser ("wrong email"));
  
  EXPECT_EQ (email, database_users.get_email (username));
  EXPECT_EQ (std::string(), database_users.get_email ("wrong username"));
  
  EXPECT_TRUE (database_users.usernameExists (username));
  EXPECT_FALSE (database_users.usernameExists ("invalid username"));
  
  EXPECT_TRUE (database_users.emailExists (email));
  EXPECT_FALSE (database_users.emailExists ("invalid email"));
  
  EXPECT_EQ (level, database_users.get_level (username));
  EXPECT_EQ (Filter_Roles::guest (), database_users.get_level ("invalid username"));
  
  level = 7;
  database_users.set_level (username, level);
  EXPECT_EQ (level, database_users.get_level (username));
  
  database_users.removeUser (username);
  EXPECT_FALSE (database_users.usernameExists (username));
  
  EXPECT_EQ (" UPDATE users SET email =  'email@site.nl'  WHERE username =  'unit test'  ; ", database_users.updateEmailQuery (username, email));
}

// Test administrators and updating email.
TEST (database, users2)
{
  refresh_sandbox (true);
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  // Test data for two admins.
  std::string username1 = "unit test1";
  std::string username2 = "unit test2";
  std::string password = "pazz";
  int level = Filter_Roles::admin();
  std::string email = "email@site";
  
  database_users.add_user (username1, password, level, email);
  std::vector <std::string> admins = database_users.getAdministrators ();
  EXPECT_EQ (1, static_cast<int> (admins.size()));
  if (!admins.empty()) EXPECT_EQ (username1, admins [0]);
  
  database_users.add_user (username2, password, level, email);
  admins = database_users.getAdministrators ();
  EXPECT_EQ (2, static_cast<int> (admins.size()));
  
  // Check that a disabled admin account is not included in the number of administrators.
  database_users.set_enabled (username1, false);
  admins = database_users.getAdministrators ();
  EXPECT_EQ (1, static_cast<int> (admins.size()));
  
  // Check that once an account is enabled, it is included again in the number of administrators.
  database_users.set_enabled (username1, true);
  admins = database_users.getAdministrators ();
  EXPECT_EQ (2, static_cast<int> (admins.size()));
  
  email = "new@email.address";
  database_users.updateUserEmail (username1, email);
  EXPECT_EQ (email, database_users.get_email (username1));
  
  std::vector <std::string> users = database_users.get_users ();
  EXPECT_EQ (2, static_cast<int>(users.size()));
  
  EXPECT_EQ (md5 (password), database_users.get_md5 (username1));
}


TEST (database, users3)
{
  refresh_sandbox (true);
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  // LDAP should be off initially.
  std::string user = "unittest";
  EXPECT_FALSE (database_users.get_ldap (user));
  database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
  EXPECT_FALSE ( database_users.get_ldap (user));
  
  // Test LDAP on.
  database_users.set_ldap (user + "x", true);
  EXPECT_FALSE (database_users.get_ldap (user));
  database_users.set_ldap (user, true);
  EXPECT_TRUE (database_users.get_ldap (user));
  
  // Test LDAP off.
  database_users.set_ldap (user, false);
  EXPECT_FALSE (database_users.get_ldap (user));
}


TEST (database, users4)
{
  refresh_sandbox (true);
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  
  // Non-existing account is disabled.
  std::string user = "unittest";
  EXPECT_FALSE (database_users.get_enabled (user));
  
  // Account should be enabled initially.
  database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
  EXPECT_TRUE (database_users.get_enabled (user));
  
  // Test disable account of other user.
  database_users.set_enabled (user + "x", false);
  EXPECT_TRUE (database_users.get_enabled (user));
  
  // Test disable account.
  database_users.set_enabled (user, false);
  EXPECT_FALSE (database_users.get_enabled (user));
  
  // Test enable account.
  database_users.set_enabled (user, true);
  EXPECT_TRUE (database_users.get_enabled (user));
}


#endif


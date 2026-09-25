/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wcharacter-conversion"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/md5.h>


// Tests for a user's identifier.
TEST (session, users)
{
  refresh_sandbox (false);
  Webserver_Request webserver_request;
  database::users::create();
  webserver_request.session_logic()->set_username ("phpunit");
  EXPECT_EQ (13683715, filter::string::user_identifier (webserver_request));
  webserver_request.session_logic()->set_username ("phpunit2");
  EXPECT_EQ (13767813, filter::string::user_identifier (webserver_request));
  refresh_sandbox (true);
}


// Tests for Database_Users.
TEST (database, users1)
{
  refresh_sandbox (false);
  database::users::create ();
  database::users::upgrade ();
  
  std::string username = "unit test";
  std::string password = "pazz";
  int level = 10;
  std::string email = "email@site.nl";
  
  database::users::optimize ();
  database::users::trim ();
  
  database::users::add_user (username, password, level, email);
  
  EXPECT_TRUE (database::users::match_user_password (username, password));
  EXPECT_FALSE (database::users::match_user_password (username, "wrong password"));
  
  EXPECT_TRUE (database::users::match_email_password (email, password));
  EXPECT_FALSE (database::users::match_email_password (email, "wrong password"));
  
  // No matches for a disabled account.
  database::users::set_enabled (username, false);
  EXPECT_FALSE (database::users::match_user_password (username, password));
  EXPECT_FALSE (database::users::match_email_password (email, password));
  
  std::string ref = "INSERT INTO users (username, password, level, email) VALUES ('unit test', '014877e71841e82d44ce524d66dcc732', 10, 'email@site.nl');";
  std::string act = database::users::add_user_query (username, password, level, email);
  EXPECT_EQ (ref, act);
  
  EXPECT_EQ (username, database::users::get_email_to_user (email));
  EXPECT_EQ (std::string(), database::users::get_email_to_user ("wrong email"));
  
  EXPECT_EQ (email, database::users::get_email (username));
  EXPECT_EQ (std::string(), database::users::get_email ("wrong username"));
  
  EXPECT_TRUE (database::users::username_exists (username));
  EXPECT_FALSE (database::users::username_exists ("invalid username"));
  
  EXPECT_TRUE (database::users::email_exists (email));
  EXPECT_FALSE (database::users::email_exists ("invalid email"));
  
  EXPECT_EQ (level, database::users::get_level (username));
  EXPECT_EQ (roles::guest, database::users::get_level ("invalid username"));
  
  level = 7;
  database::users::set_level (username, level);
  EXPECT_EQ (level, database::users::get_level (username));
  
  database::users::remove_user (username);
  EXPECT_FALSE (database::users::username_exists (username));
  
  EXPECT_EQ (" UPDATE users SET email =  'email@site.nl'  WHERE username =  'unit test'  ; ", database::users::update_email_query (username, email));
}

// Test administrators and updating email.
TEST (database, users2)
{
  refresh_sandbox (false);
  database::users::create ();
  database::users::upgrade ();
  
  // Test data for two admins.
  std::string username1 = "unit test1";
  std::string username2 = "unit test2";
  std::string password = "pazz";
  int level = roles::admin;
  std::string email = "email@site";
  
  database::users::add_user (username1, password, level, email);
  std::vector <std::string> admins = database::users::get_administrators ();
  EXPECT_EQ (1, static_cast<int> (admins.size()));
  if (not admins.empty())
  {
      EXPECT_EQ (username1, admins [0]);
  }
  
  database::users::add_user (username2, password, level, email);
  admins = database::users::get_administrators ();
  EXPECT_EQ (2, static_cast<int> (admins.size()));
  
  // Check that a disabled admin account is not included in the number of administrators.
  database::users::set_enabled (username1, false);
  admins = database::users::get_administrators ();
  EXPECT_EQ (1, static_cast<int> (admins.size()));
  
  // Check that once an account is enabled, it is included again in the number of administrators.
  database::users::set_enabled (username1, true);
  admins = database::users::get_administrators ();
  EXPECT_EQ (2, static_cast<int> (admins.size()));
  
  email = "new@email.address";
  database::users::update_user_email (username1, email);
  EXPECT_EQ (email, database::users::get_email (username1));
  
  std::vector <std::string> users = database::users::get_users ();
  EXPECT_EQ (2, static_cast<int>(users.size()));
  
  EXPECT_EQ (md5 (password), database::users::get_md5 (username1));
}


TEST (database, users3)
{
  refresh_sandbox (false);
  database::users::create ();
  database::users::upgrade ();
  
  // LDAP should be off initially.
  std::string user = "unittest";
  EXPECT_FALSE (database::users::get_ldap (user));
  database::users::add_user (user, "password", roles::consultant, "email@site");
  EXPECT_FALSE ( database::users::get_ldap (user));
  
  // Test LDAP on.
  database::users::set_ldap (user + "x", true);
  EXPECT_FALSE (database::users::get_ldap (user));
  database::users::set_ldap (user, true);
  EXPECT_TRUE (database::users::get_ldap (user));
  
  // Test LDAP off.
  database::users::set_ldap (user, false);
  EXPECT_FALSE (database::users::get_ldap (user));
}


TEST (database, users4)
{
  refresh_sandbox (false);
  database::users::create ();
  database::users::upgrade ();
  
  // Non-existing account is disabled.
  std::string user = "unittest";
  EXPECT_FALSE (database::users::get_enabled (user));
  
  // Account should be enabled initially.
  database::users::add_user (user, "password", roles::consultant, "email@site");
  EXPECT_TRUE (database::users::get_enabled (user));
  
  // Test disable account of other user.
  database::users::set_enabled (user + "x", false);
  EXPECT_TRUE (database::users::get_enabled (user));
  
  // Test disable account.
  database::users::set_enabled (user, false);
  EXPECT_FALSE (database::users::get_enabled (user));
  
  // Test enable account.
  database::users::set_enabled (user, true);
  EXPECT_TRUE (database::users::get_enabled (user));
}


#endif


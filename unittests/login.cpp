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
#include <database/login.h>
#include <database/sqlite.h>
#include <filter/url.h>


TEST (database, login)
{
  {
    refresh_sandbox (false);
    database::login::create ();
    const std::string path = database::sqlite::get_file (database::login::database ());
    filter_url_file_put_contents (path, "damaged database");
    EXPECT_EQ (false, database::login::healthy ());
    database::login::optimize ();
    EXPECT_EQ (true, database::login::healthy ());
    refresh_sandbox (false);
  }
  
  refresh_sandbox (true);
  database::login::create ();
  database::login::optimize ();
  
  const std::string username = "unittest";
  const std::string username2 = "unittest2";
  const std::string address = "192.168.1.0";
  const std::string agent = "Browser's user agent";
  const std::string fingerprint = "ԴԵԶԸ";
  const std::string cookie = "abcdefghijklmnopqrstuvwxyz";
  const std::string cookie2 = "abcdefghijklmnopqrstuvwxyzabc";
  bool daily;
  
  // Testing whether setting tokens and reading the username, and removing the tokens works.
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, database::login::get_username (cookie, daily));
  database::login::remove_tokens (username);
  EXPECT_EQ ("", database::login::get_username (cookie, daily));
  
  // Testing whether a persistent login gets removed after about a year.
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, database::login::get_username (cookie, daily));
  database::login::test_timestamp ();
  database::login::trim ();
  EXPECT_EQ ("", database::login::get_username (cookie, daily));
  
  // Testing whether storing touch enabled
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (true, database::login::get_touch_enabled (cookie));
  database::login::remove_tokens (username);
  EXPECT_EQ (false, database::login::get_touch_enabled (cookie));
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (false, database::login::get_touch_enabled (cookie + "x"));
  
  // Testing that removing tokens for one set does not remove all tokens for a user.
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, database::login::get_username (cookie, daily));
  database::login::set_tokens (username, address, agent, fingerprint, cookie2, true);
  EXPECT_EQ (username, database::login::get_username (cookie2, daily));
  database::login::remove_tokens (username, cookie2);
  EXPECT_EQ (username, database::login::get_username (cookie, daily));
  EXPECT_EQ ("", database::login::get_username (cookie2, daily));
  
  // Test moving tokens to a new username.
  database::login::remove_tokens (username);
  database::login::set_tokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, database::login::get_username (cookie, daily));
  database::login::rename_tokens (username, username2, cookie);
  EXPECT_EQ (username2, database::login::get_username (cookie, daily));
}


#endif


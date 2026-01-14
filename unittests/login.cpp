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
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/login.h>
#include <database/sqlite.h>
#include <filter/url.h>


TEST (database, login)
{
  {
    refresh_sandbox (false);
    Database_Login::create ();
    const std::string path = database::sqlite::get_file (Database_Login::database ());
    filter_url_file_put_contents (path, "damaged database");
    EXPECT_EQ (false, Database_Login::healthy ());
    Database_Login::optimize ();
    EXPECT_EQ (true, Database_Login::healthy ());
    refresh_sandbox (false);
  }
  
  refresh_sandbox (true);
  Database_Login::create ();
  Database_Login::optimize ();
  
  const std::string username = "unittest";
  const std::string username2 = "unittest2";
  const std::string address = "192.168.1.0";
  const std::string agent = "Browser's user agent";
  const std::string fingerprint = "ԴԵԶԸ";
  const std::string cookie = "abcdefghijklmnopqrstuvwxyz";
  const std::string cookie2 = "abcdefghijklmnopqrstuvwxyzabc";
  bool daily;
  
  // Testing whether setting tokens and reading the username, and removing the tokens works.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, Database_Login::getUsername (cookie, daily));
  Database_Login::removeTokens (username);
  EXPECT_EQ ("", Database_Login::getUsername (cookie, daily));
  
  // Testing whether a persistent login gets removed after about a year.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, Database_Login::getUsername (cookie, daily));
  Database_Login::testTimestamp ();
  Database_Login::trim ();
  EXPECT_EQ ("", Database_Login::getUsername (cookie, daily));
  
  // Testing whether storing touch enabled
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (true, Database_Login::getTouchEnabled (cookie));
  Database_Login::removeTokens (username);
  EXPECT_EQ (false, Database_Login::getTouchEnabled (cookie));
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (false, Database_Login::getTouchEnabled (cookie + "x"));
  
  // Testing that removing tokens for one set does not remove all tokens for a user.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, Database_Login::getUsername (cookie, daily));
  Database_Login::setTokens (username, address, agent, fingerprint, cookie2, true);
  EXPECT_EQ (username, Database_Login::getUsername (cookie2, daily));
  Database_Login::removeTokens (username, cookie2);
  EXPECT_EQ (username, Database_Login::getUsername (cookie, daily));
  EXPECT_EQ ("", Database_Login::getUsername (cookie2, daily));
  
  // Test moving tokens to a new username.
  Database_Login::removeTokens (username);
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  EXPECT_EQ (username, Database_Login::getUsername (cookie, daily));
  Database_Login::renameTokens (username, username2, cookie);
  EXPECT_EQ (username2, Database_Login::getUsername (cookie, daily));
}


#endif


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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/confirm.h>
#include <filter/string.h>


TEST (database, confirm)
{
#ifdef HAVE_CLOUD
  
  refresh_sandbox (false);
  database::confirm::create ();
  database::confirm::upgrade();
  database::confirm::optimize ();
  database::confirm::trim ();
  
  // New ID generation test.
  unsigned int id = database::confirm::get_new_id ();
  if (id < 10'000) EXPECT_EQ (std::string("Should be greater than 10000"), std::to_string(id));
  
  // Store data for the ID.
  database::confirm::store (id, "SELECT x, y, z FROM a;", "email", "subject", "body", "username");
  
  // Search for this ID based on subject.
  unsigned int id2 = database::confirm::search_id ("Subject line CID" + std::to_string (id) + " Re:");
  EXPECT_EQ (id, id2);
  
  // Retrieve data for the ID.
  std::string query = database::confirm::get_query (id);
  EXPECT_EQ ("SELECT x, y, z FROM a;", query);
  
  std::string to = database::confirm::get_mail_to (id);
  EXPECT_EQ ("email", to);
  
  std::string subject = database::confirm::get_subject (id);
  EXPECT_EQ ("subject", subject);
  
  std::string body = database::confirm::get_body (id);
  EXPECT_EQ ("body", body);
  
  std::string username = database::confirm::get_username(id);
  EXPECT_EQ ("username", username);
  username = database::confirm::get_username(id + 1);
  EXPECT_EQ (std::string(), username);

  // Delete this ID.
  database::confirm::erase (id);
  query = database::confirm::get_query (id);
  EXPECT_EQ ("", query);

#endif
}

#endif

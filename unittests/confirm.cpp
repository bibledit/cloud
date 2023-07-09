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
#include <unittests/utilities.h>
#include <database/confirm.h>
#include <filter/string.h>
using namespace std;


TEST (database, confirm)
{
#ifdef HAVE_CLOUD
  
  refresh_sandbox (true);
  Database_Confirm database_confirm;
  database_confirm.create ();
  database_confirm.upgrade();
  database_confirm.optimize ();
  database_confirm.trim ();
  
  // New ID generation test.
  unsigned int id = database_confirm.get_new_id ();
  if (id < 10'000) EXPECT_EQ (string("Should be greater than 10000"), to_string(id));
  
  // Store data for the ID.
  database_confirm.store (id, "SELECT x, y, z FROM a;", "email", "subject", "body", "username");
  
  // Search for this ID based on subject.
  unsigned int id2 = database_confirm.search_id ("Subject line CID" + to_string (id) + " Re:");
  EXPECT_EQ (id, id2);
  
  // Retrieve data for the ID.
  string query = database_confirm.get_query (id);
  EXPECT_EQ ("SELECT x, y, z FROM a;", query);
  
  string to = database_confirm.get_mail_to (id);
  EXPECT_EQ ("email", to);
  
  string subject = database_confirm.get_subject (id);
  EXPECT_EQ ("subject", subject);
  
  string body = database_confirm.get_body (id);
  EXPECT_EQ ("body", body);
  
  string username = database_confirm.get_username(id);
  EXPECT_EQ ("username", username);
  username = database_confirm.get_username(id + 1);
  EXPECT_EQ (string(), username);

  vector <int> ids = database_confirm.get_ids();
  vector <int> standard {static_cast<int>(id)};
  EXPECT_EQ (standard, ids);

  // Delete this ID.
  database_confirm.erase (id);
  query = database_confirm.get_query (id);
  evaluate (__LINE__, __func__,"", query);

#endif
}

#endif

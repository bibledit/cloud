/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>


TEST (database, logs_1)
{
  // Tests for Database_Logs.
  {
    refresh_sandbox (false);
    // Log some items.
    Database_Logs::log ("description1", 2);
    Database_Logs::log ("description2", 3);
    Database_Logs::log ("description3", 4);
    // Rotate the items.
    Database_Logs::rotate ();
    // Get the items from the SQLite database.
    std::string lastfilename;
    std::vector <std::string> result = Database_Logs::get (lastfilename);
    EXPECT_EQ (3, result.size ());
    refresh_sandbox (false);
  }
}


TEST (database, logs_2)
{
  // Test huge journal entry.
  refresh_sandbox (false);
  const std::string huge (60'000, 'x');
  Database_Logs::log (huge);
  Database_Logs::rotate ();
  std::string s = "0";
  std::vector <std::string> result = Database_Logs::get (s);
  if (result.size () == 1) {
    s = result [0];
    const std::string path = filter_url_create_path ({Database_Logs::folder (), s});
    const std::string contents = filter_url_file_get_contents (path);
    EXPECT_EQ (50'006, contents.find ("This entry was too large and has been truncated: 60000 bytes"));
  } else {
    EXPECT_EQ (1, static_cast<int>(result.size ()));
  }
  refresh_sandbox (true, {"This entry was too large and has been truncated"});
}


TEST (database, logs_3)
{
  // Test the getNext function of the Journal.
  refresh_sandbox (false);
  Database_Logs::log ("description");
  int second = filter::date::seconds_since_epoch ();
  std::string filename = std::to_string (second) + "00000000";
  // First time: getNext gets the logged entry.
  std::string s = Database_Logs::next (filename);
  EXPECT_NE (std::string(), s);
  // Since variable "filename" is updated and set to the last filename,
  // next time function getNext gets nothing.
  s = Database_Logs::next (filename);
  EXPECT_EQ (std::string(), s);
  refresh_sandbox (false);
}


#endif


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
#include <checksum/logic.h>
#include <database/state.h>
#include <webserver/request.h>
#include <filter/md5.h>


TEST (checksum, basic)
{
  // Get1
  {
    const std::string data = "\\v Verse 1";
    const std::string checksum = checksum_logic::get (data);
    EXPECT_EQ ("10", checksum);
  }
  // Get2
  {
    const std::string data = "Line one\nLine 2\n";
    const std::string checksum = checksum_logic::get (data);
    EXPECT_EQ ("16", checksum);
  }
  // Send1
  {
    const std::string data = "\\v Verse 1";
    const std::string checksum = checksum_logic::send (data, false);
    const std::string standard = "10\n0\n" + data;
    EXPECT_EQ (standard, checksum);
  }
  // Send2
  {
    const std::string data = "Line one\nLine 2\n";
    const std::string checksum = checksum_logic::send (data, true);
    const std::string standard = "16\n1\n" + data;
    EXPECT_EQ (standard, checksum);
  }

  // Setup some data.
  refresh_sandbox (true);
  Database_State::create ();
  Webserver_Request webserver_request;
  database::bibles::store_chapter ("phpunit1", 1, 2, "data1");
  database::bibles::store_chapter ("phpunit1", 1, 3, "data2");
  database::bibles::store_chapter ("phpunit1", 1, 4, "data3");
  database::bibles::store_chapter ("phpunit2", 2, 5, "data4");

  // GetChapter1
  {
    const std::string checksum = checksum_logic::get_chapter ("phpunit1", 1, 2);
    EXPECT_EQ (md5 ("data1"), checksum);
  }
  // GetChapter2
  {
    const std::string checksum = checksum_logic::get_chapter ("phpunit2", 2, 6);
    EXPECT_EQ (md5 (""), checksum);
  }
  // GetBook1
  {
    const std::string checksum = checksum_logic::get_book ("phpunit1", 1);
    EXPECT_EQ ("2ab6425924e6cd38b2474c543c5ea602", checksum);
  }
  // GetBook2
  {
    const std::string checksum = checksum_logic::get_book ("phpunit3", 1);
    EXPECT_EQ ("d41d8cd98f00b204e9800998ecf8427e", checksum);
  }
  // GetBible1
  {
    const std::string checksum = checksum_logic::get_bible ("phpunit1");
    EXPECT_EQ ("f9dc679a8712eb6f65b584e9688e9680", checksum);
  }
  // GetBible2
  {
    const std::string checksum = checksum_logic::get_bible ("phpunit2");
    EXPECT_EQ ("ee84a85bac14adb35e887c3d89bc80ab", checksum);
  }
  // GetBibles1
  {
    const std::string checksum = checksum_logic::get_bibles ({"phpunit1", "phpunit2"});
    EXPECT_EQ ("440b2008993816f0bc8c557b64fbdaf2", checksum);
  }
  // GetBibles2
  {
    const std::string checksum = checksum_logic::get_bibles ({"phpunit3", "phpunit4"});
    EXPECT_EQ ("020eb29b524d7ba672d9d48bc72db455", checksum);
  }
}

#endif


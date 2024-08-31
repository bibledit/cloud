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
#include <client/logic.h>
#include <database/logic.h>
#include <filter/url.h>


TEST (client, basic)
{
  refresh_sandbox (false);
  
  // Test client enabled logic.
  {
    bool enabled = client_logic_client_enabled ();
    EXPECT_EQ (false, enabled);
    client_logic_enable_client (true);
    // When a client is disabled in config.h, it remains disabled, no matter the setting in the database.
    // It means that this unit test depends on client mode to be off in ./configure.
    enabled = client_logic_client_enabled ();
#ifndef HAVE_CLIENT
    EXPECT_EQ (false, enabled);
#endif
    client_logic_enable_client (false);
    enabled = client_logic_client_enabled ();
    EXPECT_EQ (false, enabled);
  }

  // Test create consultation note encoding and decoding.
  {
    std::string data = client_logic_create_note_encode ("bible", 1, 2, 3, "summary", "line1\nline2", false);
    std::string standard =
    "bible\n"
    "1\n"
    "2\n"
    "3\n"
    "summary\n"
    "0\n"
    "line1\n"
    "line2";
    EXPECT_EQ (standard, data);

    std::string bible;
    int book, chapter, verse;
    std::string summary, contents;
    bool raw;
    client_logic_create_note_decode (standard, bible, book, chapter, verse, summary, contents, raw);
    EXPECT_EQ ("bible", bible);
    EXPECT_EQ (1, book);
    EXPECT_EQ (2, chapter);
    EXPECT_EQ (3, verse);
    EXPECT_EQ ("summary", summary);
    EXPECT_EQ (false, raw);
    standard =
    "line1\n"
    "line2";
    EXPECT_EQ (standard, contents);
  }
  
  // Testing logic for resources not to cache.
  {
    std::string path = client_logic_no_cache_resources_path ();
    std::string standard = filter_url_create_root_path ({"databases", "client", "no_cache_resources.txt"});
    EXPECT_EQ (standard, path);
    
    std::vector <std::string> resources = client_logic_no_cache_resources_get ();
    EXPECT_EQ (std::vector <std::string>{}, resources);

    std::string name1 = "comparative test";
    std::string name2 = "comparative greek";

    client_logic_no_cache_resource_add (name1);
    resources = client_logic_no_cache_resources_get ();
    EXPECT_EQ (std::vector <std::string>{name1}, resources);

    client_logic_no_cache_resource_add (name2);
    resources = client_logic_no_cache_resources_get ();
    EXPECT_EQ ((std::vector <std::string>{name1, name2}), resources);

    client_logic_no_cache_resource_remove (name1);
    resources = client_logic_no_cache_resources_get ();
    EXPECT_EQ (std::vector <std::string>{name2}, resources);

    client_logic_no_cache_resource_remove (name2);
    resources = client_logic_no_cache_resources_get ();
    EXPECT_EQ (std::vector <std::string>{}, resources);
  }
  
}

#endif

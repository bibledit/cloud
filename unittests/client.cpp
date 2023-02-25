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


#include <unittests/client.h>
#include <unittests/utilities.h>
#include <client/logic.h>
#include <database/logic.h>
#include <filter/url.h>
using namespace std;


void test_client ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  // Test client enabled logic.
  {
    bool enabled = client_logic_client_enabled ();
    evaluate (__LINE__, __func__, false, enabled);
    client_logic_enable_client (true);
    // When a client is disabled in config.h, it remains disabled, no matter the setting in the database.
    // It means that this unit test depends on client mode to be off in ./configure.
    enabled = client_logic_client_enabled ();
#ifndef HAVE_CLIENT
    evaluate (__LINE__, __func__, false, enabled);
#endif
    client_logic_enable_client (false);
    enabled = client_logic_client_enabled ();
    evaluate (__LINE__, __func__, false, enabled);
  }

  // Test create consultation note encoding and decoding.
  {
    string data = client_logic_create_note_encode ("bible", 1, 2, 3, "summary", "line1\nline2", false);
    string standard =
    "bible\n"
    "1\n"
    "2\n"
    "3\n"
    "summary\n"
    "0\n"
    "line1\n"
    "line2";
    evaluate (__LINE__, __func__, standard, data);

    string bible;
    int book, chapter, verse;
    string summary, contents;
    bool raw;
    client_logic_create_note_decode (standard, bible, book, chapter, verse, summary, contents, raw);
    evaluate (__LINE__, __func__, "bible", bible);
    evaluate (__LINE__, __func__, 1, book);
    evaluate (__LINE__, __func__, 2, chapter);
    evaluate (__LINE__, __func__, 3, verse);
    evaluate (__LINE__, __func__, "summary", summary);
    evaluate (__LINE__, __func__, false, raw);
    standard =
    "line1\n"
    "line2";
    evaluate (__LINE__, __func__, standard, contents);
  }
  
  // Testing logic for resources not to cache.
  {
    string path = client_logic_no_cache_resources_path ();
    string standard = filter_url_create_root_path ({"databases", "client", "no_cache_resources.txt"});
    evaluate (__LINE__, __func__, standard, path);
    
    vector <string> resources = client_logic_no_cache_resources_get ();
    evaluate (__LINE__, __func__, {}, resources);

    string name1 = "comparative test";
    string name2 = "comparative greek";

    client_logic_no_cache_resource_add (name1);
    resources = client_logic_no_cache_resources_get ();
    evaluate (__LINE__, __func__, {name1}, resources);

    client_logic_no_cache_resource_add (name2);
    resources = client_logic_no_cache_resources_get ();
    evaluate (__LINE__, __func__, {name1, name2}, resources);

    client_logic_no_cache_resource_remove (name1);
    resources = client_logic_no_cache_resources_get ();
    evaluate (__LINE__, __func__, {name2}, resources);

    client_logic_no_cache_resource_remove (name2);
    resources = client_logic_no_cache_resources_get ();
    evaluate (__LINE__, __func__, {}, resources);
  }
  
}

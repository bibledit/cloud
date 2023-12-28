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
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <jsonxx/jsonxx.h>
#pragma GCC diagnostic pop
#include <filter/url.h>


// Test included JSON libraries.
TEST (json, basic)
{
  // Convert JSON to xml.
  {
    const std::string json ("{"
                            R"(  "foo" : 1,)"
                            R"(  "bar" : false,)"
                            R"(  "person" : {"name" : "GWB", "age" : 60,},)"
                            R"(  "data": ["abcd", 42],)"
                            "}");
    jsonxx::Object object;
    object.parse (json);
    std::string path = filter_url_create_root_path ({"unittests", "tests", "json1.txt"});
    std::string xml = filter_url_file_get_contents (path);
    EXPECT_EQ (xml, object.xml (jsonxx::JSONx));
  }

  // Test malformed JSON.
  {
    std::string json ("{"
                      R"(  "foo" 1,)"
                      R"(  "bar : false,)"
                      R"(  "person" : me" : "GWB", "age" : 60,},)"
                      R"(  "data": "abcd", 42],)"
                      );
    jsonxx::Object object;
    object.parse (json);
    EXPECT_EQ ("{\n} \n", object.json ());
  }
  
  // JSON roundtrip.
  {
    std::string json;
    {
      jsonxx::Array array;
      {
        jsonxx::Object object;
        object << "key1" << "content1";
        object << "key2" << "content2";
        array << object;
      }
      {
        jsonxx::Object object;
        object << "key3" << "content3";
        object << "key4" << "content4";
        array << object;
      }
      json = array.json ();
      std::string path = filter_url_create_root_path ({"unittests", "tests", "json2.txt"});
      std::string standard = filter_url_file_get_contents (path);
      EXPECT_EQ (standard, json);
    }
    {
      jsonxx::Array array;
      array.parse (json);
      jsonxx::Object object;
      object = array.get<jsonxx::Object>(0);
      EXPECT_EQ ("content1", object.get<jsonxx::String>("key1"));
      EXPECT_EQ ("content2", object.get<jsonxx::String>("key2"));
      EXPECT_EQ (false, object.has<jsonxx::String>("key3"));
      object = array.get<jsonxx::Object>(1);
      EXPECT_EQ ("content3", object.get<jsonxx::String>("key3"));
      EXPECT_EQ ("content4", object.get<jsonxx::String>("key4"));
      EXPECT_EQ (false, object.has<jsonxx::String>("key5"));
    }
  }
}

#endif


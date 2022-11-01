/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/json.h>
#include <unittests/utilities.h>
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <jsonxx/jsonxx.h>
#pragma GCC diagnostic pop
#include <filter/url.h>
using namespace std;
using namespace jsonxx;


// Test included JSON libraries.
void test_json ()
{
  trace_unit_tests (__func__);
  
  // Convert JSON to xml.
  {
    string json ("{"
                 R"(  "foo" : 1,)"
                 R"(  "bar" : false,)"
                 R"(  "person" : {"name" : "GWB", "age" : 60,},)"
                 R"(  "data": ["abcd", 42],)"
                 "}");
    Object object;
    object.parse (json);
    string path = filter_url_create_root_path ({"unittests", "tests", "json1.txt"});
    string xml = filter_url_file_get_contents (path);
    evaluate (__LINE__, __func__, xml, object.xml (JSONx));
  }

  // Test malformed JSON.
  {
    string json ("{"
                 R"(  "foo" 1,)"
                 R"(  "bar : false,)"
                 R"(  "person" : me" : "GWB", "age" : 60,},)"
                 R"(  "data": "abcd", 42],)"
                 );
    Object object;
    object.parse (json);
    evaluate (__LINE__, __func__, "{\n} \n", object.json ());
  }
  
  // JSON roundtrip.
  {
    string json;
    {
      Array array;
      {
        Object object;
        object << "key1" << "content1";
        object << "key2" << "content2";
        array << object;
      }
      {
        Object object;
        object << "key3" << "content3";
        object << "key4" << "content4";
        array << object;
      }
      json = array.json ();
      string path = filter_url_create_root_path ({"unittests", "tests", "json2.txt"});
      string standard = filter_url_file_get_contents (path);
      evaluate (__LINE__, __func__, standard, json);
    }
    {
      Array array;
      array.parse (json);
      Object object;
      object = array.get<Object>(0);
      evaluate (__LINE__, __func__, "content1", object.get<String>("key1"));
      evaluate (__LINE__, __func__, "content2", object.get<String>("key2"));
      evaluate (__LINE__, __func__, false, object.has<String>("key3"));
      object = array.get<Object>(1);
      evaluate (__LINE__, __func__, "content3", object.get<String>("key3"));
      evaluate (__LINE__, __func__, "content4", object.get<String>("key4"));
      evaluate (__LINE__, __func__, false, object.has<String>("key5"));
    }
  }
}

/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <codecvt>
#include <unittests/utilities.h>
#include <unittests/c++17.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/text.h>
#include <filter/image.h>
#include <editor/html2usfm.h>
#include <editor/html2format.h>
#include <styles/logic.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <database/bibleimages.h>
#include <webserver/request.h>
#include <user/logic.h>
#include <pugixml/pugixml.hpp>
#include <html/text.h>
#include <checks/usfm.h>
#include <resource/logic.h>
#include <type_traits>


using namespace pugi;


struct cpp17_struct {
    mutable int x1 : 2;
    mutable string y1;
};
cpp17_struct cpp17_function() { return cpp17_struct {1, "2.3"}; }


void test_cpp17 ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  {
    set <string> myset {"hello"};
    int inserted {0}, skipped{0};
    for (int i {2}; i; --i) {
      if (auto [iter, success] = myset.insert("Hello"); success) {
        evaluate (__LINE__, __func__, "Hello", *iter);
        inserted++;
      }
      else {
        evaluate (__LINE__, __func__, "Hello", *iter);
        skipped++;
      }
    }
    evaluate (__LINE__, __func__, 1, inserted);
    evaluate (__LINE__, __func__, 1, skipped);
  }

  {
    struct fields {
      int b {1}, d {2}, p {3}, q {4};
    };
    {
      const auto [b, d, p, q] = fields{};
      evaluate (__LINE__, __func__, 1, b);
      evaluate (__LINE__, __func__, 2, d);
      evaluate (__LINE__, __func__, 3, p);
      evaluate (__LINE__, __func__, 4, q);
    }
    {
      const auto [b, d, p, q] = []
      {
        return fields{4, 3, 2, 1};
      } ();
      evaluate (__LINE__, __func__, 4, b);
      evaluate (__LINE__, __func__, 3, d);
      evaluate (__LINE__, __func__, 2, p);
      evaluate (__LINE__, __func__, 1, q);
    }
  }

  {
    const auto [x, y] = cpp17_function();
    // x is an int lvalue identifying the 2-bit bit field
    // y is a const volatile double lvalue
    evaluate (__LINE__, __func__, 1, x);
    evaluate (__LINE__, __func__, "2.3", y);
    x = -2;
    y = "-2.3";
    evaluate (__LINE__, __func__, -2, x);
    evaluate (__LINE__, __func__, "-2.3", y);
  }

  {
    static_assert(true);
  }

  {
    [[maybe_unused]] auto x1 = { 1, 2 };
    [[maybe_unused]] auto x2 = { 1.0, 2.0 };
    [[maybe_unused]] auto x3 { 2 };
    [[maybe_unused]] auto x4 = { 3 };
    
    float x {1.1};
    char  y {'a'};
    int   z {1};
    tuple<float&, char&&, int> tpl (x, move(y), z);
    [[maybe_unused]] const auto& [a,b,c] = tpl;
    // a names a structured binding that refers to x; decltype(a) is float&
    evaluate (__LINE__, __func__, x, a);
    // b names a structured binding that refers to y; decltype(b) is char&&
    evaluate (__LINE__, __func__, y, b);
    // c names a structured binding that refers to the 3rd element of tpl; decltype(c) is const int
    evaluate (__LINE__, __func__, z, c);
  }
  
  refresh_sandbox (true);
}

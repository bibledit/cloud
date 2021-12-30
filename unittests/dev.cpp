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


void test_dev ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  vector <string> payload;
  payload.push_back ("Content-Type: multipart/alternative; boundary=\"------------010001060501040600060905\"");

  static_assert(true);
  
  [[maybe_unused]] auto x1 = { 1, 2 }; // decltype(x1) is std::initializer_list<int>
  [[maybe_unused]] auto x2 = { 1.0, 2.0 }; // error: cannot deduce element type
  [[maybe_unused]] auto x3 { 2 }; // error: not a single element
  [[maybe_unused]] auto x4 = { 3 }; // decltype(x4) is std::initializer_list<int>
  [[maybe_unused]] auto x5 { 3 }; // decltype(x5) is int
  
  //for (auto s : payload) cout << s << endl;
  refresh_sandbox (true);
}

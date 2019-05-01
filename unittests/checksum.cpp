/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <unittests/checksum.h>
#include <unittests/utilities.h>
#include <checksum/logic.h>
#include <database/state.h>
#include <webserver/request.h>
#include <filter/md5.h>


void test_checksum ()
{
  trace_unit_tests (__func__);
  
  // Get1
  {
    string data = "\\v Verse 1";
    string checksum = Checksum_Logic::get (data);
    evaluate (__LINE__, __func__, "10", checksum);
  }
  // Get2
  {
    string data = "Line one\nLine 2\n";
    string checksum = Checksum_Logic::get (data);
    evaluate (__LINE__, __func__, "16", checksum);
  }
  // Send1
  {
    string data = "\\v Verse 1";
    string checksum = Checksum_Logic::send (data, false);
    string standard = "10\n0\n" + data;
    evaluate (__LINE__, __func__, standard, checksum);
  }
  // Send2
  {
    string data = "Line one\nLine 2\n";
    string checksum = Checksum_Logic::send (data, true);
    string standard = "16\n1\n" + data;
    evaluate (__LINE__, __func__, standard, checksum);
  }
  // Setup some data.
  refresh_sandbox (true);
  Database_State::create ();
  Webserver_Request request;
  request.database_bibles()->storeChapter ("phpunit1", 1, 2, "data1");
  request.database_bibles()->storeChapter ("phpunit1", 1, 3, "data2");
  request.database_bibles()->storeChapter ("phpunit1", 1, 4, "data3");
  request.database_bibles()->storeChapter ("phpunit2", 2, 5, "data4");
  // GetChapter1
  {
    string checksum = Checksum_Logic::getChapter (&request, "phpunit1", 1, 2);
    evaluate (__LINE__, __func__, md5 ("data1"), checksum);
  }
  // GetChapter2
  {
    string checksum = Checksum_Logic::getChapter (&request, "phpunit2", 2, 6);
    evaluate (__LINE__, __func__, md5 (""), checksum);
  }
  // GetBook1
  {
    string checksum = Checksum_Logic::getBook (&request, "phpunit1", 1);
    evaluate (__LINE__, __func__, "2ab6425924e6cd38b2474c543c5ea602", checksum);
  }
  // GetBook2
  {
    string checksum = Checksum_Logic::getBook (&request, "phpunit3", 1);
    evaluate (__LINE__, __func__, "d41d8cd98f00b204e9800998ecf8427e", checksum);
  }
  // GetBible1
  {
    string checksum = Checksum_Logic::getBible (&request, "phpunit1");
    evaluate (__LINE__, __func__, "f9dc679a8712eb6f65b584e9688e9680", checksum);
  }
  // GetBible2
  {
    string checksum = Checksum_Logic::getBible (&request, "phpunit2");
    evaluate (__LINE__, __func__, "ee84a85bac14adb35e887c3d89bc80ab", checksum);
  }
  // GetBibles1
  {
    string checksum = Checksum_Logic::getBibles (&request, {"phpunit1", "phpunit2"});
    evaluate (__LINE__, __func__, "440b2008993816f0bc8c557b64fbdaf2", checksum);
  }
  // GetBibles2
  {
    string checksum = Checksum_Logic::getBibles (&request, {"phpunit3", "phpunit4"});
    evaluate (__LINE__, __func__, "020eb29b524d7ba672d9d48bc72db455", checksum);
  }
}

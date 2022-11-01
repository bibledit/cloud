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


#include <unittests/ldap.h>
#include <unittests/utilities.h>
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
using namespace std;


void test_database_logs ()
{
  trace_unit_tests (__func__);
  
  // Tests for Database_Logs.
  {
    refresh_sandbox (true);
    // Log some items.
    Database_Logs::log ("description1", 2);
    Database_Logs::log ("description2", 3);
    Database_Logs::log ("description3", 4);
    // Rotate the items.
    Database_Logs::rotate ();
    // Get the items from the SQLite database.
    string lastfilename;
    vector <string> result = Database_Logs::get (lastfilename);
    evaluate (__LINE__, __func__, 3, result.size ());
    refresh_sandbox (false);
  }
  {
    // Test huge journal entry.
    refresh_sandbox (true);
    string huge (60'000, 'x');
    Database_Logs::log (huge);
    Database_Logs::rotate ();
    string s = "0";
    vector <string> result = Database_Logs::get (s);
    if (result.size () == 1) {
      s = result [0];
      string path = filter_url_create_path ({Database_Logs::folder (), s});
      string contents = filter_url_file_get_contents (path);
      evaluate (__LINE__, __func__, 50'006, contents.find ("This entry was too large and has been truncated: 60000 bytes"));
    } else {
      evaluate (__LINE__, __func__, 1, static_cast<int>(result.size ()));
    }
    refresh_sandbox (true, {"This entry was too large and has been truncated"});
  }
  {
    // Test the getNext function of the Journal.
    refresh_sandbox (true);
    Database_Logs::log ("description");
    int second = filter::date::seconds_since_epoch ();
    string filename = convert_to_string (second) + "00000000";
    // First time: getNext gets the logged entry.
    string s;
    s = Database_Logs::next (filename);
    if (s == "") evaluate (__LINE__, __func__, "...description", s);
    // Since variable "filename" is updated and set to the last filename,
    // next time function getNext gets nothing.
    s = Database_Logs::next (filename);
    evaluate (__LINE__, __func__, "", s);
    refresh_sandbox (false);
  }
}

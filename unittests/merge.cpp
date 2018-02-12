/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <unittests/merge.h>
#include <unittests/utilities.h>
#include <filter/merge.h>
#include <filter/url.h>


void test_merge ()
{
  trace_unit_tests (__func__);
  
  {
    string ancestor_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    string server_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    string user_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti ‘muka Allah’ dalam bahasa Ibrani.\f* karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    string standard = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti ‘muka Allah’ dalam bahasa Ibrani.\f* karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    vector <tuple <string, string, string, string, string>> conflicts;
    string result = filter_merge_run (ancestor_usfm, server_usfm, user_usfm, true, conflicts);
    evaluate (__LINE__, __func__, standard, result);
  }

  refresh_sandbox (true);
}

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


#include <unittests/http.h>
#include <unittests/utilities.h>
#include <webserver/http.h>


void test_http ()
{
  trace_unit_tests (__func__);

  string host, line;
  
  line = "192.168.1.139:8080";
  host = http_parse_host (line);
  evaluate (__LINE__, __func__, "192.168.1.139", host);

  line = "localhost:8080";
  host = http_parse_host (line);
  evaluate (__LINE__, __func__, "localhost", host);

  line = "[::1]:8080";
  host = http_parse_host (line);
  evaluate (__LINE__, __func__, "[::1]", host);

  line = "[fe80::601:25ff:fe07:6801]:8080";
  host = http_parse_host (line);
  evaluate (__LINE__, __func__, "[fe80::601:25ff:fe07:6801]", host);
}

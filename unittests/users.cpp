/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <unittests/users.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <filter/string.h>


void test_users ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  // Tests for a user's identifier.
  {
    Webserver_Request request;
    request.database_users ()->create ();
    request.session_logic()->setUsername ("phpunit");
    evaluate (__LINE__, __func__, 13683715, filter_string_user_identifier (&request));
    request.session_logic()->setUsername ("phpunit2");
    evaluate (__LINE__, __func__, 13767813, filter_string_user_identifier (&request));
  }
  refresh_sandbox (true);
}

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


#include <developer/delay.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <config/globals.h>


const char * developer_delay_url ()
{
  return "developer/delay";
}


bool developer_delay_acl (void * webserver_request) // Todo only in demo mode.
{
  return true;
}


string developer_delay (void * webserver_request)
{
  (void) webserver_request;
  // Here is a delay routine that waits multiple seconds before sending the reponse.
  // The purpose is to test timeouts of the website live monitors.
  //this_thread::sleep_for(chrono::seconds(10));
  // Done.
  return "OK";
}

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


#ifndef INCLUDED_DEBUG_LOGIC_H
#define INCLUDED_DEBUG_LOGIC_H


#include <config/libraries.h>


void developer_logic_log_network_write ();

class Developer_Logic_Tracer
{
public:
  Developer_Logic_Tracer(void * webserver_request);
  ~Developer_Logic_Tracer();
  int seconds1 = 0;
  int microseconds1 = 0;
  string rfc822;
  string remote_address;
  string request_get;
  string request_query;
  string username;
};


#endif

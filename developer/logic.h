/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

void developer_logic_log_network_write ();

class Developer_Logic_Tracer
{
public:
  Developer_Logic_Tracer(Webserver_Request& webserver_request);
  ~Developer_Logic_Tracer();
  int seconds1 {0};
  int microseconds1 {0};
  std::string rfc822 {};
  std::string remote_address {};
  std::string request_get {};
  std::string request_query {};
  std::string username {};
};

void developer_logic_import_changes ();

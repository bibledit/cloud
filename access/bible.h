/*
Copyright (©) 2003-2025 Teus Benschop.

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

namespace access_bible {

bool read (Webserver_Request& webserver_request, const std::string & bible, std::string user = std::string());
bool write (Webserver_Request& webserver_request, const std::string & bible, std::string user = std::string());
bool book_write (Webserver_Request& webserver_request, std::string user, const std::string & bible, int book);
std::vector <std::string> bibles (Webserver_Request& webserver_request, std::string user = std::string());
std::string clamp (Webserver_Request& webserver_request, std::string bible);
std::tuple <bool, bool> any (Webserver_Request& webserver_request);

}

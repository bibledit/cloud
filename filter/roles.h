/*
Copyright (©) 2003-2026 Teus Benschop.

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

namespace roles {

constexpr const int guest {1};
constexpr const int member {2};
constexpr const int consultant {3};
constexpr const int translator {4};
constexpr const int manager {5};
constexpr const int admin {6};
constexpr const int lowest {guest};
constexpr const int highest {admin};
std::string english (const int role);
std::string text (const int role);
bool access_control (Webserver_Request& webserver_request, const int role);

}

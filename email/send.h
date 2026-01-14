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

namespace email {

// Maximum email size, on the safe side, that libcurl can send without it crashing.
constexpr const int max_email_size {100000};

void send ();
std::string send (std::string to_mail, std::string to_name, std::string subject, std::string body, bool verbose = false);
void schedule (std::string to, std::string subject, std::string body, int time = 0);
std::string setup_information (bool require_send, bool require_receive);

}

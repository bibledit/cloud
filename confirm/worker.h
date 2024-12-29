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

#ifdef HAVE_CLOUD

namespace confirm::worker {

void setup (Webserver_Request& webserver_request,
            const std::string& mailto, const std::string& username,
            const std::string& initial_subject, std::string initial_body,
            const std::string& query,
            const std::string& subsequent_subject, const std::string& subsequent_body);

bool handle_link (Webserver_Request& webserver_request, std::string& email);

}

#endif

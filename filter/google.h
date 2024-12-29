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

namespace filter::google {

std::tuple <std::string, std::string> get_json_key_value_error ();
std::tuple <bool, std::string> activate_service_account ();
std::tuple <bool, std::string> print_store_access_token ();
void refresh_access_token ();
std::tuple <bool, std::string, std::string> translate (const std::string text, const char * source, const char * target);
std::vector <std::pair <std::string, std::string> > get_languages (const std::string & target);

}

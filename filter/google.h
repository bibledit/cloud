/*
Copyright (©) 2003-2022 Teus Benschop.

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

tuple <string, string> get_json_key_value_error ();
tuple <bool, string> activate_service_account ();
tuple <bool, string> print_store_access_token ();
void refresh_access_token ();
tuple <bool, string, string> translate (const string text, const char * source, const char * target);
vector <pair <string, string> > get_languages (const string & target);

}

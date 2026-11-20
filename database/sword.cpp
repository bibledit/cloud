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

#include <database/sword.h>
#include "filter/url.h"

// Databases resilience:
// They contain cached data.
// Rarely written to.
// Often read from.

namespace database::sword {

static std::string json_file_path (const std::string& module)
{
    return filter_url_create_root_path({"databases", "sword_" + module + ".json"});
}

bool exists(const std::string& module)
{
    return file_or_dir_exists(json_file_path(module));
}

void store(const std::string& module, const std::string& data)
{
    filter_url_file_put_contents(json_file_path(module), data);
}

std::string retrieve(const std::string& module)
{
    return filter_url_file_get_contents(json_file_path(module));
}

void remove(const std::string& module)
{
    filter_url_unlink(json_file_path(module));
}

}

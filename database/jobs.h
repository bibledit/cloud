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

namespace database_jobs {

void create();
void optimize();
void trim();
[[nodiscard]] bool id_exists(int id);
[[nodiscard]] int get_new_id();
void set_level(int id, int level);
[[nodiscard]] int get_level(int id);
void set_start(int id, const std::string& start);
[[nodiscard]] std::string get_start(int id);
void set_percentage(int id, int percentage);
[[nodiscard]] std::string get_percentage(int id);
void set_progress(int id, const std::string& progress);
[[nodiscard]] std::string get_progress(int id);
void set_result(int id, const std::string& result);
[[nodiscard]] std::string get_result(int id);

}

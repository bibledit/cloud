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

namespace database::logs {

std::string folder ();

void logv1 (std::string description, int minimum_role = 5);

template <typename ... Args, int minimum_role = 5>
void log (Args&& ... args)
{
    std::ostringstream oss{};
    (void(oss << std::forward<Args>(args) << ' '), ...);
    std::string msg = std::move(oss).str();
    if (not msg.empty() and msg.back() == ' ')
        msg.pop_back();
    logv1(std::move(msg), minimum_role);
}

void rotate ();

std::vector <std::string> get (std::string & last_filename);

std::string next (std::string &filename);

void clear ();

}

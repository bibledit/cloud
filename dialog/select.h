/*
Copyright (©) 2021 Aranggi Toar.

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

std::string dialog_select_create_options(const std::vector<std::string>& values,
                                         const std::vector<std::string>& displayed,
                                         const std::string& selected);
std::string dialog_select_create(const std::string& identification,
                                 const std::vector<std::string>& values,
                                 const std::vector<std::string>& displayed,
                                 const std::string& selected,
                                 std::vector<std::pair<std::string,std::string>> parameters);

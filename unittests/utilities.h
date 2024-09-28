/*
Copyright (©) 2003-2024 Teus Benschop.

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

extern std::string testing_directory;
void refresh_sandbox (bool displayjournal, std::vector <std::string> allowed = {});
int odf2txt (std::string odf, std::string txt);

class scoped_timer {
public:
  scoped_timer() : m_beg(std::chrono::high_resolution_clock::now())
  { }
  ~scoped_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - m_beg);
    std::cout << dur.count() << " microseconds" << std::endl;
  }
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
};

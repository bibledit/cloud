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


// A C++ equivalent for PHP's function.
template <typename T>
std::vector <T> array_intersect (std::vector <T> a, std::vector <T> b)
{
  std::vector <T> result;
  std::set <T> aset (a.begin(), a.end());
  for (auto & item : b) {
    if (aset.find (item) != aset.end()) {
      result.push_back (item);
    }
  }
  return result;
}


// A C++ equivalent for PHP's function.
template <typename T>
bool in_array (const T & needle, const std::vector <T> & haystack)
{
  return (find (haystack.begin(), haystack.end(), needle) != haystack.end());
}


// Clip a value to not be less than "lower" and not more than "higher"
template <typename T>
T clip (const T& n, const T& lower, const T& upper) {
  return std::max<T> (lower, std::min<T> (n, upper));
}


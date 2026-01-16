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


namespace filter::string {


// Return the intersection of two containers.
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


// Return whether the needle is found in the haystack.
template <typename T>
bool in_array (const T & needle, const std::vector <T> & haystack)
{
  return (find (haystack.begin(), haystack.end(), needle) != haystack.end());
}


// Returns items in "from" which are not present in "against".
template <typename T>
std::vector<T> array_diff (const std::vector<T>& from, const std::vector<T>& against) {
  std::vector <T> result {};
  for (const auto& item : from) {
    if (std::find(against.cbegin(), against.cend(), item) == against.cend())
      result.push_back(item);
  }
  return result;
}


// Takes a range and materializes it into a vector.
auto range2vector(auto&& r) {
  std::vector<std::ranges::range_value_t<decltype(r)>> v;
  std::ranges::copy(r, std::back_inserter(v));
  return v;
}


} // Namespace.

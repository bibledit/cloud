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
std::vector<T> array_intersect(std::vector<T> a, std::vector<T> b)
{
    std::vector<T> result;
    std::set<T> aset(a.begin(), a.end());
    for (auto& item : b)
    {
        if (aset.find(item) != aset.end())
        {
            result.push_back(item);
        }
    }
    return result;
}


// Return whether the needle is found in the haystack.
template <typename T>
bool in_array(const T& needle, const std::vector<T>& haystack)
{
    return (find(haystack.begin(), haystack.end(), needle) != haystack.end());
}


// Returns items in "from" which are not present in "against".
template <typename T>
std::vector<T> array_diff(const std::vector<T>& from, const std::vector<T>& against)
{
    std::vector<T> result{};
    for (const auto& item : from)
    {
        if (std::find(against.cbegin(), against.cend(), item) == against.cend())
            result.push_back(item);
    }
    return result;
}


template <typename T>
std::vector<T> array_unique (const std::vector<T>& values)
{
    std::vector<T> result;
    std::ranges::for_each(values, [&result](const auto& value)
    {
        if (std::ranges::find(result, value) == result.cend())
            result.emplace_back(value);
    });
    return result;
}


// Takes a range and materializes it into a vector.
auto range2vector(auto&& r)
{
    std::vector<std::ranges::range_value_t<decltype(r)>> v;
    std::ranges::copy(r, std::back_inserter(v));
    return v;
}


// This function is unusual in the sense that it does not sort one container,
// as the majority of sort functions do, but it accepts two containers.
// It sorts the first, and reorders the second container at the same time,
// following the reordering done in the first container.
template <typename One, typename Two>
concept quick_sortable = true
and not std::is_same_v<One, bool>
and not std::is_same_v<Two, bool>
and (std::is_integral_v<One> or std::is_same_v<One, std::string>)
and (std::is_integral_v<Two> or std::is_same_v<Two, std::string>);
template <typename One, typename Two>
requires quick_sortable<One, Two>
void quick_sort(std::vector<One>& one, std::vector<Two>& two, const unsigned int beg, const unsigned int end)
{
    if (end > beg + 1)
    {
        const One piv = one[beg];
        unsigned int l = beg + 1;
        unsigned int r = end;
        while (l < r)
        {
            if (one[l] <= piv)
            {
                l++;
            }
            else
            {
                --r;
                std::swap(one[l], one[r]);
                std::swap(two[l], two[r]);
            }
        }
        --l;
        std::swap(one[l], one[beg]);
        std::swap(two[l], two[beg]);
        quick_sort(one, two, beg, l);
        quick_sort(one, two, r, end);
    }
}


} // Namespace.

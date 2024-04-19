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


#include <dialog/list2.h>


// Generate the option tags based on the inserted key and its value.
std::string Options_To_Select::add_selection (std::string text, std::string value, std::string html)
{
    if (value == "") {
        html.append ("<option value=''>" + text + "</option>");
    } else {
        html.append ("<option value='" + value + "'>" + text + "</option>");
    }

    return html;
}


// Mark the current selected option's option tag.
std::string Options_To_Select::mark_selected (std::string value, std::string html)
{
    const std::string new_value = "value='" + value + "'";
    const size_t new_pos = html.find (new_value) + new_value.length ();
    const std::string mark = " selected";

    if (html.find (mark) != std::string::npos) {
        html.erase (html.find (mark), mark.length ());
    }

    html.insert (new_pos, mark);

    return html;
}

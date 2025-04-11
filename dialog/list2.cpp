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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


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


std::string dialog_list2_create_options(const std::vector<std::string>& values,
                                        const std::vector<std::string>& displayed,
                                        const std::string& selected)
{
  pugi::xml_document document {};
  for (size_t i {0}; i < values.size(); i++) {
    pugi::xml_node option_node = document.append_child("option");
    option_node.append_attribute("value") = values[i].c_str();
    if (selected == values[i])
      option_node.append_attribute("selected");
    const std::string display = (i >= displayed.size()) ? values[i] : displayed[i];
    option_node.text().set(display.c_str());
  }
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  return html_ss.str();
}

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

namespace dialog::select {

std::string create_options(const std::vector<std::string>& values,
                           const std::vector<std::string>& displayed,
                           const std::string& selected);

struct Settings {
  // The key of the value to be POSTed, and the Flate double hash value to place the html and Javascript into.
  const char* identification;
  // The values for the <select> element.
  std::vector<std::string> values;
  // The displayed values for the <select> element. May be left empty.
  std::vector<std::string> displayed;
  // If any value is to be selected as the default.
  const std::optional<std::string> selected {std::nullopt};
  // The query parameters to be added to the POSTed URL, like url?key=value
  const std::vector<std::pair<std::string,std::string>> parameters;
  // Whether the <select> element is to be disabled.
  bool disabled {false};
  // The tooltip on the selector.
  const std::optional<std::string> tooltip {std::nullopt};
};

std::string ajax(Settings& settings);
std::string form(Settings& settings);

}

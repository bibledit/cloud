/*
Copyright (©) 2003-2025 Teus Benschop.

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
namespace database::styles1 { struct Item; }
namespace stylesv2 { struct Style; }

namespace filter::note {

class citation
{
public:
  citation ();
  void set_sequence_v1 (int numbering, const std::string & usersequence);
  void set_sequence_v2 (std::string sequence_in);
  void set_restart_v1 (int setting);
  void set_restart_v2 (const std::string& setting);
  std::string get (std::string citation_in);
  void run_restart (const std::string & moment);
private:
  std::string restart {};
  unsigned int pointer {0};
  std::vector <std::string> sequence {};
};

class citations
{
public:
  void evaluate_style_v1 (const database::styles1::Item& style);
  void evaluate_style_v2 (const stylesv2::Style& style);
  std::string get (const std::string& marker, const std::string& citation);
  void restart (const std::string& moment);
private:
  // Container [marker] = citation.
  std::map <std::string, citation> cache {};
};


}

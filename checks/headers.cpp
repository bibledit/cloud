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


#include <checks/headers.h>
#include <filter/string.h>
#include <database/check.h>
#include <locale/translate.h>
#include <checks/issues.h>


void checks_headers::no_punctuation_at_end (const std::string& bible, int book, int chapter,
                                            const std::map <int, std::string>& headings,
                                            const std::string& centermarks, const std::string& endmarks)
{
  for (const auto & element : headings) {
    const int verse = element.first;
    const std::string heading = element.second;
    // Full stops often occur in the inspired headings of many Psalms in verse 0.
    // Skip these.
    if ((book == 19) && (verse == 0))
      continue;
    std::string last_character {};
    if (!heading.empty ())
      last_character = heading.substr (heading.size () - 1);
    bool message {false};
    if (centermarks.find (last_character) != std::string::npos)
      message = true;
    if (endmarks.find (last_character) != std::string::npos)
      message = true;
    if (message) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::punctuation_at_end_of_heading) + ": " + heading);
    }
  }
}

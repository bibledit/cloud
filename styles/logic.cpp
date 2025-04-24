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


#include <styles/logic.h>
#include <locale/translate.h>
#include <filter/string.h>


// The name of the "Standard" stylesheet.
std::string styles_logic_standard_sheet ()
{
  return "Standard";
}


// This contains the styles logic.
// categoryText - Returns the $category as human readable text.
std::string styles_logic_category_text (std::string category)
{
  if (category == "id")  return translate ("Identification information");
  if (category == "ith") return translate ("Introduction titles and headings");
  if (category == "ipp") return translate ("Introduction paragraphs and poetry");
  if (category == "ioe") return translate ("Introduction other elements");
  if (category == "t"  ) return translate ("Titles");
  if (category == "h"  ) return translate ("Headings");
  if (category == "cv" ) return translate ("Chapters and verses");
  if (category == "p"  ) return translate ("Paragraphs");
  if (category == "l"  ) return translate ("Lists");
  if (category == "pe" ) return translate ("Poetry elements");
  if (category == "te" ) return translate ("Table elements");
  if (category == "f"  ) return translate ("Footnotes");
  if (category == "x"  ) return translate ("Crossreferences");
  if (category == "st" ) return translate ("Special text");
  if (category == "cs" ) return translate ("Character styles");
  if (category == "sb" ) return translate ("Spacings and breaks");
  if (category == "sf" ) return translate ("Special features");
  if (category == "pm" ) return translate ("Peripheral materials");
  return translate ("Extra styles");
}

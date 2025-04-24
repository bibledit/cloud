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


// Returns the $type as human readable text.
std::string styles_logic_type_text (int type)
{
  return "--";
}


// This returns the $subtype as human readable text.
std::string styles_logic_subtype_text (int type, int subtype)
{
  return "--";
}


// Returns true if the fontsize is relevant for $type.
bool styles_logic_fontsize_is_relevant (int type)
{
  return false;
}


// Returns true if the italic, bold, etc. settings are relevant for $type and $subtype.
bool styles_logic_italic_bold_underline_smallcaps_are_relevant (int type)
{
  return false;
}


// Returns true if the italic, bold, etc. settings are fully applicable for $type and $subtype. Full means it also has inherit and toggle values.
bool styles_logic_italic_bold_underline_smallcaps_are_full (int type)
{
  return false;
}


// Returns the $value as human readable text.
std::string styles_logic_off_on_inherit_toggle_text (int value)
{
  return "--";
}


// Returns true if the superscript setting is relevant for $type.
bool styles_logic_superscript_is_relevant (int type)
{
  return false;
}


// Returns true if the paragraph treats are relevant for $type.
bool styles_logic_paragraph_treats_are_relevant (int type)
{
  return false;
}


// Returns the $value as human readable text.
std::string styles_logic_alignment_text (int value)
{
  return "--";
}


// Returns true if the columns are relevant for $type and $subtype
bool styles_logic_columns_are_relevant (int type, int subtype)
{
  return false;
}


// Returns true if the color is relevant for $type.
bool styles_logic_color_is_relevant (int type)
{
  return false;
}


// Returns true if the print setting is relevant (just now: never).
bool styles_logic_print_is_relevant ()
{
  return false;
}


// Returns the function of userbool1.
int styles_logic_get_userbool1_function ()
{
  return 0;
}


// Returns the text of userbool1's function.
std::string styles_logic_get_userbool1_text (int function)
{
  return std::string();
}


// Returns the function of userbool2 for type and subtype
int styles_logic_get_userbool2_function (int type, int subtype)
{
  (void) type;
  (void) subtype;
  return 0;
}


// Returns the text of userbool2's function.
std::string styles_logic_get_userbool2_text (int function)
{
  return std::string();
}


// Returns the function of userbool3 for type and subtype
int styles_logic_get_userbool3_function (int type, int subtype)
{
  (void) type;
  (void) subtype;
  return 0;
}


// Returns the text of userbool3's function.
std::string styles_logic_get_userbool3_text (int function)
{
  return std::string();
}


// Returns the function of userint1.
int styles_logic_get_userint1_function ()
{
  return 0;
}


// Returns the value as human readable text for note numbering.
std::string styles_logic_note_numbering_text (int value)
{
  return std::to_string (value);
}


// Returns the function of userint2.
int styles_logic_get_userint2_function ()
{
  return 0;
}


// Returns the value as human readable text for when to restart the note numbering.
std::string styles_logic_note_restart_numbering_text (int value)
{
  return std::to_string (value);
}


// Returns the value as human readable text for the position of the endnotes.
std::string styles_logic_end_note_position_text (int value)
{
  return std::to_string (value);
}


// Returns the function of userint3 for type and subtype
int styles_logic_get_userint3_function (int type, int subtype)
{
  if (type) {};
  if (subtype) {};
  return 0;
}


// Returns the function of userstring1 for type.
int styles_logic_get_userstring1_function (int type)
{
  return 0;
}


// Returns the function of userstring2 for type and subtype
int styles_logic_get_userstring2_function ([[maybe_unused]]int type, [[maybe_unused]]int subtype)
{
  return 0;
}


// Returns the function of userstring3 for type and subtype
int styles_logic_get_userstring3_function (int type, int subtype)
{
  if (type) {};
  if (subtype) {};
  return 0;
}


// It returns true if the combination of type and subtype start a new line in well-formed USFM.
// Otherwise it returns false.
bool styles_logic_starts_new_line_in_usfm (int type, [[maybe_unused]]int subtype)
{
  return false;
}


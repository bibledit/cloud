/*
Copyright (©) 2003-2021 Teus Benschop.

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

void editone_logic_prefix_html (string usfm, string stylesheet, string & html, string & last_p_style);
void editone_logic_editable_html (string usfm, string stylesheet, string & html);
void editone_logic_suffix_html (string editable_last_p_style, string usfm, string stylesheet, string & html);
string editone_logic_html_to_usfm (string stylesheet, string html);
void editone_logic_move_notes_v2 (string & prefix, string & suffix);

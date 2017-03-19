/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_EDITONE_LOGIC_H
#define INCLUDED_EDITONE_LOGIC_H


#include <config/libraries.h>


void editone_logic_prefix_html (string usfm, string stylesheet, string & html, string & last_p_style);
void editone_old_logic_editable_html (string prefix_last_p_style, string usfm, string stylesheet, string & html, string & editable_last_p_style, string & editable_first_applied_p_style);
void editone_logic_editable_html (string usfm, string stylesheet, string & html);
void editone_logic_suffix_html (string editable_last_p_style, string usfm, string stylesheet, string & html);
string editone_old_logic_html_to_usfm (string stylesheet, string html, string applied_p_style);
string editone_logic_html_to_usfm (string stylesheet, string html);
void editone_logic_move_notes (string & prefix, string & suffix);


#endif

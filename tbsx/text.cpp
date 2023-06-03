/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <tbsx/text.h>
#include <filter/string.h>
#include <filter/url.h>
using namespace std;


// Class for creating Trinitarian Bible Society online Bible documents. 


void Tbsx_Text::set_book_id (string id)
{
  flush ();
  output.push_back ("###" + id);
}


void Tbsx_Text::set_book_name (string name)
{
  flush ();
  output.push_back ("###! " + name);
}


void Tbsx_Text::set_chapter (int chapter)
{
  flush ();
  output.push_back ("##" + filter::strings::convert_to_string (chapter));
}


void Tbsx_Text::set_header (string header)
{
  flush ();
  output.push_back ("##! " + header);
}


void Tbsx_Text::open_paragraph ()
{
  flush ();
  in_note = false;
  output.push_back ("#%");
}


void Tbsx_Text::open_verse (int verse)
{
  flush ();
  in_note = false;
  add_text (filter::strings::convert_to_string (verse));
}


void Tbsx_Text::add_text (string text, bool supplied)
{
  if (supplied) buffer.append ("*");
  buffer.append (text);
  if (supplied) buffer.append ("*");
}


void Tbsx_Text::open_note ()
{
  buffer.append ("[");
  in_note = true;
}


void Tbsx_Text::close_note ()
{
  in_note = false;
  buffer.append ("]");
}


void Tbsx_Text::line_break ()
{
  flush ();
  in_note = false;
  output.push_back ("%");
}


void Tbsx_Text::flush ()
{
  // Add the text buffer to the output.
  if (!buffer.empty()) {
    output.push_back ("#" + buffer);
    buffer.clear();
  }
}


string Tbsx_Text::line ()
{
  return buffer;
}


string Tbsx_Text::get_document ()
{
  flush ();
  return filter::strings::implode (output, "\n");
}


void Tbsx_Text::save_document (string filename)
{
  filter_url_file_put_contents (filename, get_document ());
}

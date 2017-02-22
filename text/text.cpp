/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <text/text.h>
#include <filter/string.h>
#include <filter/url.h>


// Class for creating plain text documents.


void Text_Text::paragraph (string text)
{
  if (thisline != "") {
    // The filter that converts from USFM to clear texts inserts some stuff
    // that's being removed here again, as not desirable in clear text.
    thisline = filter_string_str_replace (en_space(), " ", thisline);
    thisline = filter_string_str_replace ("  ", " ", thisline);
    output.push_back (thisline);
    thisline = "";
  }
  addtext (text);
}


string Text_Text::line ()
{
  return thisline;
}


void Text_Text::addtext (string text)
{
  thisline += text;
}


string Text_Text::get ()
{
  paragraph ();
  return filter_string_implode (output, "\n");
}


void Text_Text::save (string name)
{
  filter_url_file_put_contents (name, get ());
}


// Opens a new clear text note.
void Text_Text::note (string text)
{
  if (!thisnoteline.empty ()) {
    notes.push_back (thisnoteline);
    thisnoteline.clear ();
  }
  addnotetext (text);
}


void Text_Text::addnotetext (string text)
{
  thisnoteline.append (text);
}


string Text_Text::getnote ()
{
  note ();
  return filter_string_implode (notes, "\n");
}



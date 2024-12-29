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


#include <text/text.h>
#include <filter/string.h>
#include <filter/url.h>


// Class for creating plain text documents.


void Text_Text::paragraph (std::string text)
{
  if (thisline != "") {
    // The filter that converts from USFM to clear texts inserts some stuff
    // that's being removed here again, as not desirable in clear text.
    thisline = filter::strings::replace (filter::strings::en_space_u2002(), " ", thisline);
    thisline = filter::strings::collapse_whitespace (thisline);
    output.push_back (thisline);
    thisline = "";
  }
  addtext (text);
}


std::string Text_Text::line ()
{
  return thisline;
}


void Text_Text::addtext (std::string text)
{
  thisline += text;
}


std::string Text_Text::get ()
{
  paragraph ();
  return filter::strings::implode (output, "\n");
}


void Text_Text::save (std::string name)
{
  filter_url_file_put_contents (name, get ());
}


// Opens a new clear text note.
void Text_Text::note (std::string text)
{
  if (!thisnoteline.empty ()) {
    notes.push_back (thisnoteline);
    thisnoteline.clear ();
  }
  addnotetext (text);
}


void Text_Text::addnotetext (std::string text)
{
  thisnoteline.append (text);
}


std::string Text_Text::getnote ()
{
  note ();
  return filter::strings::implode (notes, "\n");
}

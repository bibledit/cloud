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


#include <filter/note.h>
#include <styles/logic.h>
#include <filter/string.h>


namespace filter::note {


citation::citation ()
{
  pointer = 0;
}

void citation::set_sequence (int numbering, const std::string& usersequence)
{
  if (numbering == NoteNumbering123) {
    this->sequence.clear();
  }
  else if (numbering == NoteNumberingAbc) {
    this->sequence = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};
  }
  else if (numbering == NoteNumberingUser) {
    if (!usersequence.empty()) this->sequence = filter::strings::explode (usersequence, ' ');
  }
  else {
    this->sequence = {"1", "2", "3", "4", "5", "6", "7", "8", "9"}; // Fallback sequence.
  }
  // How the above works:
  // The note will be numbered as follows:
  // If a sequence is given, then this sequence is followed for the citations.
  // If no sequence is given, then the note gets numerical citations.
}

void citation::set_restart (int setting)
{
  if (setting == NoteRestartNumberingNever) this->restart = "never";
  else if (setting == NoteRestartNumberingEveryBook) this->restart = "book";
  else this->restart = "chapter";
}

std::string citation::get (std::string citation_in)
{
  // Handle USFM automatic note citation.
  if (citation_in == "+") {
    // If the sequence is empty, then the note citation starts at 1 and increases each time.
    if (sequence.empty()) {
      pointer++;
      citation_in = std::to_string (pointer);
    }
    // The sequence of note callers is not empty.
    // So take the note citaton from the sequence,
    // and then iterate to the next one.
    else {
      citation_in = sequence [pointer];
      pointer++;
      if (pointer >= sequence.size ()) pointer = 0;
    }
  }

  // Handle situation in USFM that no note citation is to be displayed.
  else if (citation_in == "-") {
    citation_in.clear();
  }
  
  // Done.
  return citation_in;
}

void citation::run_restart (const std::string& moment)
{
  if (restart == moment) {
    pointer = 0;
  }
}

void citations::evaluate_style (const Database_Styles_Item & style)
{
  // Evaluate the style to find out whether to create a note citation for it.
  bool create = false;
  if (style.type == StyleTypeFootEndNote) {
    if (style.subtype == FootEndNoteSubtypeFootnote) create = true;
    if (style.subtype == FootEndNoteSubtypeEndnote) create = true;
  }
  if (style.type == StyleTypeCrossreference) {
    if (style.subtype == CrossreferenceSubtypeCrossreference) create = true;
  }
  if (!create) return;

  // Create a new note citation at this point.
  citation citation;
  // Handle caller sequence.
  citation.set_sequence(style.userint1, style.userstring1);
  // Handle note caller restart moment.
  citation.set_restart(style.userint2);
  // Store the citation for later use.
  cache [style.marker] = citation;
}


std::string citations::get (const std::string& marker, const std::string& citation)
{
  return cache[marker].get(citation);
}


// This resets the note citations data.
// Resetting means that the note citations start to count afresh.
// $moment: what type of reset to apply, e.g. 'chapter' or 'book'.
void citations::restart (const std::string& moment)
{
  for (auto & notecitation : cache) {
    notecitation.second.run_restart (moment);
  }
}


}

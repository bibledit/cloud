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
#include <database/styles.h>


namespace filter::note {


citation::citation ()
{
  pointer = 0;
}

void citation::set_sequence (std::string sequence_in)
{
  sequence = filter::strings::explode (std::move(sequence_in), ' ');
  // The note will be numbered as follows:
  // If a sequence is given, then this sequence is followed for the citations.
  // If an empty sequence is given, then the note gets ever increasing numerical citations.
}

void citation::set_restart (const std::string& setting)
{
  // Check if the input is valid, if so, store it, else store default restart moment.
  using namespace stylesv2;
  if (setting == notes_numbering_restart_never)
    restart = notes_numbering_restart_never;
  else if (setting == notes_numbering_restart_book)
    restart = notes_numbering_restart_book;
  else
    restart = notes_numbering_restart_chapter;
}

std::string citation::get (std::string citation_in)
{
  // Handle USFM automatic note citation.
  if (citation_in == "+") {
    // If the sequence is empty, then the note citation starts at 1 and keeps increasing each time.
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
      if (pointer >= sequence.size ())
        pointer = 0;
    }
  }

  // Handle situation in USFM that no note citation is to be displayed.
  else if (citation_in == "-") {
    citation_in.clear();
  }
  
  // If no update is made on the citation input, it is passed on as i.
  return citation_in;
}

void citation::run_restart(const std::string& moment)
{
  if (restart == moment) {
    pointer = 0;
  }
}


void citations::evaluate_style (const stylesv2::Style& style)
{
  // Evaluate the style to find out whether to create a note citation for it.
  const auto create = [&style] () {
    if (style.type == stylesv2::Type::footnote_wrapper)
      return true;
    if (style.type == stylesv2::Type::endnote_wrapper)
      return true;
    if (style.type == stylesv2::Type::crossreference_wrapper)
      return true;
    return false;
  };
  if (!create())
    return;
  
  // Create a new note citation at this point.
  citation citation;
  // Handle caller sequence.
  std::string sequence = stylesv2::get_parameter<std::string>(&style, stylesv2::Property::note_numbering_sequence);
  citation.set_sequence(std::move(sequence));
  // Handle note caller restart moment.
  citation.set_restart(stylesv2::get_parameter<std::string>(&style, stylesv2::Property::note_numbering_restart));
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
void citations::restart(const std::string& moment)
{
  for (auto & notecitation : cache) {
    notecitation.second.run_restart(moment);
  }
}


}

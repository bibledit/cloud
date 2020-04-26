/*
Copyright (©) 2003-2020 Teus Benschop.

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



// Looks at $verse, which can be a range of verses, or a sequence of them,
// and returns an array of all verse numbers contained in it.
function usfm_get_verse_numbers (verse)
{
  var verses = [];
  
  // If there is a range, take the beginning and the end, and fill up in between.
  if (verse.indexOf ("-") >= 0) {
    var position = verse.indexOf ("-");
    var start_range = parseInt (verse.substring (0, position));
    verse = verse.slice (position + 1);
    var end_range = parseInt (verse);
    for (i = start_range; i <= end_range; i++) {
      verses.push (parseInt (i));
    }
  }
  
  // Else if there is a sequence, take each verse in the sequence, and store it.
  else if (verse.indexOf (",") >= 0) {
    var iterations = 0;
    do {
      // In case of an unusual range formation, do not hang, but give message.
      iterations++;
      if (iterations > 50) {
        break;
      }
      var position = verse.indexOf (",");
      var vs;
      if (position < 0) {
        vs = verse;
        verse = "";
      } else {
        vs = verse.substring (0, position);
        position++;
        verse = verse.slice (position);
      }
      verses.push (parseInt (vs));
    } while (verse.length);
  }
  
  // No range and no sequence: a single verse.
  else {
    verses.push (parseInt (verse));
  }

  return verses;
}

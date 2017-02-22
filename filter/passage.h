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


#ifndef INCLUDED_FILTER_PASSAGE_H
#define INCLUDED_FILTER_PASSAGE_H


#include <config/libraries.h>


class Passage
{
public:
  Passage ();
  Passage (string bible_in, int book_in, int chapter_in, string verse_in);
  string bible;
  int book;
  int chapter;
  string verse;
  bool equal (Passage & passage);
  string encode ();
  static Passage decode (const string& encoded);
};


string filter_passage_display (int book, int chapter, string verse);
string filter_passage_display_inline (vector <Passage> passages);
string filter_passage_display_multiline (vector <Passage> passages);
int filter_passage_to_integer (Passage passage);
Passage filter_integer_to_passage (int integer);
int filter_passage_interpret_book (string book);
string filter_passage_clean_passage (string text);
Passage filter_passage_explode_passage (string text);
Passage filter_passage_interpret_passage (Passage currentPassage, string rawPassage);
vector <string> filter_passage_handle_sequences_ranges (const string& passage);
string filter_passage_link_for_opening_editor_at (int book, int chapter, string verse);
vector <int> filter_passage_get_ordered_books (const string& bible);


#endif

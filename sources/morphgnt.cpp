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


#include <sources/morphgnt.h>
#include <database/logs.h>
#include <database/morphgnt.h>
#include <filter/string.h>
#include <filter/url.h>


void sources_morphgnt_parse ()
{
  Database_Logs::log ("Start parsing MorphGNT");
  Database_MorphGnt database_morphgnt;
  database_morphgnt.create ();

  vector <string> files;
  DIR * dir = opendir ("sources/morphgnt");
  struct dirent * direntry;
  while ((direntry = readdir (dir)) != NULL) {
    string name = direntry->d_name;
    if (name.find ("morphgnt.txt") == string::npos) continue;
    files.push_back (name);
  }
  closedir (dir);

  set <string> parsings;
  
  for (auto file : files) {
    file.insert (0, "sources/morphgnt/");
    Database_Logs::log (file);
    string contents = filter_url_file_get_contents (file);
    vector <string> lines = filter_string_explode (contents, '\n');
    for (auto line : lines) {
      vector <string> bits = filter_string_explode (line, ' ');
      if (bits.size () != 7) {
        Database_Logs::log (line);
        Database_Logs::log ("Should be seven bits");
        continue;
      }

      string passage = bits [0];
      int book = convert_to_int (passage.substr (0, 2)) + 39;
      int chapter = convert_to_int (passage.substr (2, 2));
      int verse = convert_to_int (passage.substr (4, 2));
      string pos = bits[1];
      string parsing = bits[2];
      parsings.insert (parsing.substr (7, 1)); // degree
      string word = bits[3];
      string lemma = bits[6];

      // Casefold and transliterate the lemma: This enables searching on the lemma.
      lemma = unicode_string_casefold (lemma);
      lemma = unicode_string_transliterate (lemma);

      database_morphgnt.store (book, chapter, verse, pos, parsing, word, lemma);
    }
  }

  database_morphgnt.optimize ();
  Database_Logs::log ("Finished parsing MorphGNT");
}

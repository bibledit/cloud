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

  std::vector <std::string> files;
  DIR * dir = opendir ("sources/morphgnt");
  dirent * direntry;
  while ((direntry = readdir (dir)) != nullptr) {
    std::string name = direntry->d_name;
    if (name.find ("morphgnt.txt") == std::string::npos) continue;
    files.push_back (name);
  }
  closedir (dir);

  std::set <std::string> parsings;
  
  for (auto file : files) {
    file.insert (0, "sources/morphgnt/");
    Database_Logs::log (file);
    std::string contents = filter_url_file_get_contents (file);
    std::vector <std::string> lines = filter::string::explode (contents, '\n');
    for (auto line : lines) {
      std::vector <std::string> bits = filter::string::explode (line, ' ');
      if (bits.size () != 7) {
        Database_Logs::log (line);
        Database_Logs::log ("Should be seven bits");
        continue;
      }

      std::string passage = bits [0];
      int book = filter::string::convert_to_int (passage.substr (0, 2)) + 39;
      int chapter = filter::string::convert_to_int (passage.substr (2, 2));
      int verse = filter::string::convert_to_int (passage.substr (4, 2));
      std::string pos = bits[1];
      std::string parsing = bits[2];
      parsings.insert (parsing.substr (7, 1)); // degree
      std::string word = bits[3];
      std::string lemma = bits[6];

      // Casefold and transliterate the lemma: This enables searching on the lemma.
      lemma = filter::string::unicode_string_casefold (lemma);
      lemma = filter::string::unicode_string_transliterate (lemma);

      database_morphgnt.store (book, chapter, verse, pos, parsing, word, lemma);
    }
  }

  database_morphgnt.optimize ();
  Database_Logs::log ("Finished parsing MorphGNT");
}

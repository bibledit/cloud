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


#pragma once

#include <config/libraries.h>

class Database_Etcbc4
{
public:
  void create ();
  std::string raw (int book, int chapter, int verse);
  void store (int book, int chapter, int verse, std::string data);
  void store (int book, int chapter, int verse,
              std::string word, std::string vocalized_lexeme, std::string consonantal_lexeme,
              std::string gloss, std::string pos, std::string subpos,
              std::string gender, std::string number, std::string person,
              std::string state, std::string tense, std::string stem,
              std::string phrase_function, std::string phrase_type, std::string phrase_relation,
              std::string phrase_a_relation, std::string clause_text_type, std::string clause_type, std::string clause_relation);
  std::vector <int> books ();
  std::vector <int> chapters (int book);
  std::vector <int> verses (int book, int chapter);
  std::vector <int> rowids (int book, int chapter, int verse);
  std::string word (int rowid);
  std::string vocalized_lexeme (int rowid);
  std::string consonantal_lexeme (int rowid);
  std::string gloss (int rowid);
  std::string pos (int rowid);
  std::string subpos (int rowid);
  std::string gender (int rowid);
  std::string number (int rowid);
  std::string person (int rowid);
  std::string state (int rowid);
  std::string tense (int rowid);
  std::string stem (int rowid);
  std::string phrase_function (int rowid);
  std::string phrase_type (int rowid);
  std::string phrase_relation (int rowid);
  std::string phrase_a_relation (int rowid);
  std::string clause_text_type (int rowid);
  std::string clause_type (int rowid);
  std::string clause_relation (int rowid);
private:
  sqlite3 * connect ();
  int get_id (sqlite3 * db, const char * table_row, std::string item);
  std::string get_item (const char * item, int rowid);
};

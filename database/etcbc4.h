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


#ifndef INCLUDED_DATABASE_ETCBC4_H
#define INCLUDED_DATABASE_ETCBC4_H


#include <config/libraries.h>


class Database_Etcbc4
{
public:
  void create ();
  string raw (int book, int chapter, int verse);
  void store (int book, int chapter, int verse, string data);
  void store (int book, int chapter, int verse,
              string word, string vocalized_lexeme, string consonantal_lexeme,
              string gloss, string pos, string subpos,
              string gender, string number, string person,
              string state, string tense, string stem,
              string phrase_function, string phrase_type, string phrase_relation,
              string phrase_a_relation, string clause_text_type, string clause_type, string clause_relation);
  vector <int> books ();
  vector <int> chapters (int book);
  vector <int> verses (int book, int chapter);
  vector <int> rowids (int book, int chapter, int verse);
  string word (int rowid);
  string vocalized_lexeme (int rowid);
  string consonantal_lexeme (int rowid);
  string gloss (int rowid);
  string pos (int rowid);
  string subpos (int rowid);
  string gender (int rowid);
  string number (int rowid);
  string person (int rowid);
  string state (int rowid);
  string tense (int rowid);
  string stem (int rowid);
  string phrase_function (int rowid);
  string phrase_type (int rowid);
  string phrase_relation (int rowid);
  string phrase_a_relation (int rowid);
  string clause_text_type (int rowid);
  string clause_type (int rowid);
  string clause_relation (int rowid);
private:
  sqlite3 * connect ();
  int get_id (sqlite3 * db, const char * table_row, string item);
  string get_item (const char * item, int rowid);
};


#endif

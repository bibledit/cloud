/*
Copyright (©) 2003-2018 Teus Benschop.

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


#include <unittests/pairs.h>
#include <unittests/utilities.h>
#include <database/check.h>
#include <checks/pairs.h>


void test_pairs ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_Check database_check;
  database_check.create ();
  
  string bible = "bible";
  int book = 2;
  int chapter = 3;
  map <int, string> verses;
  vector <pair <string, string> > pairs = {
    make_pair ("[", "]"),
    make_pair ("(", ")"),
    make_pair ("“", "”"),
  };
  vector <Database_Check_Hit> results;
  
  {
    verses = {
      make_pair (2, "Verse two."),
      make_pair (3, "Verse three."),
      make_pair (4, "Verse four.")
    };
    Checks_Pairs::run (bible, book, chapter, verses, pairs, false);
    results = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, results.size());
    database_check.truncateOutput ("");
  }

  {
    verses = {
      make_pair (2, "Verse [two."),
      make_pair (3, "Verse three]."),
      make_pair (4, "Verse four.")
    };
    Checks_Pairs::run (bible, book, chapter, verses, pairs, false);
    results = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, results.size());
    database_check.truncateOutput ("");
  }

  {
    verses = {
      make_pair (2, "Verse [two."),
      make_pair (3, "Verse (three."),
      make_pair (4, "Verse four.")
    };
    Checks_Pairs::run (bible, book, chapter, verses, pairs, false);
    results = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, results.size());
    if (results.size () == 2) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, bible, hit.bible);
      evaluate (__LINE__, __func__, book, hit.book);
      evaluate (__LINE__, __func__, chapter, hit.chapter);
      evaluate (__LINE__, __func__, 2, hit.verse);
      evaluate (__LINE__, __func__, "Opening character \"[\" without its matching closing character \"]\"", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, "Opening character \"(\" without its matching closing character \")\"", hit.data);
    }
    database_check.truncateOutput ("");
  }

  {
    verses = {
      make_pair (2, "Verse [two."),
      make_pair (3, "Verse three."),
      make_pair (4, "Verse four).")
    };
    Checks_Pairs::run (bible, book, chapter, verses, pairs, false);
    results = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, results.size());
    if (results.size () == 2) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, bible, hit.bible);
      evaluate (__LINE__, __func__, book, hit.book);
      evaluate (__LINE__, __func__, chapter, hit.chapter);
      evaluate (__LINE__, __func__, 4, hit.verse);
      evaluate (__LINE__, __func__, "Closing character \")\" without its matching opening character \"(\"", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, 2, hit.verse);
      evaluate (__LINE__, __func__, "Opening character \"[\" without its matching closing character \"]\"", hit.data);
    }
    database_check.truncateOutput ("");
  }
}

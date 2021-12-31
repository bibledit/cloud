/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/verses.h>
#include <unittests/utilities.h>
#include <database/check.h>
#include <checks/verses.h>


void test_verses ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_Check database_check;
  database_check.create ();
  // Test Missing Punctuation At End
  {
    map <int, string> verses = {
      make_pair (2, "He said."),
      make_pair (3, "He didn't say"),
      make_pair (4, "He said."),
      make_pair (5, "He said: “Jesus.”")
    };
    Checks_Verses::missingPunctuationAtEnd ("1", 1, 1, verses, ", ;", ". ! ? :", "”");
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)results.size());
    if (results.size ()) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "1", hit.bible);
      evaluate (__LINE__, __func__, 1, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 3, hit.verse);
      evaluate (__LINE__, __func__, "No punctuation at end of verse: y", hit.data);
    }
  }
  database_check.truncateOutput ("");
  // Test Pattern
  {
    map <int, string> verses = {
      make_pair (2, "He said."),
      make_pair (3, "He didn't say"),
      make_pair (4, "He said.")
    };
    Checks_Verses::patterns ("1", 1, 1, verses, {"did"});
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)results.size());
    if (results.size ()) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "1", hit.bible);
      evaluate (__LINE__, __func__, 1, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 3, hit.verse);
      evaluate (__LINE__, __func__, "Pattern found in text: did", hit.data);
    }
  }
  database_check.truncateOutput ("");
  // Test Pattern
  {
    map <int, string> verses = {
      make_pair (2, "He said."),
      make_pair (3, "He didn't say"),
      make_pair (4, "He said.")
    };
    Checks_Verses::patterns ("1", 1, 1, verses, {"Did"});
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, (int)results.size());
  }
  database_check.truncateOutput ("");
  // Test Pattern
  {
    map <int, string> verses = {
      make_pair (2, "He said."),
      make_pair (3, "He didn't say"),
      make_pair (4, "He said.")
    };
    Checks_Verses::patterns ("1", 1, 1, verses, {"said"});
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)results.size());
    if (results.size () == 2) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "1", hit.bible);
      evaluate (__LINE__, __func__, 1, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 2, hit.verse);
      evaluate (__LINE__, __func__, "Pattern found in text: said", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, 2, hit.rowid);
      evaluate (__LINE__, __func__, "1", hit.bible);
      evaluate (__LINE__, __func__, 1, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 4, hit.verse);
      evaluate (__LINE__, __func__, "Pattern found in text: said", hit.data);
    }
  }
  database_check.truncateOutput ("");
}

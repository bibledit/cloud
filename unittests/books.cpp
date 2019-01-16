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


#include <unittests/archive.h>
#include <unittests/utilities.h>
#include <database/books.h>


// Tests for the Database_Books object.
void test_database_books ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  evaluate (__LINE__, __func__, 109, Database_Books::getIDs ().size());
  evaluate (__LINE__, __func__, 2, Database_Books::getIdFromEnglish ("Exodus"));
  evaluate (__LINE__, __func__, 0, Database_Books::getIdFromEnglish ("exodus"));
  evaluate (__LINE__, __func__, "Leviticus", Database_Books::getEnglishFromId (3));
  evaluate (__LINE__, __func__, "NUM", Database_Books::getUsfmFromId (4));
  evaluate (__LINE__, __func__, "Deu", Database_Books::getBibleworksFromId (5));
  evaluate (__LINE__, __func__, 22, Database_Books::getIdFromUsfm ("SNG"));
  evaluate (__LINE__, __func__, 13, Database_Books::getIdFromOsis ("1Chr"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdFromBibleworks ("2Ki"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdLikeText ("2Ki"));
  evaluate (__LINE__, __func__, 14, Database_Books::getIdLikeText ("2Chron"));
  evaluate (__LINE__, __func__, 1, Database_Books::getIdLikeText ("Genes"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdFromOnlinebible ("2Ki"));
  evaluate (__LINE__, __func__, "De", Database_Books::getOnlinebibleFromId (5));
  evaluate (__LINE__, __func__, 7, Database_Books::getOrderFromId (5));
  evaluate (__LINE__, __func__, "nt", Database_Books::getType (40));
  evaluate (__LINE__, __func__, "ot", Database_Books::getType (39));
  evaluate (__LINE__, __func__, "", Database_Books::getType (0));
  evaluate (__LINE__, __func__, 105, Database_Books::getIdFromUsfm ("INT"));
}



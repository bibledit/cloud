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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/localization.h>
#include <unittests/utilities.h>
#include <filter/url.h>
#include <database/localization.h>
using namespace std;


TEST (database, localization)
{
  refresh_sandbox (true);
  string file_po = filter_url_create_root_path ({"unittests", "tests", "nl.po"});
  Database_Localization database_localization = Database_Localization ("nl");
  database_localization.create (file_po);
  
  string msgid = "phpunit";
  string msgstr = "phpunit";
  string result = database_localization.translate (msgid);
  EXPECT_EQ (msgstr, result);
  result = database_localization.backtranslate (msgstr);
  EXPECT_EQ (msgid, result);
  
  msgid = "To display all the notes for a certain passage, enter the following URL:";
  msgstr = "Om alle aantekeningen voor een bepaalde passage te tonen voert u de volgende URL in:";
  result = database_localization.translate (msgid);
  EXPECT_EQ (msgstr, result);
  result = database_localization.backtranslate (msgstr);
  EXPECT_EQ (msgid, result);
}

#endif

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <checks/space.h>
#include <database/check.h>


TEST (checks, space)
{
  refresh_sandbox (false);
  database::check::create ();
  std::string bible = "unit test";
  
  // Test reporting space at end of verse: No spaces here.
  {
    database::check::truncate_output (bible);
    std::string usfm =
    "\\v 2 This is verse 2.\n"
    "\\v 3 This is verse 3.\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, hits.size ());
  }

  // Test reporting space at end of verse: One space here.
  {
    database::check::truncate_output (bible);
    std::string usfm =
    "\\v 4 This is verse 4. \n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }

  // Test reporting space at end of verse: One space here.
  // The space is not right at the end in the USFM.
  // The space is followed by empty embedded markup.
  // So in the pure text stream, the space is at the end of a verse.
  // Check that it gets reported.
  {
    database::check::truncate_output (bible);
    std::string usfm =
    "\\v 5 This is verse \\add 5. \\add*\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  
  // Test there's no space in the cleaned text at the end of the verse.
  {
    database::check::truncate_output (bible);
    std::string usfm =
    "\\v 6 This is verse \\add 6.\\add*\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, hits.size ());
  }
  
  // Check that it catches a double space in USFM.
  {
    database::check::truncate_output (bible);
    std::string usfm = R"(\v 1 This contains  a double space.)";
    checks::space::double_space_usfm (bible, 2, 3, 4, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  
  // Check that it transposes spaces in notes.
  {
    std::string usfm = R"(\v 1 Verse\f + \fr 3.1\fk  keyword\ft  Text.\f* one.)";
    std::string standard = R"(\v 1 Verse\f + \fr 3.1 \fk keyword \ft Text.\f* one.)";
    bool transposed = checks::space::transpose_note_space (usfm);
    EXPECT_EQ (true, transposed);
    EXPECT_EQ (standard, usfm);
  }
  {
    std::string usfm = R"(\v 2 Verse\x + \xo 3.2\xt  Text.\x* two.)";
    std::string standard = R"(\v 2 Verse\x + \xo 3.2 \xt Text.\x* two.)";
    bool transposed = checks::space::transpose_note_space (usfm);
    EXPECT_EQ (true, transposed);
    EXPECT_EQ (standard, usfm);
  }
  
  // Check on a space before final note and cross reference markup.
  {
    database::check::truncate_output (bible);
    int verse = 4;
    std::string usfm = R"(\v 1 Note \f ... \f*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  {
    database::check::truncate_output (bible);
    int verse = 5;
    std::string usfm = R"(\v 2 Endnote \fe ... \fe*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  {
    database::check::truncate_output (bible);
    int verse = 6;
    std::string usfm = R"(\v 3 Cross reference \x ... \x*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }

}

#endif


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
#include <database/state.h>


TEST (database, state)
{
  refresh_sandbox (false);
  Database_State::create ();

  // Test notes checksums.
  {
    // No checksum yet.
    EXPECT_EQ ("", Database_State::getNotesChecksum (100, 1000));
    
    // Store and retrieve checksum in a defined range.
    Database_State::putNotesChecksum (100, 1000, "phpunit");
    EXPECT_EQ ("phpunit", Database_State::getNotesChecksum (100, 1000));
    // Store it again, with a different value.
    Database_State::putNotesChecksum (100, 1000, "phpunit2");
    EXPECT_EQ ("phpunit2", Database_State::getNotesChecksum (100, 1000));
    
    // Erase a note within the defined range, which should erase that range.
    Database_State::eraseNoteChecksum (100);
    EXPECT_EQ ("", Database_State::getNotesChecksum (100, 1000));
    
    // Define a few ranges, store checksums, and erase one note within that range, and test it.
    Database_State::putNotesChecksum (100, 1000, "100-1000");
    Database_State::putNotesChecksum (200, 1100, "200-1100");
    Database_State::putNotesChecksum (300, 900,  "300-900");
    Database_State::putNotesChecksum (2000, 9000, "2000-9000");
    EXPECT_EQ ("100-1000", Database_State::getNotesChecksum (100,  1000));
    EXPECT_EQ ("200-1100", Database_State::getNotesChecksum (200,  1100));
    EXPECT_EQ ("300-900", Database_State::getNotesChecksum (300,  900));
    EXPECT_EQ ("2000-9000", Database_State::getNotesChecksum (2000, 9000));
    Database_State::eraseNoteChecksum (500);
    EXPECT_EQ ("", Database_State::getNotesChecksum (100,  1000));
    EXPECT_EQ ("", Database_State::getNotesChecksum (200,  1100));
    EXPECT_EQ ("", Database_State::getNotesChecksum (300,  900));
    EXPECT_EQ ("2000-9000", Database_State::getNotesChecksum (2000, 9000));
  }

  // Test export flagging.
  {
    // Flag some data for export.
    Database_State::setExport ("1", 2, 3);
    Database_State::setExport ("4", 5, 6);
    Database_State::setExport ("7", 8, 9);
    
    // Test the data.
    EXPECT_EQ (true, Database_State::getExport ("1", 2, 3));
    EXPECT_EQ (true, Database_State::getExport ("4", 5, 6));
    EXPECT_EQ (false, Database_State::getExport ("1", 2, 1));
    
    // Clear flag.
    Database_State::clearExport ("1", 2, 3);
    
    // Test the data.
    EXPECT_EQ (false, Database_State::getExport ("1", 2, 3));
    EXPECT_EQ (true, Database_State::getExport ("4", 5, 6));
    EXPECT_EQ (false, Database_State::getExport ("1", 2, 1));
  }
}


#endif


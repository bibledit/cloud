/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <database/jobs.h>


TEST (database, jobs)
{
  {
    refresh_sandbox (false);
    Database_Jobs database_jobs = Database_Jobs ();
    database_jobs.create ();
    
    // Test Optimize
    database_jobs.optimize ();
    
    // Test Identifiers
    int id = database_jobs.get_new_id ();
    bool exists = database_jobs.id_exists (id);
    EXPECT_EQ (true, exists);
    
    // Test Level.
    id = database_jobs.get_new_id ();
    int level = database_jobs.get_level (id);
    EXPECT_EQ (0, level);
    database_jobs.set_level (id, 123);
    level = database_jobs.get_level (id);
    EXPECT_EQ (123, level);
    
    // Test Start
    id = database_jobs.get_new_id ();
    std::string start = database_jobs.get_start (id);
    EXPECT_EQ ("", start);
    database_jobs.set_start (id, "start");
    start = database_jobs.get_start (id);
    EXPECT_EQ ("start", start);
    
    // Test Progress
    id = database_jobs.get_new_id ();
    std::string progress = database_jobs.get_progress (id);
    EXPECT_EQ ("", progress);
    database_jobs.set_progress (id, "progress");
    progress = database_jobs.get_progress (id);
    EXPECT_EQ ("progress", progress);
    
    // Test Percentage
    id = database_jobs.get_new_id ();
    std::string percentage = database_jobs.get_percentage (id);
    EXPECT_EQ ("", percentage);
    database_jobs.set_percentage (id, 55);
    percentage = database_jobs.get_percentage (id);
    EXPECT_EQ ("55", percentage);
    
    // Test Result.
    id = database_jobs.get_new_id ();
    std::string result = database_jobs.get_result (id);
    EXPECT_EQ ("", result);
    database_jobs.set_result (id, "result");
    result = database_jobs.get_result (id);
    EXPECT_EQ ("result", result);
  }
}

#endif

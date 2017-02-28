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


#include <unittests/jobs.h>
#include <unittests/utilities.h>
#include <database/jobs.h>


void test_database_jobs ()
{
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    Database_Jobs database_jobs = Database_Jobs ();
    database_jobs.create ();
    
    // Test Optimize
    database_jobs.optimize ();
    
    // Test Identifiers
    int id = database_jobs.getNewId ();
    bool exists = database_jobs.idExists (id);
    evaluate (__LINE__, __func__, true, exists);
    
    // Test Level.
    id = database_jobs.getNewId ();
    int level = database_jobs.getLevel (id);
    evaluate (__LINE__, __func__, 0, level);
    database_jobs.setLevel (id, 123);
    level = database_jobs.getLevel (id);
    evaluate (__LINE__, __func__, 123, level);
    
    // Test Start
    id = database_jobs.getNewId ();
    string start = database_jobs.getStart (id);
    evaluate (__LINE__, __func__, "", start);
    database_jobs.setStart (id, "start");
    start = database_jobs.getStart (id);
    evaluate (__LINE__, __func__, "start", start);
    
    // Test Progress
    id = database_jobs.getNewId ();
    string progress = database_jobs.getProgress (id);
    evaluate (__LINE__, __func__, "", progress);
    database_jobs.setProgress (id, "progress");
    progress = database_jobs.getProgress (id);
    evaluate (__LINE__, __func__, "progress", progress);
    
    // Test Percentage
    id = database_jobs.getNewId ();
    string percentage = database_jobs.getPercentage (id);
    evaluate (__LINE__, __func__, "", percentage);
    database_jobs.setPercentage (id, 55);
    percentage = database_jobs.getPercentage (id);
    evaluate (__LINE__, __func__, "55", percentage);
    
    // Test Result.
    id = database_jobs.getNewId ();
    string result = database_jobs.getResult (id);
    evaluate (__LINE__, __func__, "", result);
    database_jobs.setResult (id, "result");
    result = database_jobs.getResult (id);
    evaluate (__LINE__, __func__, "result", result);
  }
}

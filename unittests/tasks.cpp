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
#include <tasks/logic.h>


TEST (tasks, logic)
{
  refresh_sandbox (false);
  tasks_logic_queue ("task1");
  tasks_logic_queue ("task3");
  tasks_logic_queue ("task4", { "parameter1", "parameter2" });
  EXPECT_EQ (true, tasks_logic_queued ("task1"));
  EXPECT_EQ (false, tasks_logic_queued ("task2"));
  EXPECT_EQ (false, tasks_logic_queued ("task1", { "parameter" }));
  EXPECT_EQ (true, tasks_logic_queued ("task4"));
  EXPECT_EQ (true, tasks_logic_queued ("task4", { "parameter1" }));
  EXPECT_EQ (true, tasks_logic_queued ("task4", { "parameter1", "parameter2" }));
  EXPECT_EQ (false, tasks_logic_queued ("task4", { "parameter1", "parameter3" }));
  EXPECT_EQ (false, tasks_logic_queued ("task4", { "parameter2" }));
  
}

#endif


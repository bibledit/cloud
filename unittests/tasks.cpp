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


#include <unittests/tasks.h>
#include <unittests/utilities.h>
#include <tasks/logic.h>


void test_tasks_logic ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  tasks_logic_queue ("task1");
  tasks_logic_queue ("task3");
  tasks_logic_queue ("task4", { "parameter1", "parameter2" });
  evaluate (__LINE__, __func__, true, tasks_logic_queued ("task1"));
  evaluate (__LINE__, __func__, false, tasks_logic_queued ("task2"));
  evaluate (__LINE__, __func__, false, tasks_logic_queued ("task1", { "parameter" }));
  evaluate (__LINE__, __func__, true, tasks_logic_queued ("task4"));
  evaluate (__LINE__, __func__, true, tasks_logic_queued ("task4", { "parameter1" }));
  evaluate (__LINE__, __func__, true, tasks_logic_queued ("task4", { "parameter1", "parameter2" }));
  evaluate (__LINE__, __func__, false, tasks_logic_queued ("task4", { "parameter1", "parameter3" }));
  evaluate (__LINE__, __func__, false, tasks_logic_queued ("task4", { "parameter2" }));
  
}

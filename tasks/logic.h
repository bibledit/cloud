/*
Copyright (©) 2003-2026 Teus Benschop.

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


#pragma once

#include <config/libraries.h>
#include "enums.h"

std::string to_string (const tasks::enums::task& task);
void tasks_logic_queue (tasks::enums::task task, std::vector <std::string> parameters = {});
bool tasks_logic_queued (tasks::enums::task task, std::vector <std::string> parameters = {});
void tasks_logic_start_thread_pool(std::size_t num_threads);
void tasks_logic_stop_thread_pool();
int tasks_logic_queue_size ();
int tasks_logic_active_jobs_count ();

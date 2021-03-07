/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <checks/logic.h>
#include <database/bibles.h>
#include <tasks/logic.h>


void checks_logic_start_all ()
{
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibles.getBibles ();
  for (auto bible : bibles) {
    checks_logic_start (bible);
  }
}


void checks_logic_start (string bible)
{
  tasks_logic_queue (CHECKBIBLE, {bible});
}

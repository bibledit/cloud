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


#ifndef INCLUDED_SETUP_LOGIC_H
#define INCLUDED_SETUP_LOGIC_H


#include <config/libraries.h>


void setup_conditionally (const char * package);
void setup_copy_library (const char * package);
void setup_write_access ();
void setup_wait_till_main_folders_present ();
void setup_initialize_data ();
void setup_set_admin_details (string username, string password, string email);
void setup_complete_gui ();
void setup_generate_locale_databases (bool progress);
void setup_generate_verse_mapping_databases ();
void setup_generate_versification_databases ();


#endif

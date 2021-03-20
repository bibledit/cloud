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


#ifndef INCLUDED_DEMO_LOGIC_H
#define INCLUDED_DEMO_LOGIC_H


#include <config/libraries.h>


bool demo_acl (string user, string pass);
string demo_address ();
string demo_address_secure ();
int demo_port ();
int demo_port_secure ();
string demo_client_warning ();
void demo_clean_data ();
string demo_sample_bible_name ();
void demo_create_sample_bible ();
void demo_prepare_sample_bible ();
void demo_create_sample_notes (void * webserver_request);
string demo_workspace ();
void demo_create_sample_workspaces (void * webserver_request);
vector <string> demo_logic_default_resources ();


#endif

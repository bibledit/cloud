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

class Webserver_Request;

bool demo_acl (std::string user, std::string pass);

// Returns the address of the current demo server.
constexpr std::string_view demo_address () {return "http://bibledit.org";}
constexpr std::string_view demo_address_secure () {return "https://bibledit.org";}

// The port number of the current demo server.
constexpr int demo_port () {return 8090;}
constexpr int demo_port_secure () {return 8091;}

std::string demo_client_warning ();
void demo_clean_data ();
std::string demo_sample_bible_name ();
void demo_create_sample_bible ();
void demo_prepare_sample_bible ();
void demo_create_sample_notes (Webserver_Request& webserver_request);
std::string demo_workspace ();
std::vector <std::string> demo_logic_default_resources ();

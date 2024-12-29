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


#pragma once

#include <config/libraries.h>

namespace database::cache::sql {

std::string fragment ();
void create (const std::string& resource, int book);
void remove (const std::string& resource);
void remove (const std::string&, int book);
bool exists (const std::string& resource);
bool exists (const std::string& resource, int book);
bool exists (const std::string& resource, int book, int chapter, int verse);
void cache (const std::string& resource, int book, int chapter, int verse, const std::string& value);
std::string retrieve (const std::string& resource, const int book, const int chapter, const int verse);
int count (const std::string& resource);
bool ready (const std::string& resource, const int book);
void ready (const std::string& resource, const int book, const bool ready);
int size (const std::string& resource, const int book);
std::string path (const std::string& resource, int book);

}

namespace database::cache::file {

bool exists (std::string schema);
void put (std::string schema, const std::string& contents);
std::string get (std::string schema);
void trim (bool clear);

}

namespace database::cache {

bool can_cache (const std::string & error, const std::string & html);

}

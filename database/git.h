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


#ifdef HAVE_CLOUD


namespace database::git {

void create ();
void optimize ();
void store_chapter (const std::string& user, const std::string& bible, int book, int chapter,
                    const std::string& oldusfm, const std::string& newusfm);
std::vector <std::string> get_users (const std::string& bible);
std::vector <int> get_rowids (const std::string& user, const std::string& bible);
bool get_chapter (int rowid,
                  std::string & user, std::string & bible, int & book, int & chapter,
                  std::string & oldusfm, std::string & newusfm);
void erase_rowid (int rowid);
void touch_timestamps (int timestamp);

}


#endif

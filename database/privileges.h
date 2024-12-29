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

class DatabasePrivileges
{
public:
  static const char * database ();
  static void create ();
  static void upgrade ();
  static void optimize ();
  static bool healthy ();
  static std::string save (const std::string& username);
  static void load (const std::string& username, const std::string & data);
  static void set_bible_book (const std::string& username, const std::string& bible, const int book, const bool write);
  static void set_bible (const std::string& username, const std::string& bible, const bool write);
  static void get_bible_book (const std::string& username, const std::string& bible, const int book, bool & read, bool & write);
  static std::tuple <bool, bool> get_bible (const std::string& username, const std::string& bible);
  static int get_bible_book_count ();
  static bool get_bible_book_exists (const std::string& username, const std::string& bible, const int book);
  static void remove_bible_book (const std::string& username, const std::string& bible, const int book);
  static void remove_bible (const std::string& bible);
  static void set_feature (const std::string& username, const int feature, const bool enabled);
  static bool get_feature (const std::string& username, const int feature);
  static void remove_user (const std::string& username);
private:
  static const char * bibles_start ();
  static const char * bibles_end ();
  static const char * features_start ();
  static const char * features_end ();
  static const char * on ();
  static const char * off ();
};

std::string database_privileges_directory (const std::string & user);
std::string database_privileges_file ();
std::string database_privileges_client_path (const std::string & user);
void database_privileges_client_create (const std::string & user, bool force);
void database_privileges_client_remove (const std::string & user);

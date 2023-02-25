/*
Copyright (©) 2003-2023 Teus Benschop.

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

class Database_Privileges
{
public:
  static const char * database ();
  static void create ();
  static void upgrade ();
  static void optimize ();
  static bool healthy ();
  static std::string save (std::string username);
  static void load (std::string username, const std::string & data);
  static void setBibleBook (std::string username, std::string bible, int book, bool write);
  static void setBible (std::string username, std::string bible, bool write);
  static void getBibleBook (std::string username, std::string bible, int book, bool & read, bool & write);
  static std::tuple <bool, bool> getBible (std::string username, std::string bible);
  static int getBibleBookCount ();
  static bool getBibleBookExists (std::string username, std::string bible, int book);
  static void removeBibleBook (std::string username, std::string bible, int book);
  static void removeBible (std::string bible);
  static void setFeature (std::string username, int feature, bool enabled);
  static bool getFeature (std::string username, int feature);
  static void removeUser (std::string username);
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

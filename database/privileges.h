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


#ifndef INCLUDED_DATABASE_PRIVILEGES_H
#define INCLUDED_DATABASE_PRIVILEGES_H


#include <config/libraries.h>


class Database_Privileges
{
public:
  static const char * database ();
  static void create ();
  static void upgrade ();
  static void optimize ();
  static bool healthy ();
  static string save (string username);
  static void load (string username, const string & data);
  static void setBibleBook (string username, string bible, int book, bool write);
  static void setBible (string username, string bible, bool write);
  static void getBibleBook (string username, string bible, int book, bool & read, bool & write);
  static void getBible (string username, string bible, bool & read, bool & write);
  static int getBibleBookCount ();
  static bool getBibleBookExists (string username, string bible, int book);
  static void removeBibleBook (string username, string bible, int book);
  static void removeBible (string bible);
  static void setFeature (string username, int feature, bool enabled);
  static bool getFeature (string username, int feature);
  static void removeUser (string username);
private:
  static const char * bibles_start ();
  static const char * bibles_end ();
  static const char * features_start ();
  static const char * features_end ();
  static const char * on ();
  static const char * off ();
};


string database_privileges_directory (const string & user);
string database_privileges_file ();
string database_privileges_client_path (const string & user);
void database_privileges_client_create (const string & user, bool force);
void database_privileges_client_remove (const string & user);


#endif

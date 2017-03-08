/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_USERS_H
#define INCLUDED_DATABASE_USERS_H


#include <config/libraries.h>


class Database_Users
{
public:
  void create ();
  void upgrade ();
  void trim ();
  void optimize ();
  void add_user (string user, string password, int level, string email);
  void set_password (string user, string password);
  bool matchUserPassword (string user, string password);
  bool matchEmailPassword (string email, string password);
  string add_userQuery (string user, string password, int level, string email);
  string getEmailToUser (string email);
  string get_email (string user);
  bool usernameExists (string user);
  bool emailExists (string email);
  int get_level (string user);
  void set_level (string user, int level);
  void removeUser (string user);
  vector <string> getAdministrators ();
  string updateEmailQuery (string user, string email);
  void updateUserEmail (string user, string email);
  vector <string> getUsers ();
  string get_md5 (string user);
  void execute (string sqlfragment);
  void set_ldap (string user, bool on);
  bool get_ldap (string user);
  void set_enabled (string user, bool on);
  bool get_enabled (string user);
private:
  const char * filename ();
};


#endif

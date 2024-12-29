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

class Database_Users
{
public:
  void create ();
  void upgrade ();
  void trim ();
  void optimize ();
  void add_user (std::string user, std::string password, int level, std::string email);
  void set_password (std::string user, std::string password);
  bool matchUserPassword (std::string user, std::string password);
  bool matchEmailPassword (std::string email, std::string password);
  std::string add_userQuery (std::string user, std::string password, int level, std::string email);
  std::string getEmailToUser (std::string email);
  std::string get_email (std::string user);
  bool usernameExists (std::string user);
  bool emailExists (std::string email);
  int get_level (std::string user);
  void set_level (std::string user, int level);
  void removeUser (std::string user);
  std::vector <std::string> getAdministrators ();
  std::string updateEmailQuery (std::string user, std::string email);
  void updateUserEmail (std::string user, std::string email);
  std::vector <std::string> get_users ();
  std::string get_md5 (std::string user);
  void execute (std::string sqlfragment);
  void set_ldap (std::string user, bool on);
  bool get_ldap (std::string user);
  void set_enabled (std::string user, bool on);
  bool get_enabled (std::string user);
private:
  const char * filename ();
};

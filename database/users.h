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

namespace database::users {

}

class Database_Users
{
public:
  void create () const;
  void upgrade () const;
  void trim ();
  void optimize ();
  void add_user (const std::string& user, const std::string& password, int level, const std::string& email);
  void set_password (const std::string& user, const std::string& password);
  [[nodiscard]] bool match_user_password (const std::string& user, const std::string& password) const;
  [[nodiscard]] bool match_email_password (std::string email, std::string password);
  [[nodiscard]] std::string add_user_query (std::string user, std::string password, int level, std::string email);
  [[nodiscard]] std::string get_email_to_user (std::string email);
  [[nodiscard]] std::string get_email (std::string user);
  [[nodiscard]] bool username_exists (std::string user);
  [[nodiscard]] bool email_exists (std::string email);
  [[nodiscard]] int get_level (std::string user);
  void set_level (std::string user, int level);
  void remove_user (std::string user);
  [[nodiscard]] std::vector <std::string> getAdministrators ();
  [[nodiscard]] std::string update_email_query (std::string user, std::string email);
  void update_user_email (std::string user, std::string email);
  [[nodiscard]] std::vector <std::string> get_users () const;
  [[nodiscard]] std::string get_md5 (std::string user);
  void execute (std::string sqlfragment);
  void set_ldap (std::string user, bool on);
  [[nodiscard]] bool get_ldap (std::string user);
  void set_enabled (std::string user, bool on);
  [[nodiscard]] bool get_enabled (std::string user);
private:
  [[nodiscard]] const char * filename () const;
};

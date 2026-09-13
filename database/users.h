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

void create();
void upgrade();
void trim();
void optimize();
void add_user (const std::string& user, const std::string& password, int level, const std::string& email);
void set_password (const std::string& user, const std::string& password);
[[nodiscard]] bool match_user_password (const std::string& user, const std::string& password);
[[nodiscard]] bool match_email_password (const std::string& email, const std::string& password);
[[nodiscard]] std::string add_user_query (std::string user, std::string password, int level, std::string email);
[[nodiscard]] std::string get_email_to_user (const std::string& email);
[[nodiscard]] std::string get_email (const std::string& user);
[[nodiscard]] bool username_exists (const std::string& user);
[[nodiscard]] bool email_exists (const std::string& email);
[[nodiscard]] int get_level (const std::string& user);
void set_level (const std::string& user, int level);
void remove_user (const std::string& user);
[[nodiscard]] std::vector <std::string> get_administrators ();
[[nodiscard]] std::string update_email_query (const std::string& user, const std::string& email);
void execute (const std::string& sql_fragment);
void update_user_email (const std::string& user, const std::string& email);
[[nodiscard]] std::vector <std::string> get_users ();
[[nodiscard]] std::string get_md5 (const std::string& user);
void set_ldap (const std::string& user, bool on);
[[nodiscard]] bool get_ldap (const std::string& user);
void set_enabled (const std::string& user, bool on);
[[nodiscard]] bool get_enabled (const std::string& user);
}

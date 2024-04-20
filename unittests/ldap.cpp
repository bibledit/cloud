/*
Copyright (©) 2003-2024 Teus Benschop.

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/url.h>
#include <ldap/logic.h>
#include <filter/roles.h>
#include <database/logs.h>


TEST (ldap, logic)
{
  refresh_sandbox (false);
  
  // Copy the default LDAP server configuration into place.
  std::string ldap_txt = filter_url_create_root_path ({config::logic::config_folder (), "ldap.txt"});
  std::string ldap_conf = filter_url_create_root_path ({config::logic::config_folder (), "ldap.conf"});
  filter_url_file_cp (ldap_txt, ldap_conf);
  // Initialize LDAP configuration.
  ldap_logic_initialize ();
  
  // Authenticate a user and check the results.
  std::string user = "boyle";
  std::string password = "password";
  bool okay;
  bool access;
  std::string email;
  int role;
  okay = ldap_logic_fetch (user, password, access, email, role, false);
  EXPECT_EQ (true, okay);
  EXPECT_EQ (true, access);
  EXPECT_EQ ("boyle@ldap.forumsys.com", email);
  EXPECT_EQ (Filter_Roles::guest (), role);
  
  // Check there is one journal entry as a result of authenticating a user.
  std::string last = "0";
  std::vector <std::string> logs = Database_Logs::get (last);
  EXPECT_EQ (1, logs.size ());
  
  // Clear LDAP settings.
  ldap_logic_clear ();
  refresh_sandbox (false);
}

#endif


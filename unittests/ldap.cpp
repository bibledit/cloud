/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/ldap.h>
#include <unittests/utilities.h>
#include <filter/url.h>
#include <ldap/logic.h>
#include <filter/roles.h>
#include <database/logs.h>


void test_ldap ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  // Copy the default LDAP server configuration into place.
  string ldap_txt = filter_url_create_root_path ({config_logic_config_folder (), "ldap.txt"});
  string ldap_conf = filter_url_create_root_path ({config_logic_config_folder (), "ldap.conf"});
  filter_url_file_cp (ldap_txt, ldap_conf);
  // Initialize LDAP configuration.
  ldap_logic_initialize ();
  
  // Authenticate a user and check the results.
  string user = "boyle";
  string password = "password";
  bool okay;
  bool access;
  string email;
  int role;
  okay = ldap_logic_fetch (user, password, access, email, role, false);
  evaluate (__LINE__, __func__, true, okay);
  evaluate (__LINE__, __func__, true, access);
  evaluate (__LINE__, __func__, "boyle@ldap.forumsys.com", email);
  evaluate (__LINE__, __func__, Filter_Roles::guest (), role);
  
  // Check there is one journal entry as a result of authenticating a user.
  string last = "0";
  vector <string> logs = Database_Logs::get (last);
  evaluate (__LINE__, __func__, 1, logs.size ());
  
  // Clear LDAP settings.
  ldap_logic_clear ();
  refresh_sandbox (false);
}

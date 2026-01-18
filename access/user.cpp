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


#include <access/user.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <filter/roles.h>
#include <filter/string.h>


// This function returns users assigned to the logged-in user.
std::vector<std::string> access_user::assignees (Webserver_Request& webserver_request)
{
  const int mylevel = webserver_request.session_logic()->get_level();
  
  // Get sorted list of users.
  std::vector<std::string> users = webserver_request.database_users()->get_users();
  std::ranges::sort(users);

  // The filter: Assignees should have a level less than or equal to my level.
  const auto filter_level = [&] (const auto& user) {
    return webserver_request.database_users()->get_level(user) <= mylevel;
  };
  auto range = users | std::views::filter(filter_level);
  return filter::string::range2vector(range);
}

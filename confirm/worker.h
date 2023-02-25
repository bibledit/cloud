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

#ifdef HAVE_CLOUD

class Confirm_Worker
{
public:
  Confirm_Worker (void * webserver_request_in);
  void setup (std::string mailto, std::string username,
              std::string initial_subject, std::string initial_body,
              std::string query,
              std::string subsequent_subject, std::string subsequent_body);
  bool handleEmail (std::string from, std::string subject, std::string body);
  bool handleLink (std::string & email);
private:
  void * webserver_request {nullptr};
  void informManagers (std::string email, std::string body);
};

#endif

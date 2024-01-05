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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

class Filter_Roles
{
public:
  static int guest ();
  static int member ();
  static int consultant ();
  static int translator ();
  static int manager ();
  static int admin ();
  static int lowest ();
  static int highest ();
  static std::string english (int role);
  static std::string text (int role);
  static bool access_control (Webserver_Request& webserver_request, int role);
private:
};

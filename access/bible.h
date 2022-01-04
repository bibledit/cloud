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


#pragma once


#include <string>
#include <vector>
#include <tuple>
using namespace std;


class AccessBible {
public:
  static bool Read (void * webserver_request, const string & bible, string user = string());
  static bool Write (void * webserver_request, const string & bible, string user = string());
  static bool BookWrite (void * webserver_request, string user, const string & bible, int book);
  static vector <string> Bibles (void * webserver_request, string user = string());
  static string Clamp (void * webserver_request, string bible);
  static tuple<bool, bool> Any (void * webserver_request);
};

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

#include <config/libraries.h>


#ifdef HAVE_CLOUD


class Database_Confirm
{
public:
  void create ();
  void upgrade ();
  void optimize ();
  unsigned int get_new_id ();
  bool id_exists (unsigned int id);
  void store (unsigned int id, string query, string to, string subject, string body, string username);
  unsigned int search_id (string subject);
  vector <int> get_ids ();
  string get_query (unsigned int id);
  string get_mail_to (unsigned int id);
  string get_subject (unsigned int id);
  string get_body (unsigned int id);
  string get_username (unsigned int id);
  void erase (unsigned int id);
  void trim ();
private:
  const char * filename ();
};


#endif


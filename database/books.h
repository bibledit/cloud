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

class Database_Books
{
public:
  static vector <int> get_ids ();
  static int get_id_from_english (string english);
  static string get_english_from_id (int id);
  static string get_usfm_from_id (int id);
  static string get_bibleworks_from_id (int id);
  static string get_osis_from_id (int id);
  static int get_id_from_usfm (string usfm);
  static int get_id_from_osis (string osis);
  static int get_id_from_bibleworks (string bibleworks);
  static int get_id_like_text (string text);
  static int get_id_from_onlinebible (string onlinebible);
  static string get_onlinebible_from_id (int id);
  static int get_order_from_id (int id);
  static string get_type (int id);
private:
  static unsigned int data_count ();
};

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

enum class book_type {
  unknown,
  old_testament,
  new_testament,
  front_back,
  other,
  apocryphal,
};

namespace database::books {

std::vector <int> get_ids ();
int get_id_from_english (const std::string & english);
std::string get_english_from_id (int id);
std::string get_usfm_from_id (int id);
std::string get_bibleworks_from_id (int id);
std::string get_osis_from_id (int id);
int get_id_from_usfm (const std::string & usfm);
int get_id_from_osis (const std::string & osis);
int get_id_from_bibleworks (const std::string & bibleworks);
int get_id_like_text (const std::string & text);
int get_id_from_onlinebible (const std::string & onlinebible);
std::string get_onlinebible_from_id (int id);
int get_order_from_id (int id);
book_type get_type_v2 (int id);
std::string book_type_to_string (book_type type);

};

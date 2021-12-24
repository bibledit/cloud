/*
Copyright (©) 2003-2021 Teus Benschop.

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
  static vector <int> getIDs ();
  static int getIdFromEnglish (string english);
  static string getEnglishFromId (int id);
  static string getUsfmFromId (int id);
  static string getBibleworksFromId (int id);
  static string getOsisFromId (int id);
  static int getIdFromUsfm (string usfm);
  static int getIdFromOsis (string osis);
  static int getIdFromBibleworks (string bibleworks);
  static int getIdLikeText (string text);
  static int getIdFromOnlinebible (string onlinebible);
  static string getOnlinebibleFromId (int id);
  static int getOrderFromId (int id);
  static string getType (int id);
private:
  static unsigned int data_count ();
};

/*
Copyright (©) 2003-2016 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_CACHE_H
#define INCLUDED_DATABASE_CACHE_H


#include <config/libraries.h>


class Database_Cache
{
public:
  static void create (string resource, int book);
  static void remove (string resource);
  static void remove (string resource, int book);
  static bool exists (string resource);
  static bool exists (string resource, int book);
  static bool exists (string resource, int book, int chapter, int verse);
  static void cache (string resource, int book, int chapter, int verse, string value);
  static string retrieve (string resource, int book, int chapter, int verse);
  static int count (string resource);
  static bool ready (string resource, int book);
  static void ready (string resource, int book, bool ready);
  static int size (string resource, int book);
  static string fragment ();
  static string path (string resource, int book);
private:
  static string filename (string resource, int book);
};


bool database_filebased_cache_exists (string schema);
void database_filebased_cache_put (string schema, string contents);
string database_filebased_cache_get (string schema);
void database_filebased_cache_remove (string schema);


void database_cache_trim ();


#endif

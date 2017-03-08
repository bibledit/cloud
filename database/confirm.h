/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_CONFIRM_H
#define INCLUDED_DATABASE_CONFIRM_H


#include <config/libraries.h>


#ifdef HAVE_CLOUD


class Database_Confirm
{
public:
  void create ();
  void optimize ();
  unsigned int getNewID ();
  bool IDExists (unsigned int id);
  void store (unsigned int id, string query, string to, string subject, string body);
  unsigned int searchID (string subject);
  string getQuery (unsigned int id);
  string getMailTo (unsigned int id);
  string getSubject (unsigned int id);
  string getBody (unsigned int id);
  void erase (unsigned int id);
  void trim ();
private:
  const char * filename ();
};


#endif


#endif

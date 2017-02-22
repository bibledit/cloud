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


#ifndef INCLUDED_CHECKSUM_LOGIC_H
#define INCLUDED_CHECKSUM_LOGIC_H


#include <config/libraries.h>


class Checksum_Logic
{
public:
  static string send (string data, bool readwrite);
  static string get (string data);
  static string get (const vector <string>& data);
  static string getChapter (void * webserver_request, string bible, int book, int chapter);
  static string getBook (void * webserver_request, string bible, int book);
  static string getBible (void * webserver_request, string bible);
  static string getBibles (void * webserver_request, const vector <string> & bibles);
private:
};


#endif

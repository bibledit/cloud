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


#ifndef INCLUDED_ACCESS_BIBLE_H
#define INCLUDED_ACCESS_BIBLE_H


#include <config/libraries.h>


bool access_bible_read (void * webserver_request, const string & bible, string user = "");
bool access_bible_write (void * webserver_request, const string & bible, string user = "");
bool access_bible_book_write (void * webserver_request, string user, const string & bible, int book);
vector <string> access_bible_bibles (void * webserver_request, string user = "");
string access_bible_clamp (void * webserver_request, string bible);
void access_a_bible (void * webserver_request, bool & read, bool & write);


#endif

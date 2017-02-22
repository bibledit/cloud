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


#ifndef INCLUDED_RSS_LOGIC_H
#define INCLUDED_RSS_LOGIC_H


#include <config/libraries.h>


#ifdef HAVE_CLOUD
void rss_logic_schedule_update (string user, string bible, int book, int chapter,
                                string oldusfm, string newusfm);
void rss_logic_execute_update (string user, string bible, int book, int chapter,
                               string oldusfm, string newusfm);
string rss_logic_xml_path ();
void rss_logic_update_xml (vector <string> titles, vector <string> authors, vector <string> descriptions);
#endif


#endif

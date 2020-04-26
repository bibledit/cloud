/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#ifndef INCLUDED_CHECK_VERSES_H
#define INCLUDED_CHECK_VERSES_H


#include <config/libraries.h>


class Checks_Verses
{
public:
  static void missingPunctuationAtEnd (string bible, int book, int chapter, map <int, string> verses,
                                       string center_marks, string end_marks, string disregards);
  static void patterns (string bible, int book, int chapter, map <int, string> verses, vector <string> patterns);
private:
};


#endif

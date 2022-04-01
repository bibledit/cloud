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
#include <database/styles.h>

namespace filter::note {

class citation
{
public:
  citation ();
  void set_sequence (int numbering, const string & usersequence);
  void set_restart (int setting);
  string get (string citation);
  void run_restart (const string & moment);
private:
  string restart;
  unsigned int pointer;
  vector <string> sequence;
};

class citations
{
public:
  void evaluate_style (const Database_Styles_Item & style);
  string get (const string & marker, const string & citation);
  void restart (const string & moment);
private:
  map <string, citation> cache;
};


}

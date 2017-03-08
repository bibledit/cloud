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


#ifndef INCLUDED_DIALOG_ENTRY_H
#define INCLUDED_DIALOG_ENTRY_H


#include <config/libraries.h>


class Dialog_Entry
{
public:
  Dialog_Entry (string url, string question, string value, string submit, string help);
  ~Dialog_Entry ();
  void add_query (string parameter, string value);
  string run ();
private:
  void * assets_view = NULL;
  string base_url;
};


#endif

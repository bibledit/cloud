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


#ifndef INCLUDED_DIALOG_LIST_H
#define INCLUDED_DIALOG_LIST_H


#include <config/libraries.h>


class Dialog_List
{
public:
  Dialog_List (string url, string question, string info_top, string info_bottom, bool post = false);
  ~Dialog_List ();
  void add_query (string parameter, string value);
  void add_row (string text, string parameter, string value);
  string run ();
private:
  void * assets_view = NULL;
  string base_url;
  string list_block;
  bool post_result = false;
};


#endif

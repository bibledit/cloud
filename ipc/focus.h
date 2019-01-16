/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#ifndef INCLUDED_IPC_FOCUS_H
#define INCLUDED_IPC_FOCUS_H


#include <config/libraries.h>


class Ipc_Focus
{
public:
  static void set (void * webserver_request, int book, int chapter, int verse);
  static int getBook (void * webserver_request);
  static int getChapter (void * webserver_request);
  static int getVerse (void * webserver_request);
private:
};


#endif

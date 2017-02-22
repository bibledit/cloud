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


#ifndef INCLUDED_ACCESS_LOGIC_H
#define INCLUDED_ACCESS_LOGIC_H


#include <config/libraries.h>


#define PRIVILEGE_VIEW_RESOURCES 1
int access_logic_view_resources_role ();
bool access_logic_privilege_view_resources (void * webserver_request, string user = "");

#define PRIVILEGE_VIEW_NOTES 2
int access_logic_view_notes_role ();
bool access_logic_privilege_view_notes (void * webserver_request, string user = "");

#define PRIVILEGE_CREATE_COMMENT_NOTES 3
int access_logic_create_comment_notes_role ();
bool access_logic_privilege_create_comment_notes (void * webserver_request, string user = "");

void access_logic_user_level (void * webserver_request, string & user, int & level);
void access_logic_create_client_files ();

#endif

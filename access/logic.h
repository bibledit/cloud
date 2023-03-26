/*
Copyright (©) 2003-2023 Teus Benschop.

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
using namespace std;

#define PRIVILEGE_VIEW_RESOURCES 1

namespace access_logic {

int view_resources_role ();
bool privilege_view_resources (void * webserver_request, std::string user = std::string());

#define PRIVILEGE_VIEW_NOTES 2
int view_notes_role ();
bool privilege_view_notes (void * webserver_request, std::string user = std::string());

#define PRIVILEGE_CREATE_COMMENT_NOTES 3
int create_comment_notes_role ();
bool privilege_create_comment_notes (void * webserver_request, std::string user = std::string());

int delete_consultation_notes_role ();
bool privilege_delete_consultation_notes (void * webserver_request, std::string user = std::string());

int use_advanced_mode_role ();
bool privilege_use_advanced_mode (void * webserver_request, std::string user = std::string());

int set_stylesheets_role ();
bool privilege_set_stylesheets (void * webserver_request, std::string user = std::string());

void user_level (void * webserver_request, std::string & user, int & level);
void create_client_files ();

set <string> default_privilege_usernames ();

}

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


#ifndef INCLUDED_WORKSPACE_LOGIC_H
#define INCLUDED_WORKSPACE_LOGIC_H


#include <config/libraries.h>


vector <string> workspace_get_default_names ();
map <int, string> workspace_get_default_urls (int id);
map <int, string> workspace_get_default_widths (int id);
map <int, string> workspace_get_default_heights (int id);
void workspace_create_defaults (void * webserver_request);
string workspace_get_active_name (void * webserver_request);
string workspace_process_units (string length);
void workspace_set_values (void * webserver_request, int selector, const map <int, string> & values);
void workspace_set_urls (void * webserver_request, const map <int, string> & values);
void workspace_set_widths (void * webserver_request, const map <int, string> & values);
void workspace_set_heights (void * webserver_request, const map <int, string> & values);
void workspace_set_entire_width (void * webserver_request, string value);
map <int, string> workspace_get_values (void * webserver_request, int selector, bool use);
map <int, string> workspace_get_urls (void * webserver_request, bool use);
map <int, string> workspace_get_widths (void * webserver_request);
map <int, string> workspace_get_heights (void * webserver_request);
string workspace_get_entire_width (void * webserver_request);
vector <string> workspace_get_names (void * webserver_request, bool add_default = true);
void workspace_delete (void * webserver_request, string workspace);
void workspace_reorder (void * webserver_request, const vector <string> & workspaces);
void workspace_copy (void * webserver_request, string source, string destination);
void workspace_cache_for_cloud (void * webserver_request, bool urls, bool widths, bool heights);
string workspace_get_default_name ();
void workspace_send (void * webserver_request, string workspace, string user);
string workspace_query_key_readonly ();
void workspace_add_bible_readonly (map <int, string> & urls);


#endif

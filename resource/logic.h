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


#ifndef INCLUDED_RESOURCE_LOGIC_H
#define INCLUDED_RESOURCE_LOGIC_H


#include <config/libraries.h>


string resource_logic_get_html (void * webserver_request,
                                string resource, int book, int chapter, int verse,
                                bool add_verse_numbers);
string resource_logic_get_verse (void * webserver_request, string resource, int book, int chapter, int verse);
string resource_logic_get_contents_for_client (string resource, int book, int chapter, int verse);
string resource_logic_client_fetch_cache_from_cloud (string resource, int book, int chapter, int verse);

vector <string> resource_logic_get_names (void * webserver_request);

void resource_logic_import_images (string resource, string path);

string resource_logic_yellow_divider ();
string resource_logic_green_divider ();
string resource_logic_blue_divider ();
string resource_logic_violet_divider ();
string resource_logic_red_divider ();
string resource_logic_orange_divider ();
bool resource_logic_is_divider (string resource);
string resource_logic_get_divider (string resource);

string resource_logic_web_cache_get (string url, string & error);

string resource_logic_selector_page (void * webserver_request);
string resource_logic_selector_caller (void * webserver_request);

string resource_logic_default_user_url ();

void resource_logic_create_cache ();

string resource_logic_bible_gateway_module_list_path ();
string resource_logic_bible_gateway_module_list_refresh ();
vector <string> resource_logic_bible_gateway_module_list_get ();
string resource_logic_bible_gateway_get (string resource, int book, int chapter, int verse);


string resource_logic_study_light_module_list_path ();
string resource_logic_study_light_module_list_refresh ();
vector <string> resource_logic_study_light_module_list_get ();
string resource_logic_study_light_get (string resource, int book, int chapter, int verse);


#endif

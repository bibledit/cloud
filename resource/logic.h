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


#ifndef INCLUDED_RESOURCE_LOGIC_H
#define INCLUDED_RESOURCE_LOGIC_H


#include <config/libraries.h>


string resource_logic_get_html (void * webserver_request,
                                string resource, int book, int chapter, int verse,
                                bool add_verse_numbers);
string resource_logic_get_verse (void * webserver_request, string resource, int book, int chapter, int verse);
string resource_logic_get_comparison (void * webserver_request,
                                      string resource, int book, int chapter, int verse,
                                      bool add_verse_numbers);
string resource_logic_get_contents_for_client (string resource, int book, int chapter, int verse);
string resource_logic_client_fetch_cache_from_cloud (string resource, int book, int chapter, int verse);

vector <string> resource_logic_get_names (void * webserver_request, bool bibles_only);

void resource_logic_import_images (string resource, string path);

string resource_logic_yellow_divider ();
string resource_logic_green_divider ();
string resource_logic_blue_divider ();
string resource_logic_violet_divider ();
string resource_logic_red_divider ();
string resource_logic_orange_divider ();
string resource_logic_rich_divider ();
string resource_logic_get_divider (string resource);
bool resource_logic_parse_rich_divider (string input, string & title, string & link, string & foreground, string & background);
string resource_logic_assemble_rich_divider (string title, string link,
                                             string foreground, string background);

string resource_logic_web_or_cache_get (string url, string & error);

string resource_logic_selector_page (void * webserver_request);
string resource_logic_selector_caller (void * webserver_request);

string resource_logic_default_user_url ();

void resource_logic_create_cache ();
bool resource_logic_can_cache (string resource);

string resource_logic_bible_gateway_module_list_path ();
string resource_logic_bible_gateway_module_list_refresh ();
vector <string> resource_logic_bible_gateway_module_list_get ();
string resource_logic_bible_gateway_get (string resource, int book, int chapter, int verse);

string resource_logic_study_light_module_list_path ();
string resource_logic_study_light_module_list_refresh ();
vector <string> resource_logic_study_light_module_list_get ();
string resource_logic_study_light_get (string resource, int book, int chapter, int verse);

bool resource_logic_is_bible (string resource);
bool resource_logic_is_usfm (string resource);
bool resource_logic_is_external (string resource);
bool resource_logic_is_image (string resource);
bool resource_logic_is_lexicon (string resource);
bool resource_logic_is_sword (string resource);
bool resource_logic_is_divider (string resource);
bool resource_logic_is_biblegateway (string resource);
bool resource_logic_is_studylight (string resource);
bool resource_logic_is_comparative (string resource);

string resource_logic_comparative_resource_v2 ();
bool resource_logic_parse_comparative_resource_v2 (string input, string * title = nullptr, string * base = nullptr, string * update = nullptr, string * remove = nullptr, string * replace = nullptr, bool * diacritics = nullptr, bool * casefold = nullptr);
string resource_logic_assemble_comparative_resource_v2 (string title, string base = "", string update = "", string remove = "", string replace = "", bool diacritics = false, bool casefold = false);


#endif

/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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

class Webserver_Request;

std::string resource_logic_get_html (Webserver_Request& webserver_request,
                                     std::string resource, int book, int chapter, int verse,
                                     bool add_verse_numbers);
std::string resource_logic_get_verse (Webserver_Request& webserver_request, std::string resource, int book, int chapter, int verse);
std::string resource_logic_cloud_get_comparison (Webserver_Request& webserver_request,
                                                 std::string resource, int book, int chapter, int verse,
                                                 bool add_verse_numbers);
std::string resource_logic_cloud_get_translation (Webserver_Request& webserver_request,
                                                  const std::string & resource, int book, int chapter, int verse,
                                                  bool add_verse_numbers);
std::string resource_logic_get_contents_for_client (std::string resource, int book, int chapter, int verse);
std::string resource_logic_client_fetch_cache_from_cloud (std::string resource, int book, int chapter, int verse);

std::vector <std::string> resource_logic_get_names (Webserver_Request& webserver_request, bool bibles_only);

std::string resource_logic_yellow_divider ();
std::string resource_logic_green_divider ();
std::string resource_logic_blue_divider ();
std::string resource_logic_violet_divider ();
std::string resource_logic_red_divider ();
std::string resource_logic_orange_divider ();
std::string resource_logic_rich_divider ();
std::string resource_logic_get_divider (std::string resource);
bool resource_logic_parse_rich_divider (std::string input, std::string & title, std::string & link, std::string & foreground, std::string & background);
std::string resource_logic_assemble_rich_divider (std::string title, std::string link,
                                                  std::string foreground, std::string background);

std::string resource_logic_web_or_cache_get (std::string url, std::string & error);

std::string resource_logic_selector_page (Webserver_Request& webserver_request);
std::string resource_logic_selector_caller (Webserver_Request& webserver_request);

std::string resource_logic_default_user_url ();

void resource_logic_create_cache ();
bool resource_logic_can_cache (std::string resource);

std::string resource_logic_bible_gateway_module_list_path ();
std::string resource_logic_bible_gateway_module_list_refresh ();
std::vector <std::string> resource_logic_bible_gateway_module_list_get ();
std::string resource_logic_bible_gateway_get (std::string resource, int book, int chapter, int verse);

std::string resource_logic_study_light_module_list_path ();
std::string resource_logic_study_light_module_list_refresh ();
std::vector <std::string> resource_logic_study_light_module_list_get ();
std::string resource_logic_study_light_get (std::string resource, int book, int chapter, int verse);

std::string resource_logic_easy_english_bible_name ();
std::string resource_logic_easy_english_bible_get (int book, int chapter, int verse);
bool resource_logic_easy_english_bible_handle_chapter_heading (const std::string & paragraph,
                                                               int chapter,
                                                               bool & near_passage,
                                                               bool & at_passage);
bool resource_logic_easy_english_bible_handle_passage_heading (const std::string & paragraph,
                                                               int chapter, int verse,
                                                               bool & near_passage,
                                                               bool & at_passage);
void resource_logic_easy_english_bible_handle_verse_marker (const std::string & paragraph,
                                                            int verse,
                                                            bool & at_passage);


bool resource_logic_is_bible (std::string resource);
bool resource_logic_is_usfm (std::string resource);
bool resource_logic_is_external (std::string resource);
bool resource_logic_is_lexicon (std::string resource);
bool resource_logic_is_sword (std::string resource);
bool resource_logic_is_divider (std::string resource);
bool resource_logic_is_biblegateway (std::string resource);
bool resource_logic_is_studylight (std::string resource);
bool resource_logic_is_comparative (const std::string & resource);
bool resource_logic_is_translated (const std::string & resource);

std::string resource_logic_comparative_resource ();
bool resource_logic_parse_comparative_resource (const std::string & input,
                                                std::string * title = nullptr,
                                                std::string * base = nullptr,
                                                std::string * update = nullptr,
                                                std::string * remove = nullptr,
                                                std::string * replace = nullptr,
                                                bool * diacritics = nullptr,
                                                bool * casefold = nullptr,
                                                bool * cache = nullptr);
std::string resource_logic_assemble_comparative_resource (std::string title,
                                                          std::string base = std::string(),
                                                          std::string update = std::string(),
                                                          std::string remove = std::string(),
                                                          std::string replace = std::string(),
                                                          bool diacritics = false,
                                                          bool casefold = false,
                                                          bool cache = false);
std::string resource_logic_comparative_resources_list_path ();
std::vector <std::string> resource_logic_comparative_resources_get_list_on_client ();

std::string resource_logic_translated_resource ();
bool resource_logic_parse_translated_resource (const std::string & input,
                                               std::string * title = nullptr,
                                               std::string * original_resource = nullptr,
                                               std::string * source_language = nullptr,
                                               std::string * target_language = nullptr,
                                               bool * cache = nullptr);
std::string resource_logic_assemble_translated_resource (std::string title,
                                                         std::string original_resource = std::string(),
                                                         std::string source_language = std::string(),
                                                         std::string target_language = std::string(),
                                                         bool cache = false);
std::string resource_logic_translated_resources_list_path ();
std::vector <std::string> resource_logic_translated_resources_get_list_on_client ();


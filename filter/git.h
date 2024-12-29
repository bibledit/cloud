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
#include <filter/passage.h>

class Webserver_Request;

#ifdef HAVE_CLOUD
std::string filter_git_directory (std::string object);
bool filter_git_init (std::string directory, bool bare = false);
void filter_git_sync_modifications_to_git (std::string bible, std::string repository);
void filter_git_sync_bible_to_git (std::string bible, std::string repository);
void filter_git_sync_git_to_bible (std::string repository, std::string bible);
void filter_git_sync_git_chapter_to_bible (std::string repository, std::string bible, int book, int chapter);
bool filter_git_remote_read (std::string url, std::string & error);
bool filter_git_remote_clone (std::string url, std::string path, int jobid, std::string & error);
bool filter_git_add_remove_all (std::string repository, std::string & error);
bool filter_git_commit (std::string repository, std::string user, std::string message,
                        std::vector <std::string> & messages, std::string & error);
void filter_git_config_set_bool (std::string repository, std::string name, bool value);
void filter_git_config_set_int (std::string repository, std::string name, int value);
void filter_git_config_set_string (std::string repository, std::string name, std::string value);
Passage filter_git_get_passage (std::string line);
std::vector <std::string> filter_git_status (std::string repository, bool porcelain = false);
bool filter_git_pull (std::string repository, std::vector <std::string> & messages);
bool filter_git_push (std::string repository, std::vector <std::string> & messages, bool all = false);
bool filter_git_resolve_conflicts (std::string repository, std::vector <std::string> & paths, std::string & error);
void filter_git_config (std::string repository);
std::string filter_git_user (std::string user);
std::string filter_git_email (std::string user);
#endif

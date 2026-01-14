/*
Copyright (©) 2003-2026 Teus Benschop.

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


class Sync_Logic_Range
{
public:
  int low;
  int high;
};


class Sync_Logic
{
public:
  Sync_Logic (Webserver_Request& webserver_request);

  static constexpr int settings_get_total_checksum = 1;
  static constexpr int settings_send_workspace_urls = 2;
  static constexpr int settings_get_workspace_urls = 3;
  static constexpr int settings_send_workspace_widths = 4;
  static constexpr int settings_get_workspace_widths = 5;
  static constexpr int settings_send_workspace_heights = 6;
  static constexpr int settings_get_workspace_heights = 7;
  static constexpr int settings_send_resources_organization = 8;
  static constexpr int settings_get_resources_organization = 9;
  static constexpr int settings_get_bible_id = 10;
  static constexpr int settings_get_bible_font = 11;
  static constexpr int settings_send_platform = 12;
  static constexpr int settings_get_privilege_delete_consultation_notes = 13;
  
  static constexpr int bibles_get_total_checksum = 1;
  static constexpr int bibles_get_bibles = 2;
  static constexpr int bibles_get_bible_checksum = 3;
  static constexpr int bibles_get_books = 4;
  static constexpr int bibles_get_book_checksum = 5;
  static constexpr int bibles_get_chapters = 6;
  static constexpr int bibles_get_chapter_checksum = 7;
  static constexpr int bibles_send_chapter = 8;
  static constexpr int bibles_get_chapter = 9;
  
  static constexpr int notes_get_total = 1;
  static constexpr int notes_get_identifiers = 2;
  static constexpr int notes_get_summary = 3;
  static constexpr int notes_get_contents = 4;
  static constexpr int notes_get_subscribers = 5;
  static constexpr int notes_get_assignees = 6;
  static constexpr int notes_get_status = 7;
  static constexpr int notes_get_passages = 8;
  static constexpr int notes_get_severity = 9;
  static constexpr int notes_get_bible = 10;
  static constexpr int notes_get_modified = 11;
  static constexpr int notes_put_create_initiate = 12;
  static constexpr int notes_put_create_complete = 13;
  static constexpr int notes_put_summary = 14;
  static constexpr int notes_put_contents = 15;
  static constexpr int notes_put_comment = 16;
  static constexpr int notes_put_subscribe = 17;
  static constexpr int notes_put_unsubscribe = 18;
  static constexpr int notes_put_assign = 19;
  static constexpr int notes_put_unassign = 20;
  static constexpr int notes_put_status = 21;
  static constexpr int notes_put_passages = 22;
  static constexpr int notes_put_severity = 23;
  static constexpr int notes_put_bible = 24;
  static constexpr int notes_put_mark_delete = 25;
  static constexpr int notes_put_unmark_delete = 26;
  static constexpr int notes_put_delete = 27;
  static constexpr int notes_get_bulk = 28;
  
  static constexpr int usfmresources_get_total_checksum = 1;
  static constexpr int usfmresources_get_resources = 2;
  static constexpr int usfmresources_get_resource_checksum = 3;
  static constexpr int usfmresources_get_books = 4;
  static constexpr int usfmresources_get_book_checksum = 5;
  static constexpr int usfmresources_get_chapters = 6;
  static constexpr int usfmresources_get_chapter_checksum = 7;
  static constexpr int usfmresources_get_chapter = 8;
  
  static constexpr int changes_delete_modification = 1;
  static constexpr int changes_get_checksum = 2;
  static constexpr int changes_get_identifiers = 3;
  static constexpr int changes_get_modification = 4;
  
  static constexpr int resources_request_text = 0;
  static constexpr int resources_request_database = 1;
  static constexpr int resources_request_download = 2;

  bool security_okay ();
  bool credentials_okay ();
  
  std::string settings_checksum (const std::vector <std::string> & bibles);
  
  std::string checksum (const std::vector <int> & identifiers);
  std::vector <Sync_Logic_Range> create_range (int start, int end);
  
  std::string post (std::map <std::string, std::string> & post, const std::string& url, std::string & error, bool burst = false);
  
  static std::string usfm_resources_checksum ();
  static std::string usfm_resource_checksum (const std::string& name);
  static std::string usfm_resource_book_checksum (const std::string& name, int book);
  static std::string usfm_resource_chapter_checksum (const std::string& name, int book, int chapter);
  
  static std::string changes_checksum (const std::string & username);

  static constexpr int files_total_checksum = 1;
  static constexpr int files_directory_checksum = 2;
  static constexpr int files_directory_files = 3;
  static constexpr int files_file_checksum = 4;
  static constexpr int files_file_download = 5;
  static std::vector <std::string> files_get_directories (int version, const std::string & user);
  static int files_get_total_checksum (int version, const std::string & user);
  static int files_get_directory_checksum (std::string directory);
  static std::vector <std::string> files_get_files (std::string directory);
  static int files_get_file_checksum (std::string directory, std::string file);

  void prioritized_ip_address_record ();
  bool prioritized_ip_address_active ();
  
private:
  Webserver_Request& m_webserver_request;
};


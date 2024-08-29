/*
Copyright (©) 2003-2024 Teus Benschop.

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

struct Database_Notes_Text
{
  std::string raw {};
  std::string localized {};
};


class Database_Notes
{

public:
  Database_Notes (Webserver_Request& webserver_request);
private:
  Webserver_Request& m_webserver_request;

public:
  void create ();

private:
  std::string database_path ();
  std::string checksums_database_path ();
  std::string main_folder_path ();
  std::string note_file (int identifier);

public:
  bool healthy ();
  bool checksums_healthy ();
  bool checkup ();
  bool checkup_checksums ();
  void trim ();
  void trim_server ();
  void optimize ();
  void sync ();

public:
  bool identifier_exists (int identifier);
  void set_identifier (int identifier, int new_identifier);

public:
  std::vector <int> get_identifiers ();
  int store_new_note (const std::string& bible, int book, int chapter, int verse, std::string summary, std::string contents, bool raw);
private:
  int get_new_unique_identifier ();
  
public:
  std::vector <int> select_notes (std::vector <std::string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const std::string& status_selector, std::string bible_selector, std::string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const std::string& search_text, int limit);
private:
  std::string notes_select_identifier ();
  std::string notes_optional_fulltext_search_relevance_statement (std::string search);
  std::string notes_from_where_statement ();
  std::string notes_optional_fulltext_search_statement (std::string search);
  std::string notes_order_by_relevance_statement ();

public:
  std::string get_summary (int identifier);
  void set_summary (int identifier, const std::string& summary);
private:
  std::string summary_key ();

public:
  std::string get_contents (int identifier);
  void set_contents (int identifier, const std::string& contents);
private:
  std::string contents_key ();
  std::string assemble_contents (int identifier, std::string contents);
  void set_raw_contents (int identifier, const std::string& contents);
  
public:
  void erase (int identifier);
  
public:
  void add_comment (int identifier, const std::string& comment);
  
public:
  void subscribe (int identifier);
  void unsubscribe (int identifier);
  void subscribe_user (int identifier, const std::string& user);
  void unsubscribe_user (int identifier, const std::string& user);
  bool is_subscribed (int identifier, const std::string& user);
  std::vector <std::string> get_subscribers (int identifier);
  void set_subscribers (int identifier, std::vector <std::string> subscribers);
private:
  std::string subscriptions_key ();
  std::string get_raw_subscriptions (int identifier);
  void set_raw_subscriptions (int identifier, const std::string& subscriptions);

public:
  void assign_user (int identifier, const std::string& user);
  bool is_assigned (int identifier, const std::string& user);
  void unassign_user (int identifier, const std::string& user);
  std::vector <std::string> get_all_assignees (const std::vector <std::string>& bibles);
  std::vector <std::string> get_assignees (int identifier);
  void set_assignees (int identifier, std::vector <std::string> assignees);
private:
  std::string assigned_key ();
  std::vector <std::string> get_assignees_internal (std::string assignees);
  void set_raw_assigned (int identifier, const std::string& assigned);
  std::string get_raw_assigned (int identifier);
  
public:
  std::string get_bible (int identifier);
  void set_bible (int identifier, const std::string& bible);
  std::vector <std::string> get_all_bibles ();
private:
  std::string bible_key ();

public:
  std::string encode_passage (int book, int chapter, int verse);
  Passage decode_passage (std::string passage);
  std::string decode_passage (int identifier);
  std::vector <Passage> get_passages (int identifier);
  void set_passages (int identifier, const std::vector <Passage>& passages, bool import = false);
  void set_raw_passage (int identifier, const std::string& passage);
  void index_raw_passage (int identifier, const std::string& passage);
private:
  std::string passage_key ();
  std::string get_raw_passage (int identifier);
  
public:
  std::string get_raw_status (int identifier);
  std::string get_status (int identifier);
  void set_status (int identifier, const std::string& status, bool import = false);
  std::vector <Database_Notes_Text> get_possible_statuses ();
private:
  std::string status_key ();

public:
  std::string get_severity (int identifier);
  int get_raw_severity (int identifier);
  void set_raw_severity (int identifier, int severity);
  std::vector <Database_Notes_Text> get_possible_severities ();
private:
  std::string severity_key ();
  std::vector <std::string> standard_severities ();

public:
  int get_modified (int identifier);
  void set_modified (int identifier, int time);
private:
  std::string modified_key ();
  void note_modified_actions (int identifier);

public:
  bool get_public (int identifier);
  void set_public (int identifier, bool value);
private:
  std::string public_key ();
  
public:
  std::string get_search_field (int identifier);
  void update_search_fields (int identifier);
  std::vector <int> search_notes (std::string search, const std::vector <std::string> & bibles);

public:
  void mark_for_deletion (int identifier);
  void unmark_for_deletion (int identifier);
  bool is_marked_for_deletion (int identifier);
private:
  std::string expiry_key ();
  void touch_marked_for_deletion ();
  std::vector <int> get_due_for_deletion ();

public:
  void set_checksum (int identifier, const std::string & checksum);
  std::string get_checksum (int identifier);
  void delete_checksum (int identifier);
  void update_checksum (int identifier);
  std::string get_multiple_checksum (const std::vector <int> & identifiers);
  std::vector <int> get_notes_in_range_for_bibles (int lowId, int highId, std::vector <std::string> bibles, bool anybible);
  
public:
  void set_availability (bool available);
  bool available ();
private:
  std::string availability_flag ();

public:
  std::string get_bulk (std::vector <int> identifiers);
  std::vector <std::string> set_bulk (std::string json);
  
private:
  void update_database (int identifier);
  void update_database_internal (int identifier, int modified, std::string assigned, std::string subscriptions, std::string bible, std::string passage, std::string status, int severity, std::string summary, std::string contents);
  
private:
  friend void test_database_notes ();
  friend void test_indexing_fixes_damaged_note ();

private:
  std::string get_field (int identifier, const std::string& key);
  void set_field (int identifier, std::string key, std::string value);

};


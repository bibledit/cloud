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


#ifndef INCLUDED_DATABASE_NOTES_H
#define INCLUDED_DATABASE_NOTES_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Notes_Text
{
public:
  string raw;
  string localized;
};


class Database_Notes
{

public:
  Database_Notes (void * webserver_request_in);
private:
  void * webserver_request;

public:
  void create_v12 ();

private:
  string database_path_v12 ();
  string checksums_database_path_v12 ();
  string main_folder_v12 ();
  string note_folder_v1 (int identifier);
  string note_file_v2 (int identifier);

public:
  bool healthy_v12 ();
  bool checksums_healthy_v12 ();
  bool checkup_v12 ();
  bool checkup_checksums_v12 ();
  void trim_v12 ();
  void trim_server_v12 ();
  void optimize_v12 ();
  void sync_v12 ();

public:
  bool identifier_exists_v12 (int identifier);
  void set_identifier_v12 (int identifier, int new_identifier);
private:
  void set_identifier_v1 (int identifier, int new_identifier);
  void set_identifier_v2 (int identifier, int new_identifier);

public:
  vector <int> get_identifiers_v12 ();
  int store_new_note_v2 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
private:
  int store_new_note_v1 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
  int get_new_unique_identifier_v12 ();
  
public:
  vector <int> select_notes_v12 (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit);
private:
  string notes_select_identifier ();
  string notes_optional_fulltext_search_relevance_statement (string search);
  string notes_from_where_statement ();
  string notes_optional_fulltext_search_statement (string search);
  string notes_order_by_relevance_statement ();

public:
  string get_summary_v12 (int identifier);
  void set_summary_v12 (int identifier, const string& summary);
private:
  string summary_file_v1 (int identifier);
  string summary_key_v2 ();
  string get_summary_v1 (int identifier);
  string get_summary_v2 (int identifier);
  void set_summary_v1 (int identifier, const string& summary);
  void set_summary_v2 (int identifier, string summary);

public:
  string get_contents_v12 (int identifier);
  void set_contents_v12 (int identifier, const string& contents);
private:
  string contents_file_v1 (int identifier);
  string contents_key_v2 ();
  string assemble_contents_v12 (int identifier, string contents);
  string get_contents_v1 (int identifier);
  string get_contents_v2 (int identifier);
  void set_raw_contents_v1 (int identifier, const string& contents);
  void set_raw_contents_v2 (int identifier, const string& contents);
  void set_contents_v1 (int identifier, const string& contents);
  void set_contents_v2 (int identifier, const string& contents);
  
public:
  void erase_v12 (int identifier);
  
public:
  void add_comment_v12 (int identifier, const string& comment);
private:
  void add_comment_v1 (int identifier, const string& comment);
  void add_comment_v2 (int identifier, const string& comment);
  
public:
  void subscribe_v12 (int identifier);
  void unsubscribe_v12 (int identifier);
  void subscribe_user_v12 (int identifier, const string& user);
  void unsubscribe_user_v12 (int identifier, const string& user);
  bool is_subscribed_v12 (int identifier, const string& user);
  vector <string> get_subscribers_v12 (int identifier);
  void set_subscribers_v12 (int identifier, vector <string> subscribers);
private:
  string subscriptions_file_v1 (int identifier);
  string subscriptions_key_v2 ();
  vector <string> get_subscribers_v1 (int identifier);
  vector <string> get_subscribers_v2 (int identifier);
  string get_raw_subscriptions_v2 (int identifier);
  void subscribe_v1 (int identifier);
  void subscribe_v2 (int identifier);
  void subscribe_user_v1 (int identifier, const string& user);
  void subscribe_user_v2 (int identifier, const string& user);
  void unsubscribe_v1 (int identifier);
  void unsubscribe_v2 (int identifier);
  void set_raw_subscriptions_v2 (int identifier, const string& subscriptions);
  bool is_subscribed_v1 (int identifier, const string& user);
  bool is_subscribed_v2 (int identifier, const string& user);
  void set_subscribers_v1 (int identifier, vector <string> subscribers);
  void set_subscribers_v2 (int identifier, vector <string> subscribers);
  void unsubscribe_user_v1 (int identifier, const string& user);
  void unsubscribe_user_v2 (int identifier, const string& user);

public:
  void assign_user_v12 (int identifier, const string& user);
  bool is_assigned_v12 (int identifier, const string& user);
  void unassign_user_v12 (int identifier, const string& user);
  vector <string> get_all_assignees_v12 (const vector <string>& bibles);
  vector <string> get_assignees_v12 (int identifier);
  void set_assignees_v12 (int identifier, vector <string> assignees);
private:
  string assigned_file_v1 (int identifier);
  string assigned_key_v2 ();
  void assign_user_v1 (int identifier, const string& user);
  void assign_user_v2 (int identifier, const string& user);
  bool is_assigned_v1 (int identifier, const string& user);
  bool is_assigned_v2 (int identifier, const string& user);
  void unassign_user_v1 (int identifier, const string& user);
  void unassign_user_v2 (int identifier, const string& user);
  vector <string> get_assignees_internal_v12 (string assignees);
  vector <string> get_assignees_v1 (int identifier);
  vector <string> get_assignees_v2 (int identifier);
  void set_assignees_v1 (int identifier, vector <string> assignees);
  void set_assignees_v2 (int identifier, vector <string> assignees);
  void set_raw_assigned_v2 (int identifier, const string& assigned);
  string get_raw_assigned_v2 (int identifier);
  
public:
  string get_bible_v12 (int identifier);
  void set_bible_v12 (int identifier, const string& bible);
  vector <string> get_all_bibles_v12 ();
private:
  string bible_file_v1 (int identifier);
  string bible_key_v2 ();
  string get_bible_v1 (int identifier);
  string get_bible_v2 (int identifier);
  void set_bible_v1 (int identifier, const string& bible);
  void set_bible_v2 (int identifier, const string& bible);

public:
  string encode_passage_v12 (int book, int chapter, int verse);
  Passage decode_passage_v12 (string passage);
  string get_raw_passage_v12 (int identifier);
  vector <Passage> get_passages_v12 (int identifier);
  void set_passages_v12 (int identifier, const vector <Passage>& passages, bool import = false);
  void set_raw_passage_v12 (int identifier, const string& passage);
  void index_raw_passage_v12 (int identifier, const string& passage);
private:
  void set_passages_v1 (int identifier, const vector <Passage>& passages, bool import = false);
  void set_passages_v2 (int identifier, const vector <Passage>& passages, bool import = false);
  string passage_file_v1 (int identifier);
  string passage_key_v2 ();
  string get_raw_passage_v1 (int identifier);
  string get_raw_passage_v2 (int identifier);
  vector <Passage> get_passages_v1 (int identifier);
  vector <Passage> get_passages_v2 (int identifier);
  void set_raw_passage_v1 (int identifier, const string& passage);
  void set_raw_passage_v2 (int identifier, const string& passage);
  
public:
  string get_raw_status_v12 (int identifier);
  string get_status_v12 (int identifier);
  void set_status_v12 (int identifier, const string& status, bool import = false);
  vector <Database_Notes_Text> get_possible_statuses_v12 ();
private:
  string status_file_v1 (int identifier);
  string status_key_v2 ();
  string get_raw_status_v1 (int identifier);
  string get_raw_status_v2 (int identifier);
  string get_status_v1 (int identifier);
  string get_status_v2 (int identifier);
  void set_status_v1 (int identifier, const string& status, bool import = false);
  void set_status_v2 (int identifier, const string& status, bool import = false);

public:
  string get_severity_v12 (int identifier);
  int get_raw_severity_v12 (int identifier);
  void set_raw_severity_v12 (int identifier, int severity);
  vector <Database_Notes_Text> get_possible_severities_v12 ();
private:
  string severity_file_v1 (int identifier);
  string severity_key_v2 ();
  vector <string> standard_severities_v12 ();
  int get_raw_severity_v1 (int identifier);
  int get_raw_severity_v2 (int identifier);
  string get_severity_v1 (int identifier);
  string get_severity_v2 (int identifier);
  void set_raw_severity_v1 (int identifier, int severity);
  void set_raw_severity_v2 (int identifier, int severity);

public:
  int get_modified_v12 (int identifier);
  void set_modified_v12 (int identifier, int time);
private:
  string modified_file_v1 (int identifier);
  string modified_key_v2 ();
  int get_modified_v1 (int identifier);
  int get_modified_v2 (int identifier);
  void set_modified_v1 (int identifier, int time);
  void set_modified_v2 (int identifier, int time);
  void note_modified_actions_v12 (int identifier);

public:
  bool get_public_v12 (int identifier);
  void set_public_v12 (int identifier, bool value);
private:
  string public_file_v1 (int identifier);
  string public_key_v2 ();
  bool get_public_v1 (int identifier);
  bool get_public_v2 (int identifier);
  void set_public_v1 (int identifier, bool value);
  void set_public_v2 (int identifier, bool value);
  
public:
  string get_search_field_v12 (int identifier);
  void update_search_fields_v12 (int identifier);
  vector <int> search_notes_v12 (string search, const vector <string> & bibles);
private:
  void update_search_fields_v1 (int identifier);
  void update_search_fields_v2 (int identifier);

public:
  void mark_for_deletion_v12 (int identifier);
  void unmark_for_deletion_v12 (int identifier);
  bool is_marked_for_deletion_v12 (int identifier);
private:
  string expiry_file_v1 (int identifier);
  string expiry_key_v2 ();
  void mark_for_deletion_v1 (int identifier);
  void mark_for_deletion_v2 (int identifier);
  bool is_marked_for_deletion_v1 (int identifier);
  bool is_marked_for_deletion_v2 (int identifier);
  void unmark_for_deletion_v1 (int identifier);
  void unmark_for_deletion_v2 (int identifier);
  void touch_marked_for_deletion_v12 ();
  vector <int> get_due_for_deletion_v1 ();
  vector <int> get_due_for_deletion_v2 ();

public:
  void set_checksum_v12 (int identifier, const string & checksum);
  string get_checksum_v12 (int identifier);
  void delete_checksum_v12 (int identifier);
  void update_checksum_v12 (int identifier);
  string get_multiple_checksum_v12 (const vector <int> & identifiers);
  vector <int> get_notes_in_range_for_bibles_v12 (int lowId, int highId, vector <string> bibles, bool anybible);
private:
  void update_checksum_v1 (int identifier);
  void update_checksum_v2 (int identifier);
  
public:
  void set_availability_v12 (bool available);
  bool available_v12 ();
private:
  string availability_flag ();

public:
  string get_bulk_v12 (vector <int> identifiers);
  vector <string> set_bulk_v2 (string json);
private:
  vector <string> set_bulk_v1 (string json);
  
private:
  void update_database_v12 (int identifier);
  void update_database_v1 (int identifier);
  void update_database_v2 (int identifier);
  void update_database_internal (int identifier, int modified, string assigned, string subscriptions, string bible, string passage, string status, int severity, string summary, string contents);
  
public:
  bool is_v1 (int identifier);
  bool gradually_convert_v1_to_v2 (int amount, vector <int> & ids);
private:
  void convert_v1_to_v2 (int identifier);
  
private:
  friend void test_database_notes ();

private:
  sqlite3 * connect ();
  sqlite3 * connect_checksums ();
  
private:
  string get_field_v2 (int identifier, string key);
  void set_field_v2 (int identifier, string key, string value);

};


#endif

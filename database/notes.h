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
  void create ();

private:
  string database_path ();
  string checksums_database_path ();
  string main_folder_path ();
  string note_file (int identifier);

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
  vector <int> get_identifiers ();
  int store_new_note (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
private:
  int get_new_unique_identifier ();
  
public:
  vector <int> select_notes (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit);
private:
  string notes_select_identifier ();
  string notes_optional_fulltext_search_relevance_statement (string search);
  string notes_from_where_statement ();
  string notes_optional_fulltext_search_statement (string search);
  string notes_order_by_relevance_statement ();

public:
  string get_summary (int identifier);
  void set_summary (int identifier, const string& summary);
private:
  string summary_key ();

public:
  string get_contents (int identifier);
  void set_contents (int identifier, const string& contents);
private:
  string contents_key ();
  string assemble_contents (int identifier, string contents);
  void set_raw_contents (int identifier, const string& contents);
  
public:
  void erase (int identifier);
  
public:
  void add_comment (int identifier, const string& comment);
  
public:
  void subscribe (int identifier);
  void unsubscribe (int identifier);
  void subscribe_user (int identifier, const string& user);
  void unsubscribe_user (int identifier, const string& user);
  bool is_subscribed (int identifier, const string& user);
  vector <string> get_subscribers (int identifier);
  void set_subscribers (int identifier, vector <string> subscribers);
private:
  string subscriptions_key ();
  string get_raw_subscriptions (int identifier);
  void set_raw_subscriptions (int identifier, const string& subscriptions);

public:
  void assign_user (int identifier, const string& user);
  bool is_assigned (int identifier, const string& user);
  void unassign_user (int identifier, const string& user);
  vector <string> get_all_assignees (const vector <string>& bibles);
  vector <string> get_assignees (int identifier);
  void set_assignees (int identifier, vector <string> assignees);
private:
  string assigned_key ();
  vector <string> get_assignees_internal (string assignees);
  void set_raw_assigned (int identifier, const string& assigned);
  string get_raw_assigned (int identifier);
  
public:
  string get_bible (int identifier);
  void set_bible (int identifier, const string& bible);
  vector <string> get_all_bibles ();
private:
  string bible_key ();

public:
  string encode_passage (int book, int chapter, int verse);
  Passage decode_passage (string passage);
  string decode_passage (int identifier);
  vector <Passage> get_passages (int identifier);
  void set_passages (int identifier, const vector <Passage>& passages, bool import = false);
  void set_raw_passage (int identifier, const string& passage);
  void index_raw_passage (int identifier, const string& passage);
private:
  string passage_key ();
  string get_raw_passage (int identifier);
  
public:
  string get_raw_status (int identifier);
  string get_status (int identifier);
  void set_status (int identifier, const string& status, bool import = false);
  vector <Database_Notes_Text> get_possible_statuses ();
private:
  string status_key ();

public:
  string get_severity (int identifier);
  int get_raw_severity (int identifier);
  void set_raw_severity (int identifier, int severity);
  vector <Database_Notes_Text> get_possible_severities ();
private:
  string severity_key ();
  vector <string> standard_severities ();

public:
  int get_modified (int identifier);
  void set_modified (int identifier, int time);
private:
  string modified_key ();
  void note_modified_actions (int identifier);

public:
  bool get_public (int identifier);
  void set_public (int identifier, bool value);
private:
  string public_key ();
  
public:
  string get_search_field (int identifier);
  void update_search_fields (int identifier);
  vector <int> search_notes (string search, const vector <string> & bibles);

public:
  void mark_for_deletion (int identifier);
  void unmark_for_deletion (int identifier);
  bool is_marked_for_deletion (int identifier);
private:
  string expiry_key ();
  void touch_marked_for_deletion ();
  vector <int> get_due_for_deletion ();

public:
  void set_checksum (int identifier, const string & checksum);
  string get_checksum (int identifier);
  void delete_checksum (int identifier);
  void update_checksum (int identifier);
  string get_multiple_checksum (const vector <int> & identifiers);
  vector <int> get_notes_in_range_for_bibles (int lowId, int highId, vector <string> bibles, bool anybible);
  
public:
  void set_availability (bool available);
  bool available ();
private:
  string availability_flag ();

public:
  string get_bulk (vector <int> identifiers);
  vector <string> set_bulk (string json);
  
private:
  void update_database (int identifier);
  void update_database_internal (int identifier, int modified, string assigned, string subscriptions, string bible, string passage, string status, int severity, string summary, string contents);
  
private:
  friend void test_database_notes ();

private:
  sqlite3 * connect ();
  sqlite3 * connect_checksums ();
  
private:
  string get_field (int identifier, string key);
  void set_field (int identifier, string key, string value);

};


#endif

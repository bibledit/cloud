/*
Copyright (©) 2003-2022 Teus Benschop.

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
#include <classes/merge.h>

void bible_logic_store_chapter (const string& bible, int book, int chapter, const string& usfm);
void bible_logic_delete_chapter (const string& bible, int book, int chapter);
void bible_logic_delete_book (const string& bible, int book);
void bible_logic_delete_bible (const string& bible);
void bible_logic_import_resource (string bible, string resource);
void bible_logic_log_change (const string& bible, int book, int chapter, const string& usfm, string user, const string & summary, bool force);
void bible_logic_log_merge (string user, string bible, int book, int chapter,
                            string base, string change, string prioritized_change, string result);
void bible_logic_kick_unsent_data_timer ();
void bible_logic_kick_unreceived_data_timer ();
string bible_logic_unsent_unreceived_data_warning ();
void bible_logic_merge_irregularity_mail (vector <string> users,
                                          vector <Merge_Conflict> conflicts);
void bible_logic_unsafe_save_mail (string subject, const string & explanation,
                                   const string & user,
                                   const string & usfm,
                                   int book, int chapter);
void bible_logic_client_receive_merge_mail (const string & bible, int book, int chapter,
                                            const string & user,
                                            const string & client_old,
                                            const string & client_new,
                                            const string & server);
void bible_logic_client_mail_pending_bible_updates (string user);
void bible_logic_client_no_write_access_mail (const string & bible, int book, int chapter,
                                              const string & user,
                                              const string & oldusfm, const string & newusfm);
void bible_logic_recent_save_email (const string & bible, int book, int chapter, int verse,
                                    const string & user,
                                    const string & old_usfm, const string & new_usfm);
void bible_logic_optional_merge_irregularity_email (const string & bible, int book, int chapter,
                                                    const string & user,
                                                    const string & ancestor_usfm,
                                                    const string & edited_usfm,
                                                    const string & merged_usfm);
const char * bible_logic_insert_operator ();
const char * bible_logic_delete_operator ();
const char * bible_logic_format_paragraph_operator ();
const char * bible_logic_format_character_operator ();
void bible_logic_condense_editor_updates (const vector <int> & positions_in,
                                          const vector <int> & sizes_in,
                                          const vector <bool> & additions_in,
                                          const vector <string> & content_in,
                                          vector <int> & positions_out,
                                          vector <int> & sizes_out,
                                          vector <string> & operators_out,
                                          vector <string> & content_out);
void bible_logic_html_to_editor_updates (const string & editor_html,
                                         const string & server_html,
                                         vector <int> & positions,
                                         vector <int> & sizes,
                                         vector <string> & operators,
                                         vector <string> & content);
void bible_logic_create_empty_bible (const string & name);

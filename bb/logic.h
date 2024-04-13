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
#include <classes/merge.h>

namespace bible_logic {

void store_chapter (const std::string& bible, int book, int chapter, const std::string& usfm);
void delete_chapter (const std::string& bible, int book, int chapter);
void delete_book (const std::string& bible, int book);
void delete_bible (const std::string& bible);
void import_resource (std::string bible, std::string resource);
void log_change (const std::string& bible, int book, int chapter, const std::string& usfm, std::string user, const std::string & summary, bool force);
void log_merge (const std::string& user, const std::string& bible, int book, int chapter,
                const std::string& base, const std::string& change,
                const std::string& prioritized_change, const std::string& result);
void kick_unsent_data_timer ();
void kick_unreceived_data_timer ();
std::string unsent_unreceived_data_warning ();
void merge_irregularity_mail (const std::vector <std::string>& users,
                              const std::vector <Merge_Conflict>& conflicts);
void unsafe_save_mail (std::string subject, const std::string & explanation,
                       const std::string & user,
                       const std::string & usfm,
                       int book, int chapter);
void client_receive_merge_mail (const std::string & bible, int book, int chapter,
                                const std::string & user,
                                const std::string & client_old,
                                const std::string & client_new,
                                const std::string & server);
void client_mail_pending_bible_updates (const std::string& user);
void client_no_write_access_mail (const std::string & bible, int book, int chapter,
                                  const std::string & user,
                                  const std::string & oldusfm, const std::string & newusfm);
void recent_save_email (const std::string & bible, int book, int chapter,
                        const std::string & user,
                        const std::string & old_usfm, const std::string & new_usfm);
void optional_merge_irregularity_email (const std::string & bible, int book, int chapter,
                                        const std::string & user,
                                        const std::string & ancestor_usfm,
                                        const std::string & edited_usfm,
                                        const std::string & merged_usfm);
const char * insert_operator ();
const char * delete_operator ();
const char * format_paragraph_operator ();
const char * format_character_operator ();
void condense_editor_updates (const std::vector <int> & positions_in,
                              const std::vector <int> & sizes_in,
                              const std::vector <bool> & additions_in,
                              const std::vector <std::string> & content_in,
                              std::vector <int> & positions_out,
                              std::vector <int> & sizes_out,
                              std::vector <std::string> & operators_out,
                              std::vector <std::string> & content_out);
void html_to_editor_updates (const std::string & editor_html,
                             const std::string & server_html,
                             std::vector <int> & positions,
                             std::vector <int> & sizes,
                             std::vector <std::string> & operators,
                             std::vector <std::string> & content);
void create_empty_bible (const std::string & name);

}

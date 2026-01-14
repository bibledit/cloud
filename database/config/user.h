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

class Database_Config_User
{
public:
  Database_Config_User (Webserver_Request& webserver_request);
  void trim () const;
  void remove (const std::string& username) const;
  void clear_cache () const;
  std::string get_bible () const;
  void set_bible (const std::string& bible) const;
  bool get_subscribe_to_consultation_notes_edited_by_me () const;
  void set_subscribe_to_consultation_notes_edited_by_me (bool value) const;
  bool get_notify_me_of_any_consultation_notes_edits () const;
  bool get_notify_user_of_any_consultation_notes_edits (const std::string& username) const;
  void set_notify_me_of_any_consultation_notes_edits (bool value) const;
  bool get_subscribed_consultation_note_notification () const;
  bool get_user_subscribed_consultation_note_notification (const std::string& username) const;
  void set_subscribed_consultation_note_notification (bool value) const;
  bool get_assigned_to_consultation_notes_changes () const;
  bool get_user_assigned_to_consultation_notes_changes (const std::string& username) const;
  void set_assigned_to_consultation_notes_changes (bool value) const;
  bool get_generate_change_notifications () const;
  bool get_user_generate_change_notifications (const std::string& username) const;
  void set_generate_change_notifications (bool value) const;
  bool get_assigned_consultation_note_notification () const ;
  bool get_user_assigned_consultation_note_notification (const std::string& username) const;
  void set_assigned_consultation_note_notification (bool value) const;
  int get_consultation_notes_passage_selector () const;
  void set_consultation_notes_passage_selector (int value) const;
  int get_consultation_notes_edit_selector () const;
  void set_consultation_notes_edit_selector (int value) const;
  int get_consultation_notes_non_edit_selector () const;
  void set_consultation_notes_non_edit_selector (int value) const;
  std::vector <std::string> get_consultation_notes_status_selectors () const;
  void set_consultation_notes_status_selectors (std::vector <std::string> values) const;
  std::string get_consultation_notes_bible_selector () const;
  void set_consultation_notes_bible_selector (const std::string& value) const;
  std::string get_consultation_notes_assignment_selector () const;
  void set_consultation_notes_assignment_selector (const std::string& value) const;
  bool get_consultation_notes_subscription_selector () const;
  void set_consultation_notes_subscription_selector (bool value) const;
  int get_consultation_notes_severity_selector () const;
  void set_consultation_notes_severity_selector (int value) const;
  int get_consultation_notes_text_selector () const;
  void set_consultation_notes_text_selector (int value) const;
  std::string get_consultation_notes_search_text () const;
  void set_consultation_notes_search_text (const std::string& value) const;
  int get_consultation_notes_passage_inclusion_selector () const;
  void set_consultation_notes_passage_inclusion_selector (int value) const;
  int get_consultation_notes_text_inclusion_selector () const;
  void set_consultation_notes_text_inclusion_selector (int value) const;
  bool get_bible_changes_notification () const;
  bool get_user_bible_changes_notification (const std::string& username) const;
  void set_bible_changes_notification (bool value) const;
  bool get_deleted_consultation_note_notification () const;
  bool get_user_deleted_consultation_note_notification (const std::string& username) const;
  void set_deleted_consultation_note_notification (bool value) const;
  bool get_bible_checks_notification () const;
  bool get_user_bible_checks_notification (const std::string& username) const;
  void set_bible_checks_notification (bool value) const;
  bool get_pending_changes_notification () const;
  bool get_user_pending_changes_notification (const std::string& username) const;
  void set_pending_changes_notification (bool value) const;
  bool get_user_changes_notification () const;
  bool get_user_user_changes_notification (const std::string& username) const;
  void set_user_changes_notification (bool value) const;
  bool get_assigned_notes_statistics_notification () const;
  bool get_user_assigned_notes_statistics_notification (const std::string& username) const;
  void set_assigned_notes_statistics_notification (bool value) const;
  bool get_subscribed_notes_statistics_notification () const;
  bool get_user_subscribed_notes_statistics_notification (const std::string& username) const;
  void set_subscribed_notes_statistics_notification (bool value) const;
  bool get_notify_me_of_my_posts () const;
  bool get_user_notify_me_of_my_posts (const std::string& username) const;
  void set_notify_me_of_my_posts (bool value) const;
  bool get_suppress_mail_from_your_updates_notes () const;
  bool get_user_suppress_mail_from_your_updates_notes (const std::string& username) const;
  void set_suppress_mail_from_your_updates_notes (bool value) const;
  std::vector <std::string> get_active_resources () const;
  void set_active_resources (std::vector <std::string> values) const;
  std::vector <std::string> get_consistency_resources () const;
  void set_consistency_resources (std::vector <std::string> values) const;
  int get_sprint_month () const;
  void set_sprint_month (int value) const;
  int get_sprint_year () const;
  void set_sprint_year (int value) const;
  bool get_sprint_progress_notification () const;
  bool get_user_sprint_progress_notification (const std::string& username) const;
  void set_sprint_progress_notification (bool value) const;
  bool get_user_changes_notifications_online () const;
  bool get_user_user_changes_notifications_online (const std::string& username) const;
  void set_user_changes_notifications_online (bool value) const;
  bool get_contributor_changes_notifications_online () const;
  bool get_contributor_changes_notifications_online (const std::string& username) const;
  void set_contributor_changes_notifications_online (bool value) const;
  std::string get_workspace_urls () const;
  void set_workspace_urls (const std::string& value) const;
  std::string get_workspace_widths () const;
  void set_workspace_widths (const std::string& value) const;
  std::string get_workspace_heights () const;
  void set_workspace_heights (const std::string& value) const;
  std::string get_entire_workspace_widths () const;
  void set_entire_workspace_widths (const std::string& value) const;
  std::string get_active_workspace () const;
  void set_active_workspace (const std::string& value) const;
  bool get_postpone_new_notes_mails () const;
  void set_postpone_new_notes_mails (bool value) const;
  std::string get_recently_applied_styles () const;
  void set_recently_applied_styles (const std::string& values) const;
  std::vector<int> get_focused_books () const;
  void set_focused_books (const std::vector<int>& books) const;
  std::vector<int> get_focused_chapters () const;
  void set_focused_chapters (const std::vector<int>& chapters) const;
  std::vector<int> get_focused_verses () const;
  void set_focused_verses (const std::vector<int>& verses) const;
  std::vector <int> get_updated_settings () const;
  void set_updated_settings (const std::vector <int>& values) const;
  void add_updated_setting (int value) const;
  void remove_updated_setting (int value) const;
  std::vector <int> get_removed_changes () const;
  void set_removed_changes (const std::vector <int>& values) const;
  void add_removed_change (int value) const;
  void remove_removed_change (int value) const;
  std::string get_change_notifications_checksum () const;
  void set_change_notifications_checksum (const std::string& value) const;
  void set_user_change_notifications_checksum (const std::string& user, const std::string& value) const;
  void set_live_bible_editor (int time) const;
  int get_live_bible_editor () const;
  void set_resource_verses_before (int verses) const;
  int get_resource_verses_before () const;
  void set_resource_verses_after (int verses) const;
  int get_resource_verses_after () const;
  std::string get_sync_key () const;
  void set_sync_key (const std::string& key) const;
  //  std::string get_site_language () const;
  //  void set_site_language (const std::string& value) const;
  void set_general_font_size (int size) const;
  int get_general_font_size () const;
  void set_menu_font_size (int size) const;
  int get_menu_font_size () const;
  void set_resources_font_size (int size) const;
  int get_bible_editors_font_size () const;
  void set_bible_editors_font_size (int size) const;
  int get_resources_font_size () const;
  void set_hebrew_font_size (int size) const;
  int get_hebrew_font_size () const;
  void set_greek_font_size (int size) const;
  int get_greek_font_size () const;
  void set_vertical_caret_position (int position) const;
  int get_vertical_caret_position () const;
  void set_current_theme (int index) const;
  int get_current_theme () const;
  bool get_display_breadcrumbs () const;
  void set_display_breadcrumbs (bool value) const;
  void set_workspace_menu_fadeout_delay (int value) const;
  int get_workspace_menu_fadeout_delay () const;
  int get_editing_allowed_difference_chapter () const;
  void set_editing_allowed_difference_chapter (int value) const;
  int get_editing_allowed_difference_verse () const;
  void set_editing_allowed_difference_verse (int value) const;
  bool get_basic_interface_mode_default () const;
  bool get_basic_interface_mode () const;
  void set_basic_interface_mode (bool value) const;
  bool get_main_menu_always_visible () const;
  void set_main_menu_always_visible (bool value) const;
  bool get_swipe_actions_available () const;
  void set_swipe_actions_available (bool value) const;
  bool get_fast_editor_switching_available () const;
  void set_fast_editor_switching_available (bool value) const;
  bool get_include_related_passages () const;
  void set_include_related_passages (bool value) const;
  int get_fast_switch_visual_editors () const;
  void set_fast_switch_visual_editors (int value) const;
  int get_fast_switch_usfm_editors () const;
  void set_fast_switch_usfm_editors (int value) const;
  bool get_enable_styles_button_visual_editors () const;
  void set_enable_styles_button_visual_editors (bool value) const;
  bool get_menu_changes_in_basic_mode () const;
  void set_menu_changes_in_basic_mode (bool value) const;
  bool get_privilege_use_advanced_mode () const;
  bool get_privilege_use_advanced_mode_for_user (const std::string& username) const;
  void set_privilege_use_advanced_mode_for_user (const std::string& username, bool value) const;
  bool get_privilege_delete_consultation_notes () const;
  void set_privilege_delete_consultation_notes (bool value) const;
  bool get_privilege_delete_consultation_notes_for_user (const std::string& username) const;
  void set_privilege_delete_consultation_notes_for_user (const std::string& username, bool value) const;
  bool get_privilege_set_stylesheets () const;
  bool get_privilege_set_stylesheets_for_user (const std::string& username) const;
  void set_privilege_set_stylesheets_for_user (const std::string& username, bool value) const;
  bool get_dismiss_changes_at_top () const;
  void set_dismiss_changes_at_top (bool value) const;
  bool get_quick_note_edit_link () const;
  void set_quick_note_edit_link (bool value) const;
  bool get_show_bible_in_notes_list () const;
  void set_show_bible_in_notes_list (bool value) const;
  bool get_show_note_status () const;
  void set_show_note_status (bool value) const;
  bool get_show_verse_text_at_create_note () const;
  void set_show_verse_text_at_create_note (bool value) const;
  bool get_order_changes_by_author () const;
  void set_order_changes_by_author (bool value) const;
  std::vector <std::string> get_automatic_note_assignment () const;
  void set_automatic_note_assignment (const std::vector <std::string>& values) const;
  bool get_receive_focused_reference_from_paratext () const;
  void set_receive_focused_reference_from_paratext (bool value) const;
  bool get_receive_focused_reference_from_accordance () const;
  void set_receive_focused_reference_from_accordance (bool value) const;
  bool get_use_colored_note_status_labels () const;
  void set_use_colored_note_status_labels (bool value) const;
  int get_notes_date_format () const;
  void set_notes_date_format (int value) const;
  std::vector <std::string> get_change_notifications_bibles () const;
  std::vector <std::string> get_change_notifications_bibles_for_user (const std::string & user) const;
  void set_change_notifications_bibles (const std::vector <std::string>& values) const;
  bool get_enable_spell_check () const;
  void set_enable_spell_check (bool value) const;
  bool get_show_navigation_arrows() const;
  void set_show_navigation_arrows(bool value) const;
private:
  Webserver_Request& m_webserver_request;
  std::string file (const std::string& user) const;
  std::string file (const std::string& user, const char * key) const;
  std::string mapkey (const std::string& user, const char * key) const;
  std::string get_value (const char * key, const char * default_value) const;
  bool get_boolean_value (const char * key, bool default_value) const;
  int get_numeric_value (const char * key, int default_value) const;
  std::string get_value_for_user (const std::string& user, const char * key, const char * default_value) const;
  bool get_boolean_value_for_user (const std::string& user, const char * key, bool default_value) const;
  int get_numeric_value_for_user (const std::string& user, const char * key, int default_value) const;
  void set_value (const char * key, const std::string& value) const;
  void set_boolean_value (const char * key, bool value) const;
  void set_numeric_value (const char * key, int value) const;
  void set_value_for_user (const std::string& user, const char * key, const std::string& value) const;
  void set_boolean_value_for_user (const std::string& user, const char * key, bool value) const;
  std::vector <std::string> get_list (const char * key) const;
  std::vector <std::string> get_list_for_user (const std::string& user, const char * key) const;
  void set_list (const char * key, const std::vector <std::string>& values) const;
  void set_list_for_user (const std::string& user, const char * key, const std::vector <std::string>& values) const;
  std::vector <int> get_numeric_list (const char * key) const;
  void set_numeric_list (const char * key, const std::vector <int>& values) const;
  static const char * sprint_month_key ();
  static const char * sprint_year_key ();
  bool default_bible_checks_notification () const;
};

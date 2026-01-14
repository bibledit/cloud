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


#include <tasks/run.h>
#include <tasks/logic.h>
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <email/receive.h>
#include <email/send.h>
#include <search/rebibles.h>
#include <search/renotes.h>
#include <styles/sheets.h>
#include <bb/import_run.h>
#include <compare/compare.h>
#include <database/maintenance.h>
#include <database/config/general.h>
#include <tmp/tmp.h>
#include <collaboration/link.h>
#include <sendreceive/sendreceive.h>
#include <sendreceive/settings.h>
#include <sendreceive/bibles.h>
#include <sendreceive/notes.h>
#include <sendreceive/changes.h>
#include <sendreceive/files.h>
#include <sendreceive/resources.h>
#include <demo/logic.h>
#include <resource/convert2resource.h>
#include <resource/convert2bible.h>
#include <resource/download.h>
#include <statistics/statistics.h>
#include <changes/modifications.h>
#include <sprint/burndown.h>
#include <checks/run.h>
#include <export/index.h>
#include <export/web.h>
#include <export/html.h>
#include <export/usfm.h>
#include <export/textusfm.h>
#include <export/odt.h>
#include <export/info.h>
#include <export/esword.h>
#include <export/onlinebible.h>
#include <paratext/logic.h>
#include <resource/logic.h>
#include <sword/logic.h>
#include <bb/logic.h>
#include <client/logic.h>
#include <user/logic.h>
#include <rss/logic.h>
#include <system/logic.h>
#include <notes/logic.h>
#include <changes/logic.h>
#include <database/cache.h>
#include <images/logic.h>
#include <tasks/enums.h>
#include <filter/google.h>


std::atomic <int> running_tasks (0);


void tasks_run_one (const std::string& filename)
{
  // Increase running tasks count.
  running_tasks++;

  // Read the task from disk and erase the file.
  const std::string path = filter_url_create_path ({tasks_logic_folder (), filename});
  std::vector <std::string> lines = filter::string::explode (filter_url_file_get_contents (path), '\n');
  filter_url_unlink (path);
  
  // Interpret the task's command and its parameters, if any.
  std::string command {};
  if (!lines.empty ()) {
    command = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter1 {};
  if (!lines.empty ()) {
    parameter1 = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter2 {};
  if (!lines.empty ()) {
    parameter2 = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter3 {};
  if (!lines.empty ()) {
    parameter3 = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter4 {};
  if (!lines.empty ()) {
    parameter4 = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter5 {};
  if (!lines.empty ()) {
    parameter5 = lines [0];
    lines.erase (lines.begin ());
  }
  std::string parameter6 {};
  if (!lines.empty ()) {
    parameter6 = lines [0];
    lines.erase (lines.begin ());
  }
  
  if (command == task::rotate_journal) {
    Database_Logs::rotate ();
  }
  else if (command == task::receive_email) {
    email::receive ();
  }
  else if (command == task::send_email) {
    email::send ();
  }
  else if (command == task::reindex_bibles) {
    search_reindex_bibles (filter::string::convert_to_bool (parameter1));
  }
  else if (command == task::reindex_notes) {
    search_reindex_notes ();
  }
  else if (command == task::create_css) {
    styles_sheets_create_all_run ();
  }
  else if (command == task::import_bible) {
    bible_import_run (parameter1, parameter2, filter::string::convert_to_int (parameter3), filter::string::convert_to_int (parameter4));
  }
  else if (command == task::import_resource) {
    bible_logic::import_resource (parameter1, parameter2);
  }
  else if (command == task::compare_usfm) {
    compare_compare (parameter1, parameter2, filter::string::convert_to_int (parameter3));
  }
  else if (command == task::maintain_database) {
    database_maintenance ();
  }
  else if (command == task::clean_tmp_files) {
    tmp_tmp ();
  }
  else if (command == task::link_git_repository) {
    collaboration_link (parameter1, filter::string::convert_to_int (parameter2), parameter3);
  }
  else if (command == task::send_receive_bibles) {
    sendreceive_sendreceive (parameter1);
  }
  else if (command == task::sync_notes) {
    sendreceive_notes ();
  }
  else if (command == task::sync_bibles) {
    sendreceive_bibles ();
  }
  else if (command == task::sync_settings) {
    sendreceive_settings ();
  }
  else if (command == task::sync_changes) {
    sendreceive_changes ();
  }
  else if (command == task::sync_files) {
    sendreceive_files ();
  }
  else if (command == task::sync_resources) {
    sendreceive_resources ();
  }
  else if (command == task::clean_demo) {
    demo_clean_data ();
  }
  else if (command == task::convert_bible_to_resource) {
    convert_bible_to_resource (parameter1);
  }
  else if (command == task::convert_resource_to_bible) {
    convert_resource_to_bible (parameter1);
  }
  else if (command == task::notes_statistics) {
    statistics_statistics ();
  }
  else if (command == task::generate_changes) {
    changes_modifications ();
  }
  else if (command == task::sprint_burndown) {
    sprint_burndown ("", 0, 0);
  }
  else if (command == task::check_bible) {
    checks_run (parameter1);
  }
  else if (command == task::export_all) {
    export_index ();
  }
  else if (command == task::export_web_main) {
    export_web_book (parameter1, filter::string::convert_to_int (parameter2), filter::string::convert_to_bool (parameter3));
  }
  else if (command == task::export_web_index) {
    export_web_index (parameter1, filter::string::convert_to_bool (parameter2));
  }
  else if (command == task::export_html) {
    export_html_book (parameter1, filter::string::convert_to_int (parameter2), filter::string::convert_to_bool (parameter3));
  }
  else if (command == task::export_usfm) {
    export_usfm (parameter1, filter::string::convert_to_bool (parameter2));
  }
  else if (command == task::export_text_usfm) {
    export_text_usfm_book (parameter1, filter::string::convert_to_int (parameter2), filter::string::convert_to_bool (parameter3));
  }
  else if (command == task::export_odt) {
    export_odt_book (parameter1, filter::string::convert_to_int (parameter2), filter::string::convert_to_bool (parameter3));
  }
  else if (command == task::export_info) {
    export_info (parameter1, filter::string::convert_to_bool (parameter2));
  }
  else if (command == task::export_esword) {
    export_esword (parameter1, filter::string::convert_to_bool (parameter2));
  }
  else if (command == task::export_online_bible) {
    export_onlinebible (parameter1, filter::string::convert_to_bool (parameter2));
  }
  else if (command == task::setup_paratext) {
    Paratext_Logic::setup (parameter1, parameter2);
  }
  else if (command == task::sync_paratext) {
    int imethod = filter::string::convert_to_int(parameter1);
    auto method = static_cast<tasks::enums::paratext_sync>(imethod);
    Paratext_Logic::synchronize (method);
  }
  else if (command == task::refresh_sword_modules) {
    sword_logic_refresh_module_list ();
  }
  else if (command == task::install_sword_module) {
    sword_logic_run_scheduled_module_install (parameter1, parameter2);
  }
  else if (command == task::update_sword_modules) {
    sword_logic_update_installed_modules ();
  }
  else if (command == task::list_usfm_resources) {
    client_logic_usfm_resources_update ();
  }
  else if (command == task::create_sample_bible) {
    demo_create_sample_bible ();
  }
  else if (command == task::cache_resources) {
    resource_logic_create_cache ();
  }
  else if (command == task::refresh_web_resources) {
    resource_logic_bible_gateway_module_list_refresh ();
    resource_logic_study_light_module_list_refresh ();
  }
#ifdef HAVE_CLOUD
  else if (command == task::rss_feed_update_chapter) {
    rss_logic_execute_update (parameter1, parameter2, filter::string::convert_to_int (parameter3), filter::string::convert_to_int (parameter4), parameter5, parameter6);
  }
#endif
#ifdef HAVE_CLIENT
  else if (command == task::produce_bibles_transferfile) {
    system_logic_produce_bibles_file (filter::string::convert_to_int (parameter1));
  }
  else if (command == task::import_bibles_transferfile) {
    system_logic_import_bibles_file (parameter1);
  }
  else if (command == task::produce_notes_transferfile) {
    system_logic_produce_notes_file (filter::string::convert_to_int (parameter1));
  }
  else if (command == task::import_notes_transferfile) {
    system_logic_import_notes_file (parameter1);
  }
  else if (command == task::produce_resources_transferfile) {
    system_logic_produce_resources_file (filter::string::convert_to_int (parameter1));
  }
  else if (command == task::import_resources_transferfile) {
    system_logic_import_resources_file (parameter1);
  }
#endif
  else if (command == task::delete_changes) {
    changes_clear_notifications_user (parameter1, parameter2);
  }
  else if (command == task::clear_caches) {
    database::cache::file::trim (true);
  }
  else if (command == task::trim_caches) {
    database::cache::file::trim (false);
  }
  else if (command == task::create_empty_bible) {
    bible_logic::create_empty_bible (parameter1);
  }
  else if (command == task::import_bible_images) {
    images_logic_import_images (parameter1);
  }
  else if (command == task::get_google_access_token) {
    filter::google::refresh_access_token ();
  }
  else {
    Database_Logs::log ("Unknown task: " + command);
  }

  // Decrease running tasks count.
  running_tasks--;
}


void tasks_run_check ()
{
  // Don't run more than so many tasks.
  if (tasks_run_active_count () >= MAX_PARALLEL_TASKS)
    return;
  // Get and start first available task.
  const std::vector <std::string> tasks = filter_url_scandir (tasks_logic_folder ());
  if (tasks.empty ())
    return;
  std::thread task_thread = std::thread (tasks_run_one, tasks [0]);
  // Detach the thread so the thread continues to run independently,
  // when the thread object goes out of scope, and no memory is leaked this way.
  task_thread.detach ();
}


int tasks_run_active_count ()
{
  const int taskscount = running_tasks;
  return taskscount;
}


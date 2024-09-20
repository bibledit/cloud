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
#include <resource/print.h>
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
#include <manage/hyphenate.h>
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
#include <nmt/logic.h>
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
  std::vector <std::string> lines = filter::strings::explode (filter_url_file_get_contents (path), '\n');
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
  
  if (command == task::rotatejournal) {
    Database_Logs::rotate ();
  }
  else if (command == task::receiveemail) {
    email_receive ();
  }
  else if (command == task::sendemail) {
    email_send ();
  }
  else if (command == task::reindexbibles) {
    search_reindex_bibles (filter::strings::convert_to_bool (parameter1));
  }
  else if (command == task::reindexnotes) {
    search_reindex_notes ();
  }
  else if (command == task::createcss) {
    styles_sheets_create_all_run ();
  }
  else if (command == task::importbible) {
    bible_import_run (parameter1, parameter2, filter::strings::convert_to_int (parameter3), filter::strings::convert_to_int (parameter4));
  }
  else if (command == task::importresource) {
    bible_logic::import_resource (parameter1, parameter2);
  }
  else if (command == task::compareusfm) {
    compare_compare (parameter1, parameter2, filter::strings::convert_to_int (parameter3));
  }
  else if (command == task::maintaindatabase) {
    database_maintenance ();
  }
  else if (command == task::cleantmpfiles) {
    tmp_tmp ();
  }
  else if (command == task::linkgitrepository) {
    collaboration_link (parameter1, filter::strings::convert_to_int (parameter2), parameter3);
  }
  else if (command == task::sendreceivebibles) {
    sendreceive_sendreceive (parameter1);
  }
  else if (command == task::syncnotes) {
    sendreceive_notes ();
  }
  else if (command == task::syncbibles) {
    sendreceive_bibles ();
  }
  else if (command == task::syncsettings) {
    sendreceive_settings ();
  }
  else if (command == task::syncchanges) {
    sendreceive_changes ();
  }
  else if (command == task::syncfiles) {
    sendreceive_files ();
  }
  else if (command == task::syncresources) {
    sendreceive_resources ();
  }
  else if (command == task::cleandemo) {
    demo_clean_data ();
  }
  else if (command == task::convertbible2resource) {
    convert_bible_to_resource (parameter1);
  }
  else if (command == task::convertresource2bible) {
    convert_resource_to_bible (parameter1);
  }
  else if (command == task::printresources) {
    resource_print_job (parameter1, parameter2, parameter3);
  }
  else if (command == task::notesstatistics) {
    statistics_statistics ();
  }
  else if (command == task::generatechanges) {
    changes_modifications ();
  }
  else if (command == task::sprintburndown) {
    sprint_burndown ("", 0, 0);
  }
  else if (command == task::checkbible) {
    checks_run (parameter1);
  }
  else if (command == task::exportall) {
    export_index ();
  }
  else if (command == task::exportwebmain) {
    export_web_book (parameter1, filter::strings::convert_to_int (parameter2), filter::strings::convert_to_bool (parameter3));
  }
  else if (command == task::exportwebindex) {
    export_web_index (parameter1, filter::strings::convert_to_bool (parameter2));
  }
  else if (command == task::exporthtml) {
    export_html_book (parameter1, filter::strings::convert_to_int (parameter2), filter::strings::convert_to_bool (parameter3));
  }
  else if (command == task::exportusfm) {
    export_usfm (parameter1, filter::strings::convert_to_bool (parameter2));
  }
  else if (command == task::exporttextusfm) {
    export_text_usfm_book (parameter1, filter::strings::convert_to_int (parameter2), filter::strings::convert_to_bool (parameter3));
  }
  else if (command == task::exportodt) {
    export_odt_book (parameter1, filter::strings::convert_to_int (parameter2), filter::strings::convert_to_bool (parameter3));
  }
  else if (command == task::exportinfo) {
    export_info (parameter1, filter::strings::convert_to_bool (parameter2));
  }
  else if (command == task::exportesword) {
    export_esword (parameter1, filter::strings::convert_to_bool (parameter2));
  }
  else if (command == task::exportonlinebible) {
    export_onlinebible (parameter1, filter::strings::convert_to_bool (parameter2));
  }
  else if (command == task::hyphenate) {
    manage_hyphenate (parameter1, parameter2);
  }
  else if (command == task::setupparatext) {
    Paratext_Logic::setup (parameter1, parameter2);
  }
  else if (command == task::syncparatext) {
    int imethod = filter::strings::convert_to_int(parameter1);
    auto method = static_cast<tasks::enums::paratext_sync>(imethod);
    Paratext_Logic::synchronize (method);
  }
  else if (command == task::importimages) {
    resource_logic_import_images (parameter1, parameter2);
  }
  else if (command == task::refreshswordmodules) {
    sword_logic_refresh_module_list ();
  }
  else if (command == task::installswordmodule) {
    sword_logic_run_scheduled_module_install (parameter1, parameter2);
  }
  else if (command == task::updateswordmodules) {
    sword_logic_update_installed_modules ();
  }
  else if (command == task::listusfmresources) {
    client_logic_usfm_resources_update ();
  }
  else if (command == task::createsamplebible) {
    demo_create_sample_bible ();
  }
  else if (command == task::cacheresources) {
    resource_logic_create_cache ();
  }
  else if (command == task::refreshwebresources) {
    resource_logic_bible_gateway_module_list_refresh ();
    resource_logic_study_light_module_list_refresh ();
  }
#ifdef HAVE_CLOUD
  else if (command == task::rssfeedupdatechapter) {
    rss_logic_execute_update (parameter1, parameter2, filter::strings::convert_to_int (parameter3), filter::strings::convert_to_int (parameter4), parameter5, parameter6);
  }
#endif
#ifdef HAVE_CLIENT
  else if (command == task::producebiblestransferfile) {
    system_logic_produce_bibles_file (filter::strings::convert_to_int (parameter1));
  }
  else if (command == task::importbiblestransferfile) {
    system_logic_import_bibles_file (parameter1);
  }
  else if (command == task::producenotestransferfile) {
    system_logic_produce_notes_file (filter::strings::convert_to_int (parameter1));
  }
  else if (command == task::importnotestransferfile) {
    system_logic_import_notes_file (parameter1);
  }
  else if (command == task::produceresourcestransferfile) {
    system_logic_produce_resources_file (filter::strings::convert_to_int (parameter1));
  }
  else if (command == task::importresourcestransferfile) {
    system_logic_import_resources_file (parameter1);
  }
#endif
  else if (command == task::deletechanges) {
    changes_clear_notifications_user (parameter1, parameter2);
  }
  else if (command == task::clearcaches) {
    database::cache::file::trim (true);
  }
  else if (command == task::trimcaches) {
    database::cache::file::trim (false);
  }
  else if (command == task::export2nmt) {
    nmt_logic_export (parameter1, parameter2);
  }
  else if (command == task::createemptybible) {
    bible_logic::create_empty_bible (parameter1);
  }
  else if (command == task::importbibleimages) {
    images_logic_import_images (parameter1);
  }
  else if (command == task::getgoogleaccesstoken) {
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


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
#include <bible/import_run.h>
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
#include <export/bibledropbox.h>
#include <export/quickbible.h>
#include <manage/hyphenate.h>
#include <paratext/logic.h>
#include <resource/logic.h>
#include <sword/logic.h>
#include <bible/logic.h>
#include <client/logic.h>
#include <user/logic.h>
#include <rss/logic.h>
#include <system/logic.h>
#include <notes/logic.h>


mutex mutex_tasks; 
int running_tasks = 0;


void tasks_run_one (string filename)
{
  // Increase running tasks count.
  mutex_tasks.lock ();
  running_tasks++;
  mutex_tasks.unlock ();

  // Read the task from disk and erase the file.
  string path = filter_url_create_path (tasks_logic_folder (), filename);
  vector <string> lines = filter_string_explode (filter_url_file_get_contents (path), '\n');
  filter_url_unlink (path);
  
  // Interpret the task's command and its parameters, if any.
  string command = "";
  if (!lines.empty ()) {
    command = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter1;
  if (!lines.empty ()) {
    parameter1 = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter2;
  if (!lines.empty ()) {
    parameter2 = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter3;
  if (!lines.empty ()) {
    parameter3 = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter4;
  if (!lines.empty ()) {
    parameter4 = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter5;
  if (!lines.empty ()) {
    parameter5 = lines [0];
    lines.erase (lines.begin ());
  }
  string parameter6;
  if (!lines.empty ()) {
    parameter6 = lines [0];
    lines.erase (lines.begin ());
  }
  
  if (command == ROTATEJOURNAL) {
    Database_Logs::rotate ();
  }
  else if (command == RECEIVEEMAIL) {
    email_receive ();
  }
  else if (command == SENDEMAIL) {
    email_send ();
  }
  else if (command == REINDEXBIBLES) {
    search_reindex_bibles (convert_to_bool (parameter1));
  }
  else if (command == REINDEXNOTES) {
    search_reindex_notes ();
  }
  else if (command == CREATECSS) {
    styles_sheets_create_all_run ();
  }
  else if (command == IMPORTBIBLE) {
    bible_import_run (parameter1, parameter2, convert_to_int (parameter3), convert_to_int (parameter4));
  }
  else if (command == IMPORTRESOURCE) {
    bible_logic_import_resource (parameter1, parameter2);
  }
  else if (command == COMPAREUSFM) {
    compare_compare (parameter1, parameter2, convert_to_int (parameter3));
  }
  else if (command == MAINTAINDATABASE) {
    database_maintenance ();
  }
  else if (command == CLEANTMPFILES) {
    tmp_tmp ();
  }
  else if (command == LINKGITREPOSITORY) {
    collaboration_link (parameter1, convert_to_int (parameter2), parameter3);
  }
  else if (command == SENDRECEIVEBIBLES) {
    sendreceive_sendreceive (parameter1);
  }
  else if (command == SYNCNOTES) {
    sendreceive_notes ();
  }
  else if (command == SYNCBIBLES) {
    sendreceive_bibles ();
  }
  else if (command == SYNCSETTINGS) {
    sendreceive_settings ();
  }
  else if (command == SYNCCHANGES) {
    sendreceive_changes ();
  }
  else if (command == SYNCFILES) {
    sendreceive_files ();
  }
  else if (command == SYNCRESOURCES) {
    sendreceive_resources ();
  }
  else if (command == CLEANDEMO) {
    demo_clean_data ();
  }
  else if (command == CONVERTBIBLE2RESOURCE) {
    convert_bible_to_resource (parameter1);
  }
  else if (command == CONVERTRESOURCE2BIBLE) {
    convert_resource_to_bible (parameter1);
  }
  else if (command == PRINTRESOURCES) {
    resource_print_job (parameter1, parameter2, parameter3);
  }
  else if (command == NOTESSTATISTICS) {
    statistics_statistics ();
  }
  else if (command == GENERATECHANGES) {
    changes_modifications ();
  }
  else if (command == SPRINTBURNDOWN) {
    sprint_burndown ("", 0, 0);
  }
  else if (command == CHECKBIBLE) {
    checks_run (parameter1);
  }
  else if (command == EXPORTALL) {
    export_index ();
  }
  else if (command == EXPORTWEBMAIN) {
    export_web_book (parameter1, convert_to_int (parameter2), convert_to_bool (parameter3));
  }
  else if (command == EXPORTWEBINDEX) {
    export_web_index (parameter1, convert_to_bool (parameter2));
  }
  else if (command == EXPORTHTML) {
    export_html_book (parameter1, convert_to_int (parameter2), convert_to_bool (parameter3));
  }
  else if (command == EXPORTUSFM) {
    export_usfm (parameter1, convert_to_bool (parameter2));
  }
  else if (command == EXPORTTEXTUSFM) {
    export_text_usfm_book (parameter1, convert_to_int (parameter2), convert_to_bool (parameter3));
  }
  else if (command == EXPORTODT) {
    export_odt_book (parameter1, convert_to_int (parameter2), convert_to_bool (parameter3));
  }
  else if (command == EXPORTINFO) {
    export_info (parameter1, convert_to_bool (parameter2));
  }
  else if (command == EXPORTESWORD) {
    export_esword (parameter1, convert_to_bool (parameter2));
  }
  else if (command == EXPORTONLINEBIBLE) {
    export_onlinebible (parameter1, convert_to_bool (parameter2));
  }
  else if (command == EXPORTQUICKBIBLE) {
    export_quickbible (parameter1, convert_to_bool (parameter2));
  }
  else if (command == HYPHENATE) {
    manage_hyphenate (parameter1, parameter2);
  }
  else if (command == SETUPPARATEXT) {
    Paratext_Logic::setup (parameter1, parameter2);
  }
  else if (command == SYNCPARATEXT) {
    Paratext_Logic::synchronize ();
  }
  else if (command == SUBMITBIBLEDROPBOX) {
    export_bibledropbox (parameter1, parameter2);
  }
  else if (command == IMPORTIMAGES) {
    resource_logic_import_images (parameter1, parameter2);
  }
  else if (command == REFRESHSWORDMODULES) {
    sword_logic_refresh_module_list ();
  }
  else if (command == INSTALLSWORDMODULE) {
    sword_logic_run_scheduled_module_install (parameter1, parameter2);
  }
  else if (command == UPDATESWORDMODULES) {
    sword_logic_update_installed_modules ();
  }
  else if (command == LISTUSFMRESOURCES) {
    client_logic_usfm_resources_update ();
  }
  else if (command == CREATESAMPLEBIBLE) {
    demo_create_sample_bible ();
  }
  else if (command == CACHERESOURCES) {
    resource_logic_create_cache ();
  }
  else if (command == NOTIFYSOFTWAREUPDATES) {
    user_logic_software_updates_notify ();
  }
  else if (command == REFRESHWEBRESOURCES) {
    resource_logic_bible_gateway_module_list_refresh ();
    resource_logic_study_light_module_list_refresh ();
  }
#ifdef HAVE_CLOUD
  else if (command == RSSFEEDUPDATECHAPTER) {
    rss_logic_execute_update (parameter1, parameter2, convert_to_int (parameter3), convert_to_int (parameter4), parameter5, parameter6);
  }
#endif
#ifdef HAVE_CLIENT
  else if (command == PRODUCEBIBLESTRANSFERFILE) {
    system_logic_produce_bibles_file (convert_to_int (parameter1));
  }
  else if (command == IMPORTBIBLESTRANSFERFILE) {
    system_logic_import_bibles_file (parameter1);
  }
  else if (command == PRODUCERENOTESTRANSFERFILE) {
    system_logic_produce_notes_file (convert_to_int (parameter1));
  }
  else if (command == IMPORTNOTESTRANSFERFILE) {
    system_logic_import_notes_file (parameter1);
  }
  else if (command == PRODUCERESOURCESTRANSFERFILE) {
    system_logic_produce_resources_file (convert_to_int (parameter1));
  }
  else if (command == IMPORTRESOURCESTRANSFERFILE) {
    system_logic_import_resources_file (parameter1);
  }
#endif
  else if (command == CONVERTCONSULTATIONNOTES) {
    notes_logic_gradual_upgrader ();
  }
  else {
    Database_Logs::log ("Unknown task: " + command);
  }

  // Decrease running tasks count.
  mutex_tasks.lock ();
  running_tasks--;
  mutex_tasks.unlock ();
}


void tasks_run_check ()
{
  // Don't run more than so many tasks.
  if (tasks_run_active_count () >= MAX_PARALLEL_TASKS) return;
  // Get and start first available task.
  vector <string> tasks = filter_url_scandir (tasks_logic_folder ());
  if (tasks.empty ()) return;
  thread task_thread = thread (tasks_run_one, tasks [0]);
  // Detach the thread to the thread continues to run independently,
  // when the thread object goes out of scope, and no memory is leaked this way.
  task_thread.detach ();
}


int tasks_run_active_count ()
{
  int taskscount = 0;
  mutex_tasks.lock ();
  taskscount = running_tasks;
  mutex_tasks.unlock ();
  return taskscount;
}


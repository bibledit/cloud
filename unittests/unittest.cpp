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


#include <unittests/unittest.h>
#include <unittests/utilities.h>
#include <config/libraries.h>
#include <library/bibledit.h>
#include <database/config/user.h>
#include <database/styles.h>
#include <database/books.h>
#include <database/state.h>
#include <config/globals.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/md5.h>
#include <filter/usfm.h>
#include <session/logic.h>
#include <unittests/sqlite.h>
#include <unittests/session.h>
#include <unittests/folders.h>
#include <unittests/flate.h>
#include <unittests/checksum.h>
#include <unittests/bibles.h>
#include <unittests/html2usfm.h>
#include <unittests/usfm2html.h>
#include <unittests/usfm2html2usfm.h>
#include <unittests/workspaces.h>
#include <unittests/client.h>
#include <unittests/sentences.h>
#include <unittests/versification.h>
#include <unittests/usfm.h>
#include <unittests/verses.h>
#include <unittests/pairs.h>
#include <unittests/hyphenate.h>
#include <unittests/search.h>
#include <unittests/json.h>
#include <unittests/related.h>
#include <unittests/editone.h>
#include <unittests/http.h>
#include <unittests/memory.h>
#include <unittests/tasks.h>
#include <unittests/biblegateway.h>
#include <unittests/rss.h>
#include <unittests/space.h>
#include <unittests/roles.h>
#include <unittests/md5.h>
#include <unittests/string.h>
#include <unittests/users.h>
#include <unittests/date.h>
#include <unittests/export.h>
#include <unittests/html.h>
#include <unittests/archive.h>
#include <unittests/odf.h>
#include <unittests/text.h>
#include <unittests/url.h>
#include <unittests/passage.h>
#include <unittests/styles.h>
#include <unittests/diff.h>
#include <unittests/git.h>
#include <unittests/ipc.h>
#include <unittests/shell.h>
#include <unittests/dev.h>
#include <unittests/sample.h>
#include <unittests/config.h>
#include <unittests/log.h>
#include <unittests/books.h>
#include <unittests/check.h>
#include <unittests/localization.h>
#include <unittests/confirm.h>
#include <unittests/jobs.h>
#include <unittests/kjv.h>
#include <unittests/oshb.h>
#include <unittests/sblgnt.h>
#include <unittests/sprint.h>
#include <unittests/mail.h>
#include <unittests/navigation.h>
#include <unittests/resources.h>
#include <unittests/notes.h>
#include <unittests/modifications.h>
#include <unittests/volatile.h>
#include <unittests/state.h>
#include <unittests/strong.h>
#include <unittests/morphgnt.h>
#include <unittests/etcbc4.h>
#include <unittests/lexicon.h>
#include <unittests/cache.h>
#include <unittests/login.h>
#include <unittests/privileges.h>
#include <unittests/statistics.h>
#include <unittests/webview.h>
#include <unittests/javascript.h>
#include <unittests/french.h>
#include <unittests/merge.h>


int main (int argc, char **argv) 
{
  (void) argc;
  (void) argv;

  cout << "Running unittests" << endl;

  // Directory where the unit tests will run.
  testing_directory = "/tmp/bibledit-unittests";  
  filter_url_mkdir (testing_directory);
  refresh_sandbox (true);
  config_globals_document_root = testing_directory;

  // Number of failed unit tests.
  error_count = 0;

  // Flag for unit tests.
  config_globals_unit_testing = true;

  // The next line is a signature for automated unit testing: Do not change it.
  // Automated Unit Tests Start Removing Code Here.
  
  refresh_sandbox (true);
  test_merge ();
  test_diff ();
  test_usfm ();
  test_archive ();
  exit (0);
  
  // The next line is a signature for automated unit testing: Do not change it.
  // Automated Unit Tests End Removing Code Here.
  
  // Run the tests.
  test_database_config_general ();
  test_database_config_bible ();
  test_database_config_user ();
  test_sqlite ();
  test_database_logs ();
  test_database_users ();
  test_session ();
  test_folders ();
  test_flate ();
  test_database_bibles ();
  test_database_books ();
  test_database_bibleactions ();
  test_database_check ();
  test_database_localization ();
  test_database_confirm ();
  test_database_ipc ();
  test_database_jobs ();
  test_database_kjv ();
  test_database_oshb ();
  test_database_sblgnt ();
  test_database_sprint ();
  test_database_mail ();
  test_database_navigation ();
  test_database_resources ();
  test_database_usfmresources ();
  test_database_mappings ();
  test_database_noteactions ();
  test_database_versifications ();
  test_database_modifications_user ();
  test_database_modifications_team ();
  test_database_modifications_notifications ();
  test_database_notes ();
  test_database_volatile ();
  test_database_state ();
  test_database_imageresources ();
  test_checksum ();
  test_bibles ();
  test_html2usfm ();
  test_usfm2html ();
  test_usfm2html2usfm ();
  test_workspaces ();
  test_verses ();
  test_client ();
  test_sentences ();
  test_versification ();
  test_usfm ();
  test_pairs ();
  test_hyphenate ();
  test_database_noteassignment ();
  test_database_strong ();
  test_database_morphgnt ();
  test_database_etcbc4 ();
  test_database_hebrewlexicon ();
  test_database_cache ();
  test_search ();
  test_database_login ();
  test_database_privileges ();
  test_json ();
  test_database_git ();
  test_database_userresources ();
  test_related ();
  test_editone_logic ();
  test_http ();
  test_memory ();
  test_database_statistics ();
  test_tasks_logic ();
  test_biblegateway ();
  test_rss_feed ();
  test_space ();
  test_roles ();
  test_md5 ();
  test_string ();
  test_users ();
  test_date ();
  test_export ();
  test_html ();
  test_archive ();
  test_odf ();
  test_text ();
  test_url ();
  test_passage ();
  test_styles ();
  test_diff ();
  test_git ();
  test_ipc ();
  test_shell ();
  test_database_sample ();
  test_filter_webview ();
  test_javascript ();
  test_filter_mail ();
  test_french ();
  test_merge ();

  // Output possible journal entries.
  refresh_sandbox (true);

  // Test results.
  if (error_count == 0) cout << "All tests passed" << endl;
  else cout << "Number of failures: " << error_count << endl;

  // Ready.
  return (error_count == 0) ? 0 : 1;
}



/*
Copyright (©) 2003-2018 Teus Benschop.

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


#include <unittests/memory.h>
#include <unittests/utilities.h>
#include <filter/memory.h>
#include <database/state.h>
#include <demo/logic.h>
#include <search/rebibles.h>
#include <search/renotes.h>
#include <database/notes.h>
#include <database/users.h>
#include <database/login.h>
#include <webserver/request.h>
#include <database/modifications.h>
#include <filter/string.h>
#include <changes/modifications.h>
#include <database/config/bible.h>
#include <database/versifications.h>
#include <checks/run.h>
#include <export/usfm.h>
#include <export/textusfm.h>
#include <export/odt.h>
#include <export/esword.h>
#include <export/onlinebible.h>
#include <sword/logic.h>


bool test_memory_run = false;
uint64_t max_memory_usage = 0;
void test_memory_record ()
{
  uint64_t maximum = 0;
  do {
    uint64_t total = filter_memory_total_usage ();
    if (total > maximum) maximum = total;
    this_thread::sleep_for (chrono::microseconds (10));
  } while (test_memory_run);
  max_memory_usage = maximum;
}


void test_memory ()
{
  return;
  
  // Measure maximum memory usage of tasks that normally run in the background.
  trace_unit_tests (__func__);

  // Creating search index for one Bible.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    search_reindex_bibles (true);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " search_reindex_bibles" << endl;
  }

  // Creating search index consultation notes.
  refresh_sandbox (false);
  {
    Database_State::create ();
    Database_Notes database_notes (NULL);
    database_notes.create_v12 ();
    for (int i = 0; i < 100; i++) {
      database_notes.store_new_note_v2 ("bible", i, i, i, "summary", "contents", true);
    }
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    search_reindex_notes ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " search_reindex_notes" << endl;
  }
  
  // Generating change notifications.
  refresh_sandbox (false);
  {
    Database_State::create ();
    Database_Users database_users;
    database_users.create ();
    Database_Login::create ();
    demo_create_sample_bible ();
    Webserver_Request request;
    request.session_logic ()->setUsername ("admin");
    request.database_config_user ()->setGenerateChangeNotifications (true);
    request.database_config_user ()->setUserChangesNotification (true);
    request.database_config_user ()->setUserChangesNotificationsOnline (true);
    request.database_config_user ()->setContributorChangesNotificationsOnline (true);
    Database_Bibles database_bibles;
    Database_Modifications database_modifications;
    string bible = demo_sample_bible_name ();
    for (int book = 1; book <= 1; book++) {
      for (int chapter = 1; chapter <= 1; chapter++) {
        string usfm = database_bibles.getChapter (bible, book, chapter);
        usfm = filter_string_str_replace ("the", "THE", usfm);
        database_modifications.storeTeamDiff (bible, book, chapter);
        database_bibles.storeChapter (bible, book, chapter, usfm);
      }
    }
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    changes_modifications ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    uint64_t after_usage = filter_memory_total_usage ();
    cout << max_memory_usage - basic_usage << " changes_modifications, leakage " << after_usage - basic_usage << endl;
  }

  // Running checks.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    Database_Config_Bible::setCheckDoubleSpacesUsfm (bible, true);
    Database_Config_Bible::setCheckFullStopInHeadings (bible, true);
    Database_Config_Bible::setCheckSpaceBeforePunctuation (bible, true);
    Database_Config_Bible::setCheckSentenceStructure (bible, true);
    Database_Config_Bible::setCheckParagraphStructure (bible, true);
    Database_Config_Bible::setCheckBooksVersification (bible, true);
    Database_Config_Bible::setCheckChaptesVersesVersification (bible, true);
    Database_Config_Bible::setCheckWellFormedUsfm (bible, true);
    Database_Config_Bible::setCheckMissingPunctuationEndVerse (bible, true);
    Database_Config_Bible::setCheckPatterns (bible, true);
    Database_Config_Bible::setCheckMatchingPairs (bible, true);
    Database_Check database_check;
    database_check.create ();
    Database_Users database_users;
    database_users.create ();
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    checks_run (bible);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " checks_run" << endl;
  }

  // Exporting Bible to USFM.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    export_usfm (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " export_usfm" << endl;
  }

  // Exporting Bible book to USFM and plain text.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    export_text_usfm_book (bible, 1, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " export_text_usfm_book" << endl;
  }

  // Exporting Bible book to Open Document.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    export_odt_book (bible, 1, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " export_odt_book" << endl;
  }

  // Exporting Bible to eSword.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    export_esword (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " export_esword" << endl;
  }

  // Exporting Bible to Online Bible.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    string bible = demo_sample_bible_name ();
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    export_onlinebible (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " export_onlinebible" << endl;
  }

  // Refreshing SWORD module list.
  refresh_sandbox (false);
  {
    thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new thread (test_memory_record);
    sword_logic_refresh_module_list ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    cout << max_memory_usage - basic_usage << " sword_logic_refresh_module_list" << endl;
  }
  
  int available = filter_memory_percentage_available ();
  (void) available;

  // Done.
  refresh_sandbox (false);
  
  /*
   
   First iteration displaying memory usage in bytes per function:
   40960 search_reindex_bibles
   8192 search_reindex_notes
   113590272 changes_modifications
   8364032 checks_run
   40960 export_usfm
   339968 export_text_usfm_book
   929792 export_odt_book
   30154752 export_esword
   15814656 export_onlinebible
   20480 sword_logic_refresh_module_list
   This is on macOS, but valgrind on Linux gives different values, lower values.
  
  */
}

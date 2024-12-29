/*
Copyright (©) 2003-2025 Teus Benschop.

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
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
#include <database/check.h>
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
  uint64_t maximum {0};
  do {
    uint64_t total = filter_memory_total_usage ();
    if (total > maximum) maximum = total;
    std::this_thread::sleep_for (std::chrono::microseconds (10));
  } while (test_memory_run);
  max_memory_usage = maximum;
}


TEST (DISABLED_memory, basic)
{
  // Measure maximum memory usage of tasks that normally run in the background.

  // Creating search index for one Bible.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    search_reindex_bibles (true);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " search_reindex_bibles" << std::endl;
  }

  // Creating search index consultation notes.
  refresh_sandbox (false);
  {
    Database_State::create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    for (int i = 0; i < 100; i++) {
      database_notes.store_new_note ("bible", i, i, i, "summary", "contents", true);
    }
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    search_reindex_notes ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " search_reindex_notes" << std::endl;
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
    request.session_logic ()->set_username (session_admin_credentials ());
    request.database_config_user ()->setGenerateChangeNotifications (true);
    request.database_config_user ()->setUserChangesNotification (true);
    request.database_config_user ()->setUserChangesNotificationsOnline (true);
    request.database_config_user ()->setContributorChangesNotificationsOnline (true);
    std::string bible = demo_sample_bible_name ();
    for (int book = 1; book <= 1; book++) {
      for (int chapter = 1; chapter <= 1; chapter++) {
        std::string usfm = database::bibles::get_chapter (bible, book, chapter);
        usfm = filter::strings::replace ("the", "THE", usfm);
        database::modifications::storeTeamDiff (bible, book, chapter);
        database::bibles::store_chapter (bible, book, chapter, usfm);
      }
    }
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    changes_modifications ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    uint64_t after_usage = filter_memory_total_usage ();
    std::cout << max_memory_usage - basic_usage << " changes_modifications, leakage " << after_usage - basic_usage << std::endl;
  }

  // Running checks.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    database::config::bible::set_check_double_spaces_usfm (bible, true);
    database::config::bible::set_check_full_stop_in_headings (bible, true);
    database::config::bible::set_check_space_before_punctuation (bible, true);
    database::config::bible::set_check_sentence_structure (bible, true);
    database::config::bible::set_check_paragraph_structure (bible, true);
    database::config::bible::set_check_books_versification (bible, true);
    database::config::bible::set_check_chaptes_verses_versification (bible, true);
    database::config::bible::set_check_well_formed_usfm (bible, true);
    database::config::bible::set_check_missing_punctuation_end_verse (bible, true);
    database::config::bible::set_check_patterns (bible, true);
    database::config::bible::set_check_matching_pairs (bible, true);
    database::check::create ();
    Database_Users database_users;
    database_users.create ();
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    checks_run (bible);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " checks_run" << std::endl;
  }

  // Exporting Bible to USFM.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    export_usfm (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " export_usfm" << std::endl;
  }

  // Exporting Bible book to USFM and plain text.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    export_text_usfm_book (bible, 1, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " export_text_usfm_book" << std::endl;
  }

  // Exporting Bible book to Open Document.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    export_odt_book (bible, 1, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " export_odt_book" << std::endl;
  }

  // Exporting Bible to eSword.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    export_esword (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " export_esword" << std::endl;
  }

  // Exporting Bible to Online Bible.
  refresh_sandbox (false);
  {
    Database_State::create ();
    demo_create_sample_bible ();
    std::string bible = demo_sample_bible_name ();
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    export_onlinebible (bible, false);
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " export_onlinebible" << std::endl;
  }

  // Refreshing SWORD module list.
  refresh_sandbox (false);
  {
    std::thread * recorder = nullptr;
    uint64_t basic_usage = filter_memory_total_usage ();
    test_memory_run = true;
    recorder = new std::thread (test_memory_record);
    sword_logic_refresh_module_list ();
    test_memory_run = false;
    recorder->join ();
    delete recorder;
    std::cout << max_memory_usage - basic_usage << " sword_logic_refresh_module_list" << std::endl;
  }
  
  [[maybe_unused]] auto available = filter_memory_percentage_available ();

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

#endif


/*
Copyright (©) 2003-2023 Teus Benschop.

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
#include <unittests/unittest.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#endif
#include <unittests/utilities.h> // Todo lots of these out.
#include <library/bibledit.h>
#include <library/locks.h>
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
#include <unittests/paratext.h>
#include <unittests/nmt.h>
#include <unittests/html2format.h>
#include <unittests/studylight.h>
#include <unittests/gbs.h>
#include <unittests/bibleimages.h>
#include <unittests/image.h>
#include <unittests/easyenglishbible.h>
using namespace std;


int main ([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
  // Directory where the unit tests will run.
  testing_directory = "/tmp/bibledit-unittests";  
  filter_url_mkdir (testing_directory);
  refresh_sandbox (true);
  config_globals_document_root = testing_directory;

  // Initialize SSL/TLS (after webroot has been set).
  filter_url_ssl_tls_initialize ();

  // Number of failed unit tests.
  error_count = 0;

  // Flag for unit tests.
  config_globals_unit_testing = true;

  refresh_sandbox (false);
  
  int gtest_result {0};
#ifdef HAVE_GTEST
  ::testing::InitGoogleTest(&argc, argv);
  gtest_result = RUN_ALL_TESTS();
#endif

  // Output possible journal entries.
  refresh_sandbox (false);

  // Ready.
  return gtest_result;
}


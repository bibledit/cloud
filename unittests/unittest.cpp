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
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#endif
#include <unittests/utilities.h>
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


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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <rss/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/bibles.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>


TEST (rss, feed)
{
#ifdef HAVE_CLOUD

  refresh_sandbox (false);
  
  Database_State::create ();
  std::string bible = "bible";
  database::bibles::create_bible (bible);
  
  std::string path = rss_logic_xml_path ();
  database::config::general::set_site_url ("http://localhost:8080/");
  
  // Enable the Bible to send its changes to the RSS feed.
  database::config::bible::set_send_changes_to_rss (bible, true);
  rss_logic_feed_on_off ();
  
  // Write two items.
  rss_logic_update_xml ({ "titleone", "titletwo" }, { "authorone", "authortwo" }, { "description one", "description two"} );
  EXPECT_EQ (849, filter_url_filesize (path));

  // Disable the Bible: Should remove the file.
  database::config::bible::set_send_changes_to_rss (bible, false);
  rss_logic_feed_on_off ();
  EXPECT_EQ (0, filter_url_filesize (path));

  // Add many entries and clipping their number.
  database::config::bible::set_send_changes_to_rss (bible, true);
  rss_logic_feed_on_off ();
  std::vector <std::string> titles;
  std::vector <std::string> authors;
  std::vector <std::string> descriptions;
  for (size_t i = 0; i < 300; i++) {
    titles.push_back ("title " + std::to_string (i));
    authors.push_back ("author " + std::to_string (i));
    descriptions.push_back ("description " + std::to_string (i));
  }
  rss_logic_update_xml (titles, authors, descriptions);
  EXPECT_EQ (25693, filter_url_filesize (path));

#endif
}

#endif


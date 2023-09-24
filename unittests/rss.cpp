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
#include <unittests/utilities.h>
#include <rss/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/bibles.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
using namespace std;


TEST (rss, feed)
{
#ifdef HAVE_CLOUD

  refresh_sandbox (false);
  
  Database_State::create ();
  Database_Bibles database_bibles;
  string bible = "bible";
  database_bibles.create_bible (bible);
  
  string path = rss_logic_xml_path ();
  Database_Config_General::setSiteURL ("http://localhost:8080/");
  
  // Enable the Bible to send its changes to the RSS feed.
  Database_Config_Bible::setSendChangesToRSS (bible, true);
  rss_logic_feed_on_off ();
  
  // Write two items.
  rss_logic_update_xml ({ "titleone", "titletwo" }, { "authorone", "authortwo" }, { "description one", "description two"} );
  EXPECT_EQ (849, filter_url_filesize (path));

  // Disable the Bible: Should remove the file.
  Database_Config_Bible::setSendChangesToRSS (bible, false);
  rss_logic_feed_on_off ();
  EXPECT_EQ (0, filter_url_filesize (path));

  // Add many entries and clipping their number.
  Database_Config_Bible::setSendChangesToRSS (bible, true);
  rss_logic_feed_on_off ();
  vector <string> titles;
  vector <string> authors;
  vector <string> descriptions;
  for (size_t i = 0; i < 300; i++) {
    titles.push_back ("title " + filter::strings::convert_to_string (i));
    authors.push_back ("author " + filter::strings::convert_to_string (i));
    descriptions.push_back ("description " + filter::strings::convert_to_string (i));
  }
  rss_logic_update_xml (titles, authors, descriptions);
  EXPECT_EQ (25693, filter_url_filesize (path));

#endif
}

#endif


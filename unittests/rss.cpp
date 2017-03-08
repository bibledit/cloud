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


#include <unittests/rss.h>
#include <unittests/utilities.h>
#include <rss/logic.h>
#include <database/config/general.h>
#include <filter/url.h>
#include <filter/string.h>


void test_rss_feed ()
{
#ifdef HAVE_CLOUD

  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  string path = rss_logic_xml_path ();
  Database_Config_General::setSiteURL ("http://localhost:8080/");
  
  // Write two items.
  Database_Config_General::setMaxRssFeedItems (10);
  rss_logic_update_xml ({ "titleone", "titletwo" }, { "authorone", "authortwo" }, { "description one", "description two"} );
  evaluate (__LINE__, __func__, 848, filter_url_filesize (path));
  
  // Set maximum number of items to 0: Should remove the file.
  Database_Config_General::setMaxRssFeedItems (0);
  rss_logic_update_xml ({ "titleone", "titletwo" }, { "authorone", "authortwo" }, { "description one", "description two"} );
  evaluate (__LINE__, __func__, 0, filter_url_filesize (path));

  // Add many entries and clipping their number.
  Database_Config_General::setMaxRssFeedItems (10);
  vector <string> titles;
  vector <string> authors;
  vector <string> descriptions;
  for (size_t i = 0; i < 100; i++) {
    titles.push_back ("title " + convert_to_string (i));
    authors.push_back ("author " + convert_to_string (i));
    descriptions.push_back ("description " + convert_to_string (i));
  }
  rss_logic_update_xml (titles, authors, descriptions);
  evaluate (__LINE__, __func__, 2598, filter_url_filesize (path));

#endif
}

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


#include <edit/logic.h>
#include <filter/string.h>
#include <database/bibles.h>
#include <database/temporal.h>
#include <webserver/request.h>


static std::string usfm_key (const std::string& bible, const int book, const int chapter,
                             const std::string& editor)
{
  std::string key {"usfm"};
  key.append (" ");
  key.append (bible);
  key.append (" ");
  key.append (filter::strings::fill (std::to_string (book), 2, '0'));
  key.append (" ");
  key.append (filter::strings::fill (std::to_string (chapter), 3, '0'));
  key.append (" ");
  key.append (editor);
  return key;
}


void store_loaded_usfm (Webserver_Request& webserver_request,
                        const std::string& bible, const int book, const int chapter,
                        const std::string& editor)
{
  const int userid = filter::strings::user_identifier (webserver_request);
  const std::string key = usfm_key (bible, book, chapter, editor);
  const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  database::temporal::set_value (userid, key, usfm);
}


std::string get_loaded_usfm (Webserver_Request& webserver_request,
                             const std::string& bible, const int book, const int chapter,
                             const std::string& editor)
{
  const int userid = filter::strings::user_identifier (webserver_request);
  const std::string key = usfm_key (bible, book, chapter, editor);
  const std::string usfm = database::temporal::get_value (userid, key);
  return usfm;
}

/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <database/volatile.h>
#include <webserver/request.h>


std::string edit2_logic_volatile_key (std::string bible, int book, int chapter, std::string editor)
{
  std::string key;
  key.append (bible);
  key.append (" ");
  key.append (filter::strings::fill (std::to_string (book), 2, '0'));
  key.append (" ");
  key.append (filter::strings::fill (std::to_string (chapter), 3, '0'));
  key.append (" ");
  key.append (editor);
  return key;
}


void storeLoadedUsfm2 (Webserver_Request& webserver_request, std::string bible, int book, int chapter, std::string editor, [[maybe_unused]] const char * message)
{
  const int userid = filter::strings::user_identifier (webserver_request);
  
  const std::string key = edit2_logic_volatile_key (bible, book, chapter, editor);
  
  const std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  
  database::volatile_::set_value (userid, key, usfm);
}


std::string getLoadedUsfm2 (Webserver_Request& webserver_request, std::string bible, int book, int chapter, std::string editor)
{
  const int userid = filter::strings::user_identifier (webserver_request);
  
  const std::string key = edit2_logic_volatile_key (bible, book, chapter, editor);
  
  const std::string usfm = database::volatile_::get_value (userid, key);
  
  return usfm;
}

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


#include <edit/logic.h>
#include <filter/string.h>
#include <database/bibles.h>
#include <database/volatile.h>
#include <webserver/request.h>
using namespace std;


string edit2_logic_volatile_key (string bible, int book, int chapter, string editor)
{
  string key;
  key.append (bible);
  key.append (" ");
  key.append (filter::strings::fill (filter::strings::convert_to_string (book), 2, '0'));
  key.append (" ");
  key.append (filter::strings::fill (filter::strings::convert_to_string (chapter), 3, '0'));
  key.append (" ");
  key.append (editor);
  return key;
}


void storeLoadedUsfm2 (void * webserver_request, string bible, int book, int chapter, string editor, [[maybe_unused]] const char * message)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  int userid = filter::strings::user_identifier (webserver_request);
  
  string key = edit2_logic_volatile_key (bible, book, chapter, editor);
  
  string usfm = request->database_bibles()->get_chapter (bible, book, chapter);
  
  Database_Volatile::setValue (userid, key, usfm);
}


string getLoadedUsfm2 (void * webserver_request, string bible, int book, int chapter, string editor)
{
  int userid = filter::strings::user_identifier (webserver_request);
  
  string key = edit2_logic_volatile_key (bible, book, chapter, editor);
  
  string usfm = Database_Volatile::getValue (userid, key);
  
  return usfm;
}

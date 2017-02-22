/*
Copyright (©) 2003-2016 Teus Benschop.

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


string edit_logic_volatile_key (string bible, int book, int chapter, string editor)
{
  string key;
  key.append (bible);
  key.append (" ");
  key.append (filter_string_fill (convert_to_string (book), 2, '0'));
  key.append (" ");
  key.append (filter_string_fill (convert_to_string (chapter), 3, '0'));
  key.append (" ");
  key.append (editor);
  return key;
}


void storeLoadedUsfm (void * webserver_request, string bible, int book, int chapter, string editor)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  int userid = filter_string_user_identifier (webserver_request);
  
  string key = edit_logic_volatile_key (bible, book, chapter, editor);
  
  string usfm = request->database_bibles ()->getChapter (bible, book, chapter);
  
  Database_Volatile::setValue (userid, key, usfm);
}


string getLoadedUsfm (void * webserver_request, string bible, int book, int chapter, string editor)
{
  int userid = filter_string_user_identifier (webserver_request);
  
  string key = edit_logic_volatile_key (bible, book, chapter, editor);
  
  string usfm = Database_Volatile::getValue (userid, key);
  
  return usfm;
}

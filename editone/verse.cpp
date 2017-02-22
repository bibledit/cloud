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


#include <editone/verse.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <access/bible.h>


string editone_verse_url ()
{
  return "editone/verse";
}


bool editone_verse_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string editone_verse (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;


  // Only act if a verse was found
  string sverse = request->query ["verse"];
  if (!sverse.empty ()) {
    
    
    // Only update navigation in case the verse changed.
    // This avoids unnecessary focus operations in the clients.
    int iverse = convert_to_int (sverse);
    if (iverse != Ipc_Focus::getVerse (request)) {
      int book = Ipc_Focus::getBook (request);
      int chapter = Ipc_Focus::getChapter (request);
      Ipc_Focus::set (request, book, chapter, iverse);
    }
    
    
  }
  
  
  return "";
}

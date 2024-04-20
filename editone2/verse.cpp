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


#include <editone2/verse.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <access/bible.h>


std::string editone2_verse_url ()
{
  return "editone2/verse";
}


bool editone2_verse_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editone2_verse (Webserver_Request& webserver_request)
{
  // Only act if a verse was found
  std::string sverse = webserver_request.query ["verse"];
  if (!sverse.empty ()) {
    
    
    // Only update navigation in case the verse changed.
    // This avoids unnecessary focus operations in the clients.
    int iverse = filter::strings::convert_to_int (sverse);
    if (iverse != Ipc_Focus::getVerse (webserver_request)) {
      int book = Ipc_Focus::getBook (webserver_request);
      int chapter = Ipc_Focus::getChapter (webserver_request);
      Ipc_Focus::set (webserver_request, book, chapter, iverse);
    }
  }
  
  
  return std::string();
}

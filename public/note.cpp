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


#include <public/note.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <public/index.h>


string public_note_url ()
{
  return "public/note";
}


bool public_note_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_note (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Note"), request);

  
  // After adding a comment to a note, when doing nothing for several seconds,
  // the browser then returns to the list of public notes.
  if (request->query.count ("temporal")) {
    header.refresh (5, "index");
  }

  
  page += header.run ();
  Assets_View view;
  string success;

  
  int id = convert_to_int (request->query ["id"]);
  view.set_variable ("id", convert_to_string (id));
  
  
  if (database_notes.getPublic (id)) {
    string summary = database_notes.getSummary (id);
    view.set_variable ("summary", summary);

    string content = database_notes.getContents (id);
    view.set_variable ("content", content);
  }

  
  page += view.render ("public", "note");
  page += Assets_Page::footer ();
  return page;
}

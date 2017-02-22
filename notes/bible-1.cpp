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


#include <notes/bible-1.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/actions.h>


string notes_bible_1_url ()
{
  return "notes/bible-1";
}


bool notes_bible_1_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string notes_bible_1 (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Bibles"), request);
  page += header.run ();
  Assets_View view;
  string success, error;
  
  
  int id = convert_to_int (request->query ["id"]);
  view.set_variable ("id", convert_to_string (id));
  
  
  if (request->query.count ("bible")) {
    string bible = request->query["bible"];
    if (bible == notes_logic.generalBibleName ()) bible = "";
    notes_logic.setBible (id, bible);
    redirect_browser (request, notes_actions_url () + "?id=" + convert_to_string (id));
    return "";
  }
  
  
  string bibleblock;
  vector <string> bibles = access_bible_bibles (webserver_request);
  bibles.push_back (notes_logic.generalBibleName ());
  for (auto & bible : bibles) {
    bibleblock.append ("<li><a href=\"bible-1?id=" + convert_to_string (id) + "&bible=" + bible + "\">" + bible + "</a></li>\n");
  }
  view.set_variable ("bibleblock", bibleblock);
  

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "bible-1");
  page += Assets_Page::footer ();
  return page;
}

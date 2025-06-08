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
#include <read/index.h>
#include <config/logic.h>


std::string public_note_url ()
{
  return "public/note";
}


bool public_note_acl (Webserver_Request& webserver_request)
{
  if (config::logic::create_no_accounts()) return false;
  return roles::access_control (webserver_request, roles::guest);
}


std::string public_note (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  
  
  std::string page {};
  Assets_Header header = Assets_Header (translate("Note"), webserver_request);

  
  // After adding a comment to a note, when doing nothing for several seconds,
  // the browser then returns to the list of public notes.
  if (webserver_request.query.count ("temporal")) {
    header.refresh (5, "index");
  }

  
  page += header.run ();
  Assets_View view;
  std::string success;

  
  const int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  view.set_variable ("id", std::to_string (id));
  
  
  if (database_notes.get_public (id)) {
    const std::string summary = database_notes.get_summary (id);
    view.set_variable ("summary", summary);

    const std::string content = database_notes.get_contents (id);
    view.set_variable ("content", content);
  }


  const std::string url_to_index = "index";
  view.set_variable ("url_to_index", url_to_index);

  
  page += view.render ("public", "note");
  page += assets_page::footer ();
  return page;
}

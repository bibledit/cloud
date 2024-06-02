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


#include <notes/summary.h>
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
#include <notes/note.h>


std::string notes_summary_url ()
{
  return "notes/summary";
}


bool notes_summary_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string notes_summary (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Note summary"), webserver_request);
  page += header.run ();
  Assets_View view;


  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  view.set_variable ("id", std::to_string (id));
  
  
  if (webserver_request.post.count ("submit")) {
    std::string summary = webserver_request.post["entry"];
    notes_logic.set_summary (id, summary);
    redirect_browser (webserver_request, notes_note_url () + "?id=" + std::to_string (id));
    return std::string();
  }
  
  
  std::string summary = database_notes.get_summary (id);
  view.set_variable ("summary", filter::strings::escape_special_xml_characters (summary));

  
  page += view.render ("notes", "summary");
  
  page += assets_page::footer ();
  
  return page;
}

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


#include <notes/severity-1.h>
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


std::string notes_severity_1_url ()
{
  return "notes/severity-1";
}


bool notes_severity_1_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string notes_severity_1 (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Severity"), webserver_request);
  page += header.run ();
  Assets_View view;
  std::string success, error;
  
  
  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  view.set_variable ("id", std::to_string (id));
  
  
  if (webserver_request.query.count ("severity")) {
    int severity = filter::strings::convert_to_int (webserver_request.query["severity"]);
    notes_logic.setRawSeverity (id, severity);
    redirect_browser (webserver_request, notes_actions_url () + "?id=" + std::to_string (id));
    return std::string();
  }
  
  
  std::stringstream severityblock;
  std::vector <Database_Notes_Text> severities = database_notes.get_possible_severities ();
  for (auto & severity : severities) {
    severityblock << "<li><a href=" << std::quoted ("severity-1?id=" + std::to_string (id) + "&severity=" + severity.raw) << ">" << severity.localized << "</a></li>" << std::endl;
  }
  view.set_variable ("severityblock", severityblock.str());
  
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("notes", "severity-1");
  page += assets_page::footer ();
  return page;
}

/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <notes/click.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/notes.h>
#include <notes/index.h>
#include <dialog/yes.h>
#include <trash/handler.h>


std::string notes_click_url ()
{
  return "notes/click";
}


bool notes_click_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


// This function is called from click.js.
std::string notes_click (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);
  
  
  if (webserver_request.query.count ("open")) {
    std::string open = webserver_request.query ["open"];
    open = filter_url_basename_web (open);
    int iopen = filter::string::convert_to_int (open);
    if (database_notes.identifier_exists (iopen)) {
      Ipc_Notes::open (webserver_request, iopen);
    }
  }
  
  
  if (webserver_request.query.count ("new")) {
    std::string snew = webserver_request.query ["new"];
    snew = filter_url_basename_web (snew);
    int inew = filter::string::convert_to_int (snew);
    std::string bible = database::modifications::getNotificationBible (inew);
    std::string summary = translate("Query about a change in the text");
    std::string contents = "<p>" + translate("Old text:") + "</p>";
    contents += database::modifications::getNotificationOldText (inew);
    contents += "<p>" +  translate("Change:") + "</p>";
    contents += "<p>" + database::modifications::getNotificationModification (inew) + "</p>";
    contents += "<p>" + translate("New text:") + "</p>";
    contents += database::modifications::getNotificationNewText (inew);
    Passage passage = database::modifications::getNotificationPassage (inew);
    int identifier = notes_logic.createNote (bible, passage.m_book, passage.m_chapter, filter::string::convert_to_int (passage.m_verse), summary, contents, false);
    Ipc_Notes::open (webserver_request, identifier);
  }
  

  return std::string();
}

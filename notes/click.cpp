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


string notes_click_url ()
{
  return "notes/click";
}


bool notes_click_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


// This function is called from click.js.
string notes_click (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  if (request->query.count ("open")) {
    string open = request->query ["open"];
    open = filter_url_basename_web (open);
    int iopen = convert_to_int (open);
    if (database_notes.identifierExists (iopen)) {
      Ipc_Notes::open (request, iopen);
    }
  }
  
  
  if (request->query.count ("new")) {
    string snew = request->query ["new"];
    snew = filter_url_basename_web (snew);
    int inew = convert_to_int (snew);
    Database_Modifications database_modifications;
    string bible = database_modifications.getNotificationBible (inew);
    string summary = translate("Query about a change in the text");
    string contents = "<p>" + translate("Old text:") + "</p>";
    contents += database_modifications.getNotificationOldText (inew);
    contents += "<p>" +  translate("Change:") + "</p>";
    contents += "<p>" + database_modifications.getNotificationModification (inew) + "</p>";
    contents += "<p>" + translate("New text:") + "</p>";
    contents += database_modifications.getNotificationNewText (inew);
    Passage passage = database_modifications.getNotificationPassage (inew);
    int identifier = notes_logic.createNote (bible, passage.book, passage.chapter, convert_to_int (passage.verse), summary, contents, false);
    Ipc_Notes::open (request, identifier);
  }
  

  return "";
}

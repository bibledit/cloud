/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <notes/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <menu/logic.h>
#include <access/logic.h>


string notes_index_url ()
{
  return "notes/index";
}


bool notes_index_acl (void * webserver_request)
{
  return access_logic_privilege_view_notes (webserver_request);
}


string notes_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Consultation Notes"), request);
  header.setNavigator ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run();
  
  Assets_View view;
  string error;
  string success;

  // Presets for notes selectors.
  // This is for the daily statistics and the workspace.
  if (request->query.count ("presetselection")) {
    request->database_config_user()->setConsultationNotesPassageSelector (3);
    request->database_config_user()->setConsultationNotesEditSelector (0);
    request->database_config_user()->setConsultationNotesNonEditSelector (0);
    request->database_config_user()->setConsultationNotesStatusSelector ("");
    request->database_config_user()->setConsultationNotesBibleSelector ("");
    request->database_config_user()->setConsultationNotesAssignmentSelector ("");
    request->database_config_user()->setConsultationNotesSubscriptionSelector (0);
    request->database_config_user()->setConsultationNotesSeveritySelector (-1);
    request->database_config_user()->setConsultationNotesTextSelector (0);
    string preset_selector = request->query ["presetselection"];
    if (preset_selector == "assigned") {
      request->database_config_user()->setConsultationNotesAssignmentSelector (request->session_logic()->currentUser ());
    }
    if (preset_selector == "subscribed") {
      request->database_config_user()->setConsultationNotesSubscriptionSelector (1);
    }
    if (preset_selector == "subscribeddayidle") {
      request->database_config_user()->setConsultationNotesSubscriptionSelector (1);
      request->database_config_user()->setConsultationNotesNonEditSelector (1);
    }
    if (preset_selector == "subscribedweekidle") {
      request->database_config_user()->setConsultationNotesSubscriptionSelector (1);
      request->database_config_user()->setConsultationNotesNonEditSelector (3);
    }
    if (preset_selector == "forverse") {
      request->database_config_user()->setConsultationNotesPassageSelector (0);
    }
  }

  int level = request->session_logic ()->currentLevel ();
  // Manager roles and higher can do mass updates on the notes.
  if (level >= Filter_Roles::manager ()) {
    // No mass updates in basic mode.
    if (!config_logic_basic_mode (webserver_request)) {
      view.enable_zone ("update");
    }
  }
  
  // Whether the user can create a new note.
  if (access_logic_privilege_create_comment_notes (webserver_request)) {
    view.enable_zone ("create");
  }
  
  page += view.render ("notes", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}

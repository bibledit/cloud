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


std::string notes_index_url ()
{
  return "notes/index";
}


bool notes_index_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_notes (webserver_request);
}


std::string notes_index (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Consultation Notes"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run();
  
  Assets_View view;
  std::string error;
  std::string success;

  // Presets for notes selectors.
  // This is for the daily statistics and the workspace.
  if (webserver_request.query.count ("presetselection")) {
    webserver_request.database_config_user()->setConsultationNotesPassageSelector (3);
    webserver_request.database_config_user()->setConsultationNotesEditSelector (0);
    webserver_request.database_config_user()->setConsultationNotesNonEditSelector (0);
    webserver_request.database_config_user()->setConsultationNotesStatusSelector ("");
    webserver_request.database_config_user()->setConsultationNotesBibleSelector ("");
    webserver_request.database_config_user()->setConsultationNotesAssignmentSelector ("");
    webserver_request.database_config_user()->setConsultationNotesSubscriptionSelector (0);
    webserver_request.database_config_user()->setConsultationNotesSeveritySelector (-1);
    webserver_request.database_config_user()->setConsultationNotesTextSelector (0);
    std::string preset_selector = webserver_request.query ["presetselection"];
    if (preset_selector == "assigned") {
      webserver_request.database_config_user()->setConsultationNotesAssignmentSelector (webserver_request.session_logic ()->get_username ());
    }
    if (preset_selector == "subscribed") {
      webserver_request.database_config_user()->setConsultationNotesSubscriptionSelector (1);
    }
    if (preset_selector == "subscribeddayidle") {
      webserver_request.database_config_user()->setConsultationNotesSubscriptionSelector (1);
      webserver_request.database_config_user()->setConsultationNotesNonEditSelector (1);
    }
    if (preset_selector == "subscribedweekidle") {
      webserver_request.database_config_user()->setConsultationNotesSubscriptionSelector (1);
      webserver_request.database_config_user()->setConsultationNotesNonEditSelector (3);
    }
    if (preset_selector == "forverse") {
      webserver_request.database_config_user()->setConsultationNotesPassageSelector (0);
    }
  }

  int level = webserver_request.session_logic ()->get_level ();
  // Manager roles and higher can do mass updates on the notes.
  if (level >= Filter_Roles::manager ()) {
    // No mass updates in basic mode.
    if (!config::logic::basic_mode (webserver_request)) {
      view.enable_zone ("update");
    }
  }
  
  // Whether the user can create a new note.
  if (access_logic::privilege_create_comment_notes (webserver_request)) {
    view.enable_zone ("create");
  }
  
  page += view.render ("notes", "index");
  
  page += assets_page::footer ();
  
  return page;
}

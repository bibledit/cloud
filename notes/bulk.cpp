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


#include <notes/bulk.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/volatile.h>
#include <database/logs.h>
#include <database/noteassignment.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <notes/index.h>
#include <dialog/yes.h>
#include <trash/handler.h>
#include <menu/logic.h>


string notes_bulk_url ()
{
  return "notes/bulk";
}


bool notes_bulk_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string notes_bulk (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  Database_NoteAssignment database_noteassignment;

  
  string page;
  
  Assets_Header header = Assets_Header (translate("Bulk update"), request);
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.addBreadCrumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;
  string success, error;

  
  vector <string> bibles = access_bible_bibles (webserver_request);
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);
  int passage_selector = request->database_config_user()->getConsultationNotesPassageSelector();
  int edit_selector = request->database_config_user()->getConsultationNotesEditSelector();
  int non_edit_selector = request->database_config_user()->getConsultationNotesNonEditSelector();
  string status_selector = request->database_config_user()->getConsultationNotesStatusSelector();
  string bible_selector = request->database_config_user()->getConsultationNotesBibleSelector();
  string assignment_selector = request->database_config_user()->getConsultationNotesAssignmentSelector();
  bool subscription_selector = request->database_config_user()->getConsultationNotesSubscriptionSelector();
  int severity_selector = request->database_config_user()->getConsultationNotesSeveritySelector();
  int text_selector = request->database_config_user()->getConsultationNotesTextSelector();
  string search_text = request->database_config_user()->getConsultationNotesSearchText();
  
  
  int userid = filter_string_user_identifier (webserver_request);
  
  
  // The admin disables notes selection on Bibles, so the admin sees all notes, even notes referring to non-existing Bibles.
  if (request->session_logic ()->currentLevel () == Filter_Roles::admin ()) bibles.clear ();

  
  
  // Action to take.
  bool subscribe = request->query.count ("subscribe");
  bool unsubscribe = request->query.count ("unsubscribe");
  bool assign = request->query.count ("assign");
  bool unassign = request->query.count ("unassign");
  bool status = request->query.count ("status");
  bool severity = request->query.count ("severity");
  bool bible = request->query.count ("bible");
  bool erase = request->query.count ("delete");
  
  
  // In case there is no relevant GET action yet,
  // that is, the first time the page gets opened,
  // assemble the list of identifiers of notes to operate on.
  // This is done to remember them as long as this page is active.
  // Thus erroneous bulk operations on notes can be rectified somewhat easier.
  if (!subscribe && !unsubscribe && !assign && !unassign && !status && !severity && !bible && !erase) {
    vector <int> identifiers = database_notes.selectNotes (bibles,
                                              book,
                                              chapter,
                                              verse,
                                              passage_selector,
                                              edit_selector,
                                              non_edit_selector,
                                              status_selector,
                                              bible_selector,
                                              assignment_selector,
                                              subscription_selector,
                                              severity_selector,
                                              text_selector,
                                              search_text,
                                              -1);
    vector <string> sids;
    for (auto id : identifiers) sids.push_back (convert_to_string (id));
    Database_Volatile::setValue (userid, "identifiers", filter_string_implode (sids, " "));
  }


  
  // Get the stored note identifiers from the database.
  vector <int> identifiers;
  {
    vector <string> sids = filter_string_explode (Database_Volatile::getValue (userid, "identifiers"), ' ');
    for (auto id : sids) identifiers.push_back (convert_to_int (id));
  }
  
  
  string identifierlist;
  for (auto identifier : identifiers) {
    identifierlist.append (" ");
    identifierlist.append (convert_to_string (identifier));
  }
  
  

  if (subscribe) {
    for (auto identifier : identifiers) {
      notes_logic.subscribe (identifier);
    }
    success = translate("You subscribed to these notes");
  }
  
  
  if (unsubscribe) {
    for (auto identifier : identifiers) {
      notes_logic.unsubscribe (identifier);
    }
    success = translate("You unsubscribed from these notes");
  }
  
  
  if (assign) {
    string assign = request->query["assign"];
    string user = request->session_logic ()->currentUser ();
    vector <string> assignees = database_noteassignment.assignees (user);
    if (in_array (assign, assignees)) {
      for (auto identifier : identifiers) {
        if (!database_notes.isAssigned (identifier, assign)) {
          notes_logic.assignUser (identifier, assign);
        }
      }
    }
    success = translate("The notes were assigned to the user");
    Database_Logs::log ("Notes assigned to user " + assign + ": " + identifierlist);
  }
  
  
  if (unassign) {
    string unassign = request->query["unassign"];
    for (auto identifier : identifiers) {
      if (database_notes.isAssigned (identifier, unassign)) {
        notes_logic.unassignUser (identifier, unassign);
      }
    }
    success = translate("The notes are no longer assigned to the user");
    Database_Logs::log ("Notes unassigned from user " + unassign + ": " + identifierlist);
  }
  
  
  if (status) {
    string status = request->query["status"];
    for (auto identifier : identifiers) {
      if (database_notes.getRawStatus (identifier) != status) {
        notes_logic.setStatus (identifier, status);
      }
    }
    success = translate("The status of the notes was updated");
    Database_Logs::log ("Status update of notes: " + identifierlist);
  }
  
  
  if (severity) {
    int severity = convert_to_int (request->query["severity"]);
    for (auto identifier : identifiers) {
      if (database_notes.getRawSeverity (identifier) != severity) {
        notes_logic.setRawSeverity (identifier, severity);
      }
    }
    success = translate("The severity of the notes was updated");
    Database_Logs::log ("Severity update of notes: " + identifierlist);
  }
  
  
  if (bible) {
    string bible = request->query["bible"];
    if (bible == notes_logic.generalBibleName ()) bible = "";
    for (auto identifier : identifiers) {
      if (database_notes.getBible (identifier) != bible) {
        notes_logic.setBible (identifier, bible);
      }
    }
    success = translate("The Bible of the notes was updated");
    Database_Logs::log ("Bible update of notes: " + identifierlist);
  }
  
  
  if (erase) {
    string confirm = request->query["confirm"];
    if (confirm != "yes") {
      Dialog_Yes dialog_yes = Dialog_Yes ("bulk", translate("Would you like to delete the notes?"));
      dialog_yes.add_query ("delete", "");
      page += dialog_yes.run ();
      return page;
    } else {
      for (auto identifier : identifiers) {
        notes_logic.erase (identifier); // Notifications handling.
      }
      success = translate("The notes were deleted");
    }
  }
  
  
  view.set_variable ("notescount", convert_to_string (identifiers.size()));
  
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  
  page += view.render ("notes", "bulk");
  
  page += Assets_Page::footer ();
  
  return page;
}

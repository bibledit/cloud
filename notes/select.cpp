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


#include <notes/select.h>
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
#include <notes/index.h>
#include <menu/logic.h>
#include <access/logic.h>


string notes_select_url ()
{
  return "notes/select";
}


bool notes_select_acl (void * webserver_request)
{
  return access_logic_privilege_view_notes (webserver_request);
}


string notes_select (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Select notes"), request);
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.addBreadCrumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;
  string success;
  
  
  if (request->query.count ("passageselector")) {
    int passage_selector = convert_to_int (request->query["passageselector"]);
    if ((passage_selector < 0) || (passage_selector > 3)) passage_selector = 0;
    request->database_config_user()->setConsultationNotesPassageSelector (passage_selector);
  }
  
  
  if (request->query.count ("editselector")) {
    int edit_selector = convert_to_int (request->query["editselector"]);
    if ((edit_selector < 0) || (edit_selector > 4)) edit_selector = 0;
    request->database_config_user()->setConsultationNotesEditSelector (edit_selector);
  }
  
  
  if (request->query.count ("noneditselector")) {
    int non_edit_selector = convert_to_int (request->query["noneditselector"]);
    if ((non_edit_selector < 0) || (non_edit_selector > 5)) non_edit_selector = 0;
    request->database_config_user()->setConsultationNotesNonEditSelector (non_edit_selector);
  }
  
  
  if (request->query.count ("statusselector")) {
    string status_selector = request->query["statusselector"];
    request->database_config_user()->setConsultationNotesStatusSelector (status_selector);
  }
  
  
  if (request->query.count ("bibleselector")) {
    string bible_selector = request->query["bibleselector"];
    request->database_config_user()->setConsultationNotesBibleSelector (bible_selector);
    // Also set the active Bible for the user.
    if (bible_selector != "") {
      request->database_config_user()->setBible (bible_selector);
    }
  }
  
  
  if (request->query.count ("assignmentselector")) {
    string assignment_selector = request->query["assignmentselector"];
    request->database_config_user()->setConsultationNotesAssignmentSelector(assignment_selector);
  }
  
  
  if (request->query.count ("subscriptionselector")) {
    bool subscription_selector = convert_to_bool (request->query["subscriptionselector"]);
    request->database_config_user()->setConsultationNotesSubscriptionSelector (subscription_selector);
  }
  
  
  if (request->query.count ("severityselector")) {
    int severity_selector = convert_to_int (request->query["severityselector"]);
    request->database_config_user()->setConsultationNotesSeveritySelector (severity_selector);
  }
  
  
  if (request->query.count ("textselector")) {
    int text_selector = convert_to_int (request->query["textselector"]);
    request->database_config_user()->setConsultationNotesTextSelector (text_selector);
  }
  if (request->post.count ("text")) {
    string search_text = request->post["text"];
    request->database_config_user()->setConsultationNotesSearchText (search_text);
    success = translate("Search text saved");
  }
  
  
  if (request->query.count ("passageinclusionselector")) {
    int passage_inclusion_selector = convert_to_int (request->query["passageinclusionselector"]);
    request->database_config_user()->setConsultationNotesPassageInclusionSelector (passage_inclusion_selector);
  }
  
  
  if (request->query.count ("textinclusionselector")) {
    int text_inclusion_selector = convert_to_int (request->query["textinclusionselector"]);
    request->database_config_user()->setConsultationNotesTextInclusionSelector (text_inclusion_selector);
  }
  
  
  string active_class = "class=\"active\"";
  
  
  int passage_selector = request->database_config_user()->getConsultationNotesPassageSelector();
  view.set_variable ("passageselector" + convert_to_string (passage_selector), active_class);

  
  int edit_selector = request->database_config_user()->getConsultationNotesEditSelector();
  view.set_variable ("editselector" + convert_to_string (edit_selector), active_class);
  
  
  int non_edit_selector = request->database_config_user()->getConsultationNotesNonEditSelector();
  view.set_variable ("noneditselector" + convert_to_string (non_edit_selector), active_class);
  
  
  string status_selector = request->database_config_user()->getConsultationNotesStatusSelector();
  if (status_selector.empty ()) view.set_variable ("anystatus", active_class);
  vector <Database_Notes_Text> possible_statuses = database_notes.getPossibleStatuses ();
  string statusblock;
  for (Database_Notes_Text possible_status : possible_statuses) {
    statusblock.append (" | ");
    statusblock.append ("<a ");
    if (status_selector == possible_status.raw) statusblock.append (active_class);
    statusblock.append (" href=\"select?statusselector=" + possible_status.raw + "\">");
    statusblock.append (possible_status.localized);
    statusblock.append ("</a>");
  }
  view.set_variable ("statusblock", statusblock);
  

  // The information about available Bibles could be gathered from the notes database.
  // But multiple teams can be hosted, the information about available Bibles
  // is gathered from the Bibles the user has access to.
  string bible_selector = request->database_config_user()->getConsultationNotesBibleSelector();
  if (bible_selector.empty ()) view.set_variable ("anybible", active_class);
  string bibleblock;
  vector <string> bibles = access_bible_bibles (webserver_request);
  // The administrator can select from all Bibles in the notes, even Bibles that do not exist.
  if (request->session_logic ()->currentLevel () == Filter_Roles::admin ()) {
    vector <string> notesbibles = database_notes.getAllBibles ();
    bibles.insert (bibles.end (), notesbibles.begin (), notesbibles.end ());
    bibles = array_unique (bibles);
  }
  for (auto bible : bibles) {
    bibleblock.append (" | ");
    bibleblock.append ("<a ");
    if (bible_selector == bible) bibleblock.append (active_class);
    bibleblock.append (" href=\"select?bibleselector=" + bible + "\">" + bible + "</a>");
  }
  view.set_variable ("bibleblock", bibleblock);
  

  string assignment_selector = request->database_config_user()->getConsultationNotesAssignmentSelector();
  if (assignment_selector.empty ()) view.set_variable ("anyassignee", active_class);
  string assigneeblock;
  vector <string> assignees = database_notes.getAllAssignees (bibles);
  for (auto assignee : assignees) {
    assigneeblock.append (" | ");
    assigneeblock.append ("<a ");
    if (assignment_selector == assignee) assigneeblock.append (active_class);
    assigneeblock.append (" href=\"select?assignmentselector=" + assignee + "\">" + assignee + "</a>");
  }
  view.set_variable ("assigneeblock", assigneeblock);
  if (assignment_selector != "") {
    if (find (assignees.begin(), assignees.end (), assignment_selector) == bibles.end ()) {
      view.enable_zone ("nonexistingassignee");
      view.set_variable ("assignmentselector", assignment_selector);
    }
  }
  
  
  bool subscription_selector = request->database_config_user()->getConsultationNotesSubscriptionSelector();
  view.set_variable ("subscriptionselector" + convert_to_string (subscription_selector), active_class);


  int severity_selector = request->database_config_user()->getConsultationNotesSeveritySelector ();
  if (severity_selector < 0) view.set_variable ("anyseverity", active_class);
  string severityblock;
  vector <Database_Notes_Text> severities = database_notes.getPossibleSeverities();
  for (int i = 0; i < (int)severities.size (); i++) {
    severityblock.append (" | ");
    severityblock.append ("<a ");
    if (severity_selector == i) severityblock.append (active_class);
    severityblock.append ("href=\"select?severityselector=" + convert_to_string (i) + "\">" + severities[i].localized + "</a>");
  }
  view.set_variable ("severityblock", severityblock);

  
  int text_selector = request->database_config_user()->getConsultationNotesTextSelector();
  view.set_variable ("textselector" + convert_to_string (text_selector), active_class);
  if (text_selector == 1) view.enable_zone ("textselection");
  string search_text = request->database_config_user()->getConsultationNotesSearchText();
  view.set_variable ("searchtext", search_text);


  int passage_inclusion_selector = request->database_config_user()->getConsultationNotesPassageInclusionSelector();
  view.set_variable ("passageinclusionselector" + convert_to_string (passage_inclusion_selector), active_class);
                     
                     
  int text_inclusion_selector = request->database_config_user()->getConsultationNotesTextInclusionSelector();
  view.set_variable ("textinclusionselector" + convert_to_string (text_inclusion_selector), active_class);
                                        

  // The admin disables notes selection on Bibles, so the admin sees all notes, even notes referring to non-existing Bibles.
  if (request->session_logic ()->currentLevel () == Filter_Roles::admin ()) bibles.clear ();
  
  
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);
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
                                                         bool (subscription_selector),
                                                         severity_selector,
                                                         text_selector,
                                                         search_text,
                                                         -1);
  view.set_variable ("count", convert_to_string (identifiers.size()));

  
  view.set_variable ("success", success);
  
  
  page += view.render ("notes", "select");
  
  page += Assets_Page::footer ();
  
  return page;
}

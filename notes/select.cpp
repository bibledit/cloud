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


std::string notes_select_url ()
{
  return "notes/select";
}


bool notes_select_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_notes (webserver_request);
}


std::string notes_select (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Select notes"), webserver_request);
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.add_bread_crumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;
  std::string success;
  
  
  if (webserver_request.query.count ("passageselector")) {
    int passage_selector = filter::strings::convert_to_int (webserver_request.query["passageselector"]);
    if ((passage_selector < 0) || (passage_selector > 3)) passage_selector = 0;
    webserver_request.database_config_user()->setConsultationNotesPassageSelector (passage_selector);
  }
  
  
  if (webserver_request.query.count ("editselector")) {
    int edit_selector = filter::strings::convert_to_int (webserver_request.query["editselector"]);
    if ((edit_selector < 0) || (edit_selector > 4)) edit_selector = 0;
    webserver_request.database_config_user()->setConsultationNotesEditSelector (edit_selector);
  }
  
  
  if (webserver_request.query.count ("noneditselector")) {
    int non_edit_selector = filter::strings::convert_to_int (webserver_request.query["noneditselector"]);
    if ((non_edit_selector < 0) || (non_edit_selector > 5)) non_edit_selector = 0;
    webserver_request.database_config_user()->setConsultationNotesNonEditSelector (non_edit_selector);
  }
  
  
  if (webserver_request.query.count ("statusselector")) {
    std::string status_selector = webserver_request.query["statusselector"];
    webserver_request.database_config_user()->setConsultationNotesStatusSelector (status_selector);
  }
  
  
  if (webserver_request.query.count ("bibleselector")) {
    std::string bible_selector = webserver_request.query["bibleselector"];
    webserver_request.database_config_user()->setConsultationNotesBibleSelector (bible_selector);
    // Also set the active Bible for the user.
    if (bible_selector != "") {
      webserver_request.database_config_user()->setBible (bible_selector);
    }
  }
  
  
  if (webserver_request.query.count ("assignmentselector")) {
    std::string assignment_selector = webserver_request.query["assignmentselector"];
    webserver_request.database_config_user()->setConsultationNotesAssignmentSelector(assignment_selector);
  }
  
  
  if (webserver_request.query.count ("subscriptionselector")) {
    bool subscription_selector = filter::strings::convert_to_bool (webserver_request.query["subscriptionselector"]);
    webserver_request.database_config_user()->setConsultationNotesSubscriptionSelector (subscription_selector);
  }
  
  
  if (webserver_request.query.count ("severityselector")) {
    int severity_selector = filter::strings::convert_to_int (webserver_request.query["severityselector"]);
    webserver_request.database_config_user()->setConsultationNotesSeveritySelector (severity_selector);
  }
  
  
  if (webserver_request.query.count ("textselector")) {
    int text_selector = filter::strings::convert_to_int (webserver_request.query["textselector"]);
    webserver_request.database_config_user()->setConsultationNotesTextSelector (text_selector);
  }
  if (webserver_request.post.count ("text")) {
    std::string search_text = webserver_request.post["text"];
    webserver_request.database_config_user()->setConsultationNotesSearchText (search_text);
    success = translate("Search text saved");
  }
  
  
  if (webserver_request.query.count ("passageinclusionselector")) {
    int passage_inclusion_selector = filter::strings::convert_to_int (webserver_request.query["passageinclusionselector"]);
    webserver_request.database_config_user()->setConsultationNotesPassageInclusionSelector (passage_inclusion_selector);
  }
  
  
  if (webserver_request.query.count ("textinclusionselector")) {
    int text_inclusion_selector = filter::strings::convert_to_int (webserver_request.query["textinclusionselector"]);
    webserver_request.database_config_user()->setConsultationNotesTextInclusionSelector (text_inclusion_selector);
  }
  
  
  std::string active_class = R"(class="active")";
  
  
  int passage_selector = webserver_request.database_config_user()->getConsultationNotesPassageSelector();
  view.set_variable ("passageselector" + filter::strings::convert_to_string (passage_selector), active_class);

  
  int edit_selector = webserver_request.database_config_user()->getConsultationNotesEditSelector();
  view.set_variable ("editselector" + filter::strings::convert_to_string (edit_selector), active_class);
  
  
  int non_edit_selector = webserver_request.database_config_user()->getConsultationNotesNonEditSelector();
  view.set_variable ("noneditselector" + filter::strings::convert_to_string (non_edit_selector), active_class);
  
  
  std::string status_selector = webserver_request.database_config_user()->getConsultationNotesStatusSelector();
  if (status_selector.empty ()) view.set_variable ("anystatus", active_class);
  std::vector <Database_Notes_Text> possible_statuses = database_notes.get_possible_statuses ();
  std::stringstream statusblock;
  for (Database_Notes_Text possible_status : possible_statuses) {
    statusblock << " | ";
    statusblock << "<a ";
    if (status_selector == possible_status.raw) statusblock << active_class;
    statusblock << " href=" << std::quoted ("select?statusselector=" + possible_status.raw) << ">";
    statusblock << possible_status.localized;
    statusblock << "</a>";
  }
  view.set_variable ("statusblock", statusblock.str());
  

  // The information about available Bibles could be gathered from the notes database.
  // But multiple teams can be hosted, the information about available Bibles
  // is gathered from the Bibles the user has access to.
  std::string bible_selector = webserver_request.database_config_user()->getConsultationNotesBibleSelector();
  if (bible_selector.empty ()) view.set_variable ("anybible", active_class);
  std::stringstream bibleblock;
  std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  // The administrator can select from all Bibles in the notes, even Bibles that do not exist.
  if (webserver_request.session_logic ()->currentLevel () == Filter_Roles::admin ()) {
    std::vector <std::string> notesbibles = database_notes.get_all_bibles ();
    bibles.insert (bibles.end (), notesbibles.begin (), notesbibles.end ());
    bibles = filter::strings::array_unique (bibles);
  }
  for (auto bible : bibles) {
    bibleblock << " | ";
    bibleblock << "<a ";
    if (bible_selector == bible) bibleblock << active_class;
    bibleblock << " href=" << std::quoted ("select?bibleselector=" + bible) << ">" << bible << "</a>";
  }
  view.set_variable ("bibleblock", bibleblock.str());
  

  std::string assignment_selector = webserver_request.database_config_user()->getConsultationNotesAssignmentSelector();
  if (assignment_selector.empty ()) view.set_variable ("anyassignee", active_class);
  std::stringstream assigneeblock;
  std::vector <std::string> assignees = database_notes.get_all_assignees (bibles);
  for (auto assignee : assignees) {
    assigneeblock << " | ";
    assigneeblock << "<a ";
    if (assignment_selector == assignee) assigneeblock << active_class;
    assigneeblock << " href=" << std::quoted ("select?assignmentselector=" + assignee) << ">" << assignee << "</a>";
  }
  view.set_variable ("assigneeblock", assigneeblock.str());
  if (assignment_selector != "") {
    if (find (assignees.begin(), assignees.end (), assignment_selector) == bibles.end ()) {
      view.enable_zone ("nonexistingassignee");
      view.set_variable ("assignmentselector", assignment_selector);
    }
  }
  
  
  bool subscription_selector = webserver_request.database_config_user()->getConsultationNotesSubscriptionSelector();
  view.set_variable ("subscriptionselector" + filter::strings::convert_to_string (subscription_selector), active_class);


  int severity_selector = webserver_request.database_config_user()->getConsultationNotesSeveritySelector ();
  if (severity_selector < 0) view.set_variable ("anyseverity", active_class);
  std::stringstream severityblock;
  std::vector <Database_Notes_Text> severities = database_notes.get_possible_severities();
  for (size_t i = 0; i < severities.size (); i++) {
    severityblock << " | ";
    severityblock << "<a ";
    if (severity_selector == static_cast<int>(i)) severityblock << active_class;
    severityblock << "href=" << std::quoted ("select?severityselector=" + filter::strings::convert_to_string (i)) << ">" << severities[i].localized << "</a>";
  }
  view.set_variable ("severityblock", severityblock.str());

  
  int text_selector = webserver_request.database_config_user()->getConsultationNotesTextSelector();
  view.set_variable ("textselector" + filter::strings::convert_to_string (text_selector), active_class);
  if (text_selector == 1) view.enable_zone ("textselection");
  std::string search_text = webserver_request.database_config_user()->getConsultationNotesSearchText();
  view.set_variable ("searchtext", search_text);


  int passage_inclusion_selector = webserver_request.database_config_user()->getConsultationNotesPassageInclusionSelector();
  view.set_variable ("passageinclusionselector" + filter::strings::convert_to_string (passage_inclusion_selector), active_class);
                     
                     
  int text_inclusion_selector = webserver_request.database_config_user()->getConsultationNotesTextInclusionSelector();
  view.set_variable ("textinclusionselector" + filter::strings::convert_to_string (text_inclusion_selector), active_class);
                                        

  // The admin disables notes selection on Bibles, so the admin sees all notes, even notes referring to non-existing Bibles.
  if (webserver_request.session_logic ()->currentLevel () == Filter_Roles::admin ()) bibles.clear ();
  
  
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);
  const std::vector <int> identifiers = database_notes.select_notes (bibles, book, chapter, verse,
                                                                passage_selector, edit_selector, non_edit_selector,
                                                                status_selector, bible_selector, assignment_selector,
                                                                subscription_selector, severity_selector, text_selector,
                                                                search_text, -1);
  view.set_variable ("count", filter::strings::convert_to_string (identifiers.size()));

  
  view.set_variable ("success", success);
  
  
  page += view.render ("notes", "select");
  
  page += assets_page::footer ();
  
  return page;
}

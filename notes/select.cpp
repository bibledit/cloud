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
#include <dialog/select.h>


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
  
  
  {
    constexpr const char* identification {"passageselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_passage_selector();
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_int(webserver_request.post_get(identification));
      if ((selected < 0) or (selected > 3)) selected = 0;
      webserver_request.database_config_user()->set_consultation_notes_passage_selector(selected);
    }
    const std::vector<std::string> values { "0", "1", "2", "3" };
    const std::vector<std::string>displayed {
      translate("the current verse"),
      translate("the current chapter"),
      translate("the current book"),
      translate("any passage"),
    };
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .displayed = displayed,
      .selected = std::to_string(selected),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  {
    constexpr const char* identification {"editselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_edit_selector();
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_int(webserver_request.post_get(identification));
      if ((selected < 0) or (selected > 4)) selected = 0;
      webserver_request.database_config_user()->set_consultation_notes_edit_selector(selected);
    }
    const std::vector<std::string> values { "0", "1", "2", "3", "4" };
    const std::vector<std::string>displayed {
      translate("at any time"),
      translate("during the last 30 days"),
      translate("during the last 7 days"),
      translate("since yesterday"),
      translate("today"),
    };
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .displayed = displayed,
      .selected = std::to_string(selected),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  {
    constexpr const char* identification {"noneditselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_non_edit_selector();
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_int(webserver_request.post_get(identification));
      if ((selected < 0) or (selected > 5)) selected = 0;
      webserver_request.database_config_user()->set_consultation_notes_non_edit_selector(selected);
    }
    const std::vector<std::string> values { "0", "1", "2", "3", "4", "5" };
    const std::vector<std::string>displayed {
      translate("any time"),
      translate("a day"),
      translate("two days"),
      translate("a week"),
      translate("a month"),
      translate("a year"),
    };
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .displayed = displayed,
      .selected = std::to_string(selected),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  {
    const std::list<std::pair<std::string,std::string>> possible_statuses {
      {"statusnew",        "New"        },
      {"statuspending",    "Pending"    },
      {"statusinprogress", "In progress"},
      {"statusdone",       "Done"       },
      {"statusreopened",   "Reopened"   },
    };
    if (webserver_request.post_count("statussubmit"))
    {
      std::vector<std::string> checked_statuses{};
      for (const auto& [post, status] : possible_statuses) {
        if (webserver_request.post_count(post))
          checked_statuses.push_back(status);
      }
      webserver_request.database_config_user()->set_consultation_notes_status_selectors(checked_statuses);
    }
    const std::vector<std::string> statuses {webserver_request.database_config_user()->get_consultation_notes_status_selectors()};
    for (const auto& [post, status] : possible_statuses) {
      if (in_array(status, statuses)) {
        view.set_variable(post, "checked");
      }
    }
  }

  
  // The information about available Bibles could be gathered from the notes database.
  // But since multiple teams can be hosted, the information about available Bibles
  // is gathered from the Bibles the user has access to.
  std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  // The administrator can select from all Bibles in the notes, even Bibles that do not exist.
  if (webserver_request.session_logic ()->get_level () == roles::admin) {
    std::vector <std::string> notesbibles = database_notes.get_all_bibles ();
    bibles.insert (bibles.end (), notesbibles.begin (), notesbibles.end ());
    bibles = filter::strings::array_unique (bibles);
  }
  
  
  {
    constexpr const char* identification {"bibleselector"};
    std::string selected = webserver_request.database_config_user()->get_consultation_notes_bible_selector();
    if (webserver_request.post_count(identification)) {
      selected = webserver_request.post_get(identification);
      webserver_request.database_config_user()->set_consultation_notes_bible_selector(selected);
      // Also set the active Bible for the user.
      if (!selected.empty()) {
        webserver_request.database_config_user()->set_bible (selected);
      }
    }
    std::vector<std::string> values {""};
    std::vector<std::string> displayed {translate("any Bible")};
    for (const auto& bible : bibles) {
      values.push_back(bible);
      displayed.push_back(bible);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = std::move(values),
      .displayed = std::move(displayed),
      .selected = selected,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  {
    constexpr const char* identification {"assignmentselector"};
    std::string selected = webserver_request.database_config_user()->get_consultation_notes_assignment_selector();
    if (webserver_request.post_count(identification)) {
      selected = webserver_request.post_get(identification);
      webserver_request.database_config_user()->set_consultation_notes_assignment_selector(selected);
    }
    std::vector<std::string> values {""};
    std::vector<std::string> displayed {translate("anyone")};
    const std::vector <std::string> assignees = database_notes.get_all_assignees (bibles);
    for (const auto& assignee : assignees) {
      values.push_back(assignee);
      displayed.push_back(assignee);
    }
    // If the currently selected assignee does not exist in the list of all assignees, then add it to the selector.
    // This will display the assignee the system is selecting notes on.
    if (!in_array(selected, values)) {
      values.push_back(selected);
      displayed.push_back(selected);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = std::move(values),
      .displayed = std::move(displayed),
      .selected = selected,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  {
    constexpr const char* identification {"subscriptionselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_subscription_selector() ? 1 : 0;
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_bool(webserver_request.post_get(identification));
      webserver_request.database_config_user()->set_consultation_notes_subscription_selector(selected);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = { "0", "1" },
      .displayed = {
        translate("do not care about this"),
        translate("notes I am subscribed to"),
      },
        .selected = selected ? "1" : "0",
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  
  {
    constexpr const char* identification {"severityselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_severity_selector();
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_int(webserver_request.post_get(identification));
      webserver_request.database_config_user()->set_consultation_notes_severity_selector(selected);
    }
    std::vector<std::string> values {"-1"};
    std::vector<std::string> displayed {translate("Any")};
    std::vector <Database_Notes_Text> severities = database_notes.get_possible_severities();
    for (size_t i = 0; i < severities.size (); i++) {
      values.push_back(std::to_string(i));
      displayed.push_back(severities[i].localized);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = std::move(values),
      .displayed = std::move(displayed),
      .selected = std::to_string(selected),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  
  {
    constexpr const char* identification {"textselector"};
    int selected = webserver_request.database_config_user()->get_consultation_notes_text_selector();
    if (webserver_request.post_count(identification)) {
      selected = filter::strings::convert_to_int(webserver_request.post_get(identification));
      webserver_request.database_config_user()->set_consultation_notes_text_selector(selected);
    }
    if (selected == 1)
      view.enable_zone ("textselection");
    dialog::select::Settings settings {
      .identification = identification,
      .values =    { "0",              "1" },
      .displayed = { translate("Any"), translate("Specific text or words:") },
      .selected = std::to_string(selected),
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  if (webserver_request.post_count("text")) {
    const std::string search_text = webserver_request.post_get("text");
    webserver_request.database_config_user()->set_consultation_notes_search_text (search_text);
    success = translate("Search text saved");
  }
  

  const std::string checkbox = webserver_request.post_get("checkbox");
  bool checked = filter::strings::convert_to_bool (webserver_request.post_get("checked"));

  
  {
    constexpr const char* identification {"passageinclusionselector"};
    if (checkbox == identification) {
      webserver_request.database_config_user()->set_consultation_notes_passage_inclusion_selector (checked ? 1 : 0);
    }
    view.set_variable (identification, filter::strings::get_checkbox_status (webserver_request.database_config_user()->get_consultation_notes_passage_inclusion_selector()));
  }

  
  {
    constexpr const char* identification {"textinclusionselector"};
    if (checkbox == identification) {
      webserver_request.database_config_user()->set_consultation_notes_text_inclusion_selector (checked ? 1 : 0);
    }
    view.set_variable (identification, filter::strings::get_checkbox_status (webserver_request.database_config_user()->get_consultation_notes_text_inclusion_selector()));
  }
  
  
  const Database_Notes::PassageSelector passage_selector = static_cast<Database_Notes::PassageSelector>(webserver_request.database_config_user()->get_consultation_notes_passage_selector());
  const int edit_selector = webserver_request.database_config_user()->get_consultation_notes_edit_selector();
  const auto non_edit_selector = static_cast<Database_Notes::NonEditSelector>(webserver_request.database_config_user()->get_consultation_notes_non_edit_selector());
  const std::vector<std::string> status_selectors = webserver_request.database_config_user()->get_consultation_notes_status_selectors();
  const std::string bible_selector = webserver_request.database_config_user()->get_consultation_notes_bible_selector();
  const std::string assignment_selector = webserver_request.database_config_user()->get_consultation_notes_assignment_selector();
  const bool subscription_selector = webserver_request.database_config_user()->get_consultation_notes_subscription_selector();
  const auto severity_selector = static_cast<Database_Notes::SeveritySelector>(webserver_request.database_config_user()->get_consultation_notes_severity_selector());
  
  const int text_selector = webserver_request.database_config_user()->get_consultation_notes_text_selector();
  const std::string search_text = text_selector ? webserver_request.database_config_user()->get_consultation_notes_search_text() : "";
  view.set_variable ("searchtext", search_text);
  
  
  // Being an admin disables notes selection on Bibles, so the admin sees all notes,
  // even notes referring to non-existing Bibles.
  if (webserver_request.session_logic()->get_level() == roles::admin)
    bibles.clear ();

  // If filtering on one Bible, put that Bible in the container.
  // Note that this overrides the previous code, that is,
  // if filtering on one Bible, even the admin does not get to see the other ones.
  if (!bible_selector.empty())
    bibles = {bible_selector};

  
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);
  const int verse = Ipc_Focus::getVerse (webserver_request);
  Database_Notes::Selector selector {
    .bibles = bibles,
    .book = book,
    .chapter = chapter,
    .verse = verse,
    .passage_selector = passage_selector,
    .edit_selector = static_cast<Database_Notes::EditSelector>(edit_selector),
    .non_edit_selector = non_edit_selector,
    .status_selectors = status_selectors,
    .assignment_selector = assignment_selector,
    .subscription_selector = subscription_selector,
    .severity_selector = severity_selector,
    .search_text = search_text,
  };
  const std::vector <int> identifiers = database_notes.select_notes (selector);
  view.set_variable ("count", std::to_string (identifiers.size()));
  
  
  view.set_variable ("success", success);
  
  
  page += view.render ("notes", "select");
  
  page += assets_page::footer ();
  
  return page;
}


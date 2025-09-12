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


#include <paratext/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/select.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <paratext/logic.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <tasks/logic.h>
#include <journal/index.h>


std::string paratext_index_url ()
{
  return "paratext/index";
}


bool paratext_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string paratext_index (Webserver_Request& webserver_request)
{
  std::string page;
  page = assets_page::header (translate ("Paratext"), webserver_request);
  Assets_View view;
  std::string success;
  std::string error;

  
  std::string bible = webserver_request.query ["bible"];
  
  
  if (webserver_request.query.count ("disable")) {
    database::config::bible::set_paratext_project (bible, std::string());
    database::config::bible::set_paratext_collaboration_enabled (bible, false);
    filter_url_rmdir (Paratext_Logic::ancestorPath (bible, 0));
    bible.clear ();
  }
  
  
  {
    constexpr const char* identification {"selectbible"};
    if (webserver_request.post_count(identification)) {
      bible = webserver_request.post_get(identification);
    }
    // The selector contains an empty value plus all Bibles.
    std::vector<std::string> values {std::string()};
    for (const auto& value : database::bibles::get_bibles())
      values.push_back(value);
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .selected = bible,
      .parameters = { {"bible", bible} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  view.set_variable ("bible", bible);
  if (!bible.empty ())
    view.enable_zone ("bibleactive");
  
  
  // Paratext Projects folder.
  std::string paratext_folder = database::config::general::get_paratext_projects_folder ();
  if (!file_or_dir_exists (paratext_folder))
    paratext_folder.clear ();
  if (webserver_request.query.count ("paratextfolder")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter the name of the Paratext projects folder"), paratext_folder, "paratextfolder", "");
    dialog_entry.add_query ("bible", bible);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post_count("paratextfolder")) {
    std::string folder = webserver_request.post_get("entry");
    if (file_or_dir_exists (folder)) {
      paratext_folder = folder;
      success = translate ("Paratext projects folder was set.");
    } else {
      error = translate ("Paratext projects folder does not exist:") + " " + folder;
      paratext_folder.clear ();
    }
  }
  if (paratext_folder.empty ())
    paratext_folder = Paratext_Logic::searchProjectsFolder ();
  database::config::general::set_paratext_projects_folder (paratext_folder);
  view.set_variable ("paratextfolder", paratext_folder);
  if (paratext_folder.empty ()) {
    view.set_variable ("paratextfolder", translate ("not found"));
  } else {
    view.enable_zone ("paratextprojectsactive");
  }

  
  // The Paratext project.
  std::string paratext_project = database::config::bible::get_paratext_project (bible);
  if (!file_or_dir_exists (filter_url_create_path ({paratext_folder, paratext_project})))
    paratext_project.clear ();
  {
    constexpr const char* identification {"paratextproject"};
    if (webserver_request.post_count(identification)) {
      paratext_project = webserver_request.post_get(identification);
    }
    std::vector<std::string> values {std::string()};
    for (const auto & value : Paratext_Logic::searchProjects (paratext_folder)) {
      values.push_back(value);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = std::move(values),
      .selected = paratext_project,
      .parameters = { {"bible", bible} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  database::config::bible::set_paratext_project (bible, paratext_project);
  if (!paratext_project.empty ())
    view.enable_zone ("paratextprojectactive");


  // Authoritative copy: Take from either Bibledit or else from Paratext.
  {
    constexpr const char* identification {"master"};
    if (webserver_request.post_count(identification)) {
      // Set collaboration up.
      const std::string master = webserver_request.post_get(identification);
      tasks_logic_queue (task::setup_paratext, { bible, master });
      success = translate ("The collaboration will be set up");
      if (database::config::general::get_repeat_send_receive () == 0) {
        database::config::general::set_repeat_send_receive (2);
      }
      view.set_variable ("master", master);
      view.enable_zone ("setuprunning");
      redirect_browser (webserver_request, journal_index_url ());
      return std::string();
      paratext_project = webserver_request.post_get(identification);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = { "", "bibledit", "paratext" },
      .displayed = { "", "Bibledit", "Paratext" },
      .selected = "",
      .parameters = { {"bible", bible} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }


  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  page += view.render ("paratext", "index");
  
  page += assets_page::footer ();
  
  return page;
}

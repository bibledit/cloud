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


#include <paratext/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
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
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string paratext_index (Webserver_Request& webserver_request)
{
  std::string page;
  page = assets_page::header (translate ("Paratext"), webserver_request);
  Assets_View view;
  std::string success;
  std::string error;

  
  std::string bible = webserver_request.query ["bible"];

  
  if (webserver_request.query.count ("selectbible")) {
    std::string select = webserver_request.query["selectbible"];
    if (select == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which Bible are you going to use?"), "", "");
      dialog_list.add_query ("bible", bible);
      std::vector <std::string> bibles = webserver_request.database_bibles()->get_bibles();
      for (auto & value : bibles) {
        dialog_list.add_row (value, "selectbible", value);
      }
      page += dialog_list.run ();
      return page;
    } else {
      bible = select;
    }
  }
  
  
  if (webserver_request.query.count ("disable")) {
    Database_Config_Bible::setParatextProject (bible, "");
    Database_Config_Bible::setParatextCollaborationEnabled (bible, false);
    filter_url_rmdir (Paratext_Logic::ancestorPath (bible, 0));
    bible.clear ();
  }

  
  view.set_variable ("bible", bible);
  if (!bible.empty ()) view.enable_zone ("bibleactive");
  
  
  // Paratext Projects folder.
  std::string paratext_folder = Database_Config_General::getParatextProjectsFolder ();
  if (!file_or_dir_exists (paratext_folder)) paratext_folder.clear ();
  
  if (webserver_request.query.count ("paratextfolder")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter the name of the Paratext projects folder"), paratext_folder, "paratextfolder", "");
    dialog_entry.add_query ("bible", bible);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("paratextfolder")) {
    std::string folder = webserver_request.post ["entry"];
    if (file_or_dir_exists (folder)) {
      paratext_folder = folder;
      success = translate ("Paratext projects folder was set.");
    } else {
      error = translate ("Paratext projects folder does not exist:") + " " + folder;
      paratext_folder.clear ();
    }
  }

  if (paratext_folder.empty ()) paratext_folder = Paratext_Logic::searchProjectsFolder ();

  Database_Config_General::setParatextProjectsFolder (paratext_folder);
  view.set_variable ("paratextfolder", paratext_folder);
  if (paratext_folder.empty ()) {
    view.set_variable ("paratextfolder", translate ("not found"));
  } else {
    view.enable_zone ("paratextprojectsactive");
  }

  
  // Paratext Project.
  std::string paratext_project = Database_Config_Bible::getParatextProject (bible);
  if (!file_or_dir_exists (filter_url_create_path ({paratext_folder, paratext_project}))) paratext_project.clear ();
  
  if (webserver_request.query.count ("paratextproject")) {
    std::string project = webserver_request.query["paratextproject"];
    if (project == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which Paratext project are you going to use?"), "", "");
      dialog_list.add_query ("bible", bible);
      std::vector <std::string> projects = Paratext_Logic::searchProjects (paratext_folder);
      for (auto & value : projects) {
        dialog_list.add_row (value, "paratextproject", value);
      }
      page += dialog_list.run ();
      return page;
    } else {
      paratext_project = project;
    }
  }
  
  Database_Config_Bible::setParatextProject (bible, paratext_project);
  view.set_variable ("paratextproject", paratext_project);
  if (!paratext_project.empty ()) view.enable_zone ("paratextprojectactive");


  // Authoritative copy: Take from either Bibledit or else from Paratext.
  if (webserver_request.query.count ("master")) {
    std::string master = webserver_request.query["master"];
    if (master == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Where are you going to take the initial Bible data from?"), "", "");
      dialog_list.add_query ("bible", bible);
      dialog_list.add_row (translate ("Bibledit"), "master", "bibledit");
      dialog_list.add_row ("Paratext", "master", "paratext");
      page += dialog_list.run ();
      return page;
    } else {
      // Set collaboration up.
      tasks_logic_queue (SETUPPARATEXT, { bible, master });
      success = translate ("The collaboration will be set up");
      if (Database_Config_General::getRepeatSendReceive () == 0) {
        Database_Config_General::setRepeatSendReceive (2);
      }
      view.set_variable ("master", master);
      view.enable_zone ("setuprunning");
      redirect_browser (webserver_request, journal_index_url ());
      return std::string();
    }
  }


  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  page += view.render ("paratext", "index");
  
  page += assets_page::footer ();
  
  return page;
}

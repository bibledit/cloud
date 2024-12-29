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


#include <collaboration/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/git.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <database/config/bible.h>
#include <dialog/list.h>
#include <menu/logic.h>


std::string collaboration_index_url ()
{
  return "collaboration/index";
}


bool collaboration_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::admin ());
}


std::string collaboration_index (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Repository"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;

  
#ifdef HAVE_CLOUD

  
  std::string object = webserver_request.query ["object"];
  if (webserver_request.query.count ("select")) {
    const std::string& select = webserver_request.query["select"];
    if (select.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which Bible are you going to use?"), "", "");
      dialog_list.add_query ("object", object);
      const std::vector <std::string>& bibles = database::bibles::get_bibles();
      for (const auto& value : bibles) {
        dialog_list.add_row (value, "select", value);
      }
      page += dialog_list.run ();
      return page;
    } else {
      object = select;
    }
  }
  view.set_variable ("object", object);
  if (!object.empty ()) view.enable_zone ("objectactive");


  const std::string& repositoryfolder = filter_git_directory (object);

  
  if (webserver_request.query.count ("disable")) {
    database::config::bible::set_remote_repository_url (object, "");
    filter_url_rmdir (repositoryfolder);
  }
  const std::string& url = database::config::bible::get_remote_repository_url (object);
  view.set_variable ("url", url);
  if (url.empty ()) {
    view.enable_zone ("urlinactive");
  } else {
    view.enable_zone ("urlactive");
  }
  
  
  // Get the status of the git repository.
  // This could have been done through the following:
  // std::vector <std::string> statuslines = filter_git_status (repositoryfolder);
  // But this function does not capture standard error.
  // And the standard error output is needed in case of failures.
  // So the following is used instead.
  if (!object.empty ()) {
    std::string statusoutput, statuserror;
    filter::shell::run (repositoryfolder, filter::shell::get_executable(filter::shell::Executable::git), {"status"}, &statusoutput, &statuserror);
    view.set_variable ("status", statusoutput + " " + statuserror);
  }

  
#endif

  
  page += view.render ("collaboration", "index");
  page += assets_page::footer ();
  return page;
}

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


#include <resource/organize.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <resource/divider.h>
#include <sword/logic.h>
#include <sync/logic.h>
#include <dialog/entry.h>
#include <client/logic.h>
#include <access/logic.h>
#include <tasks/logic.h>
#include <database/config/general.h>


std::string resource_organize_url ()
{
  return "resource/organize";
}


bool resource_organize_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_organize (Webserver_Request& webserver_request)
{
  std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);


  // For administrator level default resource management purposes.
  int level = webserver_request.session_logic()->currentLevel ();
  bool is_def = false;
  if (webserver_request.query["type"] == "def" | webserver_request.post["type"] == "def") is_def = true;
  std::vector <std::string> default_active_resources = Database_Config_General::getDefaultActiveResources ();

  
  // Deal with a new added resources.
  if (webserver_request.query.count ("add") || webserver_request.post.count ("add")) {
    std::string add = webserver_request.query["add"];
    if (add.empty ()) add = webserver_request.post ["add"];
    if (add == resource_logic_rich_divider ()) {
      // Navigate to the page to set up the rich divider.
      if (is_def) 
        redirect_browser (webserver_request, filter_url_build_http_query (resource_divider_url (), "type", "def"));
      else
        redirect_browser (webserver_request, resource_divider_url ());
      return std::string();
    } else {
      // Add the new resource to the existing selection of resources for the current user.
      std::vector <std::string> resources = webserver_request.database_config_user()->getActiveResources ();
      if (is_def) resources = default_active_resources;
      resources.push_back (add);
      if (is_def) Database_Config_General::setDefaultActiveResources (resources);
      else webserver_request.database_config_user()->setActiveResources (resources);
      if (!is_def) 
        webserver_request.database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
      else 
        redirect_browser (webserver_request, resource_organize_url());
    }
  }
  
  
  if (webserver_request.query.count ("remove")) {
    int remove = filter::strings::convert_to_int (webserver_request.query["remove"]);
    std::vector <std::string> resources = webserver_request.database_config_user()->getActiveResources ();
    if (is_def) resources = default_active_resources;
    if (remove < static_cast<int>(resources.size ())) {
      resources.erase (resources.begin () + remove);
    }
    if (is_def) Database_Config_General::setDefaultActiveResources (resources);
    else webserver_request.database_config_user()->setActiveResources (resources);
    if (!is_def) 
      webserver_request.database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
    else 
      redirect_browser (webserver_request, resource_organize_url());
  }

  
  std::string movefrom = webserver_request.post ["movefrom"];
  if (!movefrom.empty ()) {
    std::string moveto =  webserver_request.post ["moveto"];
    if (!moveto.empty ()) {
      size_t from = static_cast<size_t> (filter::strings::convert_to_int (movefrom));
      size_t to = static_cast<size_t>(filter::strings::convert_to_int (moveto));
      std::vector <std::string> resources = webserver_request.database_config_user()->getActiveResources ();
      if (is_def) resources = default_active_resources;
      filter::strings::array_move_from_to (resources, from, to);
      if (is_def) Database_Config_General::setDefaultActiveResources (resources);
      else webserver_request.database_config_user()->setActiveResources (resources);
      if (!is_def) 
        webserver_request.database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
      else 
        redirect_browser (webserver_request, resource_organize_url());
    }
    return std::string();
  }
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Resources"), webserver_request);
  page = header.run ();
  Assets_View view;


  // If the user has an administrator role,
  // that useer can set default resources for the users with lower access levels.
  if (level == Filter_Roles::admin())
    view.enable_zone ("defaultresourceorganizer");

  // If the user has a role less than an administrator,
  // and an administrator has compiled a default selection of resources,
  // the user can apply that compiled selection of resources.
  if (level < Filter_Roles::admin() && !default_active_resources.empty ())
    view.enable_zone ("defaultresources");

  // Default active resources.
  if (level == Filter_Roles::admin()) {
    std::string defactivesblock;
    for (size_t i = 0; i < default_active_resources.size (); i++) {
      defactivesblock.append ("<p>&#183; ");
      defactivesblock.append ("<a href=\"?remove=" + filter::strings::convert_to_string (i) + "&type=def\">");
      defactivesblock.append (filter::strings::emoji_wastebasket ());
      defactivesblock.append ("</a>");
      defactivesblock.append (" ");
      defactivesblock.append (default_active_resources [i]);
      defactivesblock.append ("</p>");
      defactivesblock.append ("\n");
    }
    view.set_variable ("defactivesblock", defactivesblock);
  }

  
  // Active resources.
  std::vector <std::string> active_resources = webserver_request.database_config_user()->getActiveResources ();
  std::string activesblock;
  for (size_t i = 0; i < active_resources.size (); i++) {
    activesblock.append ("<p>&#183; ");
    activesblock.append ("<a href=\"?remove=" + filter::strings::convert_to_string (i) + "\">");
    activesblock.append (filter::strings::emoji_wastebasket ());
    activesblock.append ("</a>");
    activesblock.append (" ");
    activesblock.append (active_resources [i]);
    activesblock.append ("</p>");
    activesblock.append ("\n");
  }
  view.set_variable ("activesblock", activesblock);
  
  
  // Context before.
  if (webserver_request.query.count ("before")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("organize", translate("Please enter the number of verses"), filter::strings::convert_to_string (webserver_request.database_config_user ()->getResourceVersesBefore ()), "before", translate ("How many verses of context to display before the focused verse."));
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("before")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      webserver_request.database_config_user ()->setResourceVersesBefore (value);
    }
  }
  view.set_variable ("before", filter::strings::convert_to_string (webserver_request.database_config_user ()->getResourceVersesBefore ()));

  
  // Context after.
  if (webserver_request.query.count ("after")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("organize", translate("Please enter the number of verses"), filter::strings::convert_to_string (webserver_request.database_config_user ()->getResourceVersesAfter ()), "after", translate ("How many verses of context to display after the focused verse."));
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("after")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      webserver_request.database_config_user ()->setResourceVersesAfter (value);
    }
  }
  view.set_variable ("after", filter::strings::convert_to_string (webserver_request.database_config_user ()->getResourceVersesAfter ()));

  
  if (checkbox == "related") {
    webserver_request.database_config_user ()->setIncludeRelatedPassages (checked);
    return std::string();
  }
  view.set_variable ("related", filter::strings::get_checkbox_status (webserver_request.database_config_user ()->getIncludeRelatedPassages ()));


  // For users with lower than administrator access levels, they can replace
  // their resource list with the recommended resources list that has been set
  // by the administrator.
  if (webserver_request.query.count ("applydefaultresources")) {
    webserver_request.database_config_user ()->setActiveResources (default_active_resources);
    redirect_browser (webserver_request, resource_organize_url ());
  }


  // The same with above, but add the recommended resources to their current
  // list instead of replacing it.
  if (webserver_request.query.count ("adddefaultresources")) {
    std::vector <std::string> joined_resources = webserver_request.database_config_user ()->getActiveResources ();
    joined_resources.insert(joined_resources.end(), default_active_resources.begin(), default_active_resources.end());
    webserver_request.database_config_user ()->setActiveResources (joined_resources);
    redirect_browser (webserver_request, resource_organize_url ());
  }

  
  if (webserver_request.query.count ("install")) {
    std::vector <std::string> installing_resources = Database_Config_General::getResourcesToCache ();
    std::vector <std::string> active_resources_2 = webserver_request.database_config_user()->getActiveResources ();
    for (auto & resource : active_resources_2) {
      if (resource_logic_can_cache (resource)) {
        if (!in_array (resource, installing_resources)) {
          installing_resources.push_back (resource);
          Database_Config_General::setResourcesToCache (installing_resources);
        }
      }
    }
    tasks_logic_queue (SYNCRESOURCES);
    view.set_variable ("success", translate ("Will install the resources.") + " " + translate ("The journal displays the progress."));
  }

  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#endif


#ifdef HAVE_CLOUD
  view.enable_zone ("cloud");
#endif

  
  page += view.render ("resource", "organize");
  page += assets_page::footer ();
  return page;
}

/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


string resource_organize_url ()
{
  return "resource/organize";
}


bool resource_organize_acl (void * webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


string resource_organize (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);


  // For administrator level default resource management purposes.
  int level = request->session_logic()->currentLevel ();
  bool is_def = false;
  if (request->query["type"] == "def" | request->post["type"] == "def") is_def = true;

  
  // Deal with a new added resources.
  if (request->query.count ("add") || request->post.count ("add")) {
    string add = request->query["add"];
    if (add.empty ()) add = request->post ["add"];
    if (add == resource_logic_rich_divider ()) {
      // Navigate to the page to set up the rich divider.
      if (is_def) redirect_browser (webserver_request, filter_url_build_http_query (resource_divider_url (), "type", "def"));
      else redirect_browser (webserver_request, resource_divider_url ());
      return "";
    } else {
      // Add the new resource to the existing list of resources for the current user.
      vector <string> resources = request->database_config_user()->getActiveResources ();
      if (is_def) resources = Database_Config_General::getDefaultActiveResources ();
      resources.push_back (add);
      if (is_def) Database_Config_General::setDefaultActiveResources (resources);
      else request->database_config_user()->setActiveResources (resources);
      if (!is_def) request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
    }
  }
  
  
  if (request->query.count ("remove")) {
    int remove = convert_to_int (request->query["remove"]);
    vector <string> resources = request->database_config_user()->getActiveResources ();
    if (is_def) resources = Database_Config_General::getDefaultActiveResources ();
    if (remove < static_cast<int>(resources.size ())) {
      resources.erase (resources.begin () + remove);
    }
    if (is_def) Database_Config_General::setDefaultActiveResources (resources);
    else request->database_config_user()->setActiveResources (resources);
    if (!is_def) request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
  }

  
  string movefrom = request->post ["movefrom"];
  if (!movefrom.empty ()) {
    string moveto =  request->post ["moveto"];
    if (!moveto.empty ()) {
      size_t from = static_cast<size_t> (convert_to_int (movefrom));
      size_t to = static_cast<size_t>(convert_to_int (moveto));
      vector <string> resources = request->database_config_user()->getActiveResources ();
      if (is_def) resources = Database_Config_General::getDefaultActiveResources ();
      array_move_from_to (resources, from, to);
      if (is_def) Database_Config_General::setDefaultActiveResources (resources);
      else request->database_config_user()->setActiveResources (resources);
      if (!is_def) request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
    }
    return "";
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Resources"), request);
  page = header.run ();
  Assets_View view;


  // If the user is with an administrator access level, he can set default
  // resources for the users with lower access levels.
  if (level == 6) view.enable_zone ("defaultresourceorganizer");


  // If the user is with less than an administrator access level and an
  // administrator has compiled a default selection of resources, the user can
  // apply that compiled selection of resources.
  if (level < 6 && !Database_Config_General::getDefaultActiveResources ().empty ()) view.enable_zone ("defaultresources");


  // Default active resources.
  if (level == 6) {
    vector <string> default_active_resources = Database_Config_General::getDefaultActiveResources ();
    string defactivesblock;
    for (size_t i = 0; i < default_active_resources.size (); i++) {
      defactivesblock.append ("<p>&#183; ");
      defactivesblock.append ("<a href=\"?remove=" + convert_to_string (i) + "&type=def\">");
      defactivesblock.append (emoji_wastebasket ());
      defactivesblock.append ("</a>");
      defactivesblock.append (" ");
      defactivesblock.append (default_active_resources [i]);
      defactivesblock.append ("</p>");
      defactivesblock.append ("\n");
    }
    view.set_variable ("defactivesblock", defactivesblock);
  }

  
  // Active resources.
  vector <string> active_resources = request->database_config_user()->getActiveResources ();
  string activesblock;
  for (size_t i = 0; i < active_resources.size (); i++) {
    activesblock.append ("<p>&#183; ");
    activesblock.append ("<a href=\"?remove=" + convert_to_string (i) + "\">");
    activesblock.append (emoji_wastebasket ());
    activesblock.append ("</a>");
    activesblock.append (" ");
    activesblock.append (active_resources [i]);
    activesblock.append ("</p>");
    activesblock.append ("\n");
  }
  view.set_variable ("activesblock", activesblock);
  
  
  // Context before.
  if (request->query.count ("before")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("organize", translate("Please enter the number of verses"), convert_to_string (request->database_config_user ()->getResourceVersesBefore ()), "before", translate ("How many verses of context to display before the focused verse."));
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("before")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      request->database_config_user ()->setResourceVersesBefore (value);
    }
  }
  view.set_variable ("before", convert_to_string (request->database_config_user ()->getResourceVersesBefore ()));

  
  // Context after.
  if (request->query.count ("after")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("organize", translate("Please enter the number of verses"), convert_to_string (request->database_config_user ()->getResourceVersesAfter ()), "after", translate ("How many verses of context to display after the focused verse."));
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("after")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      request->database_config_user ()->setResourceVersesAfter (value);
    }
  }
  view.set_variable ("after", convert_to_string (request->database_config_user ()->getResourceVersesAfter ()));

  
  if (checkbox == "related") {
    request->database_config_user ()->setIncludeRelatedPassages (checked);
    return "";
  }
  view.set_variable ("related", get_checkbox_status (request->database_config_user ()->getIncludeRelatedPassages ()));


  // For users with lower than administrator access levels, they can replace
  // their resource list with the recommended resources list that has been set
  // by the administrator.
  if (request->query.count ("applydefaultresources")) {
    request->database_config_user ()->setActiveResources (Database_Config_General::getDefaultActiveResources ());
    view.set_variable ("success", translate ("Your resource list has been replaced by the default selection of resources. You may need to reload the page to see changes."));
  }


  // The same with above, but add the recommended resources to their current
  // list instead of replacing it.
  if (request->query.count ("adddefaultresources")) {
    vector <string> joined_resources = request->database_config_user ()->getActiveResources ();
    vector <string> default_resources = Database_Config_General::getDefaultActiveResources ();
    joined_resources.insert(joined_resources.end(), default_resources.begin(), default_resources.end());

    request->database_config_user ()->setActiveResources (joined_resources);
    view.set_variable ("success", translate ("Default selection of resources has been added to your resource list. You may need to reload the page to see changes."));
  }

  
  if (request->query.count ("install")) {
    vector <string> installing_resources = Database_Config_General::getResourcesToCache ();
    vector <string> active_resources_2 = request->database_config_user()->getActiveResources ();
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

  
  page += view.render ("resource", "organize");
  page += assets_page::footer ();
  return page;
}

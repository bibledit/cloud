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


#include <resource/organize.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <sword/logic.h>
#include <sync/logic.h>
#include <dialog/entry.h>
#include <client/logic.h>
#include <access/logic.h>
#include <tasks/logic.h>
#include <database/config/general.h>


string resource_organize_url ()
{
  return "resource/organize";
}


bool resource_organize_acl (void * webserver_request)
{
  return access_logic_privilege_view_resources (webserver_request);
}


string resource_organize (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);

  
  if (request->query.count ("add") || request->post.count ("add")) {
    string add = request->query["add"];
    if (add.empty ()) add = request->post ["add"];
    vector <string> resources = request->database_config_user()->getActiveResources ();
    resources.push_back (add);
    request->database_config_user()->setActiveResources (resources);
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
  }
  
  
  if (request->query.count ("remove")) {
    size_t remove = convert_to_int (request->query["remove"]);
    vector <string> resources = request->database_config_user()->getActiveResources ();
    if (remove < resources.size ()) {
      resources.erase (resources.begin () + remove);
    }
    request->database_config_user()->setActiveResources (resources);
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
  }

  
  if (request->query.count ("moveup")) {
    size_t moveup = convert_to_int (request->query["moveup"]);
    vector <string> resources = request->database_config_user()->getActiveResources ();
    array_move_up_down (resources, moveup, true);
    request->database_config_user()->setActiveResources (resources);
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
  }

  
  if (request->query.count ("movedown")) {
    size_t movedown = convert_to_int (request->query["movedown"]);
    vector <string> resources = request->database_config_user()->getActiveResources ();
    array_move_up_down (resources, movedown, false);
    request->database_config_user()->setActiveResources (resources);
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Resources"), request);
  page = header.run ();
  Assets_View view;

  
  // Active resources.
  vector <string> active_resources = request->database_config_user()->getActiveResources ();
  string activesblock;
  for (size_t i = 0; i < active_resources.size (); i++) {
    activesblock.append ("<p>");
    activesblock.append ("<a href=\"?remove=" + convert_to_string (i) + "\">");
    activesblock.append (emoji_wastebasket ());
    activesblock.append ("</a>");
    activesblock.append (" ");
    activesblock.append ("<a href=\"?moveup=" + convert_to_string (i) + "\">");
    activesblock.append (unicode_black_up_pointing_triangle ());
    activesblock.append ("</a>");
    activesblock.append ("<a href=\"?movedown=" + convert_to_string (i) + "\">");
    activesblock.append (unicode_black_down_pointing_triangle ());
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

  
  if (request->query.count ("install")) {
    vector <string> usfm_resources = client_logic_usfm_resources_get ();
    vector <string> original_language_resources = resource_external_get_original_language_resources ();
    vector <string> bible_resources = resource_external_get_bibles ();
    vector <string> sword_resources = sword_logic_get_available ();
    vector <string> biblegateway_resources = resource_logic_bible_gateway_module_list_get ();
    vector <string> studylight_resources = resource_logic_study_light_module_list_get ();
    vector <string> installable_resources;
    installable_resources.insert (installable_resources.end (), usfm_resources.begin (), usfm_resources.end ());
    installable_resources.insert (installable_resources.end (), original_language_resources.begin (), original_language_resources.end ());
    installable_resources.insert (installable_resources.end (), bible_resources.begin (), bible_resources.end ());
    installable_resources.insert (installable_resources.end (), sword_resources.begin (), sword_resources.end ());
    installable_resources.insert (installable_resources.end (), biblegateway_resources.begin (), biblegateway_resources.end ());
    installable_resources.insert (installable_resources.end (), studylight_resources.begin (), studylight_resources.end ());
    vector <string> installing_resources = Database_Config_General::getResourcesToCache ();
    vector <string> active_resources = request->database_config_user()->getActiveResources ();
    for (auto & resource : active_resources) {
      if (in_array (resource, installable_resources)) {
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
  page += Assets_Page::footer ();
  return page;
}

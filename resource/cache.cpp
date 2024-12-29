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


#include <resource/cache.h>
#include <resource/external.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <sword/logic.h>
#include <menu/logic.h>
#include <database/config/general.h>
#include <database/usfmresources.h>
#include <sendreceive/resources.h>
#include <client/logic.h>
#include <config/globals.h>


std::string resource_cache_url ()
{
  return "resource/cache";
}


bool resource_cache_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


std::string resource_cache (Webserver_Request& webserver_request)
{
  std::string resource = webserver_request.query ["resource"];
  
  
  std::string page;
  Assets_Header header = Assets_Header (menu_logic_resources_text (), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;

  
  if (webserver_request.query.count ("clear")) {
    sendreceive_resources_clear_all ();
  }
  std::vector <std::string> resources = database::config::general::get_resources_to_cache ();
  if (!resources.empty ()) {
    view.enable_zone ("scheduled");
    view.set_variable ("scheduled", filter::strings::implode (resources, " | "));
  }
  
  
#ifdef HAVE_CLIENT
  if (!client_logic_client_enabled ()) {
    view.enable_zone ("clientdisconnected");
  }
#endif

  
  // The first resources listed will be the active one from the resource organizer.
  // They will be listed in the order the user has determined.
  // Then there's a horizontal line dividing the rest of the list.
  // This makes it faster to spot any of the active resources in the long list.

  
  std::map <std::string, std::string> resource_types;
  std::string sword_type = "sword";
  std::string old_type = "old";
  

  std::vector <std::string> active_resources;
  resources = webserver_request.database_config_user()->getActiveResources ();
  for (auto & resource2 : resources) {
    active_resources.push_back (resource2);
  }

  
  std::vector <std::string> all_resources;
  // USFM resources.
  resources = client_logic_usfm_resources_get ();
  for (auto & resource2 : resources) {
    all_resources.push_back (resource2);
  }
  // External resources.
  if (!config_globals_hide_bible_resources) {
    resources = resource_external_names ();
    for (auto & resource2 : resources) {
      all_resources.push_back (resource2);
    }
  }
  // SWORD resources.
  if (!config_globals_hide_bible_resources) {
    resources = sword_logic_get_available ();
    for (auto & resource2 : resources) {
      resource_types [resource2] = sword_type;
      all_resources.push_back (resource2);
    }
  }
  // Any old USFM resources still available on the client.
  Database_UsfmResources database_usfmresources;
  resources = database_usfmresources.getResources ();
  for (auto & resource2 : resources) {
    resource_types [resource2] = old_type;
    all_resources.push_back (resource2);
  }
  // BibleGateway resources.
  if (!config_globals_hide_bible_resources) {
    resources = resource_logic_bible_gateway_module_list_get ();
    for (auto & resource2 : resources) {
      all_resources.push_back (resource2);
    }
  }
  // StudyLight resources.
  if (!config_globals_hide_bible_resources) {
    resources = resource_logic_study_light_module_list_get ();
    for (auto & resource2 : resources) {
      all_resources.push_back (resource2);
    }
  }

  
  // Generate the resources to be listed.
  std::string horizontal_line = "-----";
  std::vector <std::string> listed_resources = active_resources;
  listed_resources.push_back (horizontal_line);
  for (auto & resource2 : all_resources) {
    if (!in_array (resource2, listed_resources)) listed_resources.push_back (resource2);
  }
  
  
  // Generate html block with the resources.
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  std::string block;
  for (auto & resource2 : listed_resources) {
    // Skip internal Bibles and dividers.
    if (in_array (resource2, bibles)) continue;
    if (resource_logic_is_divider (resource2)) continue;
    block.append ("<p>");
    if (resource2 == horizontal_line) block.append ("<hr>");
    else {
      std::string href (resource2);
      std::string query;
      if (resource_types [resource2] == sword_type) {
        std::string source = sword_logic_get_source (resource2);
        std::string module = sword_logic_get_remote_module (resource2);
        href = sword_logic_get_resource_name (source, module);
      }
      if (resource_types [resource2] == old_type) {
        query = "&old=yes";
      }
      block.append ("<a href=\"download?name=" + href + query + "\">" + resource2 + "</a>");
    }
    block.append ("</p>\n");
  }
  

  // Display the list.
  view.set_variable ("block", block);

  
  page += view.render ("resource", "cache");
  page += assets_page::footer ();
  return page;
}

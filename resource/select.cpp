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


#include <resource/select.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/google.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <resource/sword.h>
#include <sync/logic.h>
#include <access/bible.h>
#include <client/logic.h>
#include <dialog/select.h>
#include <database/usfmresources.h>
#include <database/config/general.h>
#include <lexicon/logic.h>
#include <sword/logic.h>
#include <access/logic.h>
#include <config/globals.h>


std::string resource_select_url ()
{
  return "resource/select";
}


bool resource_select_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_select (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Resources"), webserver_request);
  page = header.run ();
  Assets_View view {};

  
  view.set_variable ("page", resource_logic_selector_page (webserver_request));
  bool is_def = false;
  if (webserver_request.query["type"] == "def") is_def = true;
  if (is_def) view.set_variable ("type", "def");
  std::string caller = resource_logic_selector_caller (webserver_request);
  view.set_variable ("caller", caller);
  

  std::string disconnected_info {};
#ifdef HAVE_CLIENT
  if (!client_logic_client_enabled ()) {
    disconnected_info = translate ("Connect to Bibledit Cloud to have access to the full range of available resources.");
  }
#endif

  
  // The layout of the page to select resources to add should not use <select> elements,
  // because if the list of resources is long,
  // the <select> makes it hard to look for a resources.
  // See issue https://github.com/bibledit/cloud/issues/1050 for more details.
  // Rather it now uses one web page listing all resources.

  for (const std::string& name : access_bible::bibles (webserver_request)) {
    view.add_iteration ("bible", {std::pair("name", name)});
  }

  {
    std::vector<std::string> usfm_resources;
#ifdef HAVE_CLIENT
    // Client takes resources available from the Cloud.
    usfm_resources = client_logic_usfm_resources_get ();
#else
    // Cloud takes its locally available USFM resources.
    Database_UsfmResources database_usfmresources;
    usfm_resources = database_usfmresources.getResources ();
#endif
    for (const std::string& name : usfm_resources) {
      view.add_iteration ("usfm", {std::pair("name", name)});
    }
  }

  for (const std::string& name : resource_external_get_original_language_resources()) {
    view.add_iteration ("web_orig", {std::pair("name", name)});
  }

  for (const std::string& name : resource_external_get_bibles()) {
    view.add_iteration ("web_bibles", {std::pair("name", name)});
  }

  for (const std::string& name : lexicon_logic_resource_names()) {
    view.add_iteration ("lexicon", {std::pair("name", name)});
  }
  
  for (const std::string& name : sword_logic_get_available()) {
    view.add_iteration ("sword", {std::pair("name", name)});
  }

  {
    const std::vector <std::string> resources = {
      resource_logic_yellow_divider (),
      resource_logic_green_divider (),
      resource_logic_blue_divider (),
      resource_logic_violet_divider (),
      resource_logic_red_divider (),
      resource_logic_orange_divider (),
      resource_logic_rich_divider(),
    };
    for (const std::string& name : resources) {
      view.add_iteration ("divider", {std::pair("name", name)});
    }
  }

  for (const std::string& name : resource_logic_bible_gateway_module_list_get()) {
    view.add_iteration ("biblegateway", {std::pair("name", name)});
  }

  for (const std::string& name : resource_logic_study_light_module_list_get()) {
    view.add_iteration ("studylight", {std::pair("name", name)});
  }
  
  // The comparative resources are stored as one resource per line.
  // The line contains multiple properties of the resource.
  // One of those properties is the title.
  // This selection mechanism here shows that title only.
  {
    std::vector <std::string> resources;
    std::vector<std::string> raw_resources =
#ifdef HAVE_CLOUD
    database::config::general::get_comparative_resources ();
#else
    resource_logic_comparative_resources_get_list_on_client ();
#endif
    for (const auto & raw_resource : raw_resources) {
      std::string title {};
      if (resource_logic_parse_comparative_resource (raw_resource, &title)) {
        resources.push_back(title);
      }
    }
    for (const std::string& name : resources) {
      view.add_iteration ("comparative", {std::pair("name", name)});
    }
  }
 
  // The translated resources are stored as one resource per line.
  // The line contains multiple properties of the resource.
  // One of those properties is the title.
  // This selection mechanism here shows that title only.
  {
    std::vector <std::string> resources;
    std::vector<std::string> raw_resources =
#ifdef HAVE_CLOUD
    database::config::general::get_translated_resources ();
#else
    resource_logic_translated_resources_get_list_on_client ();
#endif
    for (const auto & raw_resource : raw_resources) {
      std::string title {};
      if (resource_logic_parse_translated_resource (raw_resource, &title)) {
        resources.push_back(title);
      }
    }
    for (const std::string& name : resources) {
      view.add_iteration ("translated", {std::pair("name", name)});
    }
  }

  // Whether to show or to hide sensitive Bible resources.
  // This is to accomodate the followers of Jesus in countries where they could be in danger.
  if (!config_globals_hide_bible_resources) {
    view.enable_zone ("sensitive_bible_resource");
  }

  // If Google Translate has not yet been set up, then enable a bit of information about that.
  // Do this only in the Cloud.
#ifdef HAVE_CLOUD
  auto [ json_key, json_error ] = filter::google::get_json_key_value_error ();
  if (json_key.empty()) view.enable_zone("setup_translated");
#endif

  
  page += view.render ("resource", "select");
  page += assets_page::footer ();
  return page;
}

/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <workspace/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <edit/index.h>
#include <edit/index.h>
#include <editone/index.h>
#include <editusfm/index.h>
#include <search/index.h>
#include <resource/index.h>
#include <notes/index.h>
#include <consistency/index.h>
#include <sync/logic.h>
#include <locale/translate.h>
#include <database/logs.h>
#include <database/cache.h>
#include <read/index.h>


std::vector <std::string> workspace_get_default_names ()
{
  // Any of the names below should not contain commas,
  // because the sorting mechanism takes the comma as a separator,
  // so if commas are in a name, the sorting no longer works.
  return {
    translate ("Editor and Resources"),
    translate ("Editor and Notes"),
    translate ("Resources and Editor and Notes"),
    translate ("Editor and Consistency tool"),
    translate ("Visual editor and USFM editor")
  };
}


std::map <int, std::string> workspace_get_default_urls (int id)
{
  std::map <int, std::string> urls {};
  switch (id) {
    case 1:
      urls [0] = editone_index_url ();
      urls [5] = resource_index_url ();
      break;
    case 2:
      urls [0] = editone_index_url ();
      urls [1] = notes_index_url ();
      break;
    case 3:
      urls [0] = resource_index_url ();
      urls [1] = editone_index_url ();
      urls [2] = notes_index_url ();
      break;
    case 4:
      urls [0] = editone_index_url ();
      urls [1] = consistency_index_url ();
      break;
    case 5:
      urls [0] = resource_index_url ();
      urls [1] = editone_index_url ();
      urls [5] = editusfm_index_url ();
      break;
    default:
      urls [0] = editone_index_url ();
      urls [1] = resource_index_url ();
      urls [2] = notes_index_url ();
      urls [3] = search_index_url ();
      break;
  }
  return urls;
}


std::map <int, std::string> workspace_get_default_widths (int id)
{
  std::map <int, std::string> widths;
  switch (id) {
    case 1:
      widths [0] = "1";
      widths [5] = "1";
      break;
    case 2:
      widths [0] = "3";
      widths [1] = "1";
      break;
    case 3:
      widths [0] = "3";
      widths [1] = "4";
      widths [2] = "2";
      break;
    case 4:
      widths [0] = "1";
      widths [1] = "1";
      break;
    case 5:
      widths [0] = "1";
      widths [1] = "1";
      widths [5] = "1";
      break;
    default:
      widths [0] = "1";
      widths [1] = "1";
      widths [2] = "1";
      widths [3] = "1";
      break;
  }
  return widths;
}


std::map <int, std::string> workspace_get_default_heights (int id)
{
  std::map <int, std::string> heights;
  switch (id) {
    case 1:
      heights [0] = "1";
      heights [1] = "3";
      break;
    case 2:
    case 3:
    case 4:
      heights [0] = "1";
      break;
    case 5:
      heights [0] = "9";
      heights [1] = "1";
      break;
    default:
      heights [0] = "1";
      break;
  }
  return heights;
}


void workspace_create_defaults (Webserver_Request& webserver_request)
{
  // Save current active workspace.
  std::string workspace = webserver_request.database_config_user()->get_active_workspace ();

  // Create or update the default workspaces.
  const std::vector <std::string> names = workspace_get_default_names ();
  for (unsigned int i = 0; i < names.size (); i++) {
    webserver_request.database_config_user()->set_active_workspace (names [i]);
    int bench = static_cast<int>(i + 1);
    workspace_set_urls (webserver_request, workspace_get_default_urls (bench));
    workspace_set_widths (webserver_request, workspace_get_default_widths (bench));
    workspace_set_heights (webserver_request, workspace_get_default_heights (bench));
  }

  // Restore current active workspace.
  webserver_request.database_config_user()->set_active_workspace (workspace);
}


std::string workspace_get_active_name (Webserver_Request& webserver_request)
{
  std::string workspace = webserver_request.database_config_user()->get_active_workspace ();

  if (workspace.empty ()) {
    workspace = workspace_get_default_name ();
  }
  return workspace;
}


// This function processes the units for a $length value.
std::string workspace_process_units (std::string length)
{
  // If a size factor is found, great, otherwise default to 1
  if (length == std::to_string (filter::string::convert_to_int (length))) {
    return length;
  }
  return "1";
}


#define URLS 1
#define WIDTHS 2
#define HEIGHTS 3
#define ENTIREWIDTH 4


void workspace_set_values (Webserver_Request& webserver_request, int selector, const std::map <int, std::string> & values)
{
  // Store values locally, and for a client, store them also for sending to the server.
  std::string workspace = workspace_get_active_name (webserver_request);
  std::string rawvalue;
  if (selector == URLS) rawvalue = webserver_request.database_config_user()->get_workspace_urls ();
  if (selector == WIDTHS) rawvalue = webserver_request.database_config_user()->get_workspace_widths ();
  if (selector == HEIGHTS) rawvalue = webserver_request.database_config_user()->get_workspace_heights ();
  if (selector == ENTIREWIDTH) rawvalue = webserver_request.database_config_user()->get_entire_workspace_widths ();
  std::vector <std::string> currentlines = filter::string::explode (rawvalue, '\n');
  std::vector <std::string> newlines;
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) {
      newlines.push_back (line);
    }
  }
  for (auto & element : values) {
    std::string line = workspace + "_" + std::to_string (element.first) + "_" + element.second;
    newlines.push_back (line);
  }
  rawvalue = filter::string::implode (newlines, "\n");
  if (selector == URLS) {
    webserver_request.database_config_user()->set_workspace_urls (rawvalue);
    workspace_cache_for_cloud (webserver_request, true, false, false);
  }
  if (selector == WIDTHS) {
    webserver_request.database_config_user()->set_workspace_widths (rawvalue);
    workspace_cache_for_cloud (webserver_request, false, true, false);
  }
  if (selector == HEIGHTS) {
    webserver_request.database_config_user()->set_workspace_heights (rawvalue);
    workspace_cache_for_cloud (webserver_request, false, false, true);
  }
  if (selector == ENTIREWIDTH) {
    webserver_request.database_config_user()->set_entire_workspace_widths (rawvalue);
    workspace_cache_for_cloud (webserver_request, false, true, false);
  }
}


void workspace_set_urls (Webserver_Request& webserver_request, const std::map <int, std::string> & values)
{
  // Get current order of the workspaces.
  std::vector <std::string> order = workspace_get_names (webserver_request);
  // Update the values: This reorders the workspaces.
  workspace_set_values (webserver_request, URLS, values);
  // Put the workspaces in the original order.
  workspace_reorder (webserver_request, order);
}


void workspace_set_widths (Webserver_Request& webserver_request, const std::map <int, std::string> & values)
{
  workspace_set_values (webserver_request, WIDTHS, values);
}


void workspace_set_heights (Webserver_Request& webserver_request, const std::map <int, std::string> & values)
{
  workspace_set_values (webserver_request, HEIGHTS, values);
}


void workspace_set_entire_width (Webserver_Request& webserver_request, std::string value)
{
  std::map <int, std::string> values = {std::pair (0, value)};
  workspace_set_values (webserver_request, ENTIREWIDTH, values);
}


std::map <int, std::string> workspace_get_values (Webserver_Request& webserver_request, int selector, bool use)
{
  std::map <int, std::string> values;
  
  std::string workspace = workspace_get_active_name (webserver_request);
  
  std::string rawvalue;
  if (selector == URLS) rawvalue = webserver_request.database_config_user()->get_workspace_urls ();
  if (selector == WIDTHS) rawvalue = webserver_request.database_config_user()->get_workspace_widths ();
  if (selector == HEIGHTS) rawvalue = webserver_request.database_config_user()->get_workspace_heights ();
  if (selector == ENTIREWIDTH) rawvalue = webserver_request.database_config_user()->get_entire_workspace_widths ();
  std::vector <std::string> lines = filter::string::explode (rawvalue, '\n');
  for (auto & line : lines) {
    if (line.find (workspace + "_") == 0) {
      std::vector <std::string> bits = filter::string::explode (line, '_');
      if (bits.size() == 3) {
        int key = filter::string::convert_to_int (bits [1]);
        std::string value = bits [2];
        values [key] = value;
      }
    }
  }
  
  if (values.empty ()) {
    if (selector == URLS) values = workspace_get_default_urls (0);
    if (selector == WIDTHS) values = workspace_get_default_widths (0);
    if (selector == HEIGHTS) values = workspace_get_default_heights (0);
    if (selector == ENTIREWIDTH) values.clear ();
  }

  for (auto & element : values) {
    
    if ((selector == URLS) && use) {
  
      // Add query value for suppressing the topbar as the workspace already has one.
      if (!element.second.empty()) {
        element.second = filter_url_build_http_query(element.second, {{"topbar", "0"}});
      }
      
      // Transform the internal URLs to full ones.
      std::vector <std::string> bits = filter::string::explode (element.second, '/');
      if (bits.size() == 2) {
        element.second.insert (0, "/");
      }
      
      // Encode URL.
      element.second = filter::string::replace (" ", "%20", element.second);
    }

    if (selector == WIDTHS) {
      // Fix the units.
      element.second = workspace_process_units (element.second);
    }

    if (selector == HEIGHTS) {
      // Fix the units.
      element.second = workspace_process_units (element.second);
    }

  }

  return values;
}


std::map <int, std::string> workspace_get_urls (Webserver_Request& webserver_request, bool use)
{
  return workspace_get_values (webserver_request, URLS, use);
}


std::map <int, std::string> workspace_get_widths (Webserver_Request& webserver_request)
{
  return workspace_get_values (webserver_request, WIDTHS, false);
}


std::map <int, std::string> workspace_get_heights (Webserver_Request& webserver_request)
{
  return workspace_get_values (webserver_request, HEIGHTS, false);
}


std::string workspace_get_entire_width (Webserver_Request& webserver_request)
{
  std::map <int, std::string> values = workspace_get_values (webserver_request, ENTIREWIDTH, false);
  std::string width;
  for (auto & element : values) {
    width = element.second;
  }
  return width;;
}


// Returns the names of the available workspaces.
// If $add_default, if there's no workspaces, it adds a default one.
std::vector <std::string> workspace_get_names (Webserver_Request& webserver_request, bool add_default)
{
  std::vector <std::string> workspaces;
  // The names and the order of the workspaces is taken from the URLs.
  std::string rawvalue = webserver_request.database_config_user()->get_workspace_urls ();
  std::vector <std::string> lines = filter::string::explode (rawvalue, '\n');
  for (auto & line : lines) {
    std::vector <std::string> bits = filter::string::explode (line, '_');
    if (bits.size() == 3) {
      if (find (workspaces.begin(), workspaces.end(), bits[0]) == workspaces.end()) {
        workspaces.push_back (bits[0]);
      }
    }
  }
  if (workspaces.empty () && add_default) {
    workspaces.push_back (workspace_get_active_name (webserver_request));
  }
  return workspaces;
}


void workspace_delete (Webserver_Request& webserver_request, std::string workspace)
{
  std::string rawvalue;
  std::vector <std::string> currentlines;
  std::vector <std::string> newlines;
  
  rawvalue = webserver_request.database_config_user()->get_workspace_urls ();
  currentlines = filter::string::explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter::string::implode (newlines, "\n");
  webserver_request.database_config_user()->set_workspace_urls (rawvalue);
  
  rawvalue = webserver_request.database_config_user()->get_workspace_widths ();
  currentlines = filter::string::explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter::string::implode (newlines, "\n");
  webserver_request.database_config_user()->set_workspace_widths (rawvalue);
  
  rawvalue = webserver_request.database_config_user()->get_workspace_heights ();
  currentlines = filter::string::explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter::string::implode (newlines, "\n");
  webserver_request.database_config_user()->set_workspace_heights (rawvalue);
  
  webserver_request.database_config_user()->set_active_workspace ("");
  
  // For a client, store the setting for sending to the server.
  workspace_cache_for_cloud (webserver_request, true, true, true);
}


// This orders the workspaces.
// It takes the order as in array $workspaces.
void workspace_reorder (Webserver_Request& webserver_request, const std::vector <std::string> & workspaces)
{
  // The order of the workspaces is taken from the URLs.
  // Widths and heights are not considered for the order.
  
  // Retrieve the old order of the workspaces, plus their details.
  std::string rawvalue = webserver_request.database_config_user()->get_workspace_urls ();
  std::vector <std::string> oldlines = filter::string::explode (rawvalue, '\n');
  
  // Create vector with the sorted workspace definitions.
  std::vector <std::string> newlines;
  for (auto & workspace : workspaces) {
    for (auto & line : oldlines) {
      if (line.find (workspace + "_") == 0) {
        newlines.push_back (line);
        line.clear ();
      }
    }
  }
  
  // Add any extra ones.
  for (auto & line : oldlines) {
    if (!line.empty ()) {
      newlines.push_back (line);
    }
  }

  // Save everything.
  rawvalue = filter::string::implode (newlines, "\n");
  webserver_request.database_config_user()->set_workspace_urls (rawvalue);

  // Schedule for sending to Cloud.
  workspace_cache_for_cloud (webserver_request, true, false, false);
}


// Copy workspace $source to $destination
void workspace_copy (Webserver_Request& webserver_request, std::string source, std::string destination)
{
  // Save current active workspace.
  std::string active_workspace = webserver_request.database_config_user()->get_active_workspace ();
  
  // Copy source workspace to destination.
  webserver_request.database_config_user()->set_active_workspace (source);
  std::map <int, std::string> urls = workspace_get_urls (webserver_request, false);
  std::map <int, std::string> widths = workspace_get_widths (webserver_request);
  std::map <int, std::string> heights = workspace_get_heights (webserver_request);
  std::string entire_width = workspace_get_entire_width (webserver_request);
  webserver_request.database_config_user()->set_active_workspace (destination);
  workspace_set_urls (webserver_request, urls);
  workspace_set_widths (webserver_request, widths);
  workspace_set_heights (webserver_request, heights);
  workspace_set_entire_width (webserver_request, entire_width);
  
  // Restore current active workspace.
  webserver_request.database_config_user()->set_active_workspace (active_workspace);
}


// Store updated workspace settings for sending to the cloud.
void workspace_cache_for_cloud ([[maybe_unused]] Webserver_Request& webserver_request,
                                [[maybe_unused]] bool urls,
                                [[maybe_unused]] bool widths,
                                [[maybe_unused]] bool heights)
{
#ifdef HAVE_CLIENT
  // For a client, store the setting for sending to the server.
  if (urls)
    webserver_request.database_config_user()->add_updated_setting (Sync_Logic::settings_send_workspace_urls);
  if (widths)
    webserver_request.database_config_user()->add_updated_setting (Sync_Logic::settings_send_workspace_widths);
  if (heights)
    webserver_request.database_config_user()->add_updated_setting (Sync_Logic::settings_send_workspace_heights);
#endif
}


std::string workspace_get_default_name ()
{
  return "Default";
}


// Send the named $workspace to a $user name.
void workspace_send (Webserver_Request& webserver_request, std::string workspace, std::string user)
{
  // Save current active workspace.
  std::string active_workspace = webserver_request.database_config_user()->get_active_workspace ();
  
  // Retrieve settings for the $workspace of the current user.
  webserver_request.database_config_user()->set_active_workspace (workspace);
  std::map <int, std::string> urls = workspace_get_urls (webserver_request, false);
  std::map <int, std::string> widths = workspace_get_widths (webserver_request);
  std::map <int, std::string> heights = workspace_get_heights (webserver_request);
  std::string entire_width = workspace_get_entire_width (webserver_request);
  
  // Restore current active workspace.
  webserver_request.database_config_user()->set_active_workspace (active_workspace);

  // New webserver request object for the destination user.
  Webserver_Request destination_request;
  destination_request.session_logic ()->set_username (user);
  
  // Save workspace for destination user.
  active_workspace = destination_request.database_config_user()->get_active_workspace ();
  destination_request.database_config_user()->set_active_workspace (workspace);
  
  // Copy source workspace to destination.
  workspace_set_urls (destination_request, urls);
  workspace_set_widths (destination_request, widths);
  workspace_set_heights (destination_request, heights);
  workspace_set_entire_width (destination_request, entire_width);

  // Restore workspace for the destination user.
  webserver_request.database_config_user()->set_active_workspace (active_workspace);
}


// https://github.com/bibledit/cloud/issues/342
// One workspace can only have one editable Bible editor.
// The first Bible editor remains editable.
// Any subsequent Bible editors will be set read-only.
// This is related to focused caret jumping that leads to confusion.
// See also issue https://github.com/bibledit/cloud/issues/391 for
// why the editor numbers are passed to the iframes.
// The reason is that each editor's Javascript can determine
// which Bible editor number it is.
// It can then decide to make the editor read-only.
std::map <int, int> workspace_add_bible_editor_number (const std::map <int,std::string>& urls)
{
  std::map <int, int> editor_numbers;
  int bible_editor_count = 0;
  for (const auto& element : urls) {
    bool is_bible_editor = false;
    const std::string& url = element.second;
    if (url.empty()) continue;
    if (url.find (edit_index_url ()) != std::string::npos) is_bible_editor = true;
    if (url.find (editone_index_url ()) != std::string::npos) is_bible_editor = true;
    if (url.find (editusfm_index_url ()) != std::string::npos) is_bible_editor = true;
    if (is_bible_editor) {
      bible_editor_count++;
      if (bible_editor_count > 1) {
        Database_Logs::log ("Setting Bible editor " + url + " as editor number " + std::to_string (bible_editor_count));
      }
      editor_numbers [element.first] = bible_editor_count;
    }
  }
  return editor_numbers;
}



// Input: List of URLs like this: /a/b?bible=1&topbar=0
std::optional<std::string> get_first_bible_from_urls (const std::map <int,std::string>& urls)
{
  for (const auto& [key, url] : urls) {
    const std::vector <std::string> bits = filter::string::explode (url, '?');
    if (bits.size() != 2)
      continue;
    if (!bits.at(1).empty ()) {
      // Explode the data on the ampersand ( & ) and then on the equal sign ( = ).
      std::vector<std::string> keys_values = filter::string::explode(bits.at(1), '&');
      for (const auto& fragment : keys_values) {
        std::vector<std::string> key_value = filter::string::explode(fragment, '=');
        if (key_value.size() == 2)
          if (key_value.at(0) == "bible")
            return filter_url_urldecode (key_value.at(1));
      }
    }
  }
  return std::nullopt;
}

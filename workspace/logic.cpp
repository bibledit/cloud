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


#include <workspace/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <edit/index.h>
#include <editone/index.h>
#include <editverse/index.h>
#include <search/index.h>
#include <resource/index.h>
#include <notes/index.h>
#include <consistency/index.h>
#include <sync/logic.h>
#include <locale/translate.h>


vector <string> workspace_get_default_names ()
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


map <int, string> workspace_get_default_urls (int id)
{
  map <int, string> urls;
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
      urls [5] = editverse_index_url ();
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


map <int, string> workspace_get_default_widths (int id)
{
  map <int, string> widths;
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


map <int, string> workspace_get_default_heights (int id)
{
  map <int, string> heights;
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


void workspace_create_defaults (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Save current active desktop.
  string desktop = request->database_config_user()->getActiveWorkspace ();

  // Create or update the default desktops.
  vector <string> names = workspace_get_default_names ();
  for (unsigned int i = 0; i < names.size (); i++) {
    request->database_config_user()->setActiveWorkspace (names [i]);
    int bench = i + 1;
    workspace_set_urls (request, workspace_get_default_urls (bench));
    workspace_set_widths (request, workspace_get_default_widths (bench));
    workspace_set_heights (request, workspace_get_default_heights (bench));
  }

  // Restore current active desktop.
  request->database_config_user()->setActiveWorkspace (desktop);
}


string workspace_get_active_name (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string workspace = request->database_config_user()->getActiveWorkspace ();
  if (workspace.empty ()) {
    workspace = workspace_get_default_name ();
  }
  return workspace;
}


// This function processes the units for a $length value.
string workspace_process_units (string length)
{
  // If a size factor is found, great, otherwise default to 1
  if (length == convert_to_string (convert_to_int (length))) {
    return length;
  } else {
    string length = "1";
    return length;
  }
}


#define URLS 1
#define WIDTHS 2
#define HEIGHTS 3
#define ENTIREWIDTH 4


void workspace_set_values (void * webserver_request, int selector, const map <int, string> & values)
{
  // Store values locally, and for a client, store them also for sending to the server.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string workspace = workspace_get_active_name (request);
  string rawvalue;
  if (selector == URLS) rawvalue = request->database_config_user()->getWorkspaceURLs ();
  if (selector == WIDTHS) rawvalue = request->database_config_user()->getWorkspaceWidths ();
  if (selector == HEIGHTS) rawvalue = request->database_config_user()->getWorkspaceHeights ();
  if (selector == ENTIREWIDTH) rawvalue = request->database_config_user()->getEntireWorkspaceWidths ();
  vector <string> currentlines = filter_string_explode (rawvalue, '\n');
  vector <string> newlines;
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) {
      newlines.push_back (line);
    }
  }
  for (auto & element : values) {
    string line = workspace + "_" + convert_to_string (element.first) + "_" + element.second;
    newlines.push_back (line);
  }
  rawvalue = filter_string_implode (newlines, "\n");
  if (selector == URLS) {
    request->database_config_user()->setWorkspaceURLs (rawvalue);
    workspace_cache_for_cloud (request, true, false, false);
  }
  if (selector == WIDTHS) {
    request->database_config_user()->setWorkspaceWidths (rawvalue);
    workspace_cache_for_cloud (request, false, true, false);
  }
  if (selector == HEIGHTS) {
    request->database_config_user()->setWorkspaceHeights (rawvalue);
    workspace_cache_for_cloud (request, false, false, true);
  }
  if (selector == ENTIREWIDTH) {
    request->database_config_user()->setEntireWorkspaceWidths (rawvalue);
    workspace_cache_for_cloud (request, false, true, false);
  }
}


void workspace_set_urls (void * webserver_request, const map <int, string> & values)
{
  // Get current order of the workspacees.
  vector <string> order = workspace_get_names (webserver_request);
  // Update the values: This reorders the workspacees.
  workspace_set_values (webserver_request, URLS, values);
  // Put the workspacees in the original order.
  workspace_reorder (webserver_request, order);
}


void workspace_set_widths (void * webserver_request, const map <int, string> & values)
{
  workspace_set_values (webserver_request, WIDTHS, values);
}


void workspace_set_heights (void * webserver_request, const map <int, string> & values)
{
  workspace_set_values (webserver_request, HEIGHTS, values);
}


void workspace_set_entire_width (void * webserver_request, string value)
{
  map <int, string> values = {make_pair (0, value)};
  workspace_set_values (webserver_request, ENTIREWIDTH, values);
}


map <int, string> workspace_get_values (void * webserver_request, int selector, bool use)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  map <int, string> values;
  
  string workspace = workspace_get_active_name (request);
  
  string rawvalue;
  if (selector == URLS) rawvalue = request->database_config_user()->getWorkspaceURLs ();
  if (selector == WIDTHS) rawvalue = request->database_config_user()->getWorkspaceWidths ();
  if (selector == HEIGHTS) rawvalue = request->database_config_user()->getWorkspaceHeights ();
  if (selector == ENTIREWIDTH) rawvalue = request->database_config_user()->getEntireWorkspaceWidths ();
  vector <string> lines = filter_string_explode (rawvalue, '\n');
  for (auto & line : lines) {
    if (line.find (workspace + "_") == 0) {
      vector <string> bits = filter_string_explode (line, '_');
      if (bits.size() == 3) {
        int key = convert_to_int (bits [1]);
        string value = bits [2];
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
      if (element.second != "") {
        element.second = filter_url_build_http_query (element.second, "topbar", "0");
      }
      
      // Transform the internal URLs to full ones.
      vector <string> bits = filter_string_explode (element.second, '/');
      if (bits.size() == 2) {
        element.second.insert (0, "/");
      }
      
      // Encode URL.
      element.second = filter_string_str_replace (" ", "%20", element.second);
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


map <int, string> workspace_get_urls (void * webserver_request, bool use)
{
  return workspace_get_values (webserver_request, URLS, use);
}


map <int, string> workspace_get_widths (void * webserver_request)
{
  return workspace_get_values (webserver_request, WIDTHS, false);
}


map <int, string> workspace_get_heights (void * webserver_request)
{
  return workspace_get_values (webserver_request, HEIGHTS, false);
}


string workspace_get_entire_width (void * webserver_request)
{
  map <int, string> values = workspace_get_values (webserver_request, ENTIREWIDTH, false);
  for (auto & element : values) return element.second;
  return "";
}


// Returns the names of the available workspacees.
vector <string> workspace_get_names (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <string> workspacees;
  // The names and the order of the workspacees is taken from the URLs.
  string rawvalue = request->database_config_user()->getWorkspaceURLs ();
  vector <string> lines = filter_string_explode (rawvalue, '\n');
  for (auto & line : lines) {
    vector <string> bits = filter_string_explode (line, '_');
    if (bits.size() == 3) {
      if (find (workspacees.begin(), workspacees.end(), bits[0]) == workspacees.end()) {
        workspacees.push_back (bits[0]);
      }
    }
  }
  if (workspacees.empty ()) workspacees.push_back (workspace_get_active_name (request));
  return workspacees;
}


void workspace_delete (void * webserver_request, string workspace)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string rawvalue;
  vector <string> currentlines;
  vector <string> newlines;
  
  rawvalue = request->database_config_user()->getWorkspaceURLs ();
  currentlines = filter_string_explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter_string_implode (newlines, "\n");
  request->database_config_user()->setWorkspaceURLs (rawvalue);
  
  rawvalue = request->database_config_user()->getWorkspaceWidths ();
  currentlines = filter_string_explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter_string_implode (newlines, "\n");
  request->database_config_user()->setWorkspaceWidths (rawvalue);
  
  rawvalue = request->database_config_user()->getWorkspaceHeights ();
  currentlines = filter_string_explode (rawvalue, '\n');
  newlines.clear ();
  for (auto & line : currentlines) {
    if (line.find (workspace + "_") != 0) newlines.push_back (line);
  }
  rawvalue = filter_string_implode (newlines, "\n");
  request->database_config_user()->setWorkspaceHeights (rawvalue);
  
  request->database_config_user()->setActiveWorkspace ("");
  
  // For a client, store the setting for sending to the server.
  workspace_cache_for_cloud (request, true, true, true);
}


// This orders the workspacees.
// It takes the order as in array $workspacees.
void workspace_reorder (void * webserver_request, const vector <string> & workspacees)
{
  // The order of the workspacees is taken from the URLs.
  // Widths and heights are not considered for the order.
  
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Retrieve the old order of the workspacees, plus their details.
  string rawvalue = request->database_config_user()->getWorkspaceURLs ();
  vector <string> oldlines = filter_string_explode (rawvalue, '\n');
  
  // Create vector with the sorted workspace definitions.
  vector <string> newlines;
  for (auto & workspace : workspacees) {
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
  rawvalue = filter_string_implode (newlines, "\n");
  request->database_config_user()->setWorkspaceURLs (rawvalue);

  // Schedule for sending to Cloud.
  workspace_cache_for_cloud (request, true, false, false);
}


// Copy desktop $source to $destination
void workspace_copy (void * webserver_request, string source, string destination)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Save current active desktop.
  string active_desktop = request->database_config_user()->getActiveWorkspace ();
  
  // Copy source desktop to destination.
  request->database_config_user()->setActiveWorkspace (source);
  map <int, string> urls = workspace_get_urls (webserver_request, false);
  map <int, string> widths = workspace_get_widths (webserver_request);
  map <int, string> heights = workspace_get_heights (webserver_request);
  string entire_width = workspace_get_entire_width (webserver_request);
  request->database_config_user()->setActiveWorkspace (destination);
  workspace_set_urls (webserver_request, urls);
  workspace_set_widths (webserver_request, widths);
  workspace_set_heights (webserver_request, heights);
  workspace_set_entire_width (webserver_request, entire_width);
  
  // Restore current active desktop.
  request->database_config_user()->setActiveWorkspace (active_desktop);
}


// Store updated workspace settings for sending to the cloud.
void workspace_cache_for_cloud (void * webserver_request, bool urls, bool widths, bool heights)
{
#ifdef HAVE_CLIENT
  // For a client, store the setting for sending to the server.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  if (urls)
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_workspace_urls);
  if (widths)
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_workspace_widths);
  if (heights)
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_workspace_heights);
#else
  (void) webserver_request;
  (void) urls;
  (void) widths;
  (void) heights;
#endif
}


string workspace_get_default_name ()
{
  return "Default";
}


// Send the named $desktop to a $user name.
void workspace_send (void * webserver_request, string desktop, string user)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Save current active desktop.
  string active_desktop = request->database_config_user()->getActiveWorkspace ();
  
  // Retrieve settings for the $desktop of the current user.
  request->database_config_user()->setActiveWorkspace (desktop);
  map <int, string> urls = workspace_get_urls (webserver_request, false);
  map <int, string> widths = workspace_get_widths (webserver_request);
  map <int, string> heights = workspace_get_heights (webserver_request);
  string entire_width = workspace_get_entire_width (webserver_request);
  
  // Restore current active desktop.
  request->database_config_user()->setActiveWorkspace (active_desktop);

  // New webserver request object for the destination user.
  Webserver_Request destination_request;
  destination_request.session_logic ()->setUsername (user);
  
  // Save desktop for destination user.
  active_desktop = destination_request.database_config_user()->getActiveWorkspace ();
  destination_request.database_config_user()->setActiveWorkspace (desktop);
  
  // Copy source desktop to destination.
  workspace_set_urls (&destination_request, urls);
  workspace_set_widths (&destination_request, widths);
  workspace_set_heights (&destination_request, heights);
  workspace_set_entire_width (&destination_request, entire_width);

  // Restore desktop for the destination user.
  request->database_config_user()->setActiveWorkspace (active_desktop);
}

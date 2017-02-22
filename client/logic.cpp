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


#include <client/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/md5.h>
#include <database/config/general.h>
#include <database/users.h>
#include <database/usfmresources.h>
#include <database/logs.h>
#include <sync/setup.h>
#include <client/index.h>
#include <locale/translate.h>


// Returns whether Client mode is enabled.
bool client_logic_client_enabled ()
{
#ifndef HAVE_CLIENT
  return false;
#endif
  return Database_Config_General::getClientMode ();
}


// Sets the Client mode.
// $enable: boolean: true or false.
void client_logic_enable_client (bool enable)
{
  Database_Config_General::setClientMode (enable);
}


// Generates a URL for connecting to Bibledit Cloud.
// $address is the website.
// $port is the port number.
// $path is the path after the website.
string client_logic_url (string address, int port, string path)
{
  return address + ":" + convert_to_string (port) + "/" + path;
}


// This function does the initial connection from the client to the server.
// It receives settings from the server and applies them to the client.
// It returns the level of the user.
// It returns an empty string in case of failure or the response from the server.
string client_logic_connection_setup (string user, string hash)
{
  Database_Users database_users;
  
  if (user.empty ()) {
    vector <string> users = database_users.getUsers ();
    if (users.empty()) return "";
    user = users [0];
    hash = database_users.get_md5 (user);
  }
  
  string encoded_user = bin2hex (user);
  
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  
  string url = client_logic_url (address, port, sync_setup_url ()) + "?user=" + encoded_user + "&pass=" + hash;
  
  string error;
  string response = filter_url_http_get (url, error, true);
  int iresponse = convert_to_int (response);
  
  if ((iresponse >= Filter_Roles::guest ()) && (iresponse <= Filter_Roles::admin ())) {
    // Set user's role on the client to be the same as on the server.
    // Do this only when it differs, to prevent excessive database writes on the client.
    int level = database_users.get_level (user);
    if (iresponse != level) {
      database_users.set_level (user, iresponse);
    }
  } else {
    Database_Logs::log (error, Filter_Roles::translator ());
  }
  
  if (response.empty ()) response = error;
  return response;
}


string client_logic_create_note_encode (string bible, int book, int chapter, int verse,
                                        string summary, string contents, bool raw)
{
  vector <string> data;
  data.push_back (bible);
  data.push_back (convert_to_string (book));
  data.push_back (convert_to_string (chapter));
  data.push_back (convert_to_string (verse));
  data.push_back (summary);
  data.push_back (convert_to_string (raw));
  data.push_back (contents);
  return filter_string_implode (data, "\n");
}


void client_logic_create_note_decode (string data,
                                      string& bible, int& book, int& chapter, int& verse,
                                      string& summary, string& contents, bool& raw)
{
  vector <string> lines = filter_string_explode (data, '\n');
  if (!lines.empty ()) {
    bible = lines [0];
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    book = convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    chapter = convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    verse = convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    summary = lines [0];
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    raw = convert_to_bool (lines [0]);
    lines.erase (lines.begin());
  }
  contents = filter_string_implode (lines, "\n");
}


// This provides a html link to Bibledit Cloud / $path, and displays $linktext.
string client_logic_link_to_cloud (string path, string linktext)
{
  string url;
  if (client_logic_client_enabled ()) {
    string address = Database_Config_General::getServerAddress ();
    int port = Database_Config_General::getServerPort ();
    url = address + ":" + convert_to_string (port);
    if (!path.empty ()) {
      url.append ("/");
      url.append (path);
    }
  } else {
    // Client disconnected: Provide the link and the text to connect to the Cloud.
    url = "/" + client_index_url ();
    linktext.append (" ");
    linktext.append (translate("You are not yet connected to Bibledit Cloud."));
    linktext.append (" ");
    linktext.append (translate("Connect."));
  }

  if (linktext.empty ()) {
    // Empty link text: Select the link itself as the text to display.
    linktext = url;
  }
  
  string link = "<a href=\"" + url + "\">" + linktext + "</a>";
  return link;
}


// Path to the file in the client files area that contains a list of USFM resources on the server.
string client_logic_usfm_resources_path ()
{
  return filter_url_create_root_path ("databases", "client", "usfm_resources.txt");
}


void client_logic_usfm_resources_update ()
{
  // The Cloud stores the list of USFM resources.
  // It is stored in the client files area.
  // Clients can access it from there.
  string path = client_logic_usfm_resources_path ();
  Database_UsfmResources database_usfmresources;
  vector <string> resources = database_usfmresources.getResources ();
  filter_url_file_put_contents (path, filter_string_implode (resources, "\n"));
}


vector <string> client_logic_usfm_resources_get ()
{
  string contents = filter_url_file_get_contents (client_logic_usfm_resources_path ());
  return filter_string_explode (contents, '\n');
}

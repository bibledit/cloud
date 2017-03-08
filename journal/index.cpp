/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <journal/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <webserver/request.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <filter/roles.h>
#include <database/logs.h>
#include <locale/translate.h>
#include <menu/logic.h>
#include <client/logic.h>
#include <locale/logic.h>


const char * journal_index_url ()
{
  return "journal/index";
}


bool journal_index_acl (void * webserver_request)
{
  // In Client mode, anyone can view the journal.
#ifdef HAVE_CLIENT
  return true;
#endif
  // The role of Consultant or higher can view the journal.
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) {
    return true;
  }
  // No access.
  return false;
}


string render_journal_entry (string filename, int userlevel)
{
  // Sample filename: "146495380700927147".
  // The first 10 characters are the number of seconds past the Unix epoch,
  // followed by the number of microseconds within the current second.

  // Get the contents of the file.
  string path = filter_url_create_path (Database_Logs::folder (), filename);
  string entry = filter_url_file_get_contents (path);
  
  // Deal with the user-level of the entry.
  int entryLevel = convert_to_int (entry);
  // Cloud: Only render journal entries of a sufficiently high level.
  // Client: Render journal entries of any level.
#ifndef HAVE_CLIENT
  if (entryLevel > userlevel) return "";
#endif
  (void) userlevel;
  (void) entryLevel;
  // Remove the user's level.
  entry.erase (0, 2);
  
  // Split entry into lines.
  vector <string> lines = filter_string_explode (entry, '\n');
  if (!lines.empty ()) entry = lines [0];
  
  // Sanitize HTML.
  entry = filter_string_sanitize_html (entry);
  
  bool limit = entry.size () > 150;
  if (limit) {
    entry.erase (150);
    entry.append ("...");
  }
    
  // Extract the seconds since the Unix epoch from the filename.
  time_t seconds = convert_to_int (filename.substr (0, 10));
  // Localized date and time stamp.
  string timestamp = locale_logic_date_time (seconds);

  string a_open, a_close;
  if (limit || lines.size () > 1) {
    a_open = "<a href=\"" + filename + "\">";
    a_close = "</a>";
  }
  
  // Done.
  return "<p>" + timestamp + " | " + a_open + entry + a_close + "</p>\n";
}


// Deal with AJAX call for a possible new journal entry.
string journal_index_ajax (Webserver_Request * request, string filename)
{
  int userLevel = request->session_logic()->currentLevel ();
  string result = Database_Logs::next (filename);
  if (!result.empty()) {
    result = render_journal_entry (result, userLevel);
    result.insert (0, filename + "\n");
  }
  return result;
}


string journal_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int userLevel = request->session_logic()->currentLevel ();

  
  string filename = request->query ["filename"];
  if (!filename.empty ()) {
    return journal_index_ajax (request, filename);
  }
  
  
  string expansion = request->query ["expansion"];
  if (!expansion.empty ()) {
    // Get file path.
    expansion = filter_url_basename_web (expansion);
    string path = filter_url_create_path (Database_Logs::folder (), expansion);
    // Get contents of the record.
    expansion = filter_url_file_get_contents (path);
    // Remove the user's level.
    expansion.erase (0, 2);
    // The only formatting currently allowed in the journal is new lines.
    // The rest is sanitized.
    // To do this properly, the order is important:
    // 1. Clean it up.
    expansion = filter_string_sanitize_html (expansion);
    // 2. Convert \n to <br>
    expansion = filter_string_str_replace ("\n", "<br>", expansion);
    // Done.
    return expansion;
  }
  
  
  Assets_Header header = Assets_Header (translate ("Journal"), webserver_request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  string page = header.run ();


  Assets_View view;


  if (request->query.count ("clear")) {
    Database_Logs::clear ();
    // If the logbook has been cleared on a mobile device, and the screen goes off,
    // and then the user activates the screen on the mobile device,
    // the logbook will then again be cleared, because that was the last opened URL.
    // Redirecting the browser to a clean URL fixes this behaviour.
    redirect_browser (request, journal_index_url ());
    return "";
  }

  
  string lastfilename;
  vector <string> records = Database_Logs::get (lastfilename);


  string lines;
  for (string record : records) {
    string rendering = render_journal_entry (record, userLevel);
    if (!rendering.empty ()) {
      lines.append (rendering);
    }
  }
  view.set_variable ("lines", lines);

  
  // Pass the filename of the most recent entry to javascript
  // for use by the AJAX calls for getting subsequent journal entries.
  // It should be passed as a String object in JavaScript.
  // Because when it were passed as an Int, JavaScript would round the value off.
  // And rounding it off often led to double journal entries.
  string script = "var filename = \"" + lastfilename + "\";";
  view.set_variable ("script", script);


  page += view.render ("journal", "index");

  page += Assets_Page::footer ();

  return page;
}


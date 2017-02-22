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


#include <styles/sheets.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <tasks/logic.h>
#include <database/styles.h>
#include <database/logs.h>
#include <styles/css.h>


// Recreates all stylesheet.css files through a background process.
// The advantage of this is that the user interface will be more responsive.
void styles_sheets_create_all ()
{
  tasks_logic_queue (CREATECSS);
}


void styles_sheets_create_all_run ()
{
  Database_Logs::log ("Creating stylesheet.css files");
  Styles_Sheets styles_sheets;
  styles_sheets.recreate ();
}


// Recreates the various stylesheets.css files.
void Styles_Sheets::recreate ()
{
  Database_Styles database_styles;
  vector <string> stylesheets = database_styles.getSheets ();
  for (auto & stylesheet : stylesheets) {
    string path = get_location (stylesheet, false);
    create (stylesheet, path, false, "");
    path = get_location (stylesheet, true);
    create (stylesheet, path, true, "");
  }
}


void Styles_Sheets::create (string stylesheet, string path, bool editor, string export_bible)
{
  Webserver_Request request;
  Styles_Css styles_css = Styles_Css (&request, stylesheet);
  if (editor) {
    styles_css.editor ();
  }
  if (!export_bible.empty ()) {
    styles_css.exports ();
    styles_css.customize (export_bible);
    styles_css.customize ("");
  }
  styles_css.generate ();
  styles_css.css (path);
}


string Styles_Sheets::get_location (string sheet, bool editor)
{
  string path;
  if (editor) path = "editor";
  else path = "basic";
  path.append (sheet);
  path.append (".css");
  path = filter_url_create_root_path ("dyncss", path);
  return path;
}



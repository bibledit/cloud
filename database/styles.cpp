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


#include <database/styles.h>
#include <database/stylesdata.h>
#include <database/sqlite.h>
#include <filter/url.h>
#include <filter/string.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logic.h>


// This is the database for the styles.
// Resilience: It is hardly written to. 
// Chances of corruption are low.
// All default data is stored in the code in memory, not in a database on disk.


// Cache for the default styles.
// It used to store the default cache in code.
// That architecture caused 400+ calls to the localization routines during app startup.
// This cache does not make those calls during app startup.
std::map <std::string, Database_Styles_Item> default_styles_cache;
// The memory cache to speed up reading style values.
// Access a style item like this: cache [stylesheet] [marker].
// Timing a Bibledit setup phase gave this information:
// * Before the cache was implemented, fetching styles took 30 seconds (38%) of the total setup time.
// * After the cache was there, it took 17 seconds (25%) of the total setup time.
std::map <std::string, std::map <std::string, Database_Styles_Item>> database_styles_cache;
// Cache read and write lock.
std::mutex database_styles_cache_mutex;


sqlite3 * Database_Styles::connect ()
{
  return database_sqlite_connect ("styles");
}


void Database_Styles::create ()
{
  // Create database.
  sqlite3 * db = connect ();
  std::string sql;
  sql = "CREATE TABLE IF NOT EXISTS users ("
        "user text,"
        "sheet text"
        ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


// Creates a stylesheet.
void Database_Styles::createSheet (std::string sheet)
{
  // Folder for storing the stylesheet.
  filter_url_mkdir (sheetfolder (sheet));
  // Check and/or load defaults.
  if (default_styles_cache.empty ()) cache_defaults ();
  // Write all style items to file.
  for (auto & mapping : default_styles_cache) {
    auto & item = mapping.second;
    write_item (sheet, item);
  }
}


// Returns an array with the available stylesheets.
std::vector <std::string> Database_Styles::getSheets ()
{
  std::vector <std::string> sheets = filter_url_scandir (databasefolder ());
  if (find (sheets.begin (), sheets.end (), styles_logic_standard_sheet ()) == sheets.end ()) {
    sheets.push_back (styles_logic_standard_sheet ());
  }
  sort (sheets.begin(), sheets.end());
  return sheets;
}


// Deletes a stylesheet.
void Database_Styles::deleteSheet (std::string sheet)
{
  if (!sheet.empty ()) filter_url_rmdir (sheetfolder (sheet));
  database_styles_cache_mutex.lock ();
  database_styles_cache.clear ();
  database_styles_cache_mutex.unlock ();
}


// Adds a marker to the stylesheet.
void Database_Styles::addMarker (std::string sheet, std::string marker)
{
  Database_Styles_Item item = read_item (sheet, marker);
  write_item (sheet, item);
}


// Deletes a marker from a stylesheet.
void Database_Styles::deleteMarker (std::string sheet, std::string marker)
{
  filter_url_unlink (stylefile (sheet, marker));
  database_styles_cache_mutex.lock ();
  database_styles_cache.clear ();
  database_styles_cache_mutex.unlock ();
}


// Returns a map with all the markers and the names of the styles in the stylesheet.
std::map <std::string, std::string> Database_Styles::getMarkersAndNames (std::string sheet)
{
  std::map <std::string, std::string> markers_names;
  std::vector <std::string> markers = getMarkers (sheet);
  for (auto marker : markers) {
    Database_Styles_Item item = read_item (sheet, marker);
    markers_names [marker] = item.name;
  }
  return markers_names;
}


// Returns an array with all the markers of the styles in the stylesheet.
std::vector <std::string> Database_Styles::getMarkers (std::string sheet)
{
  // The markers for this stylesheet.
  std::vector <std::string> markers = filter_url_scandir (sheetfolder (sheet));
  if (markers.empty ()) {
    // Check and/or load defaults.
    if (default_styles_cache.empty ()) cache_defaults ();
    // Load all default markers.
    for (auto & mapping : default_styles_cache) {
      // The markers are the keys in the std::map.
      markers.push_back (mapping.first);
    }
  }
  // Done.
  return markers;
}


// Returns an object with all data belonging to a marker.
Database_Styles_Item Database_Styles::getMarkerData (std::string sheet, std::string marker)
{
  return read_item (sheet, marker);
}


// Updates a style's name.
void Database_Styles::updateName (std::string sheet, std::string marker, std::string name)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.name = name;
  write_item (sheet, item);
}


// Updates a style's info.
void Database_Styles::updateInfo (std::string sheet, std::string marker, std::string info)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.info = info;
  write_item (sheet, item);
}


// Updates a style's category.
void Database_Styles::updateCategory (std::string sheet, std::string marker, std::string category)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.category = category;
  write_item (sheet, item);
}


// Updates a style's type.
void Database_Styles::updateType (std::string sheet, std::string marker, int type)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.type = type;
  write_item (sheet, item);
}


// Updates a style's subtype.
void Database_Styles::updateSubType (std::string sheet, std::string marker, int subtype)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.subtype = subtype;
  write_item (sheet, item);
}


// Updates a style's font size.
void Database_Styles::updateFontsize (std::string sheet, std::string marker, float fontsize)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.fontsize = fontsize;
  write_item (sheet, item);
}


// Updates a style's italic setting.
void Database_Styles::updateItalic (std::string sheet, std::string marker, int italic)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.italic = italic;
  write_item (sheet, item);
}


// Updates a style's bold setting.
void Database_Styles::updateBold (std::string sheet, std::string marker, int bold)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.bold = bold;
  write_item (sheet, item);
}


// Updates a style's underline setting.
void Database_Styles::updateUnderline (std::string sheet, std::string marker, int underline)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.underline = underline;
  write_item (sheet, item);
}


// Updates a style's small caps setting.
void Database_Styles::updateSmallcaps (std::string sheet, std::string marker, int smallcaps)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.smallcaps = smallcaps;
  write_item (sheet, item);
}


void Database_Styles::updateSuperscript (std::string sheet, std::string marker, int superscript)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.superscript = superscript;
  write_item (sheet, item);
}


void Database_Styles::updateJustification (std::string sheet, std::string marker, int justification)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.justification = justification;
  write_item (sheet, item);
}


void Database_Styles::updateSpaceBefore (std::string sheet, std::string marker, float spacebefore)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.spacebefore = spacebefore;
  write_item (sheet, item);
}


void Database_Styles::updateSpaceAfter (std::string sheet, std::string marker, float spaceafter)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.spaceafter = spaceafter;
  write_item (sheet, item);
}


void Database_Styles::updateLeftMargin (std::string sheet, std::string marker, float leftmargin)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.leftmargin = leftmargin;
  write_item (sheet, item);
}


void Database_Styles::updateRightMargin (std::string sheet, std::string marker, float rightmargin)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.rightmargin = rightmargin;
  write_item (sheet, item);
}


void Database_Styles::updateFirstLineIndent (std::string sheet, std::string marker, float firstlineindent)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.firstlineindent = firstlineindent;
  write_item (sheet, item);
}


void Database_Styles::updateSpanColumns (std::string sheet, std::string marker, bool spancolumns)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.spancolumns = spancolumns;
  write_item (sheet, item);
}


void Database_Styles::updateColor (std::string sheet, std::string marker, std::string color)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.color = color;
  write_item (sheet, item);
}


void Database_Styles::updatePrint (std::string sheet, std::string marker, bool print)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.print = print;
  write_item (sheet, item);
}


void Database_Styles::updateUserbool1 (std::string sheet, std::string marker, bool userbool1)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userbool1 = userbool1;
  write_item (sheet, item);
}


void Database_Styles::updateUserbool2 (std::string sheet, std::string marker, bool userbool2)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userbool2 = userbool2;
  write_item (sheet, item);
}


void Database_Styles::updateUserbool3 (std::string sheet, std::string marker, bool userbool3)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userbool3 = userbool3;
  write_item (sheet, item);
}


void Database_Styles::updateUserint1 (std::string sheet, std::string marker, int userint1)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userint1 = userint1;
  write_item (sheet, item);
}


void Database_Styles::updateUserint2 (std::string sheet, std::string marker, int userint2)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userint2 = userint2;
  write_item (sheet, item);
}


void Database_Styles::updateUserstring1 (std::string sheet, std::string marker, std::string userstring1)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userstring1 = userstring1;
  write_item (sheet, item);
}


void Database_Styles::updateUserstring2 (std::string sheet, std::string marker, std::string userstring2)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userstring2 = userstring2;
  write_item (sheet, item);
}


void Database_Styles::updateUserstring3 (std::string sheet, std::string marker, std::string userstring3)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.userstring3 = userstring3;
  write_item (sheet, item);
}


void Database_Styles::updateBackgroundColor (std::string sheet, std::string marker, std::string color)
{
  Database_Styles_Item item = read_item (sheet, marker);
  item.backgroundcolor = color;
  write_item (sheet, item);
}


// Grant $user write access to stylesheet $sheet.
void Database_Styles::grantWriteAccess (std::string user, std::string sheet)
{
  SqliteSQL sql;
  sql.add ("INSERT INTO users VALUES (");
  sql.add (user);
  sql.add (",");
  sql.add (sheet);
  sql.add (");");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Revoke a $user's write access to stylesheet $sheet.
// If the $user is empty, then revoke write access of anybody to that $sheet.
void Database_Styles::revokeWriteAccess (std::string user, std::string sheet)
{
  SqliteSQL sql;
  sql.add ("DELETE FROM users WHERE");
  if (!user.empty ()) {
    sql.add ("user =");
    sql.add (user);
    sql.add ("AND");
  }
  sql.add ("sheet =");
  sql.add (sheet);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Returns true or false depending on whether $user has write access to $sheet.
bool Database_Styles::hasWriteAccess (std::string user, std::string sheet)
{
  SqliteSQL sql;
  sql.add ("SELECT rowid FROM users WHERE user =");
  sql.add (user);
  sql.add ("AND sheet =");
  sql.add (sheet);
  sql.add (";");
  sqlite3 * db = connect ();
  std::map <std::string, std::vector <std::string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  return !result["rowid"].empty ();
}


std::string Database_Styles::databasefolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "styles"});
}


std::string Database_Styles::sheetfolder (std::string sheet)
{
  return filter_url_create_path ({databasefolder (), sheet});
}


std::string Database_Styles::stylefile (std::string sheet, std::string marker)
{
  return filter_url_create_path ({sheetfolder (sheet), marker});
}


// Reads a style from file.
// If the file is not there, it takes the default value.
Database_Styles_Item Database_Styles::read_item (std::string sheet, std::string marker)
{
  Database_Styles_Item item;
  
  // Check whether sheet is in cache.
  bool cache_hit = false;
  database_styles_cache_mutex.lock ();
  if (database_styles_cache.count (sheet)) {
    // Check whether marker is in cache.
    if (database_styles_cache [sheet].count (marker)) {
      // Cache hit: Return the item.
      item = database_styles_cache [sheet] [marker];
      cache_hit = true;
    }
  }
  database_styles_cache_mutex.unlock ();
  if (cache_hit) return item;
  
  // Read the item.
  bool take_default = sheet.empty ();
  std::string filename;
  if (!take_default) {
    filename = stylefile (sheet, marker);
    if (!file_or_dir_exists (filename)) take_default = true;
  }
  if (take_default) {
    // Check and/or load defaults.
    if (default_styles_cache.empty ()) cache_defaults ();
    // Take the default style for the marker.
    if (default_styles_cache.count (marker)) {
      // Get it.
      item = default_styles_cache [marker];
      // Cache it.
      database_styles_cache_mutex.lock ();
      database_styles_cache [sheet] [marker] = item;
      database_styles_cache_mutex.unlock ();
      // Return it.
      return item;
    }
    // Non-standard marker.
    item.marker = marker;
    item.name = translate ("Name");
    item.info = translate ("Information");
  } else {
    // Read the style from file.
    std::string contents = filter_url_file_get_contents (filename);
    std::vector <std::string> lines = filter::strings::explode (contents, '\n');
    for (unsigned int i = 0; i < lines.size(); i++) {
      if (i == 0)  item.marker = lines [i];
      if (i == 1)  item.name = lines [i];
      if (i == 2)  item.info = lines [i];
      if (i == 3)  item.category = lines [i];
      if (i == 4)  item.type = filter::strings::convert_to_int (lines [i]);
      if (i == 5)  item.subtype = filter::strings::convert_to_int (lines [i]);
      if (i == 6)  item.fontsize = filter::strings::convert_to_float (lines [i]);
      if (i == 7)  item.italic = filter::strings::convert_to_int (lines [i]);
      if (i == 8)  item.bold = filter::strings::convert_to_int (lines [i]);
      if (i == 9)  item.underline = filter::strings::convert_to_int (lines [i]);
      if (i == 10) item.smallcaps = filter::strings::convert_to_int (lines [i]);
      if (i == 11) item.superscript = filter::strings::convert_to_int (lines [i]);
      if (i == 12) item.justification = filter::strings::convert_to_int (lines [i]);
      if (i == 13) item.spacebefore = filter::strings::convert_to_float (lines [i]);
      if (i == 14) item.spaceafter = filter::strings::convert_to_float (lines [i]);
      if (i == 15) item.leftmargin = filter::strings::convert_to_float (lines [i]);
      if (i == 16) item.rightmargin = filter::strings::convert_to_float (lines [i]);
      if (i == 17) item.firstlineindent = filter::strings::convert_to_float (lines [i]);
      if (i == 18) item.spancolumns = filter::strings::convert_to_bool (lines [i]);
      if (i == 19) item.color = lines [i];
      if (i == 20) item.print = filter::strings::convert_to_bool (lines [i]);
      if (i == 21) item.userbool1 = filter::strings::convert_to_bool (lines [i]);
      if (i == 22) item.userbool2 = filter::strings::convert_to_bool (lines [i]);
      if (i == 23) item.userbool3 = filter::strings::convert_to_bool (lines [i]);
      if (i == 24) item.userint1 = filter::strings::convert_to_int (lines [i]);
      if (i == 25) item.userint2 = filter::strings::convert_to_int (lines [i]);
      if (i == 26) item.userint3 = filter::strings::convert_to_int (lines [i]);
      if (i == 27) item.userstring1 = lines [i];
      if (i == 28) item.userstring2 = lines [i];
      if (i == 29) item.userstring3 = lines [i];
      if (i == 30) item.backgroundcolor = lines [i];
    }
  }
  
  // Cache the item.
  database_styles_cache_mutex.lock ();
  database_styles_cache [sheet] [marker] = item;
  database_styles_cache_mutex.unlock ();
  
  // Return the item.
  return item;
}


void Database_Styles::write_item (std::string sheet, Database_Styles_Item & item)
{
  // The style is saved to file here.
  // When the style is loaded again from file, the various parts of the style are loaded by line number.
  // Therefore it cannot handle strings with new lines in them.
  // Remove the new lines where appropriate.
  item.name = filter::strings::replace ("\n", " ", item.name);
  item.name = filter::strings::replace ("\r", " ", item.name);
  item.info = filter::strings::replace ("\n", " ", item.info);
  item.info = filter::strings::replace ("\r", " ", item.info);
  // Load the lines.
  std::vector <std::string> lines;
  lines.push_back (item.marker);
  lines.push_back (item.name);
  lines.push_back (item.info);
  lines.push_back (item.category);
  lines.push_back (filter::strings::convert_to_string (item.type));
  lines.push_back (filter::strings::convert_to_string (item.subtype));
  lines.push_back (filter::strings::convert_to_string (item.fontsize));
  lines.push_back (filter::strings::convert_to_string (item.italic));
  lines.push_back (filter::strings::convert_to_string (item.bold));
  lines.push_back (filter::strings::convert_to_string (item.underline));
  lines.push_back (filter::strings::convert_to_string (item.smallcaps));
  lines.push_back (filter::strings::convert_to_string (item.superscript));
  lines.push_back (filter::strings::convert_to_string (item.justification));
  lines.push_back (filter::strings::convert_to_string (item.spacebefore));
  lines.push_back (filter::strings::convert_to_string (item.spaceafter));
  lines.push_back (filter::strings::convert_to_string (item.leftmargin));
  lines.push_back (filter::strings::convert_to_string (item.rightmargin));
  lines.push_back (filter::strings::convert_to_string (item.firstlineindent));
  lines.push_back (filter::strings::convert_to_string (item.spancolumns));
  lines.push_back (item.color);
  lines.push_back (filter::strings::convert_to_string (item.print));
  lines.push_back (filter::strings::convert_to_string (item.userbool1));
  lines.push_back (filter::strings::convert_to_string (item.userbool2));
  lines.push_back (filter::strings::convert_to_string (item.userbool3));
  lines.push_back (filter::strings::convert_to_string (item.userint1));
  lines.push_back (filter::strings::convert_to_string (item.userint2));
  lines.push_back (filter::strings::convert_to_string (item.userint3));
  lines.push_back (item.userstring1);
  lines.push_back (item.userstring2);
  lines.push_back (item.userstring3);
  lines.push_back (item.backgroundcolor);
  // Save.
  std::string data = filter::strings::implode (lines, "\n");
  filter_url_file_put_contents (stylefile (sheet, item.marker), data);
  // Clear cache.
  database_styles_cache_mutex.lock ();
  database_styles_cache.clear ();
  database_styles_cache_mutex.unlock ();
}


void Database_Styles::cache_defaults ()
{
  // The amount of style values in the table.
  unsigned int data_count = sizeof (styles_table) / sizeof (*styles_table);

  // Iterate over the entire table.
  for (unsigned int i = 0; i < data_count; i++) {

    // Get the default values for one USFM marker.
    Database_Styles_Item item;
    item.marker = styles_table[i].marker;
    item.name = styles_table[i].name;
    item.info = styles_table[i].info;
    item.category = styles_table[i].category;
    item.type = styles_table[i].type;
    item.subtype = styles_table[i].subtype;
    item.fontsize = styles_table[i].fontsize;
    item.italic = styles_table[i].italic;
    item.bold = styles_table[i].bold;
    item.underline = styles_table[i].underline;
    item.smallcaps = styles_table[i].smallcaps;
    item.superscript = styles_table[i].superscript;
    item.justification = styles_table[i].justification;
    item.spacebefore = styles_table[i].spacebefore;
    item.spaceafter = styles_table[i].spaceafter;
    item.leftmargin = styles_table[i].leftmargin;
    item.rightmargin = styles_table[i].rightmargin;
    item.firstlineindent = styles_table[i].firstlineindent;
    item.spancolumns = styles_table[i].spancolumns;
    item.color = styles_table[i].color;
    item.print = styles_table[i].print;
    item.userbool1 = styles_table[i].userbool1;
    item.userbool2 = styles_table[i].userbool2;
    item.userbool3 = styles_table[i].userbool3;
    item.userint1 = styles_table[i].userint1;
    item.userint2 = styles_table[i].userint2;
    item.userint3 = styles_table[i].userint3;
    item.userstring1 = styles_table[i].userstring1;
    item.userstring2 = styles_table[i].userstring2;
    item.userstring3 = styles_table[i].userstring3;
    item.backgroundcolor = styles_table[i].backgroundcolor;

    // Store the default item in the global default cache.
    database_styles_cache_mutex.lock ();
    default_styles_cache [item.marker] = item;
    database_styles_cache_mutex.unlock ();
  }
}



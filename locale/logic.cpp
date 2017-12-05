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


#include <locale/logic.h>
#include <locale/translate.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>


// Filters out the default language.
// Returns the $localization, or "" in case of the default language.
string locale_logic_filter_default_language (string localization)
{
  if (localization == "default") localization.clear ();
  return localization;
}


// Takes a month from 1 to 12, and returns its localized name.
string locale_logic_month (int month)
{
  switch (month) {
    case 1:  return translate ("January");
    case 2:  return translate ("February");
    case 3:  return translate ("March");
    case 4:  return translate ("April");
    case 5:  return translate ("May");
    case 6:  return translate ("June");
    case 7:  return translate ("July");
    case 8:  return translate ("August");
    case 9:  return translate ("September");
    case 10: return translate ("October");
    case 11: return translate ("November");
    case 12: return translate ("December");
  }
  return translate ("Month") + " " + convert_to_string (month);
}


string locale_logic_date (int seconds)
{
  seconds = filter_date_local_seconds (seconds);
  int day = filter_date_numerical_month_day (seconds);
  int month = filter_date_numerical_month (seconds);
  int year = filter_date_numerical_year (seconds);
  return convert_to_string (day) + " " + locale_logic_month (month) + " " + convert_to_string (year);
}


string locale_logic_date_time (int seconds)
{
  // Localize the seconds.
  seconds = filter_date_local_seconds (seconds);
  // Convert the seconds into a human readable date and time.
  string timestamp;
  timestamp.append (locale_logic_date (seconds));
  timestamp.append (" ");
  timestamp.append (filter_string_fill (convert_to_string (filter_date_numerical_hour (seconds)), 2, '0'));
  timestamp.append (":");
  timestamp.append (filter_string_fill (convert_to_string (filter_date_numerical_minute (seconds)), 2, '0'));
  timestamp.append (":");
  timestamp.append (filter_string_fill (convert_to_string (filter_date_numerical_second (seconds)), 2, '0'));
  // Done.
  return timestamp;
}


// Return the available localizations.
map <string, string> locale_logic_localizations ()
{
  string directory = filter_url_create_root_path ("locale");
  vector <string> files = filter_url_scandir (directory);
  map <string, string> localizations = {make_pair ("", english ())};
  for (auto file : files) {
    string suffix = filter_url_get_extension (file);
    if (suffix == "po") {
      string basename = filter_string_str_replace ("." + suffix, "", file);
      string path = filter_url_create_path (directory, file);
      string contents = filter_url_file_get_contents (path);
      string language = "Unknown";
      vector <string> lines = filter_string_explode (contents, '\n');
      for (auto line : lines) {
        if (line.find ("translation for bibledit") != string::npos) {
          line.erase (0, 2);
          line.erase (line.length () - 25);
          language = line;
        }
      }
      localizations.insert (make_pair (basename, language));
    }
  }
  return localizations;
}


map <string, string> locale_logic_read_po (string file)
{
  map <string, string> translations;
  string contents = filter_url_file_get_contents (file);
  vector <string> lines = filter_string_explode (contents, '\n');
  string msgid;
  string msgstr;
  int stage = 0;
  for (size_t i = 0; i < lines.size (); i++) {
    string line = filter_string_trim (lines[i]);
    if (line.find ("msgid") == 0) {
      stage = 1;
      line.erase (0, 5);
      line = filter_string_trim (line);
    }
    if (line.find ("msgstr") == 0) {
      stage = 2;
      line.erase (0, 6);
      line = filter_string_trim (line);
    }
    // Build msgid.
    if (stage == 1) {
      if (!line.empty ()) line.erase (0, 1);
      if (!line.empty ()) line.erase (line.length () - 1);
      msgid.append (line);
    }
    // Build msgstr.
    if (stage == 2) {
      if (line.empty ()) stage = 3;
    }
    if (stage == 2) {
      if (!line.empty ()) line.erase (0, 1);
      if (!line.empty ()) line.erase (line.length () - 1);
      msgstr.append (line);
    }
    // Process data.
    if (i == (lines.size () - 1)) stage = 3;
    if (stage == 3) {
      if (!msgid.empty ()) {
        translations [msgid] = msgstr;
      }
      msgid.clear ();
      msgstr.clear ();
    }
  }
  return translations;
}


string locale_logic_text_loaded ()
{
  return translate ("Loaded");
}


string locale_logic_text_will_save ()
{
  return translate ("Will save...");
}


string locale_logic_text_saving ()
{
  return translate ("Saving...");
}


string locale_logic_text_saved ()
{
  return translate ("Saved");
}


string locale_logic_text_retrying ()
{
  return translate ("Retrying...");
}


string locale_logic_text_reformat ()
{
  return translate ("Reformat");
}


string locale_logic_text_no_privileges_modify_book ()
{
  return translate ("You do not have enough privileges to modify this book.");
}


// Returns the Unicode name of the $space.
string locale_logic_space_get_name (string space, bool english)
{
  if (space == " ") {
    if (english) return "space";
    else return translate ("space");
  }
  if (space == non_breaking_space_u00A0 ()) {
    if (english) return "non-breaking space";
    else return translate ("non-breaking space");
  }
  if (space == en_space_u2002 ()) {
    if (english) return "en space";
    else return translate ("en space");
  }
  if (space == figure_space_u2007 ()) {
    if (english) return "figure space";
    else return translate ("figure space");
  }
  if (space == narrow_non_breaking_space_u202F ()) {
    if (english) return "narrow non-breaking space";
    else return translate ("narrow non-breaking space");
  }
  return translate ("unknown");
}


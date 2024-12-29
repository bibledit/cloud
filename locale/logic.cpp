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


#include <locale/logic.h>
#include <locale/translate.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <config/globals.h>


// Takes a month from 1 to 12, and returns its localized name.
std::string locale_logic_month (int month)
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
    default: translate ("Month") + " " + std::to_string (month);
  }
  return std::string();
}


std::string locale_logic_date (int seconds)
{
  seconds = filter::date::local_seconds (seconds);
  int day = filter::date::numerical_month_day (seconds);
  int month = filter::date::numerical_month (seconds);
  int year = filter::date::numerical_year (seconds);
  return std::to_string (day) + " " + locale_logic_month (month) + " " + std::to_string (year);
}


std::string locale_logic_date_time (int seconds)
{
  // Localize the seconds.
  seconds = filter::date::local_seconds (seconds);
  // Convert the seconds into a human readable date and time.
  std::string timestamp;
  timestamp.append (locale_logic_date (seconds));
  timestamp.append (" ");
  timestamp.append (filter::strings::fill (std::to_string (filter::date::numerical_hour (seconds)), 2, '0'));
  timestamp.append (":");
  timestamp.append (filter::strings::fill (std::to_string (filter::date::numerical_minute (seconds)), 2, '0'));
  timestamp.append (":");
  timestamp.append (filter::strings::fill (std::to_string (filter::date::numerical_second (seconds)), 2, '0'));
  // Done.
  return timestamp;
}


// Return the available localizations.
std::map <std::string, std::string> locale_logic_localizations ()
{
  std::string directory = filter_url_create_root_path ({"locale"});
  std::vector <std::string> files = filter_url_scandir (directory);
  std::map <std::string, std::string> localizations = {std::pair (std::string(), filter::strings::english ())};
  for (auto file : files) {
    std::string suffix = filter_url_get_extension (file);
    if (suffix == "po") {
      std::string basename = filter::strings::replace ("." + suffix, "", file);
      std::string path = filter_url_create_path ({directory, file});
      std::string contents = filter_url_file_get_contents (path);
      std::string language = translate ("Unknown");
      std::vector <std::string> lines = filter::strings::explode (contents, '\n');
      for (auto line : lines) {
        if (line.find ("translation for bibledit") != std::string::npos) {
          line.erase (0, 2);
          line.erase (line.length () - 25);
          language = line;
        }
      }
      localizations.insert (std::pair (basename, language));
    }
  }
  return localizations;
}


std::unordered_map <std::string, std::string> locale_logic_read_msgid_msgstr (std::string file)
{
  std::unordered_map <std::string, std::string> translations;
  std::string contents = filter_url_file_get_contents (file);
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  std::string msgid;
  std::string msgstr;
  int stage = 0;
  for (size_t i = 0; i < lines.size (); i++) {
    // Clean the line up.
    std::string line = filter::strings::trim (lines[i]);
    // Skip a comment.
    if (line.find ("#") == 0) continue;
    // Deal with the messages.
    if (line.find ("msgid") == 0) {
      stage = 1;
      line.erase (0, 5);
      line = filter::strings::trim (line);
    }
    if (line.find ("msgstr") == 0) {
      stage = 2;
      line.erase (0, 6);
      line = filter::strings::trim (line);
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


std::string locale_logic_text_loaded ()
{
  return translate ("Loaded");
}


std::string locale_logic_text_will_save ()
{
  return translate ("Will save...");
}


std::string locale_logic_text_updating ()
{
  return translate ("Updating...");
}


std::string locale_logic_text_updated ()
{
  return translate ("Updated");
}


std::string locale_logic_text_saving ()
{
  return translate ("Saving...");
}


std::string locale_logic_text_saved ()
{
  return translate ("Saved");
}


std::string locale_logic_text_retrying ()
{
  return translate ("Retrying...");
}


std::string locale_logic_text_reformat ()
{
  return translate ("Reformat");
}


std::string locale_logic_text_no_privileges_modify_book ()
{
  return translate ("You do not have enough privileges to modify this book.");
}


std::string locale_logic_text_reload ()
{
  return translate("Updated Bible text was loaded.");
}


// Returns the Unicode name of the $space.
std::string locale_logic_space_get_name (std::string space, bool english)
{
  if (space == " ") {
    if (english) return "space";
    else return translate ("space");
  }
  if (space == filter::strings::non_breaking_space_u00A0 ()) {
    if (english) return "non-breaking space";
    else return translate ("non-breaking space");
  }
  if (space == filter::strings::en_space_u2002 ()) {
    if (english) return "en space";
    else return translate ("en space");
  }
  if (space == filter::strings::figure_space_u2007 ()) {
    if (english) return "figure space";
    else return translate ("figure space");
  }
  if (space == filter::strings::narrow_non_breaking_space_u202F ()) {
    if (english) return "narrow non-breaking space";
    else return translate ("narrow non-breaking space");
  }
  return translate ("unknown");
}


std::string locale_logic_deobfuscate (std::string value)
{
  // Replace longest strings first.
  
  // Change "Bbe" to "Bibledit".
  value = filter::strings::replace ("Bbe", "Bibledit", value);

  // Change "bbe" to "bibledit".
  value = filter::strings::replace ("bbe", "bibledit", value);
  
  // Change "Bb" to "Bible".
  value = filter::strings::replace ("Bb", "Bible", value);
  
  // Change "bb" to "bible". This includes "bbe" to "Bibledit".
  value = filter::strings::replace ("bb", "bible", value);
  
  // Done.
  return value;
}


bool locale_logic_obfuscate_compare_internal (const std::string& a, const std::string& b)
{
  return (a.size() > b.size());
}


void locale_logic_obfuscate_initialize ()
{
  // Load the contents of the obfuscation configuration file
  std::string filename = filter_url_create_root_path ({"obfuscate", "texts.txt"});
  std::string contents = filter_url_file_get_contents (filename);
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  
  // Container to map the original string to the obfuscated version.
  std::map <std::string, std::string> original_to_obfuscated;
  
  // Iterate over each line.
  for (auto & line : lines) {

    // Trim lines.
    line = filter::strings::trim (line);
    
    // Skip empty lines.
    if (line.empty ()) continue;
    
    // Skip lines that start with the number sign #.
    size_t pos = line.find ("#");
    if (pos != std::string::npos) if (pos < 3) continue;
    
    // Apart from texts to obfuscate,
    // the settings can also contain a line to hide the Bible resources.
    // Deal with it here.
    pos = line.find ("HideBibleResources");
    if (pos != std::string::npos) {
      config_globals_hide_bible_resources = true;
      continue;
    }
    
    // Lines require the equal sign = once.
    std::vector <std::string> obfuscation_pair = filter::strings::explode (line, '=');
    if (obfuscation_pair.size () != 2) continue;

    // Deobfuscate recognized search terms.
    std::string searchfor = locale_logic_deobfuscate (obfuscation_pair[0]);
    
    // Store the unsorted obfuscation data.
    original_to_obfuscated [searchfor] = obfuscation_pair [1];
    locale_translate_obfuscation_search.push_back (searchfor);
  }

  // Sort the original strings by their lengths.
  // This enables the obfuscation to replace the longest string first, and the shortest last.
  sort (locale_translate_obfuscation_search.begin(), locale_translate_obfuscation_search.end (), locale_logic_obfuscate_compare_internal);
  
  // Store the obfuscation data for use.
  for (auto original : locale_translate_obfuscation_search) {
    locale_translate_obfuscation_replace.push_back (original_to_obfuscated [original]);
  }
}

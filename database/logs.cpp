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


#include <database/logs.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <filter/date.h>
#include <journal/logic.h>


// Bibledit no longer uses a database for storing the journal.
// Reasons that a database is no longer used:
// 1. Simpler system.
// 2. Android has VACUUM errors due to a locked database.


// Records a journal entry.
void Database_Logs::log (string description, int level)
{
  // Trim spaces.
  description = filter_string_trim (description);
  // Discard empty line.
  if (description.empty()) return;
  // Truncate very long entry.
  int length = description.length ();
  if (length > 50000) {
    description.erase (50000);
    description.append ("... This entry was too large and has been truncated: " + convert_to_string (length) + " bytes");
  }
  // Save this logbook entry to a filename with seconds and microseconds.
  string seconds = convert_to_string (filter_date_seconds_since_epoch ());
  string time = seconds + filter_string_fill (convert_to_string (filter_date_numerical_microseconds ()), 8, '0');
  string file = filter_url_create_path (folder (), time);
  // The microseconds granularity depends on the platform.
  // On Windows it is lower than on Linux.
  // There may be the rare case of more than one entry per file.
  // Append the data so it won't overwrite an earlier entry.
  if (file_or_dir_exists (file)) {
    description.insert (0, " | ");
  } else {
    description.insert (0, convert_to_string (level) + " ");
  }
  filter_url_file_put_contents_append (file, description);
#ifdef HAVE_WINDOWS
  // Delay to cover for lower usec granularity on Windows.
  this_thread::sleep_for (chrono::milliseconds (1));
#endif
}


// Records an extended journal entry.
void Database_Logs::log (string subject, string body, int level)
{
  string description (subject);
  description.append ("\n");
  description.append (body);
  log (description, level);
}


void Database_Logs::rotate ()
{
  // Under PHP it used a mechanism that handled huge amounts of entries.
  // The PHP function scandir choked on this or took a very long time.
  // The PHP functions opendir / readdir / closedir handled it better.
  // But now, in C++, with the new journal mechanism, this is no longer relevant.
  string directory = folder ();
  vector <string> files = filter_url_scandir (directory);

  
  // Timestamp for removing older records, depending on whether it's a tiny journal.
#ifdef HAVE_TINY_JOURNAL
  int oldtimestamp = filter_date_seconds_since_epoch () - (14400);
#else
  int oldtimestamp = filter_date_seconds_since_epoch () - (6 * 86400);
#endif

  
  // Limit the journal entry count in the filesystem.
  // This speeds up subsequent reading of the journal by the users.
  // In previous versions of Bibledit, there were certain conditions
  // that led to an infinite loop, as had been noticed at times,
  // and this quickly exhausted the available inodes on the filesystem.
#ifdef HAVE_TINY_JOURNAL
  int limitfilecount = files.size () - 200;
#else
  int limitfilecount = files.size () - 2000;
#endif

  
  bool filtered_entries = false;
  for (unsigned int i = 0; i < files.size(); i++) {
    string path = filter_url_create_path (directory, files [i]);

    // Limit the number of journal entries.
    if ((int)i < limitfilecount) {
      filter_url_unlink (path);
      continue;
    }
    
    // Remove expired entries.
    int timestamp = convert_to_int (files [i].substr (0, 10));
    if (timestamp < oldtimestamp) {
      filter_url_unlink (path);
      continue;
    }

    // Filtering of certain entries.
    string entry = filter_url_file_get_contents (path);
    if (journal_logic_filter_entry (entry)) {
      filtered_entries = true;
      filter_url_unlink (path);
      continue;
    }

  }

  if (filtered_entries) {
    log (journal_logic_filtered_message ());
  }
}


// Get the logbook entries.
vector <string> Database_Logs::get (string & lastfilename)
{
  lastfilename = "0";

  // Read the journal records from the filesystem.
  vector <string> files = filter_url_scandir (folder ());
  for (unsigned int i = 0; i < files.size(); i++) {
    // Last second gets updated based on the filename.
    lastfilename = files [i];
  }

  // Done.  
  return files;
}


// Gets journal entry more recent than "filename".
// Updates "filename" to the item it got.
string Database_Logs::next (string &filename)
{
  vector <string> files = filter_url_scandir (folder ());
  for (unsigned int i = 0; i < files.size (); i++) {
    string file = files [i];
    if (file > filename) {
      filename = file;
      return file;
    }
  }
  return "";
}


// Clears all journal entries.
void Database_Logs::clear ()
{
  string directory = folder ();
  vector <string> files = filter_url_scandir (directory);
  for (auto file : files) {
    filter_url_unlink (filter_url_create_path (directory, file));
  }
  log ("The journal was cleared");
}


// The folder where to store the records.
string Database_Logs::folder ()
{
  return filter_url_create_root_path ("logbook");
}

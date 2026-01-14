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
void Database_Logs::log (std::string description, int level)
{
  // Trim spaces.
  description = filter::string::trim (description);
  // Discard empty line.
  if (description.empty()) return;
  // Truncate very long entry.
  size_t length = description.length ();
  if (length > 50000) {
    description.erase (50000);
    description.append ("... This entry was too large and has been truncated: " + std::to_string (length) + " bytes");
  }
  // Save this logbook entry to a filename with seconds and microseconds.
  std::string seconds = std::to_string (filter::date::seconds_since_epoch ());
  std::string time = seconds + filter::string::fill (std::to_string (filter::date::numerical_microseconds ()), 8, '0');
  std::string file = filter_url_create_path ({folder (), time});
  // The microseconds granularity depends on the platform.
  // On Windows it is lower than on Linux.
  // There may be the rare case of more than one entry per file.
  // Append the data so it won't overwrite an earlier entry.
  if (file_or_dir_exists (file)) {
    description.insert (0, " | ");
  } else {
    description.insert (0, std::to_string (level) + " ");
  }
  filter_url_file_put_contents_append (file, description);
#ifdef HAVE_WINDOWS
  // Delay to cover for lower usec granularity on Windows.
  std::this_thread::sleep_for (std::chrono::milliseconds (1));
#endif
}


// Records an extended journal entry.
void Database_Logs::log (std::string subject, std::string body, int level)
{
  std::string description (subject);
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
  std::string directory = folder ();
  std::vector <std::string> files = filter_url_scandir (directory);

  
  // Timestamp for removing older records, depending on whether it's a tiny journal.
#ifdef HAVE_TINY_JOURNAL
  int oldtimestamp = filter::date::seconds_since_epoch () - (14400);
#else
  int oldtimestamp = filter::date::seconds_since_epoch () - (6 * 86400);
#endif

  
  // Limit the journal entry count in the filesystem.
  // This speeds up subsequent reading of the journal by the users.
  // In previous versions of Bibledit, there were certain conditions
  // that led to an infinite loop, as had been noticed at times,
  // and this quickly exhausted the available inodes on the filesystem.
#ifdef HAVE_TINY_JOURNAL
  const int limitfilecount = static_cast<int>(files.size () - 200);
#else
  const int limitfilecount = static_cast<int>(files.size () - 2000);
#endif

  
  bool filtered_entries = false;
  for (unsigned int i = 0; i < files.size(); i++) {
    std::string path = filter_url_create_path ({directory, files [i]});

    // Limit the number of journal entries.
    if (static_cast<int> (i) < limitfilecount) {
      filter_url_unlink (path);
      continue;
    }
    
    // Remove expired entries.
    int timestamp = filter::string::convert_to_int (files [i].substr (0, 10));
    if (timestamp < oldtimestamp) {
      filter_url_unlink (path);
      continue;
    }

    // Filtering of certain entries.
    std::string entry = filter_url_file_get_contents (path);
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
std::vector <std::string> Database_Logs::get (std::string & lastfilename)
{
  lastfilename = "0";

  // Read the journal records from the filesystem.
  std::vector <std::string> files = filter_url_scandir (folder ());
  for (unsigned int i = 0; i < files.size(); i++) {
    // Last second gets updated based on the filename.
    lastfilename = files [i];
  }

  // Done.  
  return files;
}


// Gets journal entry more recent than "filename".
// Updates "filename" to the item it got.
std::string Database_Logs::next (std::string &filename)
{
  std::vector <std::string> files = filter_url_scandir (folder ());
  for (unsigned int i = 0; i < files.size (); i++) {
    std::string file = files [i];
    if (file > filename) {
      filename = file;
      return file;
    }
  }
  return std::string();
}


// Clears all journal entries.
void Database_Logs::clear ()
{
  std::string directory = folder ();
  std::vector <std::string> files = filter_url_scandir (directory);
  for (auto file : files) {
    filter_url_unlink (filter_url_create_path ({directory, file}));
  }
  log ("The journal was cleared");
}


// The folder where to store the records.
std::string Database_Logs::folder ()
{
  return filter_url_create_root_path ({"logbook"});
}

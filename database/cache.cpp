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


#include <database/cache.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <filter/shell.h>
#include <database/sqlite.h>
#include <database/logs.h>
#include <database/logic.h>
#include <database/config/general.h>


// Databases resilience:
// They contain cached data.
// Rarely written to.
// Often read from.


namespace database::cache::sql {


constexpr auto max_book {100};

static std::string filename (std::string resource, int book)
{
  // Name of the database for this resource.
  resource = filter_url_clean_filename (resource);
  std::string book_fragment;
  if (book) {
    book_fragment = "_" + std::to_string (book);
  }
  return fragment () + resource + book_fragment;
}


std::string fragment ()
{
  return "cache_resource_";
}


std::string path (const std::string& resource, int book)
{
  return filter_url_create_path ({database_logic_databases (), filename (filter_url_urlencode (resource), book) + database::sqlite::suffix ()});
}


void create (const std::string& resource, int book)
{
  SqliteDatabase sql (filename (resource, book));
  
  sql.add ("CREATE TABLE IF NOT EXISTS cache (chapter integer, verse integer, value text);");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("CREATE TABLE IF NOT EXISTS ready (ready boolean);");
  sql.execute ();
}


void remove (const std::string& resource)
{
  for (int book = 0; book < max_book; book++) {
    remove (resource, book);
  }
}


void remove (const std::string& resource, int book)
{
  const std::string file = database::sqlite::get_file (filename (resource, book));
  if (file_or_dir_exists (file)) {
    filter_url_unlink (file);
  }
}


// Returns true if the cache for the $resource exists.
bool exists (const std::string& resource)
{
  for (int book = 0; book < max_book; book++) {
    if (exists (resource, book))
      return true;
  }
  return false;
}


// Returns true if the cache for the $resource $book exists.
bool exists (const std::string& resource, int book)
{
  const std::string file = database::sqlite::get_file (filename (resource, book));
  return file_or_dir_exists (file);
}


// Returns true if a cached value for $resource/book/chapter/verse exists.
bool exists (const std::string& resource, int book, int chapter, int verse)
{
  // If the the book-based cache exists, check existence from there.
  if (exists (resource, book)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, book));
    sql.add ("SELECT count(*) FROM cache WHERE chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    std::vector <std::string> result = sql.query () ["count(*)"];
    int count = 0;
    if (!result.empty ()) 
      count = filter::strings::convert_to_int (result.at(0));
    return (count > 0);
  }
  // Else if the previous cache layout exists, check that.
  if (exists (resource, 0)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, 0));
    sql.add ("SELECT count(*) FROM cache WHERE book =");
    sql.add (book);
    sql.add ("AND chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    std::vector <std::string> result = sql.query () ["count(*)"];
    int count = 0;
    if (!result.empty ()) 
      count = filter::strings::convert_to_int (result [0]);
    return (count > 0);
  }
  // Nothing exists.
  return false;
}


// Caches a value.
void cache (const std::string& resource, int book, int chapter, int verse, const std::string& value)
{
  SqliteDatabase sql (filename (resource, book));
  
  sql.clear ();
  sql.add ("DELETE FROM cache WHERE chapter = ");
  sql.add (chapter);
  sql.add ("AND verse = ");
  sql.add (verse);
  sql.add (";");
  sql.execute ();
  
  sql.clear ();
  sql.add ("INSERT INTO cache VALUES (");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (value);
  sql.add (");");
  sql.execute ();
}


// Retrieves a cached value.
std::string retrieve (const std::string& resource, const int book, const int chapter, const int verse)
{
  // If the the book-based cache exists, retrieve it from there.
  if (exists (resource, book)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, book));
    sql.add ("SELECT value FROM cache WHERE chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    const std::vector <std::string> result = sql.query () ["value"];
    if (result.empty ())
      return std::string();
    return result.at(0);
  }
  // Else if the previous cache layout exists, retrieve it from there.
  if (exists (resource, 0)) {
    SqliteDatabase sql = SqliteDatabase (filename (resource, 0));
    sql.add ("SELECT value FROM cache WHERE book =");
    sql.add (book);
    sql.add ("AND chapter = ");
    sql.add (chapter);
    sql.add ("AND verse = ");
    sql.add (verse);
    sql.add (";");
    const std::vector <std::string> result = sql.query () ["value"];
    if (result.empty ())
      return std::string();
    return result.at(0);
  }
  return std::string();
}


// Returns how many element are in cache $resource.
int count (const std::string& resource)
{
  int count = 0;
  // Book 0 is for the old layout. Book 1++ is for the new layout.
  for (int book = 0; book < max_book; book++) {
    if (exists (resource, book)) {
      count ++;
    }
  }
  return count;
}


// Return true if the database has loaded all its expected content.
bool ready (const std::string& resource, const int book)
{
  SqliteDatabase sql (filename (resource, book));
  sql.add ("SELECT ready FROM ready;");
  const std::vector <std::string> result = sql.query () ["ready"];
  if (!result.empty()) {
    const auto ready = result.at(0);
    return filter::strings::convert_to_bool (ready);
  }
  return false;
}


// Sets the 'ready' flag in the database.
void ready (const std::string& resource, const int book, const bool ready)
{
  SqliteDatabase sql (filename (resource, book));
  
  sql.clear ();
  sql.add ("DELETE FROM ready;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("INSERT INTO ready VALUES (");
  sql.add (ready);
  sql.add (");");
  sql.execute ();
}


int size (const std::string& resource, const int book)
{
  const std::string file = database::sqlite::get_file (filename (resource, book));
  return filter_url_filesize (file);
}


} // namespace.


namespace database::cache::file {


static std::string full_path (std::string file)
{
  return filter_url_create_root_path ({database_logic_databases (), "cache", file});
}


// The purpose of splitting the file up into paths is
// to avoid that the cache folder would contain too many files
// and so would become slow.
static std::string split_file (std::string file)
{
  if (file.size () > 9) file.insert (9, "/");
  if (file.size () > 18) file.insert (18, "/");
  if (file.size () > 27) file.insert (27, "/");
  file.append (".txt");
  return file;
}


bool exists (std::string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = database::cache::file::split_file (schema);
  schema = database::cache::file::full_path (schema);
  return file_or_dir_exists (schema);
}


void put (std::string schema, const std::string& contents)
{
  schema = filter_url_clean_filename (schema);
  schema = split_file (schema);
  schema = full_path (schema);
  const std::string path = filter_url_dirname (schema);
  if (!file_or_dir_exists (path)) 
    filter_url_mkdir (path);
  filter_url_file_put_contents (schema, contents);
}


std::string get (std::string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = database::cache::file::split_file (schema);
  schema = database::cache::file::full_path (schema);
  return filter_url_file_get_contents (schema);
}


void remove (std::string schema)
{
  schema = filter_url_clean_filename (schema);
  schema = split_file (schema);
  schema = full_path (schema);
  filter_url_unlink (schema);
}


// Deletes expired cached items.
void trim (bool clear)
{
  if (clear)
    Database_Logs::log ("Clearing cache");
  
  // The directory that contains the file-based cache files.
  std::string path = database::cache::file::full_path ("");
  
  // Get the free space on the file system that contains the cache.
  std::string output, error;
  filter::shell::run (path, filter::shell::get_executable(filter::shell::Executable::df), {"."}, &output, &error);
  if (!error.empty ())
    Database_Logs::log (error);
  int percentage_disk_in_use {0};
  {
    const std::vector<std::string> bits = filter::strings::explode(output, ' ');
    for (const auto& bit : bits) {
      if (bit.find ("%") != std::string::npos) {
        percentage_disk_in_use = filter::strings::convert_to_int(bit);
        // If a real percentage was found, other than 0, then skip the remainder.
        // On macOS the first percentage found is %iused, so will be skipped.
        if (percentage_disk_in_use != 0)
          break;
      }
    }
  }
  Database_Logs::log ("Disk space in use is " + std::to_string(percentage_disk_in_use) + "%");
  
  // There have been instances that the cache takes up 4, 5, or 6 Gbytes in the Cloud.
  // If the cache is left untrimmed, the size can be even larger.
  // This leads to errors when the disk runs out of space.
  // Therefore it's good to remove cached files older than a couple of hours
  // in cases where disk space is tight.
  // Two hours.
  std::string minutes = "+120";
  // One day.
  if (percentage_disk_in_use < 70)
    minutes = "+1440";
  // One week.
  if (percentage_disk_in_use < 50)
    minutes = "+10080";
  
  // Handle clearing the cache immediately.
  if (clear)
    minutes = "+0";
  
  // Remove files that have not been modified for x minutes.
  // It uses a Linux shell command.
  // This can be done because it runs on the server only.
  // It used to do "cd /path/to/cache; find /path/to/cache ...".
  // This could remove the folder "cache" too.
  // After this folder was removed, no caching could take place anymore.
  // https://github.com/bibledit/cloud/issues/366
  // This was not visible on macOS, but it was visible on Linux.
  // The fix is to do "cd /path/to/cache; find . ...".
  output.clear ();
  error.clear ();
  filter::shell::run (path, filter::shell::get_executable(filter::shell::Executable::find), {".", "-amin", minutes, "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  // Remove empty directories.
  output.clear ();
  error.clear ();
  filter::shell::run (path, filter::shell::get_executable(filter::shell::Executable::find), {".", "-type", "d", "-empty", "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  // The directory that contains the database-based cache files.
  path = filter_url_create_root_path ({database_logic_databases ()});
  
  // The number of days to keep cached data depends on the percentage of the disk in use.
  // There have been instances that the cache takes up 4, 5, or 6 Gbytes in the Cloud.
  // This can be even more.
  // This may lead to errors when the disk runs out of space.
  // Therefore it's good to limit caches more if the space is tight.
  // By default keep the resources cache for 30 days.
  std::string days = "+30";
  // If keeping the resources cache for an extended period of time, keep it for a full year.
  if (database::config::general::get_keep_resources_cache_for_long())
    days = "+365";
  // If free disk space is tighter, keep the caches for a shorter period.
  if (percentage_disk_in_use > 80)
    days = "+14";
  if (percentage_disk_in_use > 85)
    days = "+7";
  if (percentage_disk_in_use > 90)
    days = "+1";
  
  // Handle clearing the cache immediately.
  if (clear)
    days = "0";
  
  Database_Logs::log ("Will remove resource caches not accessed for " + days + " days");
  
  // Remove database-based cached files that have not been modified for x days.
  output.clear ();
  error.clear ();
  filter::shell::run (path, filter::shell::get_executable(filter::shell::Executable::find), {path, "-name", database::cache::sql::fragment () + "*", "-atime", days, "-delete"}, &output, &error);
  if (!output.empty ()) Database_Logs::log (output);
  if (!error.empty ()) Database_Logs::log (error);
  
  if (clear)
    Database_Logs::log ("Ready clearing  cache");
}


} // namespace.


namespace database::cache {


// This returns true if the $html can be cached.
bool can_cache (const std::string& error, const std::string& html)
{
  // Normally if everything is fine, then caching is possible.
  bool cache = true;
  
  // Do not cache the data in an error situation.
  if (!error.empty()) cache = false;
  
  // Do not cache the data if Cloudflare does any kind of protection.
  // https://github.com/bibledit/cloud/issues/693.
  if (html.find ("Cloudflare") != std::string::npos) cache = false;
  
  return cache;
}


} // namespace.




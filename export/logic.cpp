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


#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>


// Schedule all Bibles for exports.
void Export_Logic::scheduleAll ()
{
  tasks_logic_queue (EXPORTALL);
}


// Schedule a Bible book for export to text and basic USFM format.
// $bible: Bible.
// $book: book.
void Export_Logic::scheduleTextAndBasicUsfm (string bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTTEXTUSFM, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule a Bible for export to USFM format.
void Export_Logic::scheduleUsfm (string bible, bool log)
{
  tasks_logic_queue (EXPORTUSFM, {bible, convert_to_string (log)});
}


// Schedule export to OpenDocument.
// $bible: Bible.
void Export_Logic::scheduleOpenDocument (string bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTODT, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule creation info documents.
// $bible: Bible.
void Export_Logic::scheduleInfo (string bible, bool log)
{
  tasks_logic_queue (EXPORTINFO, {bible, convert_to_string (log)});
}


// Schedule export to html.
// $bible: Bible.
void Export_Logic::scheduleHtml (string bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTHTML, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule export to web.
// $bible: Bible.
void Export_Logic::scheduleWeb (string bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTWEBMAIN, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule export to web index.
// $bible: Bible.
void Export_Logic::scheduleWebIndex (string bible, bool log)
{
  tasks_logic_queue (EXPORTWEBINDEX, {bible, convert_to_string (log)});
}


void Export_Logic::scheduleOnlineBible (string bible, bool log)
{
  tasks_logic_queue (EXPORTONLINEBIBLE, {bible, convert_to_string (log)});
}


void Export_Logic::scheduleESword (string bible, bool log)
{
  tasks_logic_queue (EXPORTESWORD, {bible, convert_to_string (log)});
}


void Export_Logic::scheduleQuickBible (string bible, bool log)
{
  tasks_logic_queue (EXPORTQUICKBIBLE, {bible, convert_to_string (log)});
}


// The main exports directory.
string Export_Logic::mainDirectory ()
{
  return filter_url_create_root_path ("exports");
}


// A Bible's export directory.
string Export_Logic::bibleDirectory (string bible)
{
  return filter_url_create_path (mainDirectory (), bible);
}


// Directory for the USFM.
// $type:
// 0: directory for the full USFM.
// 1: directory for the basic USFM.
// 2: root USFM directory.
string Export_Logic::USFMdirectory (string bible, int type)
{
  string directory = filter_url_create_path (bibleDirectory (bible), "usfm");
  switch (type) {
    case 0: directory = filter_url_create_path (directory, "full"); break;
    case 1: directory = filter_url_create_path (directory, "basic"); break;
    default: break;
  }
  return directory;
}


string Export_Logic::webDirectory (string bible)
{
  return filter_url_create_path (bibleDirectory (bible), "web");
}


string Export_Logic::webBackLinkDirectory (string bible)
{
  return "/exports/" + bible + "/web/";
}


// Provides the base book file name, e.g. 01_Genesis.
// Or 00_Bible for an entire Bible when $book = 0;
string Export_Logic::baseBookFileName (int book)
{
  string filename;
  if (book) {
    filename = filter_string_fill (convert_to_string (book), 2, '0') + "_" + Database_Books::getEnglishFromId (book);
  } else {
    filename = "00_Bible";
  }
  return filename;
}

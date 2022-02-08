/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <ipc/focus.h>
#include <webserver/request.h>
#include <database/cache.h>
#include <filter/string.h>


// Sets the focus.
void Ipc_Focus::set (void * webserver_request, int book, int chapter, int verse)
{
  bool set = false;
  if (book != getBook (webserver_request)) set = true;
  if (chapter != getChapter (webserver_request)) set = true;
  if (verse != getVerse (webserver_request)) set = true;
  if (set) {
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

    // Indonesian Cloud Free
    // As Simple version of Indonesian Cloud Free is basically a
    // basic demo version with less feature.
    // So everyone that goes into its instance, always logs in as the
    // user "admin". Everyone shares one user database. This is undesirable.
    // For it to behave as if they are different users, the focused
    // Bible book, chapter, and verse are saved into a file based
    // cache database with the user's IP as plus a unique string of
    // characters based on the data name's initial as the file name.
    if (config_logic_indonesian_cloud_free_simple ()) {
      // Use the client's IP and unique string to identify the data
      // as the file name.
      // Cache the Bible book, chapter, and verse index.
      string book_filename = focused_book_filebased_cache_filename (request->session_identifier);
      database_filebased_cache_put (book_filename, convert_to_string (book));
      string chapter_filename = focused_chapter_filebased_cache_filename (request->session_identifier);
      database_filebased_cache_put (chapter_filename, convert_to_string (chapter));
      string verse_filename = focused_verse_filebased_cache_filename (request->session_identifier);
      database_filebased_cache_put (verse_filename, convert_to_string (verse));
    }


    request->database_config_user()->setFocusedBook (book);
    request->database_config_user()->setFocusedChapter (chapter);
    request->database_config_user()->setFocusedVerse (verse);
  }
}


// Gets the focused book.
int Ipc_Focus::getBook (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int book = request->database_config_user()->getFocusedBook ();
  // Indonesian Cloud Free
  // Gets the Bible book index from a file based cache database.
  if (config_logic_indonesian_cloud_free_simple ()) {
    string filename = focused_book_filebased_cache_filename (request->session_identifier);
    if (database_filebased_cache_exists (filename)) {
      book = convert_to_int (database_filebased_cache_get (filename));
    } else {
      database_filebased_cache_put (filename, "43");
    }
  }
  return book;
}


// Gets the focused chapter.
int Ipc_Focus::getChapter (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int chapter = request->database_config_user()->getFocusedChapter ();
  // Indonesian Cloud Free
  // Gets the chapter index from a file based cache database.
  if (config_logic_indonesian_cloud_free_simple ()) {
    string filename = focused_chapter_filebased_cache_filename (request->session_identifier);
    if (database_filebased_cache_exists (filename)) {
      chapter = convert_to_int (database_filebased_cache_get (filename));
    } else {
      database_filebased_cache_put (filename, "1");
    }
  }
  return chapter;
}


// Gets the focused verse.
int Ipc_Focus::getVerse (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int verse = request->database_config_user()->getFocusedVerse ();
  // Indonesian Cloud Free
  // Gets the verse index from a file based cache database.
  if (config_logic_indonesian_cloud_free_simple ()) {
    string filename = focused_verse_filebased_cache_filename (request->session_identifier);
    if (database_filebased_cache_exists (filename)) {
      verse = convert_to_int (database_filebased_cache_get (filename));
    } else {
      database_filebased_cache_put (filename, "1");
    }
  }
  return verse;
}

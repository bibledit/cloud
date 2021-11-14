/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <checksum/logic.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/usfm.h>
#include <webserver/request.h>


// This function reads $data,
// calculates a checksum,
// adds $readwrite,
// and returns the result as follows:
// The first line contains the checksum.
// The second line contains the readwrite as 0 or 1.
// The rest contains the $data.
string Checksum_Logic::send (string data, bool readwrite)
{
  string checksum = get (data);
  checksum.append ("\n");
  checksum.append (convert_to_string (readwrite));
  checksum.append ("\n");
  checksum.append (data);
  return checksum;
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of 'data' in bytes.
string Checksum_Logic::get (string data)
{
  return convert_to_string (data.length ());
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of vector 'data' in bytes.
string Checksum_Logic::get (const vector <string>& data)
{
  int length = 0;
  for (auto & bit : data) length += bit.length ();
  return convert_to_string (length);
}


// Returns a proper checksum for the USFM in the chapter.
string Checksum_Logic::getChapter (void * webserver_request, string bible, int book, int chapter)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  string checksum = md5 (filter_string_trim (usfm));
  return checksum;
}


// Returns a proper checksum for the USFM in the book.
string Checksum_Logic::getBook (void * webserver_request, string bible, int book)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <int> chapters = request->database_bibles()->getChapters (bible, book);
  vector <string> checksums;
  for (auto & chapter : chapters) {
    checksums.push_back (getChapter (webserver_request, bible, book, chapter));
  }
  string checksum = filter_string_implode (checksums, "");
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the $bible.
string Checksum_Logic::getBible (void * webserver_request, string bible)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <int> books = request->database_bibles()->getBooks (bible);
  vector <string> checksums;
  for (auto & book : books) {
    checksums.push_back (getBook (webserver_request, bible, book));
  }
  string checksum = filter_string_implode (checksums, "");
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the array of $bibles.
string Checksum_Logic::getBibles (void * webserver_request, const vector <string> & bibles)
{
  vector <string> checksums;
  for (auto & bible : bibles) {
    checksums.push_back (getBible (webserver_request, bible));
  }
  string checksum = filter_string_implode (checksums, "");
  checksum = md5 (checksum);
  return checksum;
}


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


#include <checksum/logic.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/usfm.h>
#include <webserver/request.h>
using namespace std;


// This function reads $data,
// calculates a checksum,
// adds $readwrite,
// and returns the result as follows:
// The first line contains the checksum.
// The second line contains the readwrite as 0 or 1.
// The rest contains the $data.
string checksum_logic::send (const std::string& data, bool readwrite)
{
  string checksum = get (data);
  checksum.append ("\n");
  checksum.append (filter::strings::convert_to_string (readwrite));
  checksum.append ("\n");
  checksum.append (data);
  return checksum;
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of 'data' in bytes.
string checksum_logic::get (const std::string& data)
{
  return filter::strings::convert_to_string (data.length ());
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of vector 'data' in bytes.
string checksum_logic::get (const std::vector <string>& data)
{
  int length = 0;
  for (auto & bit : data) length += static_cast<int>(bit.length ());
  return filter::strings::convert_to_string (length);
}


// Returns a proper checksum for the USFM in the chapter.
string checksum_logic::get_chapter (Webserver_Request& webserver_request, const std::string& bible, int book, int chapter)
{
  string usfm = webserver_request.database_bibles()->get_chapter (bible, book, chapter);
  string checksum = md5 (filter::strings::trim (usfm));
  return checksum;
}


// Returns a proper checksum for the USFM in the book.
string checksum_logic::get_book (Webserver_Request& webserver_request, const std::string& bible, int book)
{
  vector <int> chapters = webserver_request.database_bibles()->get_chapters (bible, book);
  std::vector <std::string> checksums;
  for (auto chapter : chapters) {
    checksums.push_back (get_chapter (webserver_request, bible, book, chapter));
  }
  string checksum = filter::strings::implode (checksums, string());
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the $bible.
string checksum_logic::get_bible (Webserver_Request& webserver_request, const std::string& bible)
{
  vector <int> books = webserver_request.database_bibles()->get_books (bible);
  std::vector <std::string> checksums;
  for (auto book : books) {
    checksums.push_back (get_book (webserver_request, bible, book));
  }
  string checksum = filter::strings::implode (checksums, string());
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the array of $bibles.
string checksum_logic::get_bibles (Webserver_Request& webserver_request, const std::vector <string> & bibles)
{
  std::vector <std::string> checksums;
  for (const auto & bible : bibles) {
    checksums.push_back (get_bible (webserver_request, bible));
  }
  string checksum = filter::strings::implode (checksums, string());
  checksum = md5 (checksum);
  return checksum;
}


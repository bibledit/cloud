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


#include <checksum/logic.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/usfm.h>
#include <database/bibles.h>


// This function reads $data,
// calculates a checksum,
// adds $readwrite,
// and returns the result as follows:
// The first line contains the checksum.
// The second line contains the readwrite as 0 or 1.
// The rest contains the $data.
std::string checksum_logic::send (const std::string& data, bool readwrite)
{
  std::string checksum = get (data);
  checksum.append ("\n");
  checksum.append (filter::string::convert_to_string (readwrite));
  checksum.append ("\n");
  checksum.append (data);
  return checksum;
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of 'data' in bytes.
std::string checksum_logic::get (const std::string& data)
{
  return std::to_string (data.length ());
}


// This function gets the checksum for $data, and returns it.
// It calculates the length of vector 'data' in bytes.
std::string checksum_logic::get (const std::vector <std::string>& data)
{
  int length = 0;
  for (auto & bit : data) length += static_cast<int>(bit.length ());
  return std::to_string (length);
}


// Returns a proper checksum for the USFM in the chapter.
std::string checksum_logic::get_chapter (const std::string& bible, int book, int chapter)
{
  std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  std::string checksum = md5 (filter::string::trim (usfm));
  return checksum;
}


// Returns a proper checksum for the USFM in the book.
std::string checksum_logic::get_book (const std::string& bible, int book)
{
  std::vector <int> chapters = database::bibles::get_chapters (bible, book);
  std::vector <std::string> checksums;
  for (auto chapter : chapters) {
    checksums.push_back (get_chapter (bible, book, chapter));
  }
  std::string checksum = filter::string::implode (checksums, std::string());
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the $bible.
std::string checksum_logic::get_bible (const std::string& bible)
{
  std::vector <int> books = database::bibles::get_books (bible);
  std::vector <std::string> checksums;
  for (auto book : books) {
    checksums.push_back (get_book (bible, book));
  }
  std::string checksum = filter::string::implode (checksums, std::string());
  checksum = md5 (checksum);
  return checksum;
}


// Returns a proper checksum for the USFM in the array of $bibles.
std::string checksum_logic::get_bibles (const std::vector <std::string> & bibles)
{
  std::vector <std::string> checksums;
  for (const auto & bible : bibles) {
    checksums.push_back (get_bible (bible));
  }
  std::string checksum = filter::string::implode (checksums, std::string());
  checksum = md5 (checksum);
  return checksum;
}

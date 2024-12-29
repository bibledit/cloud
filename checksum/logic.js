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


// Checksum checking on received data.
function checksum_receive (data, reload = 0)
{
  // The checksum is the first line of the data.
  // It is the length of the data in bytes.
  var lines = data.split ("\n");
  var checksum = lines [0];
  // Clean the checksum away, keep the data itself.
  lines.splice (0, 2);
  data = lines.join ("\n");
  // Here is a work-around for the desire that the USFM editor be able to load corrupted text.
  // See issue https://github.com/bibledit/cloud/issues/482.
  // The presumption is that the checksum of corrupted data need not be correct.
  // So if the USFM editor fails to load corrupted data multiple times,
  // it eventually disables the check on the checksum.
  if (reload < 5) if (checksum != checksum_get (data)) return false;
  // Return the data to the caller.
  return data;
}


// Checksum = length in bytes.
function checksum_get (data)
{
  var checksum = unescape (encodeURIComponent (data)).length;
  return checksum;
}


function checksum_readwrite (data)
{
  var lines = data.split ("\n");
  var readwrite = !(lines [1] == "0");
  return readwrite;
}

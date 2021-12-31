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


#include <filter/md5.h>
#include <mbedtls/md5.h>


string md5 (const string str)
{
  unsigned char md5sum[16];
  const unsigned char *input = (const unsigned char *)str.c_str ();
  [[maybe_unused]] int ret = mbedtls_md5_ret (input, str.size (), md5sum);

  // Space for 32 bytes of hexits and one terminating null byte.
  char hexits [32+1];

  memset (hexits, 0, sizeof (hexits));
  for (int i = 0; i < 16; i++) sprintf (&hexits[i*2], "%02x", (unsigned int)md5sum[i]);
  
  // Resulting hexits.
  return string (hexits);
}

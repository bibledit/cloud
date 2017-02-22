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


#include <config.h>
#include <webserver/io.h>


/*
 This is our own "bind" function.
 GCC links to the correct Unix call "bind", but Clang++ takes the C++ "bind" function.
 The function below corrects that as follows:
 It is located in this .c file.
 This .c file will be compiled through the C compiler, not the C++ compiler.
 The linker then knows not to link to the C++ "bind" function but to the Unix "bind" function.
 */
int mybind (int socket, const struct sockaddr *address, unsigned int address_len)
{
  return bind (socket, address, address_len);
}

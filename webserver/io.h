/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#ifndef INCLUDED_WEBSERVER_IO_H
#define INCLUDED_WEBSERVER_IO_H


#include <config.h>
#ifdef HAVE_WINDOWS
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

int mybind (int socket, const struct sockaddr *address, unsigned int address_len);

#ifdef __cplusplus
}
#endif


#endif

/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#ifndef INCLUDED_CONFIG_CONFIG_H
#define INCLUDED_CONFIG_CONFIG_H


// Whether file upload works in the browser on the platform.
#define HAVE_UPLOAD 1


// Maxinum number of simultaneous background tasks.
#define MAX_PARALLEL_TASKS 10


// The directory separator for the platform: Windows differs from Linux.
#define DIRECTORY_SEPARATOR "/"


// Whether it runs in Cloud mode.
#define HAVE_CLOUD 1


// Whether it runs the secure webserver.
#define RUN_SECURE_SERVER 1


#ifdef HAVE_WINDOWS
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_BARE_BROWSER 1
#define HAVE_PARATEXT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 5
#define HAVE_URLSETTINGS 1
#undef DIRECTORY_SEPARATOR
#define DIRECTORY_SEPARATOR "\\"
#undef RUN_SECURE_SERVER
#endif


#ifdef HAVE_ANDROID
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 3
#define HAVE_BARE_BROWSER 1
#define HAVE_TINY_JOURNAL 1
#undef RUN_SECURE_SERVER
#endif


#ifdef HAVE_MAC
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_BARE_BROWSER 1
#undef RUN_SECURE_SERVER
#endif


#ifdef HAVE_LINUX
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_PARATEXT 1
#define HAVE_BARE_BROWSER 1
#undef RUN_SECURE_SERVER
#endif


#ifdef HAVE_IOS
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 3
#define HAVE_BARE_BROWSER 1
#define HAVE_TINY_JOURNAL 1
#undef RUN_SECURE_SERVER
#endif


#endif

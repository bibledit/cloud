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


#pragma once


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


// Whether to use std::filesystem.
#define USE_STD_FILESYSTEM 1


#ifdef HAVE_WINDOWS
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_BARE_BROWSER 1
#define HAVE_PARATEXT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 5
#define HAVE_URLSETTINGS 1
#undef DIRECTORY_SEPARATOR
#define DIRECTORY_SEPARATOR R"(\)"
#undef RUN_SECURE_SERVER
#undef USE_STD_FILESYSTEM
#endif


#ifdef HAVE_ANDROID
// Undefine upload capabilities, see issue https://github.com/bibledit/cloud/issues/896
#undef HAVE_UPLOAD
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 3
#define HAVE_BARE_BROWSER 1
#define HAVE_TINY_JOURNAL 1
#undef RUN_SECURE_SERVER
// Testing the std::fileystem in August 2024.
// Results: 5 out of 6 devices tested had crahes in C++.
// See https://github.com/bibledit/cloud/issues/952 for more info.
#undef USE_STD_FILESYSTEM
#endif


#ifdef HAVE_MAC
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_BARE_BROWSER 1
#undef RUN_SECURE_SERVER
#undef USE_STD_FILESYSTEM
#endif


#ifdef HAVE_LINUX
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#define HAVE_PARATEXT 1
#define HAVE_BARE_BROWSER 1
#undef RUN_SECURE_SERVER
#endif


#ifdef HAVE_IOS
// Undefine upload capabilities, see issue https://github.com/bibledit/cloud/issues/896
#undef HAVE_UPLOAD
#undef HAVE_CLOUD
#define HAVE_CLIENT 1
#undef MAX_PARALLEL_TASKS
#define MAX_PARALLEL_TASKS 3
#define HAVE_BARE_BROWSER 1
#define HAVE_TINY_JOURNAL 1
#undef RUN_SECURE_SERVER
// The std::filesystem makes the app so slow in the iOS simulator
// that it appears to be suck during the setup phase, where it copies files.
// This was tested in August 2024.
#undef USE_STD_FILESYSTEM
#endif

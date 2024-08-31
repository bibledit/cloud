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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <webserver/request.h>


TEST (filter, webview)
{
  Webserver_Request request;
  EXPECT_EQ ("Browser/1.0", request.user_agent);
}


/*

 This filter was originally intended to find out when to downgrade the visual Bible editors.
 It was supposed to downgrade it when the browser's user agent met certain criteria.
 
 Some of the browsers were tested:
 
 Ubuntu 14.04 AppleWebKit/538.15
 Ubuntu 15.04 AppleWebKit/601.1
 Ubuntu 16.04 AppleWebKit/603.1
 Ubuntu 16.10 AppleWebKit/603.1

 Nexus 6 AppleWebKit/537.36 Chrome/56.0.2924 Safari/537.36

 Mac Google Chrome: AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36

 Android Internet Device (Android 4.1): Android 4.1.1 AppleWebKit/534 Safari/534.30
 
 On the Genymotion emulator, Android 4.4, it works fine there.

 So it appears that it is hard to decide to downgrade based on the user agent.

*/

#endif


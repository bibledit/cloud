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


#include <images/fetch.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <database/bibleimages.h>


std::string images_fetch_url ()
{
  return "images/fetch";
}


bool images_fetch_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string images_fetch (Webserver_Request& webserver_request)
{
  // Image name.
  std::string image = webserver_request.query ["image"];

  // Set the HTTP GET parameter to the image name,
  // so the server will return the appropriate Mime type for this image.
  webserver_request.get = image;

  // Return the raw image data for sending off to the browser.
  return database::bible_images::get (image);
}

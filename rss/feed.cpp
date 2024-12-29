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


#include <rss/feed.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <rss/logic.h>


std::string rss_feed_url ()
{
  return "rss/feed";
}


bool rss_feed_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
  return true;
}


std::string rss_feed ([[maybe_unused]] Webserver_Request& webserver_request)
{
  std::string xml;
#ifdef HAVE_CLOUD
  webserver_request.response_content_type = "application/rss+xml";
  std::string path = rss_logic_xml_path ();
  xml = filter_url_file_get_contents (path);
#endif
  return xml;
}

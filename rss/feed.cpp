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


#include <rss/feed.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <rss/logic.h>


string rss_feed_url ()
{
  return "rss/feed";
}


bool rss_feed_acl (void * webserver_request)
{
  (void) webserver_request;
  return true;
}


string rss_feed (void * webserver_request)
{
  string xml;
#ifdef HAVE_CLOUD
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  request->response_content_type = "application/rss+xml";
  string path = rss_logic_xml_path ();
  xml = filter_url_file_get_contents (path);
#endif
  (void) webserver_request;
  return xml;
}

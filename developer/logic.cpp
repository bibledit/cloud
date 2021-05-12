/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <developer/logic.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>


mutex log_network_mutex;
vector <string> log_network_cache;


void developer_logic_log_network_write ()
{
  if (!log_network_cache.empty ()) {
    log_network_mutex.lock ();
    string lines;
    for (auto line : log_network_cache) {
      lines.append (line);
      lines.append ("\n");
    }
    log_network_cache.clear ();
    log_network_mutex.unlock ();
    string path = filter_url_create_root_path (filter_url_temp_dir(), "log-network.csv");
    if (!file_or_dir_exists(path)) {
      filter_url_file_put_contents_append (path, "date,IPaddress,URL,query,username\n");
    }
    filter_url_file_put_contents_append (path, lines);
  }
}


Developer_Logic_Tracer::Developer_Logic_Tracer(void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  seconds1 = filter_date_seconds_since_epoch ();
  microseconds1 = filter_date_numerical_microseconds();
  rfc822 = filter_date_rfc822 (seconds1);
  remote_address = request->remote_address;
  request_get = request->get;
  for (auto element : request->query) {
    request_query.append(" ");
    request_query.append(element.first);
    request_query.append("=");
    request_query.append(element.second);
  }
  username = request->session_logic()->currentUser();
}


Developer_Logic_Tracer::~Developer_Logic_Tracer()
{
  int seconds2 = filter_date_seconds_since_epoch();
  int microseconds2 = filter_date_numerical_microseconds();
  int microseconds = (seconds2 - seconds1) * 1000000 + microseconds2 - microseconds1;
  vector <string> bits = {rfc822, convert_to_string (microseconds), request_get, request_query, username};
  string entry = filter_string_implode(bits, ",");
  log_network_mutex.lock();
  log_network_cache.push_back(entry);
  log_network_mutex.unlock();
}

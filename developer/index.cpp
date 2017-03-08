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


#include <developer/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <tasks/logic.h>
#ifndef HAVE_CLIENT
#include <sources/etcbc4.h>
#include <sources/kjv.h>
#include <sources/morphgnt.h>
#include <sources/hebrewlexicon.h>
#endif
#include <resource/external.h>
#include <database/logs.h>
#include <config/globals.h>


const char * developer_index_url ()
{
  return "developer/index";
}


bool developer_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::admin ());
}


string developer_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  if (request->query.count ("log")) {
    string message = request->query ["log"];
    Database_Logs::log (message);
    return "";
  }
  
  string page;

  Assets_Header header = Assets_Header ("Bibledit", webserver_request);
  header.notifItOn ();
  page = header.run ();

  Assets_View view;

  string code;
  
  string debug = request->query ["debug"];
  
  // It is cleaner and easier to move the following task to the binary ./generate.
  if (debug == "etcbc4download") {
    // sources_etcbc4_download ();
    view.set_variable ("success", "Task disabled");
  }
  
  // It is cleaner and easier to move the following task to the binary ./generate.
  if (debug == "etcbc4parse") {
    //sources_etcbc4_parse ();
    view.set_variable ("success", "Task disabled");
  }
  
  // It is cleaner and easier to move the following task to the binary ./generate.
  if (debug == "parsekjv") {
    //sources_kjv_parse ();
    view.set_variable ("success", "Task disabled");
  }
  
  // It is cleaner and easier to move the following task to the binary ./generate.
  if (debug == "parsemorphgnt") {
    // sources_morphgnt_parse ();
    view.set_variable ("success", "Task disabled");
  }

  // It is cleaner and easier to move the following task to the binary ./generate.
  if (debug == "parsehebrewlexicon") {
    // sources_hebrewlexicon_parse ();
    view.set_variable ("success", "Task disabled");
  }

  if (debug == "crash") {
    // Make a bad pointer.
    // int *foo = (int*)-1;
    // Cause segmentation fault.
    // printf ("%d\n", *foo);
    view.set_variable ("success", "Task disabled");
  }
  
  if (debug == "receive") {
    tasks_logic_queue (RECEIVEEMAIL);
    view.set_variable ("success", "Receiving email and running tasks that send mail");
  }

  if (debug == "ipv6") {
    view.set_variable ("success", "Fetching data via IPv6");
    string error;
    string response = filter_url_http_request_mbed ("http://ipv6.google.com", error, {}, "", true);
    page.append (response);
    view.set_variable ("error", error);
  }
  
  if (debug == "ipv6s") {
    view.set_variable ("success", "Securely fetching data via IPv6");
    string error;
    string response = filter_url_http_request_mbed ("https://ipv6.google.com", error, {}, "", true);
    page.append (response);
    view.set_variable ("error", error);
  }
  
  if (debug == "maintain") {
    tasks_logic_queue (MAINTAINDATABASE);
    view.set_variable ("success", "Starting to maintain the databases");
  }
  
  view.set_variable ("code", code);

  page += view.render ("developer", "index");
  page += Assets_Page::footer ();

  return page;
}

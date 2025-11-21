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
#include <config/globals.h>
#include <library/bibledit.h>
#include <developer/logic.h>
#include <webserver/request.h>
#include <dialog/select.h>


const char * developer_index_url ()
{
  return "developer/index";
}


bool developer_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::admin);
}


std::string developer_index (Webserver_Request& webserver_request)
{
  if (webserver_request.query.count ("log")) {
    std::string message = webserver_request.query ["log"];
    std::cerr << message << std::endl;
    return std::string();
  }
  
  std::string page {};

  Assets_Header header = Assets_Header ("Development", webserver_request);
  header.notify_on ();
  page = header.run ();

  Assets_View view {};

  std::string code {};
  
  std::string debug = webserver_request.query ["debug"];
  
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
    //sources_hebrewlexicon_parse ();
    view.set_variable ("success", "Task disabled");
    //view.set_variable ("success", "Task running");
  }

  if (debug == "crash") {
    // Make a bad pointer.
    // int *foo = (int*)-1;
    // Cause segmentation fault.
    // printf ("%d\n", *foo);
    view.set_variable ("success", "Task disabled");
  }
  
  if (debug == "sendreceive") {
    tasks_logic_queue (task::receive_email);
    view.set_variable ("success", "Sending and receiving email");
  }

  if (debug == "ipv6") {
    view.set_variable ("success", "Fetching data via IPv6");
    std::string error {};
    std::string response = filter_url_http_request_mbed ("http://ipv6.google.com", error, {}, "", true);
    page.append (response);
    view.set_variable ("error", error);
  }
  
  if (debug == "ipv6s") {
    view.set_variable ("success", "Securely fetching data via IPv6");
    std::string error {};
    std::string response = filter_url_http_request_mbed ("https://ipv6.google.com", error, {}, "", true);
    page.append (response);
    view.set_variable ("error", error);
  }
  
  if (debug == "maintain") {
    tasks_logic_queue (task::maintain_database);
    view.set_variable ("success", "Starting to maintain the databases");
  }

  if (debug == "accordance") {
    std::string reference = bibledit_get_reference_for_accordance ();
    view.set_variable ("success", "Accordance reference: " + reference);
    bibledit_put_reference_from_accordance("PSA 3:2");
  }

  if (debug == "changes") {
    developer_logic_import_changes ();
    view.set_variable ("success", "Task was done see Journal");
  }
  
  {
    constexpr const char* identification {"selectorajax"};
    if (webserver_request.post_count(identification)) {
      [[maybe_unused]] const std::string value {webserver_request.post_get(identification)};
      return std::string();
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = {"aaa", "bbb", "ccc"},
      .selected = "aaa",
      .parameters = { {"a", "aa"}, {"b", "bb"} },
      .tooltip = "Tooltip",
    };
    view.set_variable(identification, dialog::select::ajax(settings));
  }

  {
    constexpr const char* identification {"selectorform"};
    std::string selected = "ddd";
    if (webserver_request.post_count(identification)) {
      selected = webserver_request.post_get(identification);
      view.set_variable ("success", "Submitted: " + selected);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = {"ddd", "eee", "fff"},
      .selected = selected,
      .parameters = { {"d", "dd"}, {"e", "ee"} },
      .tooltip = "Tooltip",
      .submit = "Enter",
    };
    dialog::select::Form form { .auto_submit = false };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  {
    constexpr const char* identification {"selectorautosubmit"};
    std::string selected = "ggg";
    if (webserver_request.post_count(identification)) {
      selected = webserver_request.post_get(identification);
      view.set_variable ("success", "Submitted: " + selected);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = {"ggg", "hhh", "iii"},
      .selected = selected,
      .parameters = { {"g", "gg"}, {"h", "hh"} },
      .tooltip = "Tooltip",
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  view.set_variable ("code", code);

  page += view.render ("developer", "index");
  page += assets_page::footer ();

  return page;
}

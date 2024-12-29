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


#include <changes/logic.h>
#include <changes/changes.h>
#include <changes/statistics.h>
#include <changes/manage.h>
#include <tasks/logic.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <locale/translate.h>
#include <index/listing.h>
#include <database/logs.h>
#include <database/modifications.h>
#include <database/jobs.h>
#include <filter/string.h>
#include <webserver/request.h>


void changes_logic_start ()
{
  tasks_logic_queue (task::generate_changes);
}


const char * changes_personal_category ()
{
  return "P";
}


const char * changes_bible_category ()
{
  return "B";
}


std::string changes_interlinks (Webserver_Request& webserver_request, std::string my_url)
{
  // Storage the available links.
  std::vector <std::string> urls {};
  std::vector <std::string> labels {};
  
  // Handle situation that the user has permission to view the changes.
  if (changes_changes_acl (webserver_request)) {
    
    // Handle situation that the user is not currently displaying the changes.
    if (changes_changes_url () != my_url) {
      urls.push_back (changes_changes_url ());
      labels.push_back (translate ("View"));
    }

  }
  
#ifndef HAVE_CLIENT

  if (changes_statistics_url () != my_url) {
    if (changes_statistics_acl (webserver_request)) {
      urls.push_back (changes_statistics_url ());
      labels.push_back (translate ("Statistics"));
    }
  }

  const std::string revisions = "revisions";
  if (index_listing_url (revisions) != my_url) {
    if (index_listing_acl (webserver_request, revisions)) {
      urls.push_back (index_listing_url (revisions));
      labels.push_back (translate ("Download"));
    }
  }

  if (changes_manage_url () != my_url) {
    if (changes_manage_acl (webserver_request)) {
      urls.push_back (changes_manage_url ());
      labels.push_back (translate ("Manage"));
    }
  }
  
#endif

  // Generate the links in XML.
  pugi::xml_document document {};
  bool first {true};
  for (unsigned int i = 0; i < urls.size (); i++) {
    if (!first) {
      pugi::xml_node node = document.append_child ("span");
      node.text ().set (" | ");
    }
    first = false;
    pugi::xml_node a = document.append_child ("a");
    const std::string href = "/" + urls[i];
    a.append_attribute ("href") = href.c_str();
    a.text ().set (labels[i].c_str());
  }
  
  // Convert the document to a string.
  std::stringstream output {};
  document.print (output, "", pugi::format_raw);
  return output.str ();
}


void changes_clear_notifications_user (std::string jobid, std::string username)
{
  Database_Logs::log (translate ("Start clearing change notifications") + " " + username);
  
  Database_Jobs database_jobs {};

  // Get the total amount of change notifications to clear for the user.
  std::string any_bible {};
  std::vector <int> identifiers = database::modifications::getNotificationIdentifiers (username, any_bible);
  
  // Total notes cleared.
  int total_cleared {0};
  
  // Feedback.
  database_jobs.set_percentage (filter::strings::convert_to_int (jobid), 0);
  database_jobs.set_progress (filter::strings::convert_to_int (jobid), translate ("Total:") + " " + std::to_string (identifiers.size()));


  // The amount of notifications it clears in the next iteration.
  int cleared_count_in_one_go {0};
  do {
    cleared_count_in_one_go = database::modifications::clearNotificationsUser (username);
    total_cleared += cleared_count_in_one_go;
    if (!identifiers.empty ()) {
      database_jobs.set_percentage (filter::strings::convert_to_int (jobid), 100 * total_cleared / static_cast<int> (identifiers.size()));
    }
  } while (cleared_count_in_one_go);
  
  Webserver_Request request;
  request.database_config_user ()->setUserChangeNotificationsChecksum (username, "");
  
  database_jobs.set_result (filter::strings::convert_to_int (jobid), translate ("Ready clearing change notifications"));
  
  Database_Logs::log (translate ("Ready clearing change notifications") + " " + username);
}

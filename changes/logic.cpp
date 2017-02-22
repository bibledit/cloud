/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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
#include <pugixml/pugixml.hpp>
#include <locale/translate.h>
#include <index/listing.h>


using namespace pugi;


void changes_logic_start ()
{
  tasks_logic_queue (GENERATECHANGES);
}


const char * changes_personal_category ()
{
  return "P";
}


const char * changes_bible_category ()
{
  return "B";
}


string changes_interlinks (void * webserver_request, string my_url)
{
  // The available links.
  vector <string> urls;
  vector <string> labels;
  if (changes_changes_acl (webserver_request)) {
    urls.push_back (changes_changes_url ());
    labels.push_back (translate ("View"));
  }
#ifndef HAVE_CLIENT
  if (changes_statistics_acl (webserver_request)) {
    urls.push_back (changes_statistics_url ());
    labels.push_back (translate ("Statistics"));
  }
  if (index_listing_acl (webserver_request, "revisions")) {
    urls.push_back (index_listing_url ("revisions"));
    labels.push_back (translate ("Download"));
  }
  if (changes_manage_acl (webserver_request)) {
    urls.push_back (changes_manage_url ());
    labels.push_back (translate ("Manage"));
  }
#endif

  // Generate the links in XML.
  xml_document document;
  bool first = true;
  for (unsigned int i = 0; i < urls.size (); i++) {
    if (urls[i] == my_url) continue;
    if (!first) {
      xml_node node = document.append_child ("span");
      node.text ().set (" | ");
    }
    first = false;
    xml_node a = document.append_child ("a");
    string href = "/" + urls[i];
    a.append_attribute ("href") = href.c_str();
    a.text ().set (labels[i].c_str());
  }
  
  // Convert the document to a string.
  stringstream output;
  document.print (output, "", format_raw);
  return output.str ();
}

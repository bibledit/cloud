/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <changes/change.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <assets/view.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <trash/handler.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/notes.h>
#include <locale/logic.h>
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
using namespace std;
using namespace pugi;


string changes_change_url ()
{
  return "changes/change";
}


bool changes_change_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string changes_change (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Modifications database_modifications {};
  Database_Notes database_notes = Database_Notes (request);
  Notes_Logic notes_logic = Notes_Logic (request);

  
  // Note unsubscribe handler.
  if (request->post.count ("unsubscribe")) {
    string unsubscribe = request->post["unsubscribe"];
    unsubscribe.erase (0, 11);
    notes_logic.unsubscribe (filter::strings::convert_to_int (unsubscribe));
    return string();
  }
  
  
  // Note unassign handler.
  if (request->post.count ("unassign")) {
    string unassign = request->post["unassign"];
    unassign.erase (0, 8);
    notes_logic.unassignUser (filter::strings::convert_to_int (unassign), request->session_logic()->currentUser ());
    return string();
  }
  
  
  // Note mark for deletion handler.
  if (request->post.count("delete")) {
    string erase = request->post["delete"];
    erase.erase (0, 6);
    const int identifier {filter::strings::convert_to_int (erase)};
    notes_logic.markForDeletion (identifier);
    return string();
  }
  
  
  // From here on the script will produce output.
  Assets_View view {};
  const string username {request->session_logic()->currentUser ()};
  const int level {request->session_logic ()->currentLevel ()};
  
                      
  // The identifier of the change notification.
  const int id {filter::strings::convert_to_int (request->query ["get"])};
  view.set_variable ("id", filter::strings::convert_to_string (id));
                      
                      
  // Get old text, modification, new text, date.
  const string old_text {database_modifications.getNotificationOldText (id)};
  view.set_variable ("old_text", old_text);
  const string modification {database_modifications.getNotificationModification (id)};
  view.set_variable ("modification", modification);
  const string new_text {database_modifications.getNotificationNewText (id)};
  view.set_variable ("new_text", new_text);
  const string date {locale_logic_date (database_modifications.getNotificationTimeStamp (id))};
  view.set_variable ("date", date);
  

  // Bibles and passage.
  const Passage passage {database_modifications.getNotificationPassage (id)};
  const vector <string> bibles {access_bible::bibles (request)};
  
  
  // Get notes for the passage.
  vector <int> notes = database_notes.select_notes (bibles, // Bibles.
                                                    passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse),
                                                    0,  // Passage selector.
                                                    0,  // Edit selector.
                                                    0,  // Non-edit selector.
                                                    "", // Status selector.
                                                    "", // Bible selector.
                                                    "", // Assignment selector.
                                                    0,  // Subscription selector.
                                                    -1, // Severity selector.
                                                    0,  // Text selector.
                                                    "", // Search text.
                                                    -1); // Limit.
  
  // Remove the ones marked for deletion.
  {
    vector <int> notes2;
    for (const auto note : notes) {
      if (!database_notes.is_marked_for_deletion (note)) {
        notes2.push_back (note);
      }
    }
    notes = notes2;
  }
  
  // Sort them, most recent notes first.
  vector <int> timestamps;
  for (const auto note : notes) {
    int timestap = database_notes.get_modified (note);
    timestamps.push_back (timestap);
  }
  filter::strings::quick_sort (timestamps, notes, 0, static_cast <unsigned int> (notes.size ()));
  reverse (notes.begin(), notes.end());
  
  
  // Whether there"s a live notes editor available.
  bool live_notes_editor = Ipc_Notes::alive (webserver_request, false);
  if (live_notes_editor) view.enable_zone ("alive");
  else view.enable_zone ("dead");


  // Details for the notes.
  xml_document notes_document {};
  for (const auto note : notes) {
    string summary = database_notes.get_summary (note);
    summary = filter::strings::escape_special_xml_characters (summary);
    bool subscription = database_notes.is_subscribed (note, username);
    bool assignment = database_notes.is_assigned (note, username);
    xml_node tr_node = notes_document.append_child("tr");
    xml_node td_node = tr_node.append_child("td");
    xml_node a_node = td_node.append_child("a");
    string href {};
    if (live_notes_editor) {
      a_node.append_attribute("class") = "opennote";
      href = filter::strings::convert_to_string (note);
    } else {
      href = "/notes/note?id=" + filter::strings::convert_to_string (note);
    }
    a_node.append_attribute("href") = href.c_str();
    a_node.text().set(summary.c_str());
    td_node = tr_node.append_child("td");
    if (subscription) {
      xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("unsubscribe" + filter::strings::convert_to_string (note)).c_str();
      a_node2.text().set(("[" + translate("unsubscribe") + "]").c_str());
    }
    td_node = tr_node.append_child("td");
    if (assignment) {
      xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("unassign" + filter::strings::convert_to_string (note)).c_str();
      a_node2.text().set(("[" + translate("I have done my part on it") + "]").c_str());
    }
    td_node = tr_node.append_child("td");
    if (level >= Filter_Roles::manager ()) {
      xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("delete" + filter::strings::convert_to_string (note)).c_str();
      a_node2.text().set(("[" + translate("mark for deletion") + "]").c_str());
    }
  }
  stringstream notesblock {};
  notes_document.print(notesblock, "", format_raw);
  view.set_variable ("notesblock", notesblock.str());

  
  // Display page.
  return view.render ("changes", "change");
}

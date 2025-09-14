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


std::string changes_change_url ()
{
  return "changes/change";
}


bool changes_change_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string changes_change (Webserver_Request& webserver_request)
{
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic (webserver_request);

  
  // Note unsubscribe handler.
  if (webserver_request.post_count ("unsubscribe")) {
    std::string unsubscribe = webserver_request.post_get("unsubscribe");
    unsubscribe.erase (0, 11);
    notes_logic.unsubscribe (filter::strings::convert_to_int (unsubscribe));
    return std::string();
  }
  
  
  // Note unassign handler.
  if (webserver_request.post_count ("unassign")) {
    std::string unassign = webserver_request.post_get("unassign");
    unassign.erase (0, 8);
    notes_logic.unassignUser (filter::strings::convert_to_int (unassign), webserver_request.session_logic ()->get_username ());
    return std::string();
  }
  
  
  // Note mark for deletion handler.
  if (webserver_request.post_count("delete")) {
    std::string erase = webserver_request.post_get("delete");
    erase.erase (0, 6);
    const int identifier {filter::strings::convert_to_int (erase)};
    notes_logic.markForDeletion (identifier);
    return std::string();
  }
  
  
  // From here on the script will produce output.
  Assets_View view {};
  const std::string& username {webserver_request.session_logic ()->get_username ()};
  const int level {webserver_request.session_logic ()->get_level ()};
  
                      
  // The identifier of the change notification.
  const int id {filter::strings::convert_to_int (webserver_request.query ["get"])};
  view.set_variable ("id", std::to_string (id));
                      
                      
  // Get old text, modification, new text, date.
  const std::string old_text {database::modifications::getNotificationOldText (id)};
  view.set_variable ("old_text", old_text);
  const std::string modification {database::modifications::getNotificationModification (id)};
  view.set_variable ("modification", modification);
  const std::string new_text {database::modifications::getNotificationNewText (id)};
  view.set_variable ("new_text", new_text);
  const std::string date {locale_logic_date (database::modifications::getNotificationTimeStamp (id))};
  view.set_variable ("date", date);
  

  // Bibles and passage.
  const Passage passage {database::modifications::getNotificationPassage (id)};
  const std::vector<std::string> bibles {access_bible::bibles (webserver_request)};
  
  
  // Get notes for the passage.
  Database_Notes::Selector selector {
    .bibles = bibles,
    .book = passage.m_book,
    .chapter = passage.m_chapter,
    .verse = filter::strings::convert_to_int (passage.m_verse),
    .passage_selector = Database_Notes::PassageSelector::current_verse,
  };
  std::vector<int> notes = database_notes.select_notes(selector);
  
  // Remove the ones marked for deletion.
  {
    std::vector <int> notes2;
    for (const auto note : notes) {
      if (!database_notes.is_marked_for_deletion (note)) {
        notes2.push_back (note);
      }
    }
    notes = std::move(notes2);
  }
  
  // Sort them, most recent notes first.
  std::vector <int> timestamps {};
  for (const auto note : notes) {
    const int timestap = database_notes.get_modified (note);
    timestamps.push_back (timestap);
  }
  filter::strings::quick_sort (timestamps, notes, 0, static_cast <unsigned int> (notes.size ()));
  std::reverse (notes.begin(), notes.end());
  
  
  // Whether there"s a live notes editor available.
  const bool live_notes_editor = Ipc_Notes::alive (webserver_request, false);
  if (live_notes_editor)
    view.enable_zone ("alive");
  else 
    view.enable_zone ("dead");


  // Details for the notes.
  pugi::xml_document notes_document {};
  for (const auto note : notes) {
    std::string summary = database_notes.get_summary (note);
    summary = filter::strings::escape_special_xml_characters (summary);
    const bool subscription = database_notes.is_subscribed (note, username);
    const bool assignment = database_notes.is_assigned (note, username);
    pugi::xml_node tr_node = notes_document.append_child("tr");
    pugi::xml_node td_node = tr_node.append_child("td");
    pugi::xml_node a_node = td_node.append_child("a");
    std::string href {};
    if (live_notes_editor) {
      a_node.append_attribute("class") = "opennote";
      href = std::to_string (note);
    } else {
      href = "/notes/note?id=" + std::to_string (note);
    }
    a_node.append_attribute("href") = href.c_str();
    a_node.text().set(summary.c_str());
    td_node = tr_node.append_child("td");
    if (subscription) {
      pugi::xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("unsubscribe" + std::to_string (note)).c_str();
      a_node2.text().set(("[" + translate("unsubscribe") + "]").c_str());
    }
    td_node = tr_node.append_child("td");
    if (assignment) {
      pugi::xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("unassign" + std::to_string (note)).c_str();
      a_node2.text().set(("[" + translate("I have done my part on it") + "]").c_str());
    }
    td_node = tr_node.append_child("td");
    if (level >= roles::manager) {
      pugi::xml_node a_node2 = td_node.append_child("a");
      a_node2.append_attribute("href") = ("delete" + std::to_string (note)).c_str();
      a_node2.text().set(("[" + translate("mark for deletion") + "]").c_str());
    }
  }
  std::stringstream notesblock {};
  notes_document.print(notesblock, "", pugi::format_raw);
  view.set_variable ("notesblock", notesblock.str());

  
  // Display page.
  return view.render ("changes", "change");
}

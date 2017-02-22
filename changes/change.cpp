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
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Modifications database_modifications;
  Database_Notes database_notes = Database_Notes (request);
  Notes_Logic notes_logic = Notes_Logic (request);

  
  // Note unsubscribe handler.
  if (request->post.count ("unsubscribe")) {
    string unsubscribe = request->post["unsubscribe"];
    unsubscribe.erase (0, 11);
    notes_logic.unsubscribe (convert_to_int (unsubscribe));
    return "";
  }
  
  
  // Note unassign handler.
  if (request->post.count ("unassign")) {
    string unassign = request->post["unassign"];
    unassign.erase (0, 8);
    notes_logic.unassignUser (convert_to_int (unassign), request->session_logic()->currentUser ());
    return "";
  }
  
  
  // Note mark for deletion handler.
  if (request->post.count("delete")) {
    string erase = request->post["delete"];
    erase.erase (0, 6);
    int identifier = convert_to_int (erase);
    notes_logic.markForDeletion (identifier);
    return "";
  }
  
  
  // From here on the script will produce output.
  Assets_View view;
  string username = request->session_logic()->currentUser ();
  int level = request->session_logic ()->currentLevel ();
  
                      
  // The identifier of the change notification.
  int id = convert_to_int (request->query ["get"]);
  view.set_variable ("id", convert_to_string (id));
                      
                      
  // Get old text, modification, new text, date.
  string old_text = database_modifications.getNotificationOldText (id);
  view.set_variable ("old_text", old_text);
  string modification = database_modifications.getNotificationModification (id);
  view.set_variable ("modification", modification);
  string new_text = database_modifications.getNotificationNewText (id);
  view.set_variable ("new_text", new_text);
  string date = locale_logic_date (database_modifications.getNotificationTimeStamp (id));
  view.set_variable ("date", date);
  

  // Bibles and passage.
  Passage passage = database_modifications.getNotificationPassage (id);
  vector <string> bibles = access_bible_bibles (request);
  
  
  // Get notes for the passage.
  vector <int> notes = database_notes.selectNotes (bibles, // Bibles.
                                                   passage.book, passage.chapter, convert_to_int (passage.verse),
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
  vector <int> notes2;
  for (auto note : notes) {
    if (!database_notes.isMarkedForDeletion (note)) {
      notes2.push_back (note);
    }
  }
  notes = notes2;
  
  // Sort them, most recent notes first.
  vector <int> timestamps;
  for (auto note : notes) {
    int timestap = database_notes.getModified (note);
    timestamps.push_back (timestap);
  }
  quick_sort (timestamps, notes, 0, notes.size ());
  reverse (notes.begin(), notes.end());
  
  
  // Whether there"s a live notes editor available.
  bool live_notes_editor = Ipc_Notes::alive (webserver_request, false);
  if (live_notes_editor) view.enable_zone ("alive");
  else view.enable_zone ("dead");


  // Details for the notes.
  string notesblock;
  for (auto & note : notes) {
    string summary = database_notes.getSummary (note);
    summary = filter_string_sanitize_html (summary);
    bool subscription = database_notes.isSubscribed (note, username);
    bool assignment = database_notes.isAssigned (note, username);
    notesblock.append ("<tr>\n");
    notesblock.append ("<td>\n");
    if (live_notes_editor) {
      notesblock.append ("<a class=\"opennote\" href=\"" + convert_to_string (note) + "\">" + summary + "</a>\n");
    } else {
      notesblock.append ("<a href=\"/notes/note?id=" + convert_to_string (note) + "\">" + summary + "</a>\n");
    }
    notesblock.append ("</td>\n");
    notesblock.append ("<td>");
    if (subscription) {
      notesblock.append ("<a href=\"unsubscribe\" id=\"unsubscribe" + convert_to_string (note) + "\">[" + translate("unsubscribe") + "]</a>");
    }
    notesblock.append ("</td>\n");
    notesblock.append ("<td>");
    if (assignment) {
      notesblock.append ("<a href=\"unassign\" id=\"unassign" + convert_to_string (note) + "\">[" + translate("I have done my part on it") + "]</a>");
    }
    notesblock.append ("</td>\n");
    notesblock.append ("<td>");
    if (level >= Filter_Roles::manager ()) {
      notesblock.append ("<a href=\"delete\" id=\"delete" + convert_to_string (note) + "\">[" + translate("mark for deletion") + "]</a>");
    }
    notesblock.append ("</td>\n");
    notesblock.append ("</tr>\n");
  }
  view.set_variable ("notesblock", notesblock);

  
  // Display page.
  return view.render ("changes", "change");
}

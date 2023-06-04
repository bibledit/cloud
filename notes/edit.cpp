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


#include <notes/edit.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <notes/note.h>
using namespace std;


string notes_edit_url ()
{
  return "notes/edit";
}


bool notes_edit_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string notes_edit (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Edit Note Source"), request);
  page += header.run ();
  Assets_View view;
  
  
  string myusername = request->session_logic ()->currentUser ();
  
  
  int identifier;
  const char * identifier_label = "identifier";
  if (request->query.count (identifier_label)) identifier = filter::strings::convert_to_int (request->query [identifier_label]);
  else identifier = filter::strings::convert_to_int (request->post [identifier_label]);
  if (identifier) view.set_variable (identifier_label, filter::strings::convert_to_string (identifier));
  
  
  if (request->post.count ("data")) {
    // Save note.
    string noteData = request->post["data"];
    if (database_notes.identifier_exists (identifier)) {
      vector <string> lines = filter::strings::explode (noteData, '\n');
      for (size_t i = 0; i < lines.size (); i++) {
        lines[i] = filter::strings::trim (lines[i]);
        size_t pos = lines[i].find (">");
        if (pos != string::npos) lines[i].erase (0, pos + 1);
        if (lines[i].length () >= 6) lines[i].erase (lines[i].length () - 6);
      }
      noteData = filter::strings::implode (lines, "\n");
      notes_logic.setContent (identifier, noteData);
      string url = filter_url_build_http_query (notes_note_url (), "id", filter::strings::convert_to_string (identifier));
      // View the updated note.
      redirect_browser (request, url);
      return "";
    }
  }
  
  
  if (identifier) {
    if (database_notes.identifier_exists (identifier)) {
      string noteData = database_notes.get_contents (identifier);
      bool editable = false;
      vector <string> lines = filter::strings::explode (noteData, '\n');
      for (size_t i = 0; i < lines.size (); i++) {

        lines[i] = filter::strings::trim (lines[i]);
        
        // Retrieve possible username from the line.
        // This is the pattern of a line with a username.
        // <p>adminusername (8/9/2015):</p>
        // Or:
        // <p><b>adminusername (3/3/2019):</b></p>
        string username;
        {
          // Splitting on space should yield two bits.
          vector <string> bits = filter::strings::explode (lines[i], ' ');
          if (bits.size () == 2) {
            // First bit should contain the <p> and optionally the <b>.
            if (bits[0].find ("<p>") == 0) {
              bits[0].erase (0, 3);
              if (bits[0].find ("<b>") == 0) {
                bits[0].erase (0, 3);
              }
              // Second bit should contain colon plus b or p closing element.
              size_t pos = bits[1].find (":</");
              if (pos != string::npos) {
                bits[1].erase (pos);
                // It should also contain ( and ).
                pos = bits[1].find ("(");
                if (pos != string::npos) {
                  bits[1].erase (pos, 1);
                  pos = bits[1].find (")");
                  if (pos != string::npos) {
                    bits[1].erase (pos, 1);
                    // Now deal with the data consisting of two slashes and three numbers.
                    vector <string> date = filter::strings::explode (bits[1], '/');
                    if (date.size () == 3) {
                      username = bits[0];
                    }
                  }
                }
              }
            }
          }
        }
        
        if (!username.empty () && (username != myusername)) editable = false;
        
        if (editable) {
          lines[i].insert (0, R"(<div contenteditable="true">)");
        } else {
          lines[i].insert (0, "<div " + filter_css_grey_background () + ">");
        }
        lines[i].append ("</div>");
        
        if (username == myusername) {
          editable = true;
        }
        
      }
      
      noteData = filter::strings::implode (lines, "\n");
      
      view.set_variable ("noteblock", noteData);
    }
  }
  
  
  page += view.render ("notes", "edit");
  page += assets_page::footer ();
  return page;
}

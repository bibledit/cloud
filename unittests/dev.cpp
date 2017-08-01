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


#include <unittests/dev.h>
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/archive.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <database/notes.h>
#include <webserver/request.h>


void test_dev ()
{
  trace_unit_tests (__func__);
  
  // Test updating the search database for the notes for version 1 storage. Todo write and move into place.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Test values for the note.
    string contents ("contents");
    
    // Create note.
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "", "", false);
    // Creating the note updates the search database.
    // Basic search should work now.
    vector <int> identifiers;
    
    // Search on the content of the current note.
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   0, // No book given.
                                                   0, // No chapter given.
                                                   0, // No verse given.
                                                   3, // Select any passage.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   1, // Do search on the text following.
                                                   "", // Search on any contents.
                                                   0); // Don't limit the search results.
    // Search result should be there.
    evaluate (__LINE__, __func__, { identifier }, identifiers);
    // Do a raw update of the note. The search database is not updated.
    database_notes.set_raw_contents_v1 (identifier, contents);
    // Doing a search now does not give results.
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   0, // No book given.
                                                   0, // No chapter given.
                                                   0, // No verse given.
                                                   3, // Select any passage.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   1, // Do search on the text following.
                                                   contents, // Search on certain content.
                                                   0); // Don't limit the search results.
    evaluate (__LINE__, __func__, { }, identifiers);
    // Update the search index.
    // Search results should be back to normal.
    database_notes.update_database_v1 (identifier);
    database_notes.update_search_fields_v1 (identifier);
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   0, // No book given.
                                                   0, // No chapter given.
                                                   0, // No verse given.
                                                   3, // Select any passage.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   1, // Do search on the text following.
                                                   contents, // Search on any contents.
                                                   0); // Don't limit the search results.
    evaluate (__LINE__, __func__, { identifier }, identifiers);
    
    // Search on the note's passage.
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   0, // Book given.
                                                   0, // Chapter given.
                                                   0, // Verse given.
                                                   0, // Select current verse.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   0, // Do not search on any text.
                                                   "", // No text given as being irrelevant.
                                                   0); // Don't limit the search results.
    // Search result should be there.
    evaluate (__LINE__, __func__, { identifier }, identifiers);
    // Update the passage of the note without updating the search index.
    database_notes.set_raw_passage_v1 (identifier, " 1.2.3 ");
    // There should be no search results yet when searching on the new passage.
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   1, // Book given.
                                                   2, // Chapter given.
                                                   3, // Verse given.
                                                   0, // Select current verse.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   0, // Do not search on any text.
                                                   "", // No text given as being irrelevant.
                                                   0); // Don't limit the search results.
    evaluate (__LINE__, __func__, { }, identifiers);
    // Update the search index. There should be search results now.
    database_notes.update_database_v1 (identifier);
    identifiers = database_notes.select_notes_v12 ({}, // No Bibles given.
                                                   1, // Book given.
                                                   2, // Chapter given.
                                                   3, // Verse given.
                                                   0, // Select current verse.
                                                   0, // Select any time edited.
                                                   0, // Select any time not edited.
                                                   "", // Don't consider the status.
                                                   "", // Don't consider a Bible.
                                                   "", // Don't consider assignment.
                                                   false, // Don't consider subscriptions.
                                                   -1, // Don't consider the severity.
                                                   0, // Do not search on any text.
                                                   "", // No text given as being irrelevant.
                                                   0); // Don't limit the search results.
    evaluate (__LINE__, __func__, { identifier }, identifiers);
  }
  
  
  
  cout << "dev done" << endl; // Todo
  // exit (0); // Todo
}

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


#include <unittests/notes.h>
#include <unittests/utilities.h>
#include <database/noteactions.h>
#include <database/notes.h>
#include <database/state.h>
#include <database/mail.h>
#include <database/noteassignment.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/login.h>
#include <notes/logic.h>
#include <sync/logic.h>


void test_database_noteactions ()
{
  trace_unit_tests (__func__);

  // Basic tests: create / clear / optimize.
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.clear ();
    database.optimize ();
  }

  // Record
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("unittest", 2, 3, "content");
    vector <int> notes = database.getNotes ();
    evaluate (__LINE__, __func__, {2}, notes);
  }

  // Get notes
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("unittest", 2, 3, "content");
    database.record ("unittest", 2, 4, "content");
    database.record ("unittest", 3, 3, "content");
    vector <int> notes = database.getNotes ();
    evaluate (__LINE__, __func__, {2, 3}, notes);
  }

  // Get note data
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("phpunit1", 2, 3, "content3");
    database.record ("phpunit2", 2, 4, "content4");
    database.record ("phpunit3", 3, 4, "content5");
    vector <Database_Note_Action> data = database.getNoteData (2);
    evaluate (__LINE__, __func__, 2, (int)data.size());
    int now = filter_date_seconds_since_epoch ();
    evaluate (__LINE__, __func__, 1, data[0].rowid);
    evaluate (__LINE__, __func__, "phpunit1", data[0].username);
    if ((data[0].timestamp < now - 1) || (data[0].timestamp > now + 2)) evaluate (__LINE__, __func__, now, data[0].timestamp);
    evaluate (__LINE__, __func__, 3, data[0].action);
    evaluate (__LINE__, __func__, "content3", data[0].content);
    evaluate (__LINE__, __func__, 2, data[1].rowid);
    evaluate (__LINE__, __func__, "phpunit2", data[1].username);
    if ((data[1].timestamp < now) || (data[1].timestamp > now + 1)) evaluate (__LINE__, __func__, now, data[1].timestamp);
    evaluate (__LINE__, __func__, 4, data[1].action);
    evaluate (__LINE__, __func__, "content4", data[1].content);
  }

  // Update notes.
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("unittest", 2, 3, "content");
    database.record ("unittest", 2, 4, "content");
    database.record ("unittest", 3, 3, "content");
    database.updateNotes (2, 12345);
    vector <int> notes = database.getNotes ();
    evaluate (__LINE__, __func__, {12345, 3}, notes);
  }

  // Delete.
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("phpunit1", 2, 3, "content1");
    database.record ("phpunit2", 4, 5, "content2");
    database.erase (1);
    vector <int> notes = database.getNotes ();
    evaluate (__LINE__, __func__, {4}, notes);
  }

  // Exists
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    evaluate (__LINE__, __func__, false, database.exists (2));
    database.record ("phpunit1", 2, 3, "content1");
    database.record ("phpunit2", 4, 5, "content2");
    evaluate (__LINE__, __func__, true, database.exists (2));
    evaluate (__LINE__, __func__, false, database.exists (3));
  }
}


void test_database_notes ()
{
  trace_unit_tests (__func__);

  // Database path.
  {
    refresh_sandbox (true);
    Webserver_Request request;
    Database_Notes database_notes (&request);
    string path = database_notes.database_path ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes.sqlite"), path);
    path = database_notes.checksums_database_path ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes_checksums.sqlite"), path);
  }
  
  // Test the note file routine.
  {
    Webserver_Request request;
    Database_Notes database_notes (&request);
    string file = database_notes.note_file (123456789);
    evaluate (__LINE__, __func__, filter_url_create_root_path ("consultations", "123", "456789.json"), file);
  }

  // Trim and optimize note.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    database_notes.optimize ();
    int identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    database_notes.erase (identifier);
    database_notes.trim ();
    database_notes.trim_server ();
    // The logbook will have an entry about "Deleting empty notes folder".
    refresh_sandbox (true, {"Deleting empty notes folder"});
  }

  // Identifier.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    int identifier = Notes_Logic::lowNoteIdentifier;
    evaluate (__LINE__, __func__, 100000000, identifier);
    
    identifier = Notes_Logic::highNoteIdentifier;
    evaluate (__LINE__, __func__, 999999999, identifier);
    
    identifier = database_notes.get_new_unique_identifier ();
    if ((identifier < 100000000) || (identifier > 999999999)) evaluate (__LINE__, __func__, "Out of bounds", convert_to_string (identifier));
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists (identifier));
    
    identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    evaluate (__LINE__, __func__, true, database_notes.identifier_exists (identifier));
    database_notes.erase (identifier);
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists (identifier));

    identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    evaluate (__LINE__, __func__, true, database_notes.identifier_exists (identifier));
    database_notes.erase (identifier);
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists (identifier));
  }

  // Summary and contents.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();

    string value;
    vector <string> values;
    int length = 0;
    size_t pos = 0;

    // Test inserting data for both summary and contents.
    string summary = "Summary";
    string contents = "Contents";
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, summary, contents, false);
    value = database_notes.get_summary (newidentifier);
    evaluate (__LINE__, __func__, summary, value);
    value = database_notes.get_contents (newidentifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 1) value = values[1];
    evaluate (__LINE__, __func__, "<p>Contents</p>", value);
    
    // Test that if the summary is not given, it is going to be the first line of the contents.
    contents = "This is a note.\nLine two.";
    newidentifier = database_notes.store_new_note ("", 0, 0, 0, "", contents, false);
    value = database_notes.get_summary (newidentifier);
    evaluate (__LINE__, __func__, "This is a note.", value);
    value = database_notes.get_contents (newidentifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 2) value = values[2];
    evaluate (__LINE__, __func__, "<p>Line two.</p>", value);
    
    // Test setting the summary.
    database_notes.set_summary (newidentifier, "summary2");
    value = database_notes.get_summary (newidentifier);
    evaluate (__LINE__, __func__, "summary2", value);
    
    // Test setting the note contents.
    database_notes.set_contents (newidentifier, "contents2");
    value = database_notes.get_contents (newidentifier);
    evaluate (__LINE__, __func__, "contents2", value);
    
    // Test adding comment.
    value = database_notes.get_contents (newidentifier);
    length = value.length ();
    database_notes.add_comment (newidentifier, "comment2");
    value = database_notes.get_contents (newidentifier);
    if (value.length () < (size_t) (length + 30)) evaluate (__LINE__, __func__, "Should be larger than length + 30", convert_to_string ((int)value.length()));
    pos = value.find ("comment2");
    if (pos == string::npos) evaluate (__LINE__, __func__, "Should contain 'comment2'", value);
    // Universal method to add comment.
    database_notes.add_comment (newidentifier, "comment5");
    value = database_notes.get_contents (newidentifier);
    if (value.length () < (size_t) (length + 30)) evaluate (__LINE__, __func__, "Should be larger than length + 30", convert_to_string ((int)value.length()));
    pos = value.find ("comment5");
    if (pos == string::npos) evaluate (__LINE__, __func__, "Should contain 'comment5'", value);
  }

  // Test subscriptions.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    Notes_Logic notes_logic = Notes_Logic(&request);
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    
    // Normally creating a new note would subscribe the current user to the note.
    // But since this unit test runs without sessions, it would have subscribed an empty user.
    request.session_logic()->setUsername ("");
    int identifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> subscribers = database_notes.get_subscribers (identifier);
    evaluate (__LINE__, __func__, {}, subscribers);
    
    // Create a note again, but this time set the session variable to a certain user.
    database_users.add_user ("unittest", "", 5, "");
    request.session_logic()->setUsername ("unittest");
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (true);
    identifier = database_notes.store_new_note ("", 1, 1, 1, "Summary", "Contents", false);
    notes_logic.handlerNewNote (identifier);
    subscribers = database_notes.get_subscribers (identifier);
    evaluate (__LINE__, __func__, {"unittest"}, subscribers);
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier, "unittest"));
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    // Test various other subscription related functions.
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest_unittest"));
    database_notes.unsubscribe (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe_user (identifier, "unittest_unittest_unittest");
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier, "unittest_unittest_unittest"));
    database_notes.unsubscribe_user (identifier, "unittest_unittest_unittest");
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest_unittest_unittest"));
    
    // With the username still set, test the plan subscribe and unsubscribe mechanisms.
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    identifier = database_notes.store_new_note ("", 1, 1, 1, "Summary", "Contents", false);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe (identifier);
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.unsubscribe (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe (identifier);
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.unsubscribe (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier, "unittest"));
    
    // Test subscribing and unsubscribing other users.
    database_notes.subscribe_user (identifier, "a");
    database_notes.subscribe_user (identifier, "b");
    subscribers = database_notes.get_subscribers (identifier);
    evaluate (__LINE__, __func__, {"a", "b"}, subscribers);
    database_notes.unsubscribe_user (identifier, "a");
    subscribers = database_notes.get_subscribers (identifier);
    evaluate (__LINE__, __func__, {"b"}, subscribers);
    database_notes.set_subscribers (identifier, {"aa", "bb"});
    subscribers = database_notes.get_subscribers (identifier);
    evaluate (__LINE__, __func__, {"aa", "bb"}, subscribers);
  }

  // Test assignments.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    
    request.session_logic()->setUsername ("unittest2");
    
    // Create a note and check that it was not assigned to anybody.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {}, assignees);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary2", "Contents2", false);
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {}, assignees);

    // Assign the note to a user, and check that this reflects in the list of assignees.
    database_notes.assign_user (oldidentifier, "unittest");
    assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    database_notes.assign_user (newidentifier, "unittest");
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    
    // Test the set_assignees function.
    database_notes.set_assignees (oldidentifier, {"unittest"});
    assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    database_notes.set_assignees (newidentifier, {"unittest"});
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    
    // Assign note to second user, and check it reflects.
    database_notes.assign_user (oldidentifier, "unittest2");
    assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {"unittest", "unittest2"}, assignees);
    database_notes.assign_user (newidentifier, "unittest3");
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {"unittest", "unittest3"}, assignees);
    
    // Based on the above, check the is_assigned function.
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (oldidentifier, "unittest"));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (oldidentifier, "unittest2"));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned (oldidentifier, "PHPUnit3"));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (newidentifier, "unittest"));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (newidentifier, "unittest3"));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned (newidentifier, "PHPUnit3"));
    
    // Based on the above, test get_all_assignees.
    assignees = database_notes.get_all_assignees ({""});
    evaluate (__LINE__, __func__, {"unittest", "unittest2", "unittest3"}, assignees);
    
    // Based on the above, test the unassign_user function.
    database_notes.unassign_user (oldidentifier, "unittest");
    assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {"unittest2"}, assignees);
    database_notes.unassign_user (oldidentifier, "unittest2");
    assignees = database_notes.get_assignees (oldidentifier);
    evaluate (__LINE__, __func__, {}, assignees);
    database_notes.unassign_user (newidentifier, "unittest");
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {"unittest3"}, assignees);
    database_notes.unassign_user (newidentifier, "unittest3");
    assignees = database_notes.get_assignees (newidentifier);
    evaluate (__LINE__, __func__, {}, assignees);
  }

  // Test the getters and the setters for the Bible.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    int oldidentifier = database_notes.store_new_note ("unittest", 0, 0, 0, "Summary", "Contents", false);
    string bible = database_notes.get_bible (oldidentifier);
    evaluate (__LINE__, __func__, "unittest", bible);
    int newidentifier = database_notes.store_new_note ("unittest2", 0, 0, 0, "Summary", "Contents", false);
    bible = database_notes.get_bible (newidentifier);
    evaluate (__LINE__, __func__, "unittest2", bible);
    database_notes.set_bible (oldidentifier, "PHPUnit2");
    bible = database_notes.get_bible (oldidentifier);
    evaluate (__LINE__, __func__, "PHPUnit2", bible);
    database_notes.set_bible (newidentifier, "PHPUnit3");
    bible = database_notes.get_bible (newidentifier);
    evaluate (__LINE__, __func__, "PHPUnit3", bible);
    database_notes.set_bible (oldidentifier, "");
    bible = database_notes.get_bible (oldidentifier);
    evaluate (__LINE__, __func__, "", bible);
    database_notes.set_bible (newidentifier, "");
    bible = database_notes.get_bible (newidentifier);
    evaluate (__LINE__, __func__, "", bible);
  }

  // Test getting and setting the passage(s).
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create notes for certain passages.
    int oldidentifier = database_notes.store_new_note ("", 10, 9, 8, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 5, 4, 3, "Summary", "Contents", false);
    
    // Test getting passage.
    vector <Passage> passages = database_notes.get_passages (oldidentifier);
    Passage standard = Passage ("", 10, 9, "8");
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
    passages = database_notes.get_passages (newidentifier);
    standard = Passage ("", 5, 4, "3");
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
    
    // Test setting the passage.
    standard = Passage ("", 5, 6, "7");
    database_notes.set_passages (oldidentifier, {standard});
    passages = database_notes.get_passages (oldidentifier);
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
    standard = Passage ("", 12, 13, "14");
    database_notes.set_passages (newidentifier, {standard});
    passages = database_notes.get_passages (newidentifier);
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
  }

  // Test getting and setting the note status.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create notes.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default status = New.
    string status = database_notes.get_status (oldidentifier);
    evaluate (__LINE__, __func__, "New", status);
    status = database_notes.get_status (newidentifier);
    evaluate (__LINE__, __func__, "New", status);
    
    // Test setting the status.
    database_notes.set_status (oldidentifier, "xxxxx");
    status = database_notes.get_status (oldidentifier);
    evaluate (__LINE__, __func__, "xxxxx", status);
    database_notes.set_status (newidentifier, "yyyyy");
    status = database_notes.get_status (newidentifier);
    evaluate (__LINE__, __func__, "yyyyy", status);
    
    // Test getting all possible statuses.
    vector <Database_Notes_Text> statuses = database_notes.get_possible_statuses ();
    vector <string> rawstatuses;
    for (auto & status : statuses) {
      rawstatuses.push_back (status.raw);
    }
    evaluate (__LINE__, __func__, {"yyyyy", "xxxxx", "New", "Pending", "In progress", "Done", "Reopened"}, rawstatuses);
  }

  // Getting and setting the severity.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create note.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default severity = Normal.
    string severity = database_notes.get_severity (oldidentifier);
    evaluate (__LINE__, __func__, "Normal", severity);
    severity = database_notes.get_severity (newidentifier);
    evaluate (__LINE__, __func__, "Normal", severity);
    
    // Test setting the severity.
    database_notes.set_raw_severity (oldidentifier, 0);
    severity = database_notes.get_severity (oldidentifier);
    evaluate (__LINE__, __func__, "Wish", severity);
    database_notes.set_raw_severity (newidentifier, 0);
    severity = database_notes.get_severity (newidentifier);
    evaluate (__LINE__, __func__, "Wish", severity);
    database_notes.set_raw_severity (oldidentifier, 4);
    severity = database_notes.get_severity (oldidentifier);
    evaluate (__LINE__, __func__, "Major", severity);
    database_notes.set_raw_severity (newidentifier, 4);
    severity = database_notes.get_severity (newidentifier);
    evaluate (__LINE__, __func__, "Major", severity);
    
    // Test getting all unique severities.
    vector <Database_Notes_Text> severities = database_notes.get_possible_severities ();
    vector <string> rawseverities;
    vector <string> localizedseverities;
    for (auto & severity : severities) {
      rawseverities.push_back (severity.raw);
      localizedseverities.push_back (severity.localized);
    }
    evaluate (__LINE__, __func__, {"0", "1", "2", "3", "4", "5"}, rawseverities);
    evaluate (__LINE__, __func__, {"Wish", "Minor", "Normal", "Important", "Major", "Critical"}, localizedseverities);
  }

  // Test setting and getting the "modified" property of notes.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    int time = filter_date_seconds_since_epoch ();
    
    // Create note.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test getter.
    int value = database_notes.get_modified (oldidentifier);
    if ((value < time) || (value > time + 1)) evaluate (__LINE__, __func__, time, value);
    value = database_notes.get_modified (newidentifier);
    if ((value < time) || (value > time + 1)) evaluate (__LINE__, __func__, time, value);
    
    // Test setter.
    time = 123456789;
    database_notes.set_modified (oldidentifier, time);
    value = database_notes.get_modified (oldidentifier);
    evaluate (__LINE__, __func__, time, value);;
    database_notes.set_modified (newidentifier, time);
    value = database_notes.get_modified (newidentifier);
    evaluate (__LINE__, __func__, time, value);;
  }

  // Get identifiers.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create a few notes.
    vector <int> standardids;
    for (unsigned int i = 0; i < 3; i++) {
      int identifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
      standardids.push_back (identifier);
    }
    
    // Get the identifiers.
    vector <int> identifiers = database_notes.get_identifiers ();
    sort (standardids.begin(), standardids.end());
    sort (identifiers.begin(), identifiers.end());
    evaluate (__LINE__, __func__, standardids, identifiers);
  }

  // Setting the note identifier.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create note.
    request.session_logic()->setUsername ("unittest");
    int identifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    
    // Contents of the note.
    string original_contents1 = database_notes.get_contents (identifier1);
    if (original_contents1.length () <= 20) evaluate (__LINE__, __func__, "Should be greater than 20", convert_to_string ((int) original_contents1.length ()));
    string original_contents2 = database_notes.get_contents (identifier2);
    if (original_contents2.length () <= 20) evaluate (__LINE__, __func__, "Should be greater than 20", convert_to_string ((int) original_contents2.length ()));
    
    // Checksum of the notes.
    string original_checksum1 = database_notes.get_checksum (identifier1);
    evaluate (__LINE__, __func__, 32, (int) original_checksum1.length());
    string original_checksum2 = database_notes.get_checksum (identifier2);
    evaluate (__LINE__, __func__, 32, (int) original_checksum2.length());
    
    // Change the identifier.
    int new_id1 = database_notes.get_new_unique_identifier ();
    database_notes.set_identifier (identifier1, new_id1);
    int new_id2 = database_notes.get_new_unique_identifier ();
    database_notes.set_identifier (identifier2, new_id2);
    
    // Check old and new identifier for v2 and v2.
    string contents = database_notes.get_contents (identifier1);
    evaluate (__LINE__, __func__, "", contents);
    contents = database_notes.get_contents (new_id1);
    evaluate (__LINE__, __func__, original_contents1, contents);
    contents = database_notes.get_contents (identifier2);
    evaluate (__LINE__, __func__, "", contents);
    contents = database_notes.get_contents (new_id2);
    evaluate (__LINE__, __func__, original_contents2, contents);
    
    string checksum = database_notes.get_checksum (identifier1);
    evaluate (__LINE__, __func__, "", checksum);
    checksum = database_notes.get_checksum (new_id1);
    evaluate (__LINE__, __func__, original_checksum1, checksum);
    checksum = database_notes.get_checksum (identifier2);
    evaluate (__LINE__, __func__, "", checksum);
    checksum = database_notes.get_checksum (new_id2);
    evaluate (__LINE__, __func__, original_checksum2, checksum);
  }

  // Testing note due for deletion.
  {
    // It tests whether a note marked for deletion,
    // after touching it 7 or 8 times, is returned as due for deletion,
    // and whether it is not yet due for deletion before that.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    
    database_notes.mark_for_deletion (oldidentifier);
    database_notes.mark_for_deletion (newidentifier);
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    
    vector <int> identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier, newidentifier}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier, newidentifier}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier, newidentifier}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier, newidentifier}, identifiers);
  }

  // Test unmarking a note for deletion.
  {
    // It tests whethe a note marked for deletion,
    // touched 6 times, then unmarked, touched again,
    // will not be due for deletion.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);

    database_notes.mark_for_deletion (oldidentifier);
    database_notes.mark_for_deletion (newidentifier);
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.unmark_for_deletion (oldidentifier);
    database_notes.unmark_for_deletion (newidentifier);

    vector <int> identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
  }

  // Test touching several notes marked for deletion.
  {
    // It tests whether three notes, marked for deletion on different days,
    // are properly touched so they keep their own number of days.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    int oldidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);

    database_notes.mark_for_deletion (oldidentifier1);
    database_notes.mark_for_deletion (newidentifier1);
    database_notes.touch_marked_for_deletion ();
    database_notes.mark_for_deletion (oldidentifier2);
    database_notes.mark_for_deletion (newidentifier2);
    database_notes.touch_marked_for_deletion ();
    database_notes.mark_for_deletion (oldidentifier3);
    database_notes.mark_for_deletion (newidentifier3);
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();

    vector <int> identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier1, newidentifier1}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier1, newidentifier1}, identifiers);

    database_notes.unmark_for_deletion (oldidentifier1);
    database_notes.unmark_for_deletion (newidentifier1);
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier2, newidentifier2}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier2, newidentifier2}, identifiers);
    
    database_notes.unmark_for_deletion (oldidentifier2);
    database_notes.unmark_for_deletion (newidentifier2);
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier3, newidentifier3}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    evaluate (__LINE__, __func__, {oldidentifier3, newidentifier3}, identifiers);
  }

  // Testing whether note is marked for deletion.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    int oldidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);

    database_notes.mark_for_deletion (oldidentifier1);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.mark_for_deletion (newidentifier1);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));
    
    database_notes.unmark_for_deletion (oldidentifier2);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.unmark_for_deletion (newidentifier2);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));

    database_notes.unmark_for_deletion (oldidentifier1);
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.unmark_for_deletion (newidentifier1);
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier2));

    database_notes.mark_for_deletion (oldidentifier2);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier2));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier3));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (oldidentifier3));
    database_notes.mark_for_deletion (newidentifier2);
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier2));
    evaluate (__LINE__, __func__, true, database_notes.is_marked_for_deletion (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier3));
    evaluate (__LINE__, __func__, false, database_notes.is_marked_for_deletion (newidentifier3));
  }

  // Test operations on the checksum.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create note to work with.
    int oldidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    
    // Checksum of new note should be calculated.
    string good_checksum_old = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, good_checksum_old.empty());
    string good_checksum_new = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, good_checksum_new.empty());
    // The two should match.
    evaluate (__LINE__, __func__, good_checksum_old, good_checksum_new);
    
    // Clear checksum, and recalculate it.
    string outdated_checksum = "outdated checksum";
    database_notes.set_checksum (oldidentifier, outdated_checksum);
    string checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, outdated_checksum, checksum);
    database_notes.set_checksum (newidentifier, outdated_checksum);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, outdated_checksum, checksum);
    database_notes.sync ();
    // Sometimes something strange happens:
    // At times the checksum gets erased as the sync routine cannot find the original note.
    // The sync (2) call did not make any difference.
    checksum = database_notes.get_checksum (oldidentifier);
    if (!checksum.empty()) evaluate (__LINE__, __func__, good_checksum_old, checksum);
    database_notes.set_checksum (oldidentifier, outdated_checksum);
    checksum = database_notes.get_checksum (newidentifier);
    if (!checksum.empty()) evaluate (__LINE__, __func__, good_checksum_new, checksum);
    database_notes.set_checksum (newidentifier, outdated_checksum);
    
    // Test that saving a note updates the checksum in most cases.
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_modified (oldidentifier, 1234567);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_modified (newidentifier, 1234567);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());

    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_subscribers (oldidentifier, {"subscribers"});
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_subscribers (newidentifier, {"subscribers"});
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_bible (oldidentifier, "unittest");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_bible (newidentifier, "unittest");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());

    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_passages (oldidentifier, {});
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_passages (newidentifier, {});
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_status (oldidentifier, "Status");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_status (newidentifier, "Status");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_raw_severity (oldidentifier, 123);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_raw_severity (newidentifier, 123);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_summary (oldidentifier, "new");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_summary (newidentifier, "new");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_contents (oldidentifier, "new");
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_contents (newidentifier, "new");
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
  }

  // Test sync logic checksums.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create notes to work with.
    vector <int> oldidentifiers;
    oldidentifiers.push_back (database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false));
    oldidentifiers.push_back (database_notes.store_new_note ("bible2", 2, 3, 4, "summary2", "contents2", false));
    oldidentifiers.push_back (database_notes.store_new_note ("bible3", 3, 4, 5, "summary3", "contents3", false));
    vector <int> newidentifiers;
    newidentifiers.push_back (database_notes.store_new_note ("bible4", 4, 5, 6, "summary4", "contents4", false));
    newidentifiers.push_back (database_notes.store_new_note ("bible5", 5, 6, 7, "summary5", "contents5", false));
    newidentifiers.push_back (database_notes.store_new_note ("bible6", 6, 7, 8, "summary6", "contents6", false));
    
    // Checksum calculation: slow and fast methods should be the same.
    Sync_Logic sync_logic = Sync_Logic (&request);
    string oldchecksum1 = sync_logic.checksum (oldidentifiers);
    evaluate (__LINE__, __func__, 32, (int)oldchecksum1.length());
    string oldchecksum2 = database_notes.get_multiple_checksum (oldidentifiers);
    evaluate (__LINE__, __func__, oldchecksum1, oldchecksum2);
    string newchecksum1 = sync_logic.checksum (newidentifiers);
    evaluate (__LINE__, __func__, 32, (int)newchecksum1.length());
    string newchecksum2 = database_notes.get_multiple_checksum (newidentifiers);
    evaluate (__LINE__, __func__, newchecksum1, newchecksum2);
  }

  // Test updating checksums.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create notes to work with.
    int oldidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);

    // Check checksum.
    string oldchecksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, 32, oldchecksum.length ());
    string newchecksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, oldchecksum, newchecksum);
    
    // Clear it and set the checksum again.
    database_notes.delete_checksum (oldidentifier);
    string checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.update_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    evaluate (__LINE__, __func__, oldchecksum, checksum);
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.update_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    evaluate (__LINE__, __func__, newchecksum, checksum);
  }

  // Test getting notes within a notes range for given Bibles.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int identifier = database_notes.store_new_note ("bible1", 1, 2, 3, "summary", "contents", false);
    int identifier1 = 100000000;
    database_notes.set_identifier (identifier, identifier1);
    
    identifier = database_notes.store_new_note ("bible2", 1, 2, 3, "summary", "contents", false);
    int identifier2 = 500000000;
    database_notes.set_identifier (identifier, identifier2);
    
    identifier = database_notes.store_new_note ("", 1, 2, 3, "summary", "contents", false);
    int identifier3 = 999999999;
    database_notes.set_identifier (identifier, identifier3);
    
    // Test selection mechanism for certain Bibles.
    vector <int> identifiers = database_notes.get_notes_in_range_for_bibles (100000000, 999999999, {"bible1", "bible2"}, false);
    evaluate (__LINE__, __func__, {100000000, 500000000, 999999999}, identifiers);
    
    identifiers = database_notes.get_notes_in_range_for_bibles (100000000, 999999999, {"bible1", "bible3"}, false);
    evaluate (__LINE__, __func__, {100000000, 999999999}, identifiers);
    
    identifiers = database_notes.get_notes_in_range_for_bibles (100000000, 999999999, {}, false);
    evaluate (__LINE__, __func__, {999999999}, identifiers);
    
    // Test selection mechanism for any Bible.
    identifiers = database_notes.get_notes_in_range_for_bibles (100000000, 999999999, {}, true);
    evaluate (__LINE__, __func__, {100000000, 500000000, 999999999}, identifiers);
  }

  // Test creating a range of identifiers.
  {
    Webserver_Request request;
    Sync_Logic sync_logic = Sync_Logic (&request);
    
    vector <Sync_Logic_Range> ranges = sync_logic.create_range (100000000, 999999999);
    evaluate (__LINE__, __func__, 10, (int)ranges.size());
    evaluate (__LINE__, __func__, 100000000, ranges[0].low);
    evaluate (__LINE__, __func__, 189999998, ranges[0].high);
    evaluate (__LINE__, __func__, 189999999, ranges[1].low);
    evaluate (__LINE__, __func__, 279999997, ranges[1].high);
    evaluate (__LINE__, __func__, 279999998, ranges[2].low);
    evaluate (__LINE__, __func__, 369999996, ranges[2].high);
    evaluate (__LINE__, __func__, 369999997, ranges[3].low);
    evaluate (__LINE__, __func__, 459999995, ranges[3].high);
    evaluate (__LINE__, __func__, 459999996, ranges[4].low);
    evaluate (__LINE__, __func__, 549999994, ranges[4].high);
    evaluate (__LINE__, __func__, 549999995, ranges[5].low);
    evaluate (__LINE__, __func__, 639999993, ranges[5].high);
    evaluate (__LINE__, __func__, 639999994, ranges[6].low);
    evaluate (__LINE__, __func__, 729999992, ranges[6].high);
    evaluate (__LINE__, __func__, 729999993, ranges[7].low);
    evaluate (__LINE__, __func__, 819999991, ranges[7].high);
    evaluate (__LINE__, __func__, 819999992, ranges[8].low);
    evaluate (__LINE__, __func__, 909999990, ranges[8].high);
    evaluate (__LINE__, __func__, 909999991, ranges[9].low);
    evaluate (__LINE__, __func__, 999999999, ranges[9].high);
    
    ranges = sync_logic.create_range (100000000, 100000100);
    evaluate (__LINE__, __func__, 10, (int)ranges.size());
    evaluate (__LINE__, __func__, 100000000, ranges[0].low);
    evaluate (__LINE__, __func__, 100000009, ranges[0].high);
    evaluate (__LINE__, __func__, 100000010, ranges[1].low);
    evaluate (__LINE__, __func__, 100000019, ranges[1].high);
    evaluate (__LINE__, __func__, 100000020, ranges[2].low);
    evaluate (__LINE__, __func__, 100000029, ranges[2].high);
    evaluate (__LINE__, __func__, 100000030, ranges[3].low);
    evaluate (__LINE__, __func__, 100000039, ranges[3].high);
    evaluate (__LINE__, __func__, 100000040, ranges[4].low);
    evaluate (__LINE__, __func__, 100000049, ranges[4].high);
    evaluate (__LINE__, __func__, 100000050, ranges[5].low);
    evaluate (__LINE__, __func__, 100000059, ranges[5].high);
    evaluate (__LINE__, __func__, 100000060, ranges[6].low);
    evaluate (__LINE__, __func__, 100000069, ranges[6].high);
    evaluate (__LINE__, __func__, 100000070, ranges[7].low);
    evaluate (__LINE__, __func__, 100000079, ranges[7].high);
    evaluate (__LINE__, __func__, 100000080, ranges[8].low);
    evaluate (__LINE__, __func__, 100000089, ranges[8].high);
    evaluate (__LINE__, __func__, 100000090, ranges[9].low);
    evaluate (__LINE__, __func__, 100000100, ranges[9].high);
  }

  // Test selecting based on a given Bible.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int identifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
    // Select notes while varying Bible selection.
    vector <int> identifiers = database_notes.select_notes ({"bible1"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier1}, identifiers);
    
    identifiers = database_notes.select_notes ({"bible1", "bible2"}, 0, 0, 0, 3, 0, 0, "", "bible2", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier2}, identifiers);
    
    identifiers = database_notes.select_notes ({"bible1", "bible2"}, 0, 0, 0, 3, 0, 0, "", "", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier1, identifier2}, identifiers);
    
    identifiers = database_notes.select_notes ({"bible1", "bible2", "bible4"}, 0, 0, 0, 3, 0, 0, "", "bible", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, identifiers);
    
    identifiers = database_notes.select_notes ({}, 0, 0, 0, 3, 0, 0, "", "", "", "", -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, identifiers);
    
    identifiers = database_notes.select_notes ({"bible1", "bible2", "bible3"}, 0, 0, 0, 3, 0, 0, "", "bible3", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier3}, identifiers);
    
    identifiers = database_notes.select_notes ({}, 0, 0, 0, 3, 0, 0, "", "bible3", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier3}, identifiers);
  }

  // Test the resilience of the notes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    bool healthy = database_notes.healthy ();
    evaluate (__LINE__, __func__, true, healthy);
    
    string corrupted_database = filter_url_create_root_path ("unittests", "tests", "notes.sqlite.damaged");
    string path = database_notes.database_path ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));
    
    healthy = database_notes.healthy ();
    evaluate (__LINE__, __func__, false, healthy);
    
    database_notes.checkup ();
    healthy = database_notes.healthy ();
    evaluate (__LINE__, __func__, true, healthy);

    // Clean the generated logbook entries away invisibly.
    refresh_sandbox (false);
  }

  // Test the resilience of the checksums of notes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();

    bool healthy = database_notes.checksums_healthy ();
    evaluate (__LINE__, __func__, true, healthy);
    
    string corrupted_database = filter_url_create_root_path ("unittests", "tests", "notes.sqlite.damaged");
    string path = database_notes.checksums_database_path ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));

    healthy = database_notes.checksums_healthy ();
    evaluate (__LINE__, __func__, false, healthy);

    database_notes.checkup_checksums ();
    healthy = database_notes.checksums_healthy ();
    evaluate (__LINE__, __func__, true, healthy);

    // Clear the generated and expected logbook entries away invisibly.
    refresh_sandbox (false);
  }

  // Availability.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    evaluate (__LINE__, __func__, true, database_notes.available ());
    database_notes.set_availability (false);
    evaluate (__LINE__, __func__, false, database_notes.available ());
    database_notes.set_availability (true);
    evaluate (__LINE__, __func__, true, database_notes.available ());
  }

  // Testing public notes.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int oldidentifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int oldidentifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int oldidentifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    int newidentifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int newidentifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int newidentifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
    // None of them, or others, are public notes.
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier3));
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier1 + 1));
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier1));
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier3));
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier1 + 1));
    
    // Set some public.
    database_notes.set_public (oldidentifier1, true);
    database_notes.set_public (oldidentifier2, true);
    database_notes.set_public (newidentifier1, true);
    database_notes.set_public (newidentifier2, true);
    evaluate (__LINE__, __func__, true, database_notes.get_public (oldidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.get_public (oldidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier3));
    evaluate (__LINE__, __func__, true, database_notes.get_public (newidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.get_public (newidentifier2));
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier3));
    
    // Unset some of them.
    database_notes.set_public (oldidentifier1, false);
    evaluate (__LINE__, __func__, false, database_notes.get_public (oldidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.get_public (oldidentifier2));
    database_notes.set_public (newidentifier1, false);
    evaluate (__LINE__, __func__, false, database_notes.get_public (newidentifier1));
    evaluate (__LINE__, __func__, true, database_notes.get_public (newidentifier2));
  }

  // Bulk notes transfer elaborate tests for notes stored in JSON format.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Keep the stored values for the notes.
    vector <string> v_assigned;
    vector <string> v_bible;
    vector <string> v_contents;
    vector <int> v_identifier;
    vector <int> v_modified;
    vector <string> v_passage;
    vector <int> v_severity;
    vector <string> v_status;
    vector <string> v_subscriptions;
    vector <string> v_summary;
    
    // Create several notes.
    for (int i = 0; i < 5; i++) {
      // Basic fields for the note.
      string offset = convert_to_string (i);
      string bible = "bible" + offset;
      int book = i;
      int chapter = i + 1;
      int verse = i + 2;
      string summary = "summary" + offset;
      string contents = "contents" + offset;
      int identifier = database_notes.store_new_note (bible, book, chapter, verse, summary, contents, false);
      database_notes.set_contents (identifier, contents);
      // Additional fields for the note.
      string assigned = "assigned" + offset;
      database_notes.set_raw_assigned (identifier, assigned);
      string passage = "passage" + offset;
      database_notes.set_raw_passage (identifier, passage);
      int severity = 4 * i;
      database_notes.set_raw_severity (identifier, severity);
      string status = "status" + offset;
      database_notes.set_status (identifier, status);
      string subscriptions = "subscriptions" + offset;
      database_notes.set_raw_subscriptions (identifier, subscriptions);
      // Store modification time last because the previous functions update it.
      int modified = 2 * i;
      database_notes.set_modified (identifier, modified);
      // Store all fields for the round-trip check.
      v_assigned.push_back (assigned);
      v_bible.push_back (bible);
      v_contents.push_back (contents);
      v_identifier.push_back (identifier);
      v_modified.push_back (modified);
      v_passage.push_back (passage);
      v_severity.push_back (severity);
      v_status.push_back (status);
      v_subscriptions.push_back (subscriptions);
      v_summary.push_back (summary);
    }
    
    // Get the checksums for later reference.
    vector <string> checksums;
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      database_notes.update_checksum (identifier);
      string checksum = database_notes.get_checksum (identifier);
      checksums.push_back (checksum);
    }
    
    // Get some search results for later reference.
    vector <int> search_results;
    search_results = database_notes.select_notes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    
    // Get the notes in bulk in a database.
    string json = database_notes.get_bulk (v_identifier);
    
    // Delete all notes again.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      evaluate (__LINE__, __func__, false, database_notes.get_summary (identifier).empty ());
      evaluate (__LINE__, __func__, false, database_notes.get_contents (identifier).empty ());
      evaluate (__LINE__, __func__, false, database_notes.get_bible (identifier).empty ());
      evaluate (__LINE__, __func__, false, database_notes.get_raw_passage (identifier).empty ());
      evaluate (__LINE__, __func__, false, database_notes.get_raw_status (identifier).empty ());
      evaluate (__LINE__, __func__, true, database_notes.get_raw_severity (identifier) != 2);
      evaluate (__LINE__, __func__, true, database_notes.get_modified (identifier) < 1000);
      database_notes.erase (identifier);
      evaluate (__LINE__, __func__, "", database_notes.get_summary (identifier));
      evaluate (__LINE__, __func__, "", database_notes.get_contents (identifier));
      evaluate (__LINE__, __func__, "", database_notes.get_bible (identifier));
      evaluate (__LINE__, __func__, "", database_notes.get_raw_passage (identifier));
      evaluate (__LINE__, __func__, "", database_notes.get_raw_status (identifier));
      evaluate (__LINE__, __func__, 2, database_notes.get_raw_severity (identifier));
      evaluate (__LINE__, __func__, 0, database_notes.get_modified (identifier));
    }
    
    // The checksums should now be gone.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string checksum = database_notes.get_checksum (identifier);
      evaluate (__LINE__, __func__, "", checksum);
    }
    
    // There should be no search results anymore.
    vector <int> no_search_results;
    no_search_results = database_notes.select_notes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, no_search_results);
    
    // Copy the notes from the database back to the filesystem.
    database_notes.set_bulk (json);
    
    // Check that the notes are back.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string assigned = database_notes.get_raw_assigned (identifier);
      evaluate (__LINE__, __func__, v_assigned [i], assigned);
      string bible = database_notes.get_bible (identifier);
      evaluate (__LINE__, __func__, v_bible [i], bible);
      string contents = database_notes.get_contents (identifier);
      evaluate (__LINE__, __func__, v_contents [i], contents);
      int modified = database_notes.get_modified (identifier);
      evaluate (__LINE__, __func__, v_modified [i], modified);
      string passage = database_notes.get_raw_passage (identifier);
      evaluate (__LINE__, __func__, v_passage [i], passage);
      int severity = database_notes.get_raw_severity (identifier);
      evaluate (__LINE__, __func__, v_severity [i], severity);
      string status = database_notes.get_raw_status (identifier);
      evaluate (__LINE__, __func__, v_status [i], status);
      string subscriptions = database_notes.get_raw_subscriptions (identifier);
      evaluate (__LINE__, __func__, v_subscriptions [i], subscriptions);
      string summary = database_notes.get_summary (identifier);
      evaluate (__LINE__, __func__, v_summary [i], summary);
    }
    
    // The checksums should be back also.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string checksum = database_notes.get_checksum (identifier);
      evaluate (__LINE__, __func__, checksums [i], checksum);
    }
    
    // The search results should be back too.
    vector <int> restored_search;
    restored_search = database_notes.select_notes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, search_results, restored_search);
  }

  // Test updating the search database for the notes with JSON storage.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    // Test values for the note.
    string contents ("contents");
    
    // Create note.
    int identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    // Creating the note updates the search database.
    // Basic search should work now.
    vector <int> identifiers;
    
    // Search on the content of the current note.
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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
    database_notes.set_raw_contents (identifier, contents);
    // Doing a search now does not give results.
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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
    database_notes.update_database (identifier);
    database_notes.update_search_fields (identifier);
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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
    database_notes.set_raw_passage (identifier, " 1.2.3 ");
    // There should be no search results yet when searching on the new passage.
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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
    database_notes.update_database (identifier);
    identifiers = database_notes.select_notes ({}, // No Bibles given.
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

  // Test methods for getting and setting note properties.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    
    string bible1 = "bible1";
    string bible2 = "bible2";
    Passage passage1 = Passage ("", 1, 2, "3");
    Passage passage2 = Passage ("", 4, 5, "6");
    
    // Create notes.
    int oldidentifier1 = database_notes.store_new_note (bible1, passage1.book, passage1.chapter, convert_to_int (passage1.verse), "v1", "v1", false);
    int identifier1 = oldidentifier1 + 2;
    int oldidentifier2 = database_notes.store_new_note (bible2, passage2.book, passage2.chapter, convert_to_int (passage2.verse), "v2", "v2", false);
    int identifier2 = oldidentifier2 + 4;
    
    // Call the method to set a new identifier.
    database_notes.set_identifier (oldidentifier1, identifier1);
    database_notes.set_identifier (oldidentifier2, identifier2);
    
    // Test the methods to get or to set the summaries.
    string summary1 = "summary1";
    string summary2 = "summary2";
    database_notes.set_summary (identifier1, summary1);
    database_notes.set_summary (identifier2, summary2);
    evaluate (__LINE__, __func__, summary1, database_notes.get_summary (identifier1));
    evaluate (__LINE__, __func__, summary2, database_notes.get_summary (identifier2));
    
    // Test the methods to get and to set the contents.
    string contents1 = "contents1";
    string contents2 = "contents2";
    database_notes.set_contents (identifier1, contents1);
    database_notes.set_contents (identifier2, contents2);
    string contents = database_notes.get_contents (identifier1);
    evaluate (__LINE__, __func__, true, contents.find (contents1) != string::npos);
    evaluate (__LINE__, __func__, contents, database_notes.get_contents (identifier1));
    contents = database_notes.get_contents (identifier2);
    evaluate (__LINE__, __func__, true, contents.find (contents2) != string::npos);
    evaluate (__LINE__, __func__, contents, database_notes.get_contents (identifier2));
    
    // Test the method to get the subscribers.
    string subscriber1 = "subscriber1";
    string subscriber2 = "subscriber2";
    database_notes.set_subscribers (identifier1, { subscriber1 });
    database_notes.set_subscribers (identifier2, { subscriber2 });
    evaluate (__LINE__, __func__, { subscriber1 }, database_notes.get_subscribers (identifier1));
    evaluate (__LINE__, __func__, { subscriber2 }, database_notes.get_subscribers (identifier2));
    
    // Test the method to test a subscriber to a note.
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier1, subscriber1));
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed (identifier2, subscriber2));
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier1, subscriber2));
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed (identifier2, subscriber1));
    
    // Test the methods for the assignees.
    string assignee1 = "assignee1";
    string assignee2 = "assignee2";
    database_notes.set_assignees (identifier1, { assignee1 });
    database_notes.set_assignees (identifier2, { assignee2 });
    evaluate (__LINE__, __func__, { assignee1 }, database_notes.get_assignees (identifier1));
    evaluate (__LINE__, __func__, { assignee2 }, database_notes.get_assignees (identifier2));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (identifier1, assignee1));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned (identifier2, assignee2));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned (identifier1, assignee2));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned (identifier2, assignee1));
    
    // Test the methods for the Bible.
    evaluate (__LINE__, __func__, bible1, database_notes.get_bible (identifier1));
    evaluate (__LINE__, __func__, bible2, database_notes.get_bible (identifier2));
    
    // Test the methods for the passage.
    vector <Passage> passages;
    passages = database_notes.get_passages (identifier1);
    evaluate (__LINE__, __func__, 1, passages.size());
    for (auto passage : passages) evaluate (__LINE__, __func__, true, passage1.equal (passage));
    passages = database_notes.get_passages (identifier2);
    evaluate (__LINE__, __func__, 1, passages.size());
    for (auto passage : passages) evaluate (__LINE__, __func__, true, passage2.equal (passage));
    evaluate (__LINE__, __func__, " 1.2.3 ", database_notes.decode_passage (identifier1));
    evaluate (__LINE__, __func__, " 4.5.6 ", database_notes.decode_passage (identifier2));
    
    // Test the methods for the status.
    string status1 = "status1";
    string status2 = "status2";
    database_notes.set_status (identifier1, status1);
    database_notes.set_status (identifier2, status2);
    evaluate (__LINE__, __func__, status1, database_notes.get_raw_status (identifier1));
    evaluate (__LINE__, __func__, status1, database_notes.get_status (identifier1));
    evaluate (__LINE__, __func__, status2, database_notes.get_raw_status (identifier2));
    evaluate (__LINE__, __func__, status2, database_notes.get_status (identifier2));
    
    // Test the methods for the severity.
    int severity1 = 4;
    int severity2 = 5;
    vector <string> standard_severities = database_notes.standard_severities ();
    database_notes.set_raw_severity (identifier1, severity1);
    database_notes.set_raw_severity (identifier2, severity2);
    evaluate (__LINE__, __func__, severity1, database_notes.get_raw_severity (identifier1));
    evaluate (__LINE__, __func__, severity2, database_notes.get_raw_severity (identifier2));
    evaluate (__LINE__, __func__, standard_severities [severity1], database_notes.get_severity (identifier1));
    evaluate (__LINE__, __func__, standard_severities [severity2], database_notes.get_severity (identifier2));
    
    // Test the methods for the modification time.
    int modified1 = 1000;
    int modified2 = 2000;
    database_notes.set_modified (identifier1, modified1);
    database_notes.set_modified (identifier2, modified2);
    evaluate (__LINE__, __func__, modified1, database_notes.get_modified (identifier1));
    evaluate (__LINE__, __func__, modified2, database_notes.get_modified (identifier2));
    
    // Test the methods for a note being public.
    database_notes.set_public (identifier1, true);
    database_notes.set_public (identifier2, false);
    evaluate (__LINE__, __func__, true, database_notes.get_public (identifier1));
    evaluate (__LINE__, __func__, false, database_notes.get_public (identifier2));
    database_notes.set_public (identifier1, false);
    database_notes.set_public (identifier2, true);
    evaluate (__LINE__, __func__, false, database_notes.get_public (identifier1));
    evaluate (__LINE__, __func__, true, database_notes.get_public (identifier2));
  }

}


void test_database_noteassignment ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_NoteAssignment database;
  
  bool exists = database.exists ("unittest");
  evaluate (__LINE__, __func__, false, exists);
  
  vector <string> assignees = database.assignees ("unittest");
  evaluate (__LINE__, __func__, {}, assignees);
  
  database.assignees ("unittest", {"one", "two"});
  assignees = database.assignees ("none-existing");
  evaluate (__LINE__, __func__, {}, assignees);
  
  exists = database.exists ("unittest");
  evaluate (__LINE__, __func__, true, exists);
  
  assignees = database.assignees ("unittest");
  evaluate (__LINE__, __func__, {"one", "two"}, assignees);
  
  database.assignees ("unittest", {"1", "2"});
  assignees = database.assignees ("unittest");
  evaluate (__LINE__, __func__, {"1", "2"}, assignees);
  
  exists = database.exists ("unittest", "1");
  evaluate (__LINE__, __func__, true, exists);
  exists = database.exists ("unittest", "none-existing");
  evaluate (__LINE__, __func__, false, exists);
}

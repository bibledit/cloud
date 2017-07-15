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
  // Get Notes
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
  // Get Note Data
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
  // Update Notes.
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
    string path = database_notes.database_path_v12 ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes.sqlite"), path);
    path = database_notes.checksums_database_path_v12 ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes_checksums.sqlite"), path);
  }
  
  // Test the old note folder and the new note file routines.
  {
    Webserver_Request request;
    Database_Notes database_notes (&request);
    string folder = database_notes.note_folder_v1 (123456789);
    evaluate (__LINE__, __func__, filter_url_create_root_path ("consultations", "123", "456", "789"), folder);
    string file = database_notes.note_file_v2 (123456789);
    evaluate (__LINE__, __func__, filter_url_create_root_path ("consultations", "123", "456789.json"), file);
  }
  
  // Trim and optimize.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    database_notes.optimize_v12 ();
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "", "", false);
    database_notes.erase_v12 (identifier);
    database_notes.trim_v12 ();
    database_notes.trim_server_v12 ();
    // The logbook will have an entry about "Deleting empty notes folder".
    refresh_sandbox (true, {"Deleting empty notes folder"});
  }

  // Trim and optimize note V2.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    database_notes.optimize_v12 ();
    int identifier = database_notes.store_new_note_v2 ("", 0, 0, 0, "", "", false);
    database_notes.erase_v12 (identifier);
    database_notes.trim_v12 ();
    database_notes.trim_server_v12 ();
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
    database_notes.create_v12 ();
    
    int identifier = Notes_Logic::lowNoteIdentifier;
    evaluate (__LINE__, __func__, 100000000, identifier);
    
    identifier = Notes_Logic::highNoteIdentifier;
    evaluate (__LINE__, __func__, 999999999, identifier);
    
    identifier = database_notes.getNewUniqueIdentifier ();
    if ((identifier < 100000000) || (identifier > 999999999)) evaluate (__LINE__, __func__, "Out of bounds", convert_to_string (identifier));
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists_v12 (identifier));
    
    identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "", "", false);
    evaluate (__LINE__, __func__, true, database_notes.identifier_exists_v12 (identifier));
    database_notes.erase_v12 (identifier);
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists_v12 (identifier));

    identifier = database_notes.store_new_note_v2 ("", 0, 0, 0, "", "", false);
    evaluate (__LINE__, __func__, true, database_notes.identifier_exists_v12 (identifier));
    database_notes.erase_v12 (identifier);
    evaluate (__LINE__, __func__, false, database_notes.identifier_exists_v12 (identifier));
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
    database_notes.create_v12 ();
    
    // Test inserting data for both summary and contents.
    string summary = "Summary";
    string contents = "Contents";
    // Old storage.
    int oldidentifier = database_notes.store_new_note_v1 ("", 0, 0, 0, summary, contents, false);
    string value = database_notes.get_summary_v1 (oldidentifier);
    evaluate (__LINE__, __func__, summary, value);
    value = database_notes.get_contents_v1 (oldidentifier);
    vector <string> values = filter_string_explode (value, '\n');
    if (values.size () > 2) value = values[2];
    evaluate (__LINE__, __func__, "<p>Contents</p>", value);
    // New storage.
    int newidentifier = database_notes.store_new_note_v2 ("", 0, 0, 0, summary, contents, false);
    value = database_notes.get_summary_v2 (newidentifier);
    evaluate (__LINE__, __func__, summary, value);
    value = database_notes.get_contents_v2 (newidentifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 2) value = values[2];
    evaluate (__LINE__, __func__, "<p>Contents</p>", value);
    
    // Test that if the summary is not given, it is going to be the first line of the contents.
    contents = "This is a note.\nLine two.";
    // Old storage.
    oldidentifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "", contents, false);
    value = database_notes.get_summary_v1 (oldidentifier);
    evaluate (__LINE__, __func__, "This is a note.", value);
    value = database_notes.get_contents_v1 (oldidentifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 3) value = values[3];
    evaluate (__LINE__, __func__, "<p>Line two.</p>", value);
    // New JSON storage.
    newidentifier = database_notes.store_new_note_v2 ("", 0, 0, 0, "", contents, false);
    value = database_notes.get_summary_v2 (newidentifier);
    evaluate (__LINE__, __func__, "This is a note.", value);
    value = database_notes.get_contents_v2 (newidentifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 3) value = values[3];
    evaluate (__LINE__, __func__, "<p>Line two.</p>", value);
    
    // Test setting the summary.
    database_notes.set_summary_v1 (oldidentifier, "summary1");
    value = database_notes.get_summary_v1 (oldidentifier);
    evaluate (__LINE__, __func__, "summary1", value);
    database_notes.set_summary_v2 (newidentifier, "summary2");
    value = database_notes.get_summary_v2 (newidentifier);
    evaluate (__LINE__, __func__, "summary2", value);
    
    // Test setting the note contents.
    database_notes.set_contents_v1 (oldidentifier, "contents1");
    value = database_notes.get_contents_v1 (oldidentifier);
    evaluate (__LINE__, __func__, "contents1", value);
    database_notes.set_contents_v2 (newidentifier, "contents2");
    value = database_notes.get_contents_v2 (newidentifier);
    evaluate (__LINE__, __func__, "contents2", value);
    
    // Test adding comment.
    // Old storage.
    value = database_notes.get_contents_v1 (oldidentifier);
    int length = value.length ();
    database_notes.add_comment_v1 (oldidentifier, "comment1");
    value = database_notes.get_contents_v1 (oldidentifier);
    if (value.length () < (size_t) (length + 30)) evaluate (__LINE__, __func__, "Should be larger than length + 30", convert_to_string ((int)value.length()));
    size_t pos = value.find ("comment1");
    if (pos == string::npos) evaluate (__LINE__, __func__, "Should contain 'comment1'", value);
    // New storage.
    value = database_notes.get_contents_v2 (newidentifier);
    length = value.length ();
    database_notes.add_comment_v2 (newidentifier, "comment2");
    value = database_notes.get_contents_v2 (newidentifier);
    if (value.length () < (size_t) (length + 30)) evaluate (__LINE__, __func__, "Should be larger than length + 30", convert_to_string ((int)value.length()));
    pos = value.find ("comment2");
    if (pos == string::npos) evaluate (__LINE__, __func__, "Should contain 'comment2'", value);
  }

  // Test subscriptions for the old notes storage.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    Notes_Logic notes_logic = Notes_Logic(&request);
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    
    // Normally creating a new note would subscribe the current user to the note.
    // But since this unit test runs without sessions, it would have subscribed an empty user.
    request.session_logic()->setUsername ("");
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> subscribers = database_notes.get_subscribers_v1 (identifier);
    evaluate (__LINE__, __func__, {}, subscribers);
    
    // Create a note again, but this time set the session variable to a certain user.
    database_users.add_user ("unittest", "", 5, "");
    request.session_logic()->setUsername ("unittest");
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (true);
    identifier = database_notes.store_new_note_v1 ("", 1, 1, 1, "Summary", "Contents", false);
    notes_logic.handler_new_note_v1 (identifier);
    subscribers = database_notes.get_subscribers_v1 (identifier);
    evaluate (__LINE__, __func__, {"unittest"}, subscribers);
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v1 (identifier, "unittest"));
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    // Test various other subscription related functions.
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v1 (identifier, "phpunit_phpunit"));
    database_notes.unsubscribe_v1 (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v1 (identifier, "unittest"));
    database_notes.subscribe_user_v1 (identifier, "phpunit_phpunit_phpunit");
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v1 (identifier, "phpunit_phpunit_phpunit"));
    database_notes.unsubscribe_user_v1 (identifier, "phpunit_phpunit_phpunit");
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v1 (identifier, "phpunit_phpunit_phpunit"));
  }

  // Test subscriptions for the new JSON notes storage. Todo working here.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    Notes_Logic notes_logic = Notes_Logic(&request);
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    
    // Normally creating a new note would subscribe the current user to the note.
    // But since this unit test runs without sessions, it would have subscribed an empty user.
    request.session_logic()->setUsername ("");
    int identifier = database_notes.store_new_note_v2 ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> subscribers = database_notes.get_subscribers_v2 (identifier); // Todo
    evaluate (__LINE__, __func__, {}, subscribers);
    
    // Create a note again, but this time set the session variable to a certain user.
    database_users.add_user ("unittest", "", 5, "");
    request.session_logic()->setUsername ("unittest");
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (true);
    identifier = database_notes.store_new_note_v2 ("", 1, 1, 1, "Summary", "Contents", false);
    notes_logic.handler_new_note_v2 (identifier); // Todo
    subscribers = database_notes.get_subscribers_v2 (identifier);
    // Todo fix evaluate (__LINE__, __func__, {"unittest"}, subscribers);
    // Todo fix evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v2 (identifier, "unittest"));
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    // Test various other subscription related functions.
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v2 (identifier, "unittest_unittest"));
    database_notes.unsubscribe_v2 (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v2 (identifier, "unittest"));
    database_notes.subscribe_user_v1 (identifier, "unittest_unittest_unittest");
    // Todo fix evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v2 (identifier, "unittest_unittest_unittest")); 
    database_notes.unsubscribe_user_v1 (identifier, "unittest_unittest_unittest");
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v2 (identifier, "unittest_unittest_unittest"));
    
    // With the username still set, test the plan subscribe and unsubscribe mechanisms.
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    identifier = database_notes.store_new_note_v2 ("", 1, 1, 1, "Summary", "Contents", false);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v2 (identifier, "unittest"));
    database_notes.subscribe_v2 (identifier);
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v2 (identifier, "unittest"));
    database_notes.unsubscribe_v2 (identifier);
    evaluate (__LINE__, __func__, false, database_notes.is_subscribed_v2 (identifier, "unittest"));
    
    // Test subscribing and unsubscribing other users.
    database_notes.subscribe_user_v2 (identifier, "a");
    database_notes.subscribe_user_v2 (identifier, "b");
    subscribers = database_notes.get_subscribers_v2 (identifier);
    evaluate (__LINE__, __func__, {"a", "b"}, subscribers);
    database_notes.unsubscribe_user_v2 (identifier, "a");
    subscribers = database_notes.get_subscribers_v2 (identifier);
    evaluate (__LINE__, __func__, {"b"}, subscribers);
    database_notes.set_subscribers_v2 (identifier, {"aa", "bb"});
    subscribers = database_notes.get_subscribers_v2 (identifier);
    evaluate (__LINE__, __func__, {"aa", "bb"}, subscribers);
  }
  
  // Assignments ()
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    
    request.session_logic()->setUsername ("unittest2");
    
    // Create a note and check that it was not assigned to anybody.
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {}, assignees);
    
    // Assign the note to a user, and check that this reflects in the list of assignees.
    database_notes.assign_user_v1 (identifier, "unittest");
    assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    
    // Test the set_assignees_v1 function.
    database_notes.set_assignees_v1 (identifier, {"unittest"});
    assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {"unittest"}, assignees);
    
    // Assign note to second user, and check it reflects.
    database_notes.assign_user_v1 (identifier, "unittest2");
    assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {"unittest", "unittest2"}, assignees);
    
    // Based on the above, check the is_assigned_v1 function.
    evaluate (__LINE__, __func__, true, database_notes.is_assigned_v1 (identifier, "unittest"));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned_v1 (identifier, "unittest2"));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned_v1 (identifier, "PHPUnit3"));
    
    // Based on the above, test get_all_assignees_v12().
    assignees = database_notes.get_all_assignees_v12 ({""});
    evaluate (__LINE__, __func__, {"unittest", "unittest2"}, assignees);
    
    // Based on the above, test the unassign_user_v1 function.
    database_notes.unassign_user_v1 (identifier, "unittest");
    assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {"unittest2"}, assignees);
    database_notes.unassign_user_v1 (identifier, "unittest2");
    assignees = database_notes.get_assignees_v1 (identifier);
    evaluate (__LINE__, __func__, {}, assignees);
  }
  // Bible
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    int identifier = database_notes.store_new_note_v1 ("unittest", 0, 0, 0, "Summary", "Contents", false);
    string bible = database_notes.getBible (identifier);
    evaluate (__LINE__, __func__, "unittest", bible);
    database_notes.setBible (identifier, "PHPUnit2");
    bible = database_notes.getBible (identifier);
    evaluate (__LINE__, __func__, "PHPUnit2", bible);
    database_notes.setBible (identifier, "");
    bible = database_notes.getBible (identifier);
    evaluate (__LINE__, __func__, "", bible);
  }
  // Passage.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create note for a certain passage.
    int identifier = database_notes.store_new_note_v1 ("", 10, 9, 8, "Summary", "Contents", false);
    
    // Test the getPassages method.
    vector <Passage> passages = database_notes.getPassages (identifier);
    Passage standard = Passage ("", 10, 9, "8");
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
    
    // Test the setPassage method.
    standard = Passage ("", 5, 6, "7");
    database_notes.setPassages (identifier, {standard});
    passages = database_notes.getPassages (identifier);
    evaluate (__LINE__, __func__, 1, (int)passages.size());
    evaluate (__LINE__, __func__, true, standard.equal (passages [0]));
  }
  // Status.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create note.
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default status = New.
    string status = database_notes.getStatus (identifier);
    evaluate (__LINE__, __func__, "New", status);
    
    // Test setStatus function.
    database_notes.setStatus (identifier, "xxxxx");
    status = database_notes.getStatus (identifier);
    evaluate (__LINE__, __func__, "xxxxx", status);
    
    // Test the getStatuses function.
    vector <Database_Notes_Text> statuses = database_notes.getPossibleStatuses ();
    vector <string> rawstatuses;
    for (auto & status : statuses) {
      rawstatuses.push_back (status.raw);
    }
    evaluate (__LINE__, __func__, {"xxxxx", "New", "Pending", "In progress", "Done", "Reopened"}, rawstatuses);
  }
  // Severity
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create note.
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default severity = Normal.
    string severity = database_notes.getSeverity (identifier);
    evaluate (__LINE__, __func__, "Normal", severity);
    
    // Test setSeverity.
    database_notes.setRawSeverity (identifier, 0);
    severity = database_notes.getSeverity (identifier);
    evaluate (__LINE__, __func__, "Wish", severity);
    database_notes.setRawSeverity (identifier, 4);
    severity = database_notes.getSeverity (identifier);
    evaluate (__LINE__, __func__, "Major", severity);
    
    // Test getSeverities.
    vector <Database_Notes_Text> severities = database_notes.getPossibleSeverities ();
    vector <string> rawseverities;
    vector <string> localizedseverities;
    for (auto & severity : severities) {
      rawseverities.push_back (severity.raw);
      localizedseverities.push_back (severity.localized);
    }
    evaluate (__LINE__, __func__, {"0", "1", "2", "3", "4", "5"}, rawseverities);
    evaluate (__LINE__, __func__, {"Wish", "Minor", "Normal", "Important", "Major", "Critical"}, localizedseverities);
  }
  // Modified
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    int time = filter_date_seconds_since_epoch ();
    
    // Create note.
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test getModified.
    int value = database_notes.getModified (identifier);
    if ((value < time) || (value > time + 1)) evaluate (__LINE__, __func__, time, value);
    // Test setModified.
    time = 123456789;
    database_notes.set_modified_v1 (identifier, time);
    value = database_notes.getModified (identifier);
    evaluate (__LINE__, __func__, time, value);;
  }
  // GetIdentifiers
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    request.session_logic()->setUsername ("unittest");
    
    // Create a few notes.
    vector <int> standardids;
    for (unsigned int i = 0; i < 3; i++) {
      int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
      standardids.push_back (identifier);
    }
    
    // Get the identifiers.
    vector <int> identifiers = database_notes.getIdentifiers ();
    sort (standardids.begin(), standardids.end());
    sort (identifiers.begin(), identifiers.end());
    evaluate (__LINE__, __func__, standardids, identifiers);
  }
  // SetIdentifier
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Create note.
    request.session_logic()->setUsername ("unittest");
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    
    // Contents of the note.
    string originalContents = database_notes.get_contents_v1 (identifier);
    if (originalContents.length () <= 20) evaluate (__LINE__, __func__, "Should be greater than 20", convert_to_string ((int)originalContents.length ()));
    
    // Checksum of the note.
    string originalChecksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, 32, (int)originalChecksum.length());
    
    // Change the identifier.
    int newId = 1234567;
    database_notes.setIdentifier (identifier, newId);
    
    // Check old and new identifier.
    string contents = database_notes.get_contents_v1 (identifier);
    evaluate (__LINE__, __func__, "", contents);
    contents = database_notes.get_contents_v1 (newId);
    evaluate (__LINE__, __func__, originalContents, contents);
    
    string checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    checksum = database_notes.getChecksum (newId);
    evaluate (__LINE__, __func__, originalChecksum, checksum);
  }
  // ExpireOneNote
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
    database_notes.create_v12 ();
    
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    database_notes.markForDeletion (identifier);
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    vector <int> identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {identifier}, identifiers);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {identifier}, identifiers);
  }
  // ExpireUnmarkNote.
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
    database_notes.create_v12 ();
    
    int identifier = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    database_notes.markForDeletion (identifier);
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.unmarkForDeletion (identifier);
    vector <int> identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
  }
  // ExpireMoreNotes.
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
    database_notes.create_v12 ();
    
    int identifier1 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    int identifier3 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    database_notes.markForDeletion (identifier1);
    database_notes.touchMarkedForDeletion ();
    database_notes.markForDeletion (identifier2);
    database_notes.touchMarkedForDeletion ();
    database_notes.markForDeletion (identifier3);
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    database_notes.touchMarkedForDeletion ();
    vector <int> identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {}, identifiers);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {identifier1}, identifiers);
    database_notes.unmarkForDeletion (identifier1);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {identifier2}, identifiers);
    database_notes.unmarkForDeletion (identifier2);
    database_notes.touchMarkedForDeletion ();
    identifiers = database_notes.getDueForDeletion ();
    evaluate (__LINE__, __func__, {identifier3}, identifiers);
  }
  // ExpireIsMarked
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    int identifier1 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    int identifier3 = database_notes.store_new_note_v1 ("", 0, 0, 0, "summary", "contents", false);
    database_notes.markForDeletion (identifier1);
    evaluate (__LINE__, __func__, true, database_notes.isMarkedForDeletion (identifier1));
    evaluate (__LINE__, __func__, false, database_notes.isMarkedForDeletion (identifier2));
    database_notes.unmarkForDeletion (identifier2);
    evaluate (__LINE__, __func__, true, database_notes.isMarkedForDeletion (identifier1));
    evaluate (__LINE__, __func__, false, database_notes.isMarkedForDeletion (identifier2));
    database_notes.unmarkForDeletion (identifier1);
    evaluate (__LINE__, __func__, false, database_notes.isMarkedForDeletion (identifier1));
    evaluate (__LINE__, __func__, false, database_notes.isMarkedForDeletion (identifier2));
    database_notes.markForDeletion (identifier2);
    evaluate (__LINE__, __func__, true, database_notes.isMarkedForDeletion (identifier2));
    evaluate (__LINE__, __func__, false, database_notes.isMarkedForDeletion (identifier3));
  }
  // ChecksumOne
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Create note to work with.
    int identifier = database_notes.store_new_note_v1 ("bible", 1, 2, 3, "summary", "contents", false);
    
    // Checksum of new note should be calculated.
    string good_checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, good_checksum.empty());
    
    // Clear checksum, and recalculate it.
    string outdated_checksum = "outdated checksum";
    database_notes.setChecksum (identifier, outdated_checksum);
    string checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, outdated_checksum, checksum);
    database_notes.sync ();
    checksum = database_notes.getChecksum (identifier);
    // Something strange happens:
    // At times the checksum gets erased as the sync routine cannot find the original note.
    // The sync (2) call did not make any difference.
    if (!checksum.empty()) evaluate (__LINE__, __func__, good_checksum, checksum);
    
    // Test that saving a note updates the checksum in most cases.
    database_notes.setChecksum (identifier, "");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_modified_v1 (identifier, 1234567);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_subscribers_v1 (identifier, {"subscribers"});
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.setChecksum (identifier, "");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setBible (identifier, "unittest");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setPassages (identifier, {});
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.setChecksum (identifier, "");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setStatus (identifier, "Status");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setRawSeverity (identifier, 123);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.setChecksum (identifier, "");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_summary_v1 (identifier, "new");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
    
    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.set_contents_v1 (identifier, "new");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());
  }
  // ChecksumTwo
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Create notes to work with.
    vector <int> identifiers;
    identifiers.push_back (database_notes.store_new_note_v1 ("bible1", 1, 2, 3, "summary1", "contents1", false));
    identifiers.push_back (database_notes.store_new_note_v1 ("bible2", 2, 3, 4, "summary2", "contents2", false));
    identifiers.push_back (database_notes.store_new_note_v1 ("bible3", 3, 4, 5, "summary3", "contents3", false));
    
    // Checksum calculation: slow and fast methods should be the same.
    Sync_Logic sync_logic = Sync_Logic (&request);
    string checksum1 = sync_logic.checksum (identifiers);
    evaluate (__LINE__, __func__, 32, (int)checksum1.length());
    string checksum2 = database_notes.getMultipleChecksum (identifiers);
    evaluate (__LINE__, __func__, checksum1, checksum2);
  }
  // GetNotesInRangeForBibles ()
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Create a couple of notes to work with.
    int identifier = database_notes.store_new_note_v1 ("bible1", 1, 2, 3, "summary", "contents", false);
    int identifier1 = 100000000;
    database_notes.setIdentifier (identifier, identifier1);
    
    identifier = database_notes.store_new_note_v1 ("bible2", 1, 2, 3, "summary", "contents", false);
    int identifier2 = 500000000;
    database_notes.setIdentifier (identifier, identifier2);
    
    identifier = database_notes.store_new_note_v1 ("", 1, 2, 3, "summary", "contents", false);
    int identifier3 = 999999999;
    database_notes.setIdentifier (identifier, identifier3);
    
    // Test selection mechanism for certain Bibles.
    vector <int> identifiers = database_notes.getNotesInRangeForBibles (100000000, 999999999, {"bible1", "bible2"}, false);
    evaluate (__LINE__, __func__, {100000000, 500000000, 999999999}, identifiers);
    
    identifiers = database_notes.getNotesInRangeForBibles (100000000, 999999999, {"bible1", "bible3"}, false);
    evaluate (__LINE__, __func__, {100000000, 999999999}, identifiers);
    
    identifiers = database_notes.getNotesInRangeForBibles (100000000, 999999999, {}, false);
    evaluate (__LINE__, __func__, {999999999}, identifiers);
    
    // Test selection mechanism for any Bible.
    identifiers = database_notes.getNotesInRangeForBibles (100000000, 999999999, {}, true);
    evaluate (__LINE__, __func__, {100000000, 500000000, 999999999}, identifiers);
  }
  // CreateRange
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
  // SelectBible
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    // Create a couple of notes to work with.
    int identifier1 = database_notes.store_new_note_v1 ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.store_new_note_v1 ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.store_new_note_v1 ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
    // Select notes while varying Bible selection.
    vector <int> identifiers = database_notes.selectNotes ({"bible1"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier1}, identifiers);
    
    identifiers = database_notes.selectNotes ({"bible1", "bible2"}, 0, 0, 0, 3, 0, 0, "", "bible2", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier2}, identifiers);
    
    identifiers = database_notes.selectNotes ({"bible1", "bible2"}, 0, 0, 0, 3, 0, 0, "", "", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier1, identifier2}, identifiers);
    
    identifiers = database_notes.selectNotes ({"bible1", "bible2", "bible4"}, 0, 0, 0, 3, 0, 0, "", "bible", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, identifiers);
    
    identifiers = database_notes.selectNotes ({}, 0, 0, 0, 3, 0, 0, "", "", "", "", -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, identifiers);
    
    identifiers = database_notes.selectNotes ({"bible1", "bible2", "bible3"}, 0, 0, 0, 3, 0, 0, "", "bible3", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier3}, identifiers);
    
    identifiers = database_notes.selectNotes ({}, 0, 0, 0, 3, 0, 0, "", "bible3", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {identifier3}, identifiers);
  }
  // ResilienceNotes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    bool healthy = database_notes.healthy_v12 ();
    evaluate (__LINE__, __func__, true, healthy);
    
    string corrupted_database = filter_url_create_root_path ("unittests", "tests", "notes.sqlite.damaged");
    string path = database_notes.database_path_v12 ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));
    
    healthy = database_notes.healthy_v12 ();
    evaluate (__LINE__, __func__, false, healthy);
    
    database_notes.checkup_v12 ();
    healthy = database_notes.healthy_v12 ();
    evaluate (__LINE__, __func__, true, healthy);
  }
  // ResilienceChecksumsNotes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
    bool healthy = database_notes.checksums_healthy_v12 ();
    evaluate (__LINE__, __func__, true, healthy);
    
    string corrupted_database = filter_url_create_root_path ("unittests", "tests", "notes.sqlite.damaged");
    string path = database_notes.checksums_database_path_v12 ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));
    
    healthy = database_notes.checksums_healthy_v12 ();
    evaluate (__LINE__, __func__, false, healthy);
    
    database_notes.checkup_checksums_v12 ();
    healthy = database_notes.checksums_healthy_v12 ();
    evaluate (__LINE__, __func__, true, healthy);
  }
  // Availability.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
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
    database_notes.create_v12 ();
    
    // Create a couple of notes to work with.
    int identifier1 = database_notes.store_new_note_v1 ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.store_new_note_v1 ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.store_new_note_v1 ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
    // None of them, or others, are public notes.
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier1));
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier2));
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier3));
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier1 + 1));
    
    // Set some public.
    database_notes.setPublic (identifier1, true);
    database_notes.setPublic (identifier2, true);
    evaluate (__LINE__, __func__, true, database_notes.getPublic (identifier1));
    evaluate (__LINE__, __func__, true, database_notes.getPublic (identifier2));
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier3));
    
    // Unset some of them.
    database_notes.setPublic (identifier1, false);
    evaluate (__LINE__, __func__, false, database_notes.getPublic (identifier1));
    evaluate (__LINE__, __func__, true, database_notes.getPublic (identifier2));
  }
  
  // Bulk notes transfer elaborate tests.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();
    
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
      int identifier = database_notes.store_new_note_v1 (bible, book, chapter, verse, summary, contents, false);
      database_notes.set_contents_v1 (identifier, contents);
      // Additional fields for the note.
      string assigned = "assigned" + offset;
      filter_url_file_put_contents (database_notes.assigned_file_v1 (identifier), assigned);
      int modified = 2 * i;
      filter_url_file_put_contents (database_notes.modified_file_v1 (identifier), convert_to_string (modified));
      string passage = "passage" + offset;
      filter_url_file_put_contents (database_notes.passage_file_v1 (identifier), passage);
      int severity = 4 * i;
      filter_url_file_put_contents (database_notes.severity_file_v1 (identifier), convert_to_string (severity));
      string status = "status" + offset;
      filter_url_file_put_contents (database_notes.status_file_v1 (identifier), status);
      string subscriptions = "subscriptions" + offset;
      filter_url_file_put_contents (database_notes.subscriptions_file_v1 (identifier), subscriptions);
      // Store all fields round-trip check.
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
      database_notes.updateChecksum (identifier);
      string checksum = database_notes.getChecksum (identifier);
      checksums.push_back (checksum);
    }
    
    // Get some search results for later reference.
    vector <int> search_results;
    search_results = database_notes.selectNotes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    
    // Get the notes in bulk in a database.
    string json = database_notes.getBulk (v_identifier);
    
    // Delete all notes again.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      database_notes.erase_v12 (identifier);
      evaluate (__LINE__, __func__, "", database_notes.get_summary_v1 (identifier));
      evaluate (__LINE__, __func__, "", database_notes.get_contents_v1 (identifier));
      evaluate (__LINE__, __func__, "", database_notes.getBible (identifier));
      evaluate (__LINE__, __func__, "", database_notes.getRawPassage (identifier));
      evaluate (__LINE__, __func__, "", database_notes.getRawStatus (identifier));
      evaluate (__LINE__, __func__, 2, database_notes.getRawSeverity (identifier));
      evaluate (__LINE__, __func__, 0, database_notes.getModified (identifier));
    }
    
    // The checksums should now be gone.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string checksum = database_notes.getChecksum (identifier);
      evaluate (__LINE__, __func__, "", checksum);
    }
    
    // There should be no search results anymore.
    vector <int> no_search_results;
    no_search_results = database_notes.selectNotes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, {}, no_search_results);
    
    // Copy the notes from the database back to the filesystem.
    database_notes.setBulk (json);
    
    // Check the notes are back.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string assigned = filter_url_file_get_contents (database_notes.assigned_file_v1 (identifier));
      evaluate (__LINE__, __func__, v_assigned [i], assigned);
      string bible = database_notes.getBible (identifier);
      evaluate (__LINE__, __func__, v_bible [i], bible);
      string contents = database_notes.get_contents_v1 (identifier);
      evaluate (__LINE__, __func__, v_contents [i], contents);
      int modified = database_notes.getModified (identifier);
      evaluate (__LINE__, __func__, v_modified [i], modified);
      string passage = database_notes.getRawPassage (identifier);
      evaluate (__LINE__, __func__, v_passage [i], passage);
      int severity = database_notes.getRawSeverity (identifier);
      evaluate (__LINE__, __func__, v_severity [i], severity);
      string status = database_notes.getRawStatus (identifier);
      evaluate (__LINE__, __func__, v_status [i], status);
      string subscriptions = filter_url_file_get_contents (database_notes.subscriptions_file_v1 (identifier));
      evaluate (__LINE__, __func__, v_subscriptions [i], subscriptions);
      string summary = database_notes.get_summary_v1 (identifier);
      evaluate (__LINE__, __func__, v_summary [i], summary);
    }
    
    // The checksums should be back.
    for (int i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      string checksum = database_notes.getChecksum (identifier);
      evaluate (__LINE__, __func__, checksums [i], checksum);
    }
    
    // The search results should be back.
    vector <int> restored_search;
    restored_search = database_notes.selectNotes ({"bible2"}, 0, 0, 0, 3, 0, 0, "", "bible1", "", false, -1, 0, "", -1);
    evaluate (__LINE__, __func__, search_results, restored_search);
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



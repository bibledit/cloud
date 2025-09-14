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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/noteactions.h>
#include <database/notes.h>
#include <database/state.h>
#include <database/mail.h>
#include <database/noteassignment.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/note.h>
#include <filter/text.h>
#include <webserver/request.h>
#include <database/login.h>
#include <notes/logic.h>
#include <sync/logic.h>


TEST (notes, database_noteactions)
{

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
    std::vector <int> notes = database.getNotes ();
    EXPECT_EQ (std::vector <int>{2}, notes);
  }

  // Get notes
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("unittest", 2, 3, "content");
    database.record ("unittest", 2, 4, "content");
    database.record ("unittest", 3, 3, "content");
    std::vector <int> notes = database.getNotes ();
    std::vector <int> standard_notes{2, 3};
    EXPECT_EQ (standard_notes, notes);
  }

  // Get note data
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("phpunit1", 2, 3, "content3");
    database.record ("phpunit2", 2, 4, "content4");
    database.record ("phpunit3", 3, 4, "content5");
    std::vector <Database_Note_Action> data = database.getNoteData (2);
    EXPECT_EQ (2, static_cast<int>(data.size()));
    int now = filter::date::seconds_since_epoch ();
    EXPECT_EQ (1, data[0].rowid);
    EXPECT_EQ ("phpunit1", data[0].username);
    if ((data[0].timestamp < now - 1) || (data[0].timestamp > now + 2)) EXPECT_EQ (now, data[0].timestamp);
    EXPECT_EQ (3, data[0].action);
    EXPECT_EQ ("content3", data[0].content);
    EXPECT_EQ (2, data[1].rowid);
    EXPECT_EQ ("phpunit2", data[1].username);
    if ((data[1].timestamp < now) || (data[1].timestamp > now + 1)) EXPECT_EQ (now, data[1].timestamp);
    EXPECT_EQ (4, data[1].action);
    EXPECT_EQ ("content4", data[1].content);
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
    std::vector <int> notes = database.getNotes ();
    const std::vector <int> standard_notes{12345, 3};
    EXPECT_EQ (standard_notes, notes);
  }

  // Delete.
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("phpunit1", 2, 3, "content1");
    database.record ("phpunit2", 4, 5, "content2");
    database.erase (1);
    std::vector <int> notes = database.getNotes ();
    EXPECT_EQ (std::vector <int>{4}, notes);
  }

  // Exists
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    EXPECT_EQ (false, database.exists (2));
    database.record ("phpunit1", 2, 3, "content1");
    database.record ("phpunit2", 4, 5, "content2");
    EXPECT_EQ (true, database.exists (2));
    EXPECT_EQ (false, database.exists (3));
  }
}


void test_database_notes ()
{
  // Database path.
  {
    refresh_sandbox (true);
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    std::string path = database_notes.database_path ();
    EXPECT_EQ (filter_url_create_root_path ({"databases", "notes.sqlite"}), path);
    path = database_notes.checksums_database_path ();
    EXPECT_EQ (filter_url_create_root_path ({"databases", "notes_checksums.sqlite"}), path);
  }
  
  // Test the note file routine.
  {
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    std::string file = database_notes.note_file (123456789);
    EXPECT_EQ (filter_url_create_root_path ({"consultations", "123", "456789.json"}), file);
  }

  // Trim and optimize note.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
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
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    int identifier = Notes_Logic::lowNoteIdentifier;
    EXPECT_EQ (100'000'000, identifier);
    
    identifier = Notes_Logic::highNoteIdentifier;
    EXPECT_EQ (999'999'999, identifier);
    
    identifier = database_notes.get_new_unique_identifier ();
    if ((identifier < 100'000'000) || (identifier > 999'999'999)) EXPECT_EQ ("Out of bounds", std::to_string (identifier));
    EXPECT_EQ (false, database_notes.identifier_exists (identifier));
    
    identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    EXPECT_EQ (true, database_notes.identifier_exists (identifier));
    database_notes.erase (identifier);
    EXPECT_EQ (false, database_notes.identifier_exists (identifier));

    identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    EXPECT_EQ (true, database_notes.identifier_exists (identifier));
    database_notes.erase (identifier);
    EXPECT_EQ (false, database_notes.identifier_exists (identifier));
  }

  // Summary and contents.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();

    std::string value;
    std::vector <std::string> values;
    size_t length = 0;
    size_t pos = 0;

    // Test inserting data for both summary and contents.
    std::string summary = "Summary";
    std::string contents = "Contents";
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, summary, contents, false);
    value = database_notes.get_summary (newidentifier);
    EXPECT_EQ (summary, value);
    value = database_notes.get_contents (newidentifier);
    values = filter::strings::explode (value, '\n');
    if (values.size () > 1) value = values[1];
    EXPECT_EQ ("<p>Contents</p>", value);
    
    // Test that if the summary is not given, it is going to be the first line of the contents.
    contents = "This is a note.\nLine two.";
    newidentifier = database_notes.store_new_note ("", 0, 0, 0, "", contents, false);
    value = database_notes.get_summary (newidentifier);
    EXPECT_EQ ("This is a note.", value);
    value = database_notes.get_contents (newidentifier);
    values = filter::strings::explode (value, '\n');
    if (values.size () > 2) value = values[2];
    EXPECT_EQ ("<p>Line two.</p>", value);
    
    // Test setting the summary.
    database_notes.set_summary (newidentifier, "summary2");
    value = database_notes.get_summary (newidentifier);
    EXPECT_EQ ("summary2", value);
    
    // Test setting the note contents.
    database_notes.set_contents (newidentifier, "contents2");
    value = database_notes.get_contents (newidentifier);
    EXPECT_EQ ("contents2", value);
    
    // Test adding comment.
    value = database_notes.get_contents (newidentifier);
    length = value.length ();
    database_notes.add_comment (newidentifier, "comment2");
    value = database_notes.get_contents (newidentifier);
    if (value.length () < (length + 30)) {
      EXPECT_EQ ("Should be larger than length + 30", std::to_string (value.length()));
    }
    pos = value.find ("comment2");
    if (pos == std::string::npos) {
      EXPECT_EQ ("Should contain 'comment2'", value);
    }
    // Universal method to add comment.
    database_notes.add_comment (newidentifier, "comment5");
    value = database_notes.get_contents (newidentifier);
    if (value.length () < (length + 30)) {
      EXPECT_EQ ("Should be larger than length + 30", std::to_string (value.length()));
    }
    pos = value.find ("comment5");
    if (pos == std::string::npos) {
      EXPECT_EQ ("Should contain 'comment5'", value);
    }
  }

  // Test subscriptions.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    Notes_Logic notes_logic (webserver_request);
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    
    // Normally creating a new note would subscribe the current user to the note.
    // But since this unit test runs without sessions, it would have subscribed an empty user.
    webserver_request.session_logic()->set_username ("");
    int identifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    std::vector <std::string> subscribers = database_notes.get_subscribers (identifier);
    EXPECT_EQ (std::vector <std::string>{}, subscribers);
    
    // Create a note again, but this time set the session variable to a certain user.
    database_users.add_user ("unittest", "", 5, "");
    webserver_request.session_logic()->set_username ("unittest");
    webserver_request.database_config_user()->set_subscribe_to_consultation_notes_edited_by_me (true);
    identifier = database_notes.store_new_note ("", 1, 1, 1, "Summary", "Contents", false);
    notes_logic.handlerNewNote (identifier);
    subscribers = database_notes.get_subscribers (identifier);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, subscribers);
    EXPECT_EQ (true, database_notes.is_subscribed (identifier, "unittest"));
    webserver_request.database_config_user()->set_subscribe_to_consultation_notes_edited_by_me (false);
    // Test various other subscription related functions.
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest_unittest"));
    database_notes.unsubscribe (identifier);
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe_user (identifier, "unittest_unittest_unittest");
    EXPECT_EQ (true, database_notes.is_subscribed (identifier, "unittest_unittest_unittest"));
    database_notes.unsubscribe_user (identifier, "unittest_unittest_unittest");
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest_unittest_unittest"));
    
    // With the username still set, test the plan subscribe and unsubscribe mechanisms.
    webserver_request.database_config_user()->set_subscribe_to_consultation_notes_edited_by_me (false);
    identifier = database_notes.store_new_note ("", 1, 1, 1, "Summary", "Contents", false);
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe (identifier);
    EXPECT_EQ (true, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.unsubscribe (identifier);
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.subscribe (identifier);
    EXPECT_EQ (true, database_notes.is_subscribed (identifier, "unittest"));
    database_notes.unsubscribe (identifier);
    EXPECT_EQ (false, database_notes.is_subscribed (identifier, "unittest"));
    
    // Test subscribing and unsubscribing other users.
    database_notes.subscribe_user (identifier, "a");
    database_notes.subscribe_user (identifier, "b");
    subscribers = database_notes.get_subscribers (identifier);
    std::vector<std::string> standard_subscribers {"a", "b"};
    EXPECT_EQ (standard_subscribers, subscribers);
    database_notes.unsubscribe_user (identifier, "a");
    subscribers = database_notes.get_subscribers (identifier);
    EXPECT_EQ (std::vector<std::string>{"b"}, subscribers);
    database_notes.set_subscribers (identifier, {"aa", "bb"});
    subscribers = database_notes.get_subscribers (identifier);
    standard_subscribers = {"aa", "bb"};
    EXPECT_EQ (standard_subscribers, subscribers);
  }

  // Test assignments.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    
    webserver_request.session_logic()->set_username ("unittest2");
    
    // Create a note and check that it was not assigned to anybody.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    std::vector <std::string> assignees = database_notes.get_assignees (oldidentifier);
    EXPECT_EQ (std::vector <std::string>{}, assignees);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary2", "Contents2", false);
    assignees = database_notes.get_assignees (newidentifier);
    EXPECT_EQ (std::vector <std::string>{}, assignees);

    // Assign the note to a user, and check that this reflects in the list of assignees.
    database_notes.assign_user (oldidentifier, "unittest");
    assignees = database_notes.get_assignees (oldidentifier);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, assignees);
    database_notes.assign_user (newidentifier, "unittest");
    assignees = database_notes.get_assignees (newidentifier);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, assignees);
    
    // Test the set_assignees function.
    database_notes.set_assignees (oldidentifier, {"unittest"});
    assignees = database_notes.get_assignees (oldidentifier);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, assignees);
    database_notes.set_assignees (newidentifier, {"unittest"});
    assignees = database_notes.get_assignees (newidentifier);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, assignees);
    
    // Assign note to second user, and check it reflects.
    database_notes.assign_user (oldidentifier, "unittest2");
    assignees = database_notes.get_assignees (oldidentifier);
    std::vector <std::string> standard_assignees = {"unittest", "unittest2"};
    EXPECT_EQ (standard_assignees, assignees);
    database_notes.assign_user (newidentifier, "unittest3");
    assignees = database_notes.get_assignees (newidentifier);
    standard_assignees = {"unittest", "unittest3"};
    EXPECT_EQ (standard_assignees, assignees);
    
    // Based on the above, check the is_assigned function.
    EXPECT_EQ (true, database_notes.is_assigned (oldidentifier, "unittest"));
    EXPECT_EQ (true, database_notes.is_assigned (oldidentifier, "unittest2"));
    EXPECT_EQ (false, database_notes.is_assigned (oldidentifier, "PHPUnit3"));
    EXPECT_EQ (true, database_notes.is_assigned (newidentifier, "unittest"));
    EXPECT_EQ (true, database_notes.is_assigned (newidentifier, "unittest3"));
    EXPECT_EQ (false, database_notes.is_assigned (newidentifier, "PHPUnit3"));
    
    // Based on the above, test get_all_assignees.
    assignees = database_notes.get_all_assignees ({""});
    standard_assignees = {"unittest", "unittest2", "unittest3"};
    EXPECT_EQ (standard_assignees, assignees);
    
    // Based on the above, test the unassign_user function.
    database_notes.unassign_user (oldidentifier, "unittest");
    assignees = database_notes.get_assignees (oldidentifier);
    EXPECT_EQ (std::vector<std::string>{"unittest2"}, assignees);
    database_notes.unassign_user (oldidentifier, "unittest2");
    assignees = database_notes.get_assignees (oldidentifier);
    EXPECT_EQ (std::vector<std::string>{}, assignees);
    database_notes.unassign_user (newidentifier, "unittest");
    assignees = database_notes.get_assignees (newidentifier);
    EXPECT_EQ (std::vector<std::string>{"unittest3"}, assignees);
    database_notes.unassign_user (newidentifier, "unittest3");
    assignees = database_notes.get_assignees (newidentifier);
    EXPECT_EQ (std::vector<std::string>{}, assignees);
  }

  // Test the getters and the setters for the Bible.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    int oldidentifier = database_notes.store_new_note ("unittest", 0, 0, 0, "Summary", "Contents", false);
    std::string bible = database_notes.get_bible (oldidentifier);
    EXPECT_EQ ("unittest", bible);
    int newidentifier = database_notes.store_new_note ("unittest2", 0, 0, 0, "Summary", "Contents", false);
    bible = database_notes.get_bible (newidentifier);
    EXPECT_EQ ("unittest2", bible);
    database_notes.set_bible (oldidentifier, "PHPUnit2");
    bible = database_notes.get_bible (oldidentifier);
    EXPECT_EQ ("PHPUnit2", bible);
    database_notes.set_bible (newidentifier, "PHPUnit3");
    bible = database_notes.get_bible (newidentifier);
    EXPECT_EQ ("PHPUnit3", bible);
    database_notes.set_bible (oldidentifier, "");
    bible = database_notes.get_bible (oldidentifier);
    EXPECT_EQ ("", bible);
    database_notes.set_bible (newidentifier, "");
    bible = database_notes.get_bible (newidentifier);
    EXPECT_EQ ("", bible);
  }

  // Test getting and setting the passage(s).
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    
    // Create notes for certain passages.
    int oldidentifier = database_notes.store_new_note ("", 10, 9, 8, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 5, 4, 3, "Summary", "Contents", false);
    
    // Test getting passage.
    std::vector <Passage> passages = database_notes.get_passages (oldidentifier);
    Passage standard = Passage ("", 10, 9, "8");
    EXPECT_EQ (1, static_cast<int> (passages.size()));
    EXPECT_EQ (true, standard.equal (passages [0]));
    passages = database_notes.get_passages (newidentifier);
    standard = Passage ("", 5, 4, "3");
    EXPECT_EQ (1, static_cast<int>(passages.size()));
    EXPECT_EQ (true, standard.equal (passages [0]));
    
    // Test setting the passage.
    standard = Passage ("", 5, 6, "7");
    database_notes.set_passages (oldidentifier, {standard});
    passages = database_notes.get_passages (oldidentifier);
    EXPECT_EQ (1, static_cast<int>(passages.size()));
    EXPECT_EQ (true, standard.equal (passages [0]));
    standard = Passage ("", 12, 13, "14");
    database_notes.set_passages (newidentifier, {standard});
    passages = database_notes.get_passages (newidentifier);
    EXPECT_EQ (1, static_cast<int>(passages.size()));
    EXPECT_EQ (true, standard.equal (passages [0]));
  }

  // Test getting and setting the note status.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    
    // Create notes.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default status = New.
    std::string status = database_notes.get_status (oldidentifier);
    EXPECT_EQ ("New", status);
    status = database_notes.get_status (newidentifier);
    EXPECT_EQ ("New", status);
    
    // Test setting the status.
    database_notes.set_status (oldidentifier, "xxxxx");
    status = database_notes.get_status (oldidentifier);
    EXPECT_EQ ("xxxxx", status);
    database_notes.set_status (newidentifier, "yyyyy");
    status = database_notes.get_status (newidentifier);
    EXPECT_EQ ("yyyyy", status);
    
    // Test getting all possible statuses.
    std::vector <Database_Notes_Text> statuses = database_notes.get_possible_statuses ();
    std::vector <std::string> rawstatuses;
    for (auto & note_text : statuses) {
      rawstatuses.push_back (note_text.raw);
    }
    std::vector <std::string> standard_rawstatuses {"yyyyy", "xxxxx", "New", "Pending", "In progress", "Done", "Reopened"};
    EXPECT_EQ (standard_rawstatuses, rawstatuses);
  }

  // Getting and setting the severity.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    
    // Create note.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test default severity = Normal.
    std::string severity = database_notes.get_severity (oldidentifier);
    EXPECT_EQ ("Normal", severity);
    severity = database_notes.get_severity (newidentifier);
    EXPECT_EQ ("Normal", severity);
    
    // Test setting the severity.
    database_notes.set_raw_severity (oldidentifier, 0);
    severity = database_notes.get_severity (oldidentifier);
    EXPECT_EQ ("Wish", severity);
    database_notes.set_raw_severity (newidentifier, 0);
    severity = database_notes.get_severity (newidentifier);
    EXPECT_EQ ("Wish", severity);
    database_notes.set_raw_severity (oldidentifier, 4);
    severity = database_notes.get_severity (oldidentifier);
    EXPECT_EQ ("Major", severity);
    database_notes.set_raw_severity (newidentifier, 4);
    severity = database_notes.get_severity (newidentifier);
    EXPECT_EQ ("Major", severity);
    
    // Test getting all unique severities.
    std::vector <Database_Notes_Text> severities = database_notes.get_possible_severities ();
    std::vector <std::string> rawseverities;
    std::vector <std::string> localizedseverities;
    for (auto & note_text : severities) {
      rawseverities.push_back (note_text.raw);
      localizedseverities.push_back (note_text.localized);
    }
    std::vector <std::string> standard_severities {"0", "1", "2", "3", "4", "5"};
    EXPECT_EQ (standard_severities, rawseverities);
    standard_severities = {"Wish", "Minor", "Normal", "Important", "Major", "Critical"};
    EXPECT_EQ (standard_severities, localizedseverities);
  }

  // Test setting and getting the "modified" property of notes.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    int time = filter::date::seconds_since_epoch ();
    
    // Create note.
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test getter.
    int value = database_notes.get_modified (oldidentifier);
    if ((value < time) || (value > time + 1)) EXPECT_EQ (time, value);
    value = database_notes.get_modified (newidentifier);
    if ((value < time) || (value > time + 1)) EXPECT_EQ (time, value);
    
    // Test setter.
    time = 123456789;
    database_notes.set_modified (oldidentifier, time);
    value = database_notes.get_modified (oldidentifier);
    EXPECT_EQ (time, value);;
    database_notes.set_modified (newidentifier, time);
    value = database_notes.get_modified (newidentifier);
    EXPECT_EQ (time, value);;
  }

  // Get identifiers.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    webserver_request.session_logic()->set_username ("unittest");
    
    // Create a few notes.
    std::vector <int> standardids;
    for (unsigned int i = 0; i < 3; i++) {
      int identifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
      standardids.push_back (identifier);
    }
    
    // Get the identifiers.
    std::vector <int> identifiers = database_notes.get_identifiers ();
    sort (standardids.begin(), standardids.end());
    sort (identifiers.begin(), identifiers.end());
    EXPECT_EQ (standardids, identifiers);
  }

  // Setting the note identifier.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create note.
    webserver_request.session_logic()->set_username ("unittest");
    int identifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    
    // Contents of the note.
    std::string original_contents1 = database_notes.get_contents (identifier1);
    if (original_contents1.length () <= 20) {
      EXPECT_EQ ("Should be greater than 20", std::to_string (original_contents1.length ()));
    }
    std::string original_contents2 = database_notes.get_contents (identifier2);
    if (original_contents2.length () <= 20) {
      EXPECT_EQ ("Should be greater than 20", std::to_string (original_contents2.length()));
    }
    
    // Checksum of the notes.
    std::string original_checksum1 = database_notes.get_checksum (identifier1);
    EXPECT_EQ (32, static_cast<int>(original_checksum1.length()));
    std::string original_checksum2 = database_notes.get_checksum (identifier2);
    EXPECT_EQ (32, static_cast<int>(original_checksum2.length()));
    
    // Change the identifier.
    int new_id1 = database_notes.get_new_unique_identifier ();
    database_notes.set_identifier (identifier1, new_id1);
    int new_id2 = database_notes.get_new_unique_identifier ();
    database_notes.set_identifier (identifier2, new_id2);
    
    // Check old and new identifier for v2 and v2.
    std::string contents = database_notes.get_contents (identifier1);
    EXPECT_EQ ("", contents);
    contents = database_notes.get_contents (new_id1);
    EXPECT_EQ (original_contents1, contents);
    contents = database_notes.get_contents (identifier2);
    EXPECT_EQ ("", contents);
    contents = database_notes.get_contents (new_id2);
    EXPECT_EQ (original_contents2, contents);
    
    std::string checksum = database_notes.get_checksum (identifier1);
    EXPECT_EQ ("", checksum);
    checksum = database_notes.get_checksum (new_id1);
    EXPECT_EQ (original_checksum1, checksum);
    checksum = database_notes.get_checksum (identifier2);
    EXPECT_EQ ("", checksum);
    checksum = database_notes.get_checksum (new_id2);
    EXPECT_EQ (original_checksum2, checksum);
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
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    int oldidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    
    database_notes.mark_for_deletion (oldidentifier);
    database_notes.mark_for_deletion (newidentifier);
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    
    std::vector <int> identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    std::vector <int> standard_identifiers {oldidentifier, newidentifier};
    EXPECT_EQ (standard_identifiers, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);
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
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
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

    std::vector <int> identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
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
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
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

    std::vector <int> identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (std::vector <int>{}, identifiers);
    
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    std::vector <int> standard_identifiers {oldidentifier1, newidentifier1};
    EXPECT_EQ (standard_identifiers, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);

    database_notes.unmark_for_deletion (oldidentifier1);
    database_notes.unmark_for_deletion (newidentifier1);
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    standard_identifiers = {oldidentifier2, newidentifier2};
    EXPECT_EQ (standard_identifiers, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);
    
    database_notes.unmark_for_deletion (oldidentifier2);
    database_notes.unmark_for_deletion (newidentifier2);
    database_notes.touch_marked_for_deletion ();
    identifiers = database_notes.get_due_for_deletion ();
    standard_identifiers = {oldidentifier3, newidentifier3};
    EXPECT_EQ (standard_identifiers, identifiers);
    identifiers = database_notes.get_due_for_deletion ();
    EXPECT_EQ (standard_identifiers, identifiers);
  }

  // Testing whether note is marked for deletion.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    int oldidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int oldidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier1 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier2 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);
    int newidentifier3 = database_notes.store_new_note ("", 0, 0, 0, "summary", "contents", false);

    database_notes.mark_for_deletion (oldidentifier1);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.mark_for_deletion (newidentifier1);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));
    
    database_notes.unmark_for_deletion (oldidentifier2);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.unmark_for_deletion (newidentifier2);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));

    database_notes.unmark_for_deletion (oldidentifier1);
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier2));
    database_notes.unmark_for_deletion (newidentifier1);
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier1));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier2));

    database_notes.mark_for_deletion (oldidentifier2);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier2));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (oldidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier3));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (oldidentifier3));
    database_notes.mark_for_deletion (newidentifier2);
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier2));
    EXPECT_EQ (true, database_notes.is_marked_for_deletion (newidentifier2));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier3));
    EXPECT_EQ (false, database_notes.is_marked_for_deletion (newidentifier3));
  }

  // Test operations on the checksum.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create note to work with.
    int oldidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    
    // Checksum of new note should be calculated.
    std::string good_checksum_old = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, good_checksum_old.empty());
    std::string good_checksum_new = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, good_checksum_new.empty());
    // The two should match.
    EXPECT_EQ (good_checksum_old, good_checksum_new);
    
    // Clear checksum, and recalculate it.
    std::string outdated_checksum = "outdated checksum";
    database_notes.set_checksum (oldidentifier, outdated_checksum);
    std::string checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (outdated_checksum, checksum);
    database_notes.set_checksum (newidentifier, outdated_checksum);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (outdated_checksum, checksum);
    database_notes.sync ();
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (good_checksum_old, checksum);
    database_notes.set_checksum (oldidentifier, outdated_checksum);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (good_checksum_new, checksum);
    database_notes.set_checksum (newidentifier, outdated_checksum);
    
    // Test that saving a note updates the checksum.
    database_notes.set_checksum (oldidentifier, std::string());
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (std::string(), checksum);
    database_notes.set_modified (oldidentifier, 1234567);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.set_checksum (newidentifier, std::string());
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (std::string(), checksum);
    database_notes.set_modified (newidentifier, 1234567);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());

    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_subscribers (oldidentifier, {"subscribers"});
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_subscribers (newidentifier, {"subscribers"});
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_bible (oldidentifier, "unittest");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_bible (newidentifier, "unittest");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());

    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_passages (oldidentifier, {});
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_passages (newidentifier, {});
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_status (oldidentifier, "Status");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_status (newidentifier, "Status");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
    
    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_raw_severity (oldidentifier, 123);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_raw_severity (newidentifier, 123);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
    
    database_notes.set_checksum (oldidentifier, "");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_summary (oldidentifier, "new");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.set_checksum (newidentifier, "");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_summary (newidentifier, "new");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
    
    database_notes.delete_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_contents (oldidentifier, "new");
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (false, checksum.empty());
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.set_contents (newidentifier, "new");
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (false, checksum.empty());
  }

  // Test sync logic checksums.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create notes to work with.
    std::vector <int> oldidentifiers;
    oldidentifiers.push_back (database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false));
    oldidentifiers.push_back (database_notes.store_new_note ("bible2", 2, 3, 4, "summary2", "contents2", false));
    oldidentifiers.push_back (database_notes.store_new_note ("bible3", 3, 4, 5, "summary3", "contents3", false));
    std::vector <int> newidentifiers;
    newidentifiers.push_back (database_notes.store_new_note ("bible4", 4, 5, 6, "summary4", "contents4", false));
    newidentifiers.push_back (database_notes.store_new_note ("bible5", 5, 6, 7, "summary5", "contents5", false));
    newidentifiers.push_back (database_notes.store_new_note ("bible6", 6, 7, 8, "summary6", "contents6", false));
    
    // Checksum calculation: slow and fast methods should be the same.
    Sync_Logic sync_logic (webserver_request);
    std::string oldchecksum1 = sync_logic.checksum (oldidentifiers);
    EXPECT_EQ (32, static_cast<int>(oldchecksum1.length()));
    std::string oldchecksum2 = database_notes.get_multiple_checksum (oldidentifiers);
    EXPECT_EQ (oldchecksum1, oldchecksum2);
    std::string newchecksum1 = sync_logic.checksum (newidentifiers);
    EXPECT_EQ (32, static_cast<int>(newchecksum1.length()));
    std::string newchecksum2 = database_notes.get_multiple_checksum (newidentifiers);
    EXPECT_EQ (newchecksum1, newchecksum2);
  }

  // Test updating checksums.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create notes to work with.
    int oldidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);
    int newidentifier = database_notes.store_new_note ("bible", 1, 2, 3, "summary", "contents", false);

    // Check checksum.
    std::string oldchecksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (32, oldchecksum.length ());
    std::string newchecksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (oldchecksum, newchecksum);
    
    // Clear it and set the checksum again.
    database_notes.delete_checksum (oldidentifier);
    std::string checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.update_checksum (oldidentifier);
    checksum = database_notes.get_checksum (oldidentifier);
    EXPECT_EQ (oldchecksum, checksum);
    database_notes.delete_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ ("", checksum);
    database_notes.update_checksum (newidentifier);
    checksum = database_notes.get_checksum (newidentifier);
    EXPECT_EQ (newchecksum, checksum);
  }

  // Test getting notes within a notes range for given Bibles.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int identifier = database_notes.store_new_note ("bible1", 1, 2, 3, "summary", "contents", false);
    int identifier1 = 100'000'000;
    database_notes.set_identifier (identifier, identifier1);
    
    identifier = database_notes.store_new_note ("bible2", 1, 2, 3, "summary", "contents", false);
    int identifier2 = 500'000'000;
    database_notes.set_identifier (identifier, identifier2);
    
    identifier = database_notes.store_new_note ("", 1, 2, 3, "summary", "contents", false);
    int identifier3 = 999'999'999;
    database_notes.set_identifier (identifier, identifier3);
    
    // Test selection mechanism for certain Bibles.
    std::vector <int> identifiers = database_notes.get_notes_in_range_for_bibles (100'000'000, 999'999'999, {"bible1", "bible2"}, false);
    std::vector <int> standard_identifiers {100'000'000, 50'0000'000, 99'9999'999};
    EXPECT_EQ (standard_identifiers, identifiers);
    
    identifiers = database_notes.get_notes_in_range_for_bibles (100'000'000, 999'999'999, {"bible1", "bible3"}, false);
    standard_identifiers = {100'000'000, 999'999'999};
    EXPECT_EQ (standard_identifiers, identifiers);
    
    identifiers = database_notes.get_notes_in_range_for_bibles (10'0000'000, 999'999'999, {}, false);
    EXPECT_EQ (std::vector<int>{99'9999'999}, identifiers);
    
    // Test selection mechanism for any Bible.
    identifiers = database_notes.get_notes_in_range_for_bibles (100'000'000, 999'999'999, {}, true);
    standard_identifiers = {100'000'000, 500'000'000, 999'999'999};
    EXPECT_EQ (standard_identifiers, identifiers);
  }

  // Test creating a range of identifiers.
  {
    Webserver_Request webserver_request;
    Sync_Logic sync_logic (webserver_request);
    
    std::vector <Sync_Logic_Range> ranges = sync_logic.create_range (100'000'000, 999'999'999);
    EXPECT_EQ (10, static_cast<int>(ranges.size()));
    EXPECT_EQ (100'000'000, ranges[0].low);
    EXPECT_EQ (189'999'998, ranges[0].high);
    EXPECT_EQ (189'999'999, ranges[1].low);
    EXPECT_EQ (279'999'997, ranges[1].high);
    EXPECT_EQ (279'999'998, ranges[2].low);
    EXPECT_EQ (369'999'996, ranges[2].high);
    EXPECT_EQ (369'999'997, ranges[3].low);
    EXPECT_EQ (459'999'995, ranges[3].high);
    EXPECT_EQ (459'999'996, ranges[4].low);
    EXPECT_EQ (549'999'994, ranges[4].high);
    EXPECT_EQ (549'999'995, ranges[5].low);
    EXPECT_EQ (639'999'993, ranges[5].high);
    EXPECT_EQ (639'999'994, ranges[6].low);
    EXPECT_EQ (729'999'992, ranges[6].high);
    EXPECT_EQ (729'999'993, ranges[7].low);
    EXPECT_EQ (819'999'991, ranges[7].high);
    EXPECT_EQ (819'999'992, ranges[8].low);
    EXPECT_EQ (909'999'990, ranges[8].high);
    EXPECT_EQ (909'999'991, ranges[9].low);
    EXPECT_EQ (999'999'999, ranges[9].high);
    
    ranges = sync_logic.create_range (100'000'000, 100'000'100);
    EXPECT_EQ (10, static_cast<int>(ranges.size()));
    EXPECT_EQ (100'000'000, ranges[0].low);
    EXPECT_EQ (100'000'009, ranges[0].high);
    EXPECT_EQ (100'000'010, ranges[1].low);
    EXPECT_EQ (100'000'019, ranges[1].high);
    EXPECT_EQ (100'000'020, ranges[2].low);
    EXPECT_EQ (100'000'029, ranges[2].high);
    EXPECT_EQ (100'000'030, ranges[3].low);
    EXPECT_EQ (100'000'039, ranges[3].high);
    EXPECT_EQ (100'000'040, ranges[4].low);
    EXPECT_EQ (100'000'049, ranges[4].high);
    EXPECT_EQ (100'000'050, ranges[5].low);
    EXPECT_EQ (100'000'059, ranges[5].high);
    EXPECT_EQ (100'000'060, ranges[6].low);
    EXPECT_EQ (100'000'069, ranges[6].high);
    EXPECT_EQ (100'000'070, ranges[7].low);
    EXPECT_EQ (100'000'079, ranges[7].high);
    EXPECT_EQ (100'000'080, ranges[8].low);
    EXPECT_EQ (100'000'089, ranges[8].high);
    EXPECT_EQ (100'000'090, ranges[9].low);
    EXPECT_EQ (100'000'100, ranges[9].high);
  }

  // Test selecting based on a given Bible.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int identifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    database_notes.set_status(identifier1, "status1");
    database_notes.set_status(identifier2, "status2");
    database_notes.set_status(identifier3, "status3");

    // Select notes while varying Bible selection.
    std::vector<int> standard{};
    std::vector<int> identifiers{};

    {
      Database_Notes::selector selector {
        .bibles = {"bible1"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier1};
      EXPECT_EQ (standard, identifiers);
    }

    {
      Database_Notes::selector selector {
        .bibles = {"bible2"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier2};
      EXPECT_EQ (standard, identifiers);
    }
    
    {
      Database_Notes::selector selector {
        .bibles = {"bible1", "bible2"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier1, identifier2};
      EXPECT_EQ (standard, identifiers);
    }

    {
      Database_Notes::selector selector {
        .bibles = {"bible"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {};
      EXPECT_EQ (standard, identifiers);
    }

    {
      Database_Notes::selector selector {
        .bibles = {},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = true,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {};
      EXPECT_EQ (standard, identifiers);
    }

    {
      Database_Notes::selector selector {
        .bibles = {"bible3"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier3};
      EXPECT_EQ (standard, identifiers);
    }
    
    {
      Database_Notes::selector selector {
        .bibles = {"bible3"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier3};
      EXPECT_EQ (standard, identifiers);
    }

    {
      Database_Notes::selector selector {
        .bibles = {},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {"status1", "status2"},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      identifiers = database_notes.select_notes(selector);
      standard = {identifier1, identifier2};
      EXPECT_EQ (standard, identifiers);
    }
  }

  // Test the resilience of the notes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    bool healthy = database_notes.healthy ();
    EXPECT_EQ (true, healthy);
    
    std::string corrupted_database = filter_url_create_root_path ({"unittests", "tests", "notes.sqlite.damaged"});
    std::string path = database_notes.database_path ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));
    
    healthy = database_notes.healthy ();
    EXPECT_EQ (false, healthy);
    
    database_notes.checkup ();
    healthy = database_notes.healthy ();
    EXPECT_EQ (true, healthy);

    // Clean the generated logbook entries away invisibly.
    refresh_sandbox (false);
  }

  // Test the resilience of the checksums of notes.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();

    bool healthy = database_notes.checksums_healthy ();
    EXPECT_EQ (true, healthy);
    
    std::string corrupted_database = filter_url_create_root_path ({"unittests", "tests", "notes.sqlite.damaged"});
    std::string path = database_notes.checksums_database_path ();
    filter_url_file_put_contents (path, filter_url_file_get_contents (corrupted_database));

    healthy = database_notes.checksums_healthy ();
    EXPECT_EQ (false, healthy);

    database_notes.checkup_checksums ();
    healthy = database_notes.checksums_healthy ();
    EXPECT_EQ (true, healthy);

    // Clear the generated and expected logbook entries away invisibly.
    refresh_sandbox (false);
  }

  // Availability.
  {
    refresh_sandbox (true);
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    EXPECT_EQ (true, database_notes.available ());
    database_notes.set_availability (false);
    EXPECT_EQ (false, database_notes.available ());
    database_notes.set_availability (true);
    EXPECT_EQ (true, database_notes.available ());
  }

  // Testing public notes.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Create a couple of notes to work with.
    int oldidentifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int oldidentifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int oldidentifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    int newidentifier1 = database_notes.store_new_note ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int newidentifier2 = database_notes.store_new_note ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int newidentifier3 = database_notes.store_new_note ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
    // None of them, or others, are public notes.
    EXPECT_EQ (false, database_notes.get_public (oldidentifier1));
    EXPECT_EQ (false, database_notes.get_public (oldidentifier2));
    EXPECT_EQ (false, database_notes.get_public (oldidentifier3));
    EXPECT_EQ (false, database_notes.get_public (oldidentifier1 + 1));
    EXPECT_EQ (false, database_notes.get_public (newidentifier1));
    EXPECT_EQ (false, database_notes.get_public (newidentifier2));
    EXPECT_EQ (false, database_notes.get_public (newidentifier3));
    EXPECT_EQ (false, database_notes.get_public (newidentifier1 + 1));
    
    // Set some public.
    database_notes.set_public (oldidentifier1, true);
    database_notes.set_public (oldidentifier2, true);
    database_notes.set_public (newidentifier1, true);
    database_notes.set_public (newidentifier2, true);
    EXPECT_EQ (true, database_notes.get_public (oldidentifier1));
    EXPECT_EQ (true, database_notes.get_public (oldidentifier2));
    EXPECT_EQ (false, database_notes.get_public (oldidentifier3));
    EXPECT_EQ (true, database_notes.get_public (newidentifier1));
    EXPECT_EQ (true, database_notes.get_public (newidentifier2));
    EXPECT_EQ (false, database_notes.get_public (newidentifier3));
    
    // Unset some of them.
    database_notes.set_public (oldidentifier1, false);
    EXPECT_EQ (false, database_notes.get_public (oldidentifier1));
    EXPECT_EQ (true, database_notes.get_public (oldidentifier2));
    database_notes.set_public (newidentifier1, false);
    EXPECT_EQ (false, database_notes.get_public (newidentifier1));
    EXPECT_EQ (true, database_notes.get_public (newidentifier2));
  }

  // Bulk notes transfer elaborate tests for notes stored in JSON format.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Keep the stored values for the notes.
    std::vector <std::string> v_assigned;
    std::vector <std::string> v_bible;
    std::vector <std::string> v_contents;
    std::vector <int> v_identifier;
    std::vector <int> v_modified;
    std::vector <std::string> v_passage;
    std::vector <int> v_severity;
    std::vector <std::string> v_status;
    std::vector <std::string> v_subscriptions;
    std::vector <std::string> v_summary;
    
    // Create several notes.
    for (int i = 0; i < 5; i++) {
      // Basic fields for the note.
      std::string offset = std::to_string (i);
      std::string bible = "bible" + offset;
      int book = i;
      int chapter = i + 1;
      int verse = i + 2;
      std::string summary = "summary" + offset;
      std::string contents = "contents" + offset;
      int identifier = database_notes.store_new_note (bible, book, chapter, verse, summary, contents, false);
      database_notes.set_contents (identifier, contents);
      // Additional fields for the note.
      std::string assigned = "assigned" + offset;
      database_notes.set_raw_assigned (identifier, assigned);
      std::string passage = "passage" + offset;
      database_notes.set_raw_passage (identifier, passage);
      int severity = 4 * i;
      database_notes.set_raw_severity (identifier, severity);
      std::string status = "status" + offset;
      database_notes.set_status (identifier, status);
      std::string subscriptions = "subscriptions" + offset;
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
    std::vector <std::string> checksums;
    for (size_t i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      database_notes.update_checksum (identifier);
      std::string checksum = database_notes.get_checksum (identifier);
      checksums.push_back (checksum);
    }
    
    // Get some search results for later reference.
    std::vector <int> search_results;
    {
      Database_Notes::selector selector {
        .bibles = {"bible1"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      search_results = database_notes.select_notes(selector);
    }
    
    // Get the notes in bulk in a database.
    std::string json = database_notes.get_bulk (v_identifier);
    
    // Delete all notes again.
    for (size_t i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      EXPECT_EQ (false, database_notes.get_summary (identifier).empty ());
      EXPECT_EQ (false, database_notes.get_contents (identifier).empty ());
      EXPECT_EQ (false, database_notes.get_bible (identifier).empty ());
      EXPECT_EQ (false, database_notes.get_raw_passage (identifier).empty ());
      EXPECT_EQ (false, database_notes.get_raw_status (identifier).empty ());
      EXPECT_EQ (true, database_notes.get_raw_severity (identifier) != 2);
      EXPECT_EQ (true, database_notes.get_modified (identifier) < 1000);
      database_notes.erase (identifier);
      EXPECT_EQ ("", database_notes.get_summary (identifier));
      EXPECT_EQ ("", database_notes.get_contents (identifier));
      EXPECT_EQ ("", database_notes.get_bible (identifier));
      EXPECT_EQ ("", database_notes.get_raw_passage (identifier));
      EXPECT_EQ ("", database_notes.get_raw_status (identifier));
      EXPECT_EQ (2, database_notes.get_raw_severity (identifier));
      EXPECT_EQ (0, database_notes.get_modified (identifier));
    }
    
    // The checksums should now be gone.
    for (size_t i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      std::string checksum = database_notes.get_checksum (identifier);
      EXPECT_EQ ("", checksum);
    }
    
    // There should be no search results anymore.
    std::vector <int> no_search_results;
    {
      Database_Notes::selector selector {
        .bibles = {"bible1"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      no_search_results = database_notes.select_notes(selector);
    }
    EXPECT_EQ (std::vector <int>{}, no_search_results);
    
    // Copy the notes from the database back to the filesystem.
    database_notes.set_bulk (json);
    
    // Check that the notes are back.
    for (size_t i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      std::string assigned = database_notes.get_raw_assigned (identifier);
      EXPECT_EQ (v_assigned [i], assigned);
      std::string bible = database_notes.get_bible (identifier);
      EXPECT_EQ (v_bible [i], bible);
      std::string contents = database_notes.get_contents (identifier);
      EXPECT_EQ (v_contents [i], contents);
      int modified = database_notes.get_modified (identifier);
      EXPECT_EQ (v_modified [i], modified);
      std::string passage = database_notes.get_raw_passage (identifier);
      EXPECT_EQ (v_passage [i], passage);
      int severity = database_notes.get_raw_severity (identifier);
      EXPECT_EQ (v_severity [i], severity);
      std::string status = database_notes.get_raw_status (identifier);
      EXPECT_EQ (v_status [i], status);
      std::string subscriptions = database_notes.get_raw_subscriptions (identifier);
      EXPECT_EQ (v_subscriptions [i], subscriptions);
      std::string summary = database_notes.get_summary (identifier);
      EXPECT_EQ (v_summary [i], summary);
    }
    
    // The checksums should be back also.
    for (size_t i = 0; i < 5; i++) {
      int identifier = v_identifier [i];
      std::string checksum = database_notes.get_checksum (identifier);
      EXPECT_EQ (checksums [i], checksum);
    }
    
    // The search results should be back too.
    std::vector <int> restored_search;
    {
      Database_Notes::selector selector {
        .bibles = {"bible1"},
        .book = 0,
        .chapter = 0,
        .verse = 0,
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {},
        .assignment_selector = "",
        .subscription_selector = false,
        .severity_selector = -1,
        .text_selector = 0,
        .search_text = "",
        .limit = -1
      };
      restored_search = database_notes.select_notes(selector);
    }
    EXPECT_EQ (search_results, restored_search);
  }

  // Test updating the search database for the notes with JSON storage.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    // Test values for the note.
    std::string contents ("contents");
    
    // Create note.
    int identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
    // Creating the note updates the search database.
    // Basic search should work now.
    std::vector <int> identifiers;
    
    // Search on the content of the current note.
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 0, // No book given.
        .chapter = 0, // No chapter given.
        .verse = 0, // No verse given.
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector = -1, // Don't consider the severity.
        .text_selector = 1, // Do search on the text following.
        .search_text = "", // Search on any contents.
        .limit = 0 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    // Search result should be there.
    EXPECT_EQ (std::vector <int>{identifier}, identifiers);
    // Do a raw update of the note. The search database is not updated.
    database_notes.set_raw_contents (identifier, contents);
    // Doing a search now does not give results.
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 0, // No book given.
        .chapter = 0, // No chapter given.
        .verse = 0, // No verse given.
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector = -1, // Don't consider the severity.
        .text_selector = 1, // Do search on the text following.
        .search_text = contents, // Search on certain content.
        .limit = 0 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    EXPECT_EQ (std::vector <int>{}, identifiers);
    // Update the search index.
    // Search results should be back to normal.
    database_notes.update_database (identifier);
    database_notes.update_search_fields (identifier);
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 0, // No book given.
        .chapter = 0, // No chapter given.
        .verse = 0, // No verse given.
        .passage_selector = Database_Notes::passage_selector::any_passage,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector =  -1, // Don't consider the severity.
        .text_selector = 1, // Do search on the text following.
        .search_text = contents, // Search on any contents.
        .limit = -1 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    EXPECT_EQ (std::vector <int>{identifier}, identifiers);
    
    // Search on the note's passage.
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 0, // Book given.
        .chapter = 0, // Chapter given.
        .verse = 0, // Verse given.
        .passage_selector = Database_Notes::passage_selector::current_verse,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector = -1, // Don't consider the severity.
        .text_selector = 0, // Do not search on any text.
        .search_text = "", // No text given as being irrelevant.
        .limit = 0 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    // Search result should be there.
    EXPECT_EQ (std::vector <int>{identifier}, identifiers);
    // Update the passage of the note without updating the search index.
    database_notes.set_raw_passage (identifier, " 1.2.3 ");
    // There should be no search results yet when searching on the new passage.
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 1, // Book given.
        .chapter = 2, // Chapter given.
        .verse = 3, // Verse given.
        .passage_selector = Database_Notes::passage_selector::current_verse,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector = -1, // Don't consider the severity.
        .text_selector = 0, // Do not search on any text.
        .search_text = "", // No text given as being irrelevant.
        .limit = -1 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    EXPECT_EQ (std::vector <int>{}, identifiers);
    // Update the search index. There should be search results now.
    database_notes.update_database (identifier);
    {
      Database_Notes::selector selector {
        .bibles = {}, // No Bibles given.
        .book = 1, // Book given.
        .chapter = 2, // Chapter given.
        .verse = 3, // Verse given.
        .passage_selector = Database_Notes::passage_selector::current_verse,
        .edit_selector = Database_Notes::edit_selector::at_any_time,
        .non_edit_selector = Database_Notes::non_edit_selector::any_time,
        .status_selectors = {}, // Don't consider the statuses.
        .assignment_selector = "", // Don't consider assignment.
        .subscription_selector = false, // Don't consider subscriptions.
        .severity_selector = -1, // Don't consider the severity.
        .text_selector = 0, // Do not search on any text.
        .search_text = "", // No text given as being irrelevant.
        .limit = -1 // Don't limit the search results.
      };
      identifiers = database_notes.select_notes(selector);
    }
    EXPECT_EQ (std::vector <int>{identifier}, identifiers);
  }

  // Test methods for getting and setting note properties.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Notes database_notes (webserver_request);
    database_notes.create ();
    
    std::string bible1 = "bible1";
    std::string bible2 = "bible2";
    Passage passage1 = Passage ("", 1, 2, "3");
    Passage passage2 = Passage ("", 4, 5, "6");
    
    // Create notes.
    int oldidentifier1 = database_notes.store_new_note (bible1, passage1.m_book, passage1.m_chapter, filter::strings::convert_to_int (passage1.m_verse), "v1", "v1", false);
    int identifier1 = oldidentifier1 + 2;
    int oldidentifier2 = database_notes.store_new_note (bible2, passage2.m_book, passage2.m_chapter, filter::strings::convert_to_int (passage2.m_verse), "v2", "v2", false);
    int identifier2 = oldidentifier2 + 4;
    
    // Call the method to set a new identifier.
    database_notes.set_identifier (oldidentifier1, identifier1);
    database_notes.set_identifier (oldidentifier2, identifier2);
    
    // Test the methods to get or to set the summaries.
    std::string summary1 = "summary1";
    std::string summary2 = "summary2";
    database_notes.set_summary (identifier1, summary1);
    database_notes.set_summary (identifier2, summary2);
    EXPECT_EQ (summary1, database_notes.get_summary (identifier1));
    EXPECT_EQ (summary2, database_notes.get_summary (identifier2));
    
    // Test the methods to get and to set the contents.
    std::string contents1 = "contents1";
    std::string contents2 = "contents2";
    database_notes.set_contents (identifier1, contents1);
    database_notes.set_contents (identifier2, contents2);
    std::string contents = database_notes.get_contents (identifier1);
    EXPECT_EQ (true, contents.find (contents1) != std::string::npos);
    EXPECT_EQ (contents, database_notes.get_contents (identifier1));
    contents = database_notes.get_contents (identifier2);
    EXPECT_EQ (true, contents.find (contents2) != std::string::npos);
    EXPECT_EQ (contents, database_notes.get_contents (identifier2));
    
    // Test the method to get the subscribers.
    std::string subscriber1 = "subscriber1";
    std::string subscriber2 = "subscriber2";
    database_notes.set_subscribers (identifier1, { subscriber1 });
    database_notes.set_subscribers (identifier2, { subscriber2 });
    EXPECT_EQ (std::vector <std::string>{subscriber1}, database_notes.get_subscribers (identifier1));
    EXPECT_EQ (std::vector <std::string>{subscriber2}, database_notes.get_subscribers (identifier2));
    
    // Test the method to test a subscriber to a note.
    EXPECT_EQ (true, database_notes.is_subscribed (identifier1, subscriber1));
    EXPECT_EQ (true, database_notes.is_subscribed (identifier2, subscriber2));
    EXPECT_EQ (false, database_notes.is_subscribed (identifier1, subscriber2));
    EXPECT_EQ (false, database_notes.is_subscribed (identifier2, subscriber1));
    
    // Test the methods for the assignees.
    std::string assignee1 = "assignee1";
    std::string assignee2 = "assignee2";
    database_notes.set_assignees (identifier1, { assignee1 });
    database_notes.set_assignees (identifier2, { assignee2 });
    EXPECT_EQ (std::vector <std::string>{assignee1}, database_notes.get_assignees (identifier1));
    EXPECT_EQ (std::vector <std::string>{assignee2}, database_notes.get_assignees (identifier2));
    EXPECT_EQ (true, database_notes.is_assigned (identifier1, assignee1));
    EXPECT_EQ (true, database_notes.is_assigned (identifier2, assignee2));
    EXPECT_EQ (false, database_notes.is_assigned (identifier1, assignee2));
    EXPECT_EQ (false, database_notes.is_assigned (identifier2, assignee1));
    
    // Test the methods for the Bible.
    EXPECT_EQ (bible1, database_notes.get_bible (identifier1));
    EXPECT_EQ (bible2, database_notes.get_bible (identifier2));
    
    // Test the methods for the passage.
    std::vector <Passage> passages;
    passages = database_notes.get_passages (identifier1);
    EXPECT_EQ (1, passages.size());
    for (auto passage : passages) EXPECT_EQ (true, passage1.equal (passage));
    passages = database_notes.get_passages (identifier2);
    EXPECT_EQ (1, passages.size());
    for (auto passage : passages) EXPECT_EQ (true, passage2.equal (passage));
    EXPECT_EQ (" 1.2.3 ", database_notes.decode_passage (identifier1));
    EXPECT_EQ (" 4.5.6 ", database_notes.decode_passage (identifier2));
    
    // Test the methods for the status.
    std::string status1 = "status1";
    std::string status2 = "status2";
    database_notes.set_status (identifier1, status1);
    database_notes.set_status (identifier2, status2);
    EXPECT_EQ (status1, database_notes.get_raw_status (identifier1));
    EXPECT_EQ (status1, database_notes.get_status (identifier1));
    EXPECT_EQ (status2, database_notes.get_raw_status (identifier2));
    EXPECT_EQ (status2, database_notes.get_status (identifier2));
    
    // Test the methods for the severity.
    int severity1 = 4;
    int severity2 = 5;
    std::vector <std::string> standard_severities = database_notes.standard_severities ();
    database_notes.set_raw_severity (identifier1, severity1);
    database_notes.set_raw_severity (identifier2, severity2);
    EXPECT_EQ (severity1, database_notes.get_raw_severity (identifier1));
    EXPECT_EQ (severity2, database_notes.get_raw_severity (identifier2));
    EXPECT_EQ (standard_severities [static_cast <size_t> (severity1)], database_notes.get_severity (identifier1));
    EXPECT_EQ (standard_severities [static_cast <size_t> (severity2)], database_notes.get_severity (identifier2));
    
    // Test the methods for the modification time.
    int modified1 = 1000;
    int modified2 = 2000;
    database_notes.set_modified (identifier1, modified1);
    database_notes.set_modified (identifier2, modified2);
    EXPECT_EQ (modified1, database_notes.get_modified (identifier1));
    EXPECT_EQ (modified2, database_notes.get_modified (identifier2));
    
    // Test the methods for a note being public.
    database_notes.set_public (identifier1, true);
    database_notes.set_public (identifier2, false);
    EXPECT_EQ (true, database_notes.get_public (identifier1));
    EXPECT_EQ (false, database_notes.get_public (identifier2));
    database_notes.set_public (identifier1, false);
    database_notes.set_public (identifier2, true);
    EXPECT_EQ (false, database_notes.get_public (identifier1));
    EXPECT_EQ (true, database_notes.get_public (identifier2));
  }
}


TEST (notes, database_notes)
{
  test_database_notes ();
}


TEST (notes, database_noteassignment)
{
  refresh_sandbox (false);
  Database_NoteAssignment database;
  
  bool exists = database.exists ("unittest");
  EXPECT_EQ (false, exists);
  
  std::vector <std::string> assignees = database.assignees ("unittest");
  EXPECT_EQ (std::vector <std::string>{}, assignees);
  
  database.assignees ("unittest", {"one", "two"});
  assignees = database.assignees ("none-existing");
  EXPECT_EQ (std::vector <std::string>{}, assignees);
  
  exists = database.exists ("unittest");
  EXPECT_EQ (true, exists);
  
  assignees = database.assignees ("unittest");
  std::vector <std::string> standard_assignees {"one", "two"};
  EXPECT_EQ (standard_assignees, assignees);
  
  database.assignees ("unittest", {"1", "2"});
  assignees = database.assignees ("unittest");
  standard_assignees = {"1", "2"};
  EXPECT_EQ (standard_assignees, assignees);
  
  exists = database.exists ("unittest", "1");
  EXPECT_EQ (true, exists);
  exists = database.exists ("unittest", "none-existing");
  EXPECT_EQ (false, exists);
}


// Test that notes indexing removes damaged notes.
void test_indexing_fixes_damaged_note ()
{
  refresh_sandbox (false);
  Database_State::create ();
  Webserver_Request webserver_request;
  Database_Notes database_notes (webserver_request);
  database_notes.create ();
  
  const int identifier = database_notes.store_new_note ("", 0, 0, 0, "", "", false);
  const auto path = database_notes.note_file (identifier);
  
  // The note produced above looks like this:
  // {
  //   "bible": "",
  //   "contents": "contents",
  //   "modified": "1724784992",
  //   "passage": " 0.0.0 ",
  //   "severity": "2",
  //   "status": "New",
  //   "summary": "summary"
  // }

  // Save a damaged note to disk.
  constexpr const auto note_without_passage = R"(
{
  "bible": "",
  "contents": "contents",
  "modified": "1724784992",
  "passage": "",
  "severity": "2",
  "status": "New",
  "summary": "summary"
}
)";
  filter_url_file_put_contents (path, note_without_passage);
 
  // Do the indexing of the notes.
  database_notes.sync();

  // Check that the above generated an appropriate journal entry.
  refresh_sandbox (true, {
    "Damaged consultation note found",
    "This app deleted or marked for deletion consultation note"});
}


TEST (notes, indexing_fixes_damaged_note)
{
  test_indexing_fixes_damaged_note ();
}


TEST (notes, citations)
{
  using namespace stylesv2;
  
  std::vector<std::string> numbers1to9 {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
  std::vector<std::string> alphanumeric {"1", "a", "x", "7"};

  const Style style_f = *std::find(styles.cbegin(), styles.cend(), "f");
  const Style style_fe = *std::find(styles.cbegin(), styles.cend(), "fe");

  // Test that an empty note citation sequence leads to a continually inceasing note citation.
  {
    filter::note::citations citations;
    Style style {style_f};
    style.properties.clear();
    citations.evaluate_style(style);
    for (int i {1}; i <= 100; i++) {
      EXPECT_EQ(std::to_string(i), citations.get(style_f.marker, "+"));
      EXPECT_EQ("a", citations.get(style_f.marker, "a"));
      EXPECT_EQ("", citations.get(style_f.marker, "-"));
    }
  }

  // Test the note citation as 1..9 and restarting the cycle again.
  {
    filter::note::citations citations;
    citations.evaluate_style(style_f);
    size_t pointer{0};
    for (int i {0}; i < 100; i++) {
      const auto standard = numbers1to9.at(pointer);
      pointer++;
      if (pointer >= numbers1to9.size())
        pointer = 0;
      EXPECT_EQ(standard, citations.get(style_f.marker, "+"));
      EXPECT_EQ("a", citations.get(style_f.marker, "a"));
      EXPECT_EQ("", citations.get(style_f.marker, "-"));
    }
  }
  
  // Test a user-set sequence of note callers, and restarting the cycle again.
  {
    filter::note::citations citations;
    Style style {style_f};
    style.properties[stylesv2::Property::note_numbering_sequence] = filter::strings::implode(alphanumeric, " ");
    citations.evaluate_style(style);
    size_t pointer{0};
    for (int i {0}; i < 100; i++) {
      const auto standard = alphanumeric.at(pointer);
      pointer++;
      if (pointer >= alphanumeric.size())
        pointer = 0;
      EXPECT_EQ(standard, citations.get(style.marker, "+"));
      EXPECT_EQ("a", citations.get(style.marker, "a"));
      EXPECT_EQ("", citations.get(style.marker, "-"));
    }
  }
}


TEST (notes, restart_numbering_at_chapter)
{
  const std::string usfm =
  R"(\id GEN)" "\n"
  R"(\c 1)" "\n"
  R"(\p text\f + \fr ref\ft note\f*)" "\n"
  R"(\c 2)" "\n"
  R"(\p text\f + \fr ref\ft note\f*)" "\n"
  ;
  Filter_Text filter_text = Filter_Text (std::string());
  filter_text.html_text_standard = new HtmlText (std::string());
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  const std::string html = filter_text.html_text_standard->get_inner_html();
  const std::string standard_html =
  R"(<p class="p"><span>text</span><a href="#note1" id="citation1" class="superscript">1</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note2" id="citation2" class="superscript">1</a></p>)"
  R"(<div>)"
  R"(<p class="ft"><a href="#citation1" id="note1">1</a><span> </span><span class="fr">ref</span><span>note</span></p>)"
  R"(<p class="ft"><a href="#citation2" id="note2">1</a><span> </span><span class="fr">ref</span><span>note</span></p>)"
  R"(</div>)"
  ;
  const auto make_readable = [] (const auto& html) {
    return filter::strings::replace ("</p>", "</p>\n", html);
  };
  if (html != standard_html) {
    ADD_FAILURE() << "The produced html differs from the reference html";
    std::cout << "Generated html:" << std::endl;
    std::cout << make_readable(html) << std::endl;
  }
}


#endif

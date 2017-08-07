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


void test_dev () // Todo move into place.
{
  trace_unit_tests (__func__);
  
  // Test universal methods for setting note properties.
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create_v12 ();

    string bible_v1 = "bible1";
    string bible_v2 = "bible2";
    Passage passage_v1 = Passage ("", 1, 2, "3");
    Passage passage_v2 = Passage ("", 4, 5, "6");

    // Create note in the old format, and one in the new format.
    int oldidentifier_v1 = database_notes.store_new_note_v1 (bible_v1, passage_v1.book, passage_v1.chapter, convert_to_int (passage_v1.verse), "summary1", "contents1", false);
    int identifier_v1 = oldidentifier_v1 + 2;
    int oldidentifier_v2 = database_notes.store_new_note_v2 (bible_v2, passage_v2.book, passage_v2.chapter, convert_to_int (passage_v2.verse), "summary2", "contents2", false);
    int identifier_v2 = oldidentifier_v2 + 4;
    
    // Call the universal method to set a new identifier.
    database_notes.set_identifier_v12 (oldidentifier_v1, identifier_v1);
    database_notes.set_identifier_v12 (oldidentifier_v2, identifier_v2);

    // Test the specific methods and the single universal method to get the summaries.
    evaluate (__LINE__, __func__, "summary1", database_notes.get_summary_v1 (identifier_v1));
    evaluate (__LINE__, __func__, "summary1", database_notes.get_summary_v12 (identifier_v1));
    evaluate (__LINE__, __func__, "summary2", database_notes.get_summary_v2 (identifier_v2));
    evaluate (__LINE__, __func__, "summary2", database_notes.get_summary_v12 (identifier_v2));
    evaluate (__LINE__, __func__, "", database_notes.get_summary_v1 (identifier_v2));
    evaluate (__LINE__, __func__, "", database_notes.get_summary_v2 (identifier_v1));

    // Test the specific methods and the single universal method to get the contents.
    string contents = database_notes.get_contents_v1 (identifier_v1);
    evaluate (__LINE__, __func__, true, contents.find ("contents1") != string::npos);
    evaluate (__LINE__, __func__, contents, database_notes.get_contents_v12 (identifier_v1));
    contents = database_notes.get_contents_v2 (identifier_v2);
    evaluate (__LINE__, __func__, true, contents.find ("contents2") != string::npos);
    evaluate (__LINE__, __func__, contents, database_notes.get_contents_v12 (identifier_v2));
    evaluate (__LINE__, __func__, "", database_notes.get_contents_v1 (identifier_v2));
    evaluate (__LINE__, __func__, "", database_notes.get_contents_v2 (identifier_v1));

    // Test the general method to get the subscribers.
    string subscriber_v1 = "subscriber1";
    string subscriber_v2 = "subscriber2";
    database_notes.set_subscribers_v1 (identifier_v1, { subscriber_v1 });
    database_notes.set_subscribers_v2 (identifier_v2, { subscriber_v2 });
    evaluate (__LINE__, __func__, { subscriber_v1 }, database_notes.get_subscribers_v1 (identifier_v1));
    evaluate (__LINE__, __func__, { subscriber_v2 }, database_notes.get_subscribers_v2 (identifier_v2));
    evaluate (__LINE__, __func__, { subscriber_v1 }, database_notes.get_subscribers_v12 (identifier_v1));
    evaluate (__LINE__, __func__, { subscriber_v2 }, database_notes.get_subscribers_v12 (identifier_v2));

    // Test the general method to test a subscriber to a note.
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v1 (identifier_v1, subscriber_v1));
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v2 (identifier_v2, subscriber_v2));
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v12 (identifier_v1, subscriber_v1));
    evaluate (__LINE__, __func__, true, database_notes.is_subscribed_v12 (identifier_v2, subscriber_v2));

    // Test the general methods for the assignees.
    string assignee_v1 = "assignee1";
    string assignee_v2 = "assignee2";
    database_notes.set_assignees_v1 (identifier_v1, { assignee_v1 });
    database_notes.set_assignees_v2 (identifier_v2, { assignee_v2 });
    evaluate (__LINE__, __func__, { assignee_v1 }, database_notes.get_assignees_v12 (identifier_v1));
    evaluate (__LINE__, __func__, { assignee_v2 }, database_notes.get_assignees_v12 (identifier_v2));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned_v12 (identifier_v1, assignee_v1));
    evaluate (__LINE__, __func__, true, database_notes.is_assigned_v12 (identifier_v2, assignee_v2));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned_v12 (identifier_v1, assignee_v2));
    evaluate (__LINE__, __func__, false, database_notes.is_assigned_v12 (identifier_v2, assignee_v1));

    // Test the general methods for the Bible.
    evaluate (__LINE__, __func__, bible_v1, database_notes.get_bible_v12 (identifier_v1));
    evaluate (__LINE__, __func__, bible_v2, database_notes.get_bible_v12 (identifier_v2));
    
    // Test the general methods for the passage.
    vector <Passage> passages;
    passages = database_notes.get_passages_v12 (identifier_v1);
    evaluate (__LINE__, __func__, 1, passages.size());
    for (auto passage : passages) evaluate (__LINE__, __func__, true, passage_v1.equal (passage));
    passages = database_notes.get_passages_v12 (identifier_v2);
    evaluate (__LINE__, __func__, 1, passages.size());
    for (auto passage : passages) evaluate (__LINE__, __func__, true, passage_v2.equal (passage));
    evaluate (__LINE__, __func__, " 1.2.3 ", database_notes.get_raw_passage_v12 (identifier_v1));
    evaluate (__LINE__, __func__, " 4.5.6 ", database_notes.get_raw_passage_v12 (identifier_v2));

    

    
  }

  /*
  {
    string bible = "bible";
    filter_url_file_put_contents (database_notes.bible_file_v1 (identifier), bible);
    string expiry = "7";
    filter_url_file_put_contents (database_notes.expiry_file_v1 (identifier), expiry);
    string modified = "123";
    filter_url_file_put_contents (database_notes.modified_file_v1 (identifier), modified);
    string passage = "passage";
    filter_url_file_put_contents (database_notes.passage_file_v1 (identifier), passage);
    string publicc = "1";
    filter_url_file_put_contents (database_notes.public_file_v1 (identifier), publicc);
    string severity = "55";
    filter_url_file_put_contents (database_notes.severity_file_v1 (identifier), severity);
    string status = "Important";
    filter_url_file_put_contents (database_notes.status_file_v1 (identifier), status);
    string subscriptions = "foo";
    filter_url_file_put_contents (database_notes.subscriptions_file_v1 (identifier), subscriptions);

   evaluate (__LINE__, __func__, assigned, database_notes.get_raw_assigned_v2 (identifier));
    evaluate (__LINE__, __func__, bible, database_notes.get_bible_v2 (identifier));
    evaluate (__LINE__, __func__, expiry, database_notes.get_field_v2 (identifier, database_notes.expiry_key_v2 ()));
    evaluate (__LINE__, __func__, convert_to_int (modified), database_notes.get_modified_v2 (identifier));
    evaluate (__LINE__, __func__, passage, database_notes.get_raw_passage_v2 (identifier));
    evaluate (__LINE__, __func__, convert_to_bool (publicc), database_notes.get_public_v2 (identifier));
    evaluate (__LINE__, __func__, convert_to_int (severity), database_notes.get_raw_severity_v2 (identifier));
    evaluate (__LINE__, __func__, status, database_notes.get_raw_status_v2 (identifier));
    evaluate (__LINE__, __func__, subscriptions, database_notes.get_raw_subscriptions_v2 (identifier));
  }
  */
  
  cout << "dev done" << endl;
}

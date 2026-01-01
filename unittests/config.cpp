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
#include <database/config/general.h>
#include <database/config/bible.h>
#include <filter/string.h>
#include <filter/date.h>
#include <filter/url.h>
#include <database/state.h>
#include <database/login.h>
#include <demo/logic.h>


TEST(database, config_general)
{
  refresh_sandbox (false);
  
  EXPECT_EQ ("Cloud", database::config::general::get_site_mail_name ());
  
  const std::string value {"unittest"};
  database::config::general::set_site_mail_name (value);
  EXPECT_EQ (value, database::config::general::get_site_mail_name ());
  
  database::config::general::set_site_mail_name (std::string());
  EXPECT_EQ (std::string(), database::config::general::get_site_mail_name ());
  
  EXPECT_EQ (std::string(), database::config::general::get_mail_storage_protocol ());
  
  refresh_sandbox (false);
}


TEST(database, config_bible)
{
  std::string value = database::config::bible::get_versification_system ("phpunit");
  EXPECT_EQ (filter::string::english (), value);
  
  value = database::config::bible::get_versification_system ("x");
  EXPECT_EQ (filter::string::english (), value);

  database::config::bible::set_versification_system ("phpunit", "VersificatioN");
  value = database::config::bible::get_versification_system ("phpunit");
  EXPECT_EQ ("VersificatioN", value);

  // Check default value for Bible.
  std::string bible = "A Bible";
  std::string standard = ", ;";
  std::string suffix = " suffix";
  value = database::config::bible::get_sentence_structure_middle_punctuation (bible);
  EXPECT_EQ (standard, value);
  // Change value and check it.
  database::config::bible::set_sentence_structure_middle_punctuation (bible, standard + suffix);
  value = database::config::bible::get_sentence_structure_middle_punctuation (bible);
  EXPECT_EQ (standard + suffix, value);
  // Remove that Bible and check that the value is back to default.
  database::config::bible::remove (bible);
  value = database::config::bible::get_sentence_structure_middle_punctuation (bible);
  EXPECT_EQ (standard, value);
}


// Test the user configuration database.
TEST(database, config_user)
{
  // Set it up.
  refresh_sandbox (true);
  Webserver_Request request;
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  std::string username = "username";
  std::string password = "password";
  database_users.add_user (username, password, 5, "");
  request.session_logic ()->attempt_login (username, password, true);

  // Testing setList, getList, plus add/removeUpdatedSetting.
  {
    EXPECT_EQ (std::vector<int>{}, request.database_config_user ()->get_updated_settings ());
    
    std::vector <int> standard1 = {123, 456};
    request.database_config_user ()->set_updated_settings (standard1);
    EXPECT_EQ (standard1, request.database_config_user ()->get_updated_settings ());
    
    request.database_config_user ()->add_updated_setting (789);
    standard1.push_back (789);
    EXPECT_EQ (standard1, request.database_config_user ()->get_updated_settings ());
    
    request.database_config_user ()->remove_updated_setting (456);
    std::vector <int> standard2 = {123, 789};
    EXPECT_EQ (standard2, request.database_config_user ()->get_updated_settings ());
  }
  
  // Testing the Sprint month and trimming it.
  // It should get today's month.
  {
    int month = filter::date::numerical_month (filter::date::seconds_since_epoch ());
    EXPECT_EQ (month, request.database_config_user ()->get_sprint_month ());
    // Set the sprint month to another month value: It should get this value back from the database.
    int newmonth = 123;
    request.database_config_user ()->set_sprint_month (newmonth);
    EXPECT_EQ (newmonth, request.database_config_user ()->get_sprint_month ());
    // Trim: The sprint month should not be reset.
    request.database_config_user ()->trim ();
    EXPECT_EQ (newmonth, request.database_config_user ()->get_sprint_month ());
    // Set the modification time of the sprint month record to more than two days ago:
    // Trimming resets the sprint month to the current month.
    std::string filename = filter_url_create_path ({testing_directory, "databases", "config", "user", "username", "sprint-month"});
    struct stat foo;
    utimbuf new_times;
    stat (filename.c_str(), &foo);
    new_times.actime = filter::date::seconds_since_epoch () - (2 * 24 * 3600) - 10;
    new_times.modtime = filter::date::seconds_since_epoch () - (2 * 24 * 3600) - 10;
    utime (filename.c_str(), &new_times);
    request.database_config_user ()->trim ();
    EXPECT_EQ (month, request.database_config_user ()->get_sprint_month ());
  }
  
  // Test boolean setting.
  EXPECT_EQ (false, request.database_config_user ()->get_subscribe_to_consultation_notes_edited_by_me ());
  request.database_config_user ()->set_subscribe_to_consultation_notes_edited_by_me (true);
  EXPECT_EQ (true, request.database_config_user ()->get_subscribe_to_consultation_notes_edited_by_me ());
  
  // Test integer setting.
  EXPECT_EQ (1, request.database_config_user ()->get_consultation_notes_passage_selector ());
  request.database_config_user ()->set_consultation_notes_passage_selector (11);
  EXPECT_EQ (11, request.database_config_user ()->get_consultation_notes_passage_selector ());
  
  // Test string setting.
  EXPECT_EQ (std::string(), request.database_config_user ()->get_consultation_notes_assignment_selector ());
  request.database_config_user ()->set_consultation_notes_assignment_selector ("test");
  EXPECT_EQ ("test", request.database_config_user ()->get_consultation_notes_assignment_selector ());

  // Sprint year.
  EXPECT_EQ (filter::date::numerical_year (filter::date::seconds_since_epoch ()), request.database_config_user ()->get_sprint_year ());
  
  // Test getting a Bible that does not exist: It creates one.
  EXPECT_EQ (demo_sample_bible_name (), request.database_config_user ()->get_bible ());
  
  // Test that after removing a user, the setting reverts to its default value.
  EXPECT_EQ (0, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());
  request.database_config_user ()->set_consultation_notes_text_inclusion_selector (1);
  EXPECT_EQ (1, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());
  request.database_config_user ()->remove (username);
  EXPECT_EQ (0, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());

  // Test setting privileges for a user, and the user retrieving them.
  {
    // Privilege is on by default.
    EXPECT_EQ (true, request.database_config_user ()->get_privilege_use_advanced_mode ());
    // Privilege is on for another user also.
    std::string anotheruser = "anotheruser";
    EXPECT_EQ (true, request.database_config_user ()->get_privilege_use_advanced_mode_for_user (anotheruser));
    // Set it off for the other user.
    request.database_config_user ()->set_privilege_use_advanced_mode_for_user (anotheruser, false);
    EXPECT_EQ (false, request.database_config_user ()->get_privilege_use_advanced_mode_for_user (anotheruser));
    // The privilege is still on for the current user.
    EXPECT_EQ (true, request.database_config_user ()->get_privilege_use_advanced_mode ());
  }
  
  // Filter allowed journal entries.
  refresh_sandbox (true, {"Creating sample Bible", "Sample Bible was created"});
}

#endif


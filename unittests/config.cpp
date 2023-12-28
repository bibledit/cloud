/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include "gtest/gtest.h"
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
  
  EXPECT_EQ ("Cloud", Database_Config_General::getSiteMailName ());
  
  const std::string value {"unittest"};
  Database_Config_General::setSiteMailName (value);
  EXPECT_EQ (value, Database_Config_General::getSiteMailName ());
  
  Database_Config_General::setSiteMailName (std::string());
  EXPECT_EQ (std::string(), Database_Config_General::getSiteMailName ());
  
  EXPECT_EQ (std::string(), Database_Config_General::getMailStorageProtocol ());
  
  refresh_sandbox (false);
}


TEST(database, config_bible)
{
  std::string value = Database_Config_Bible::getVersificationSystem ("phpunit");
  EXPECT_EQ (filter::strings::english (), value);
  
  value = Database_Config_Bible::getVersificationSystem ("x");
  EXPECT_EQ (filter::strings::english (), value);

  Database_Config_Bible::setVersificationSystem ("phpunit", "VersificatioN");
  value = Database_Config_Bible::getVersificationSystem ("phpunit");
  EXPECT_EQ ("VersificatioN", value);

  // Check default value for Bible.
  std::string bible = "A Bible";
  std::string standard = ", ;";
  std::string suffix = " suffix";
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  EXPECT_EQ (standard, value);
  // Change value and check it.
  Database_Config_Bible::setSentenceStructureMiddlePunctuation (bible, standard + suffix);
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  EXPECT_EQ (standard + suffix, value);
  // Remove that Bible and check that the value is back to default.
  Database_Config_Bible::remove (bible);
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
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
    EXPECT_EQ (std::vector<int>{}, request.database_config_user ()->getUpdatedSettings ());
    
    std::vector <int> standard1 = {123, 456};
    request.database_config_user ()->setUpdatedSettings (standard1);
    EXPECT_EQ (standard1, request.database_config_user ()->getUpdatedSettings ());
    
    request.database_config_user ()->addUpdatedSetting (789);
    standard1.push_back (789);
    EXPECT_EQ (standard1, request.database_config_user ()->getUpdatedSettings ());
    
    request.database_config_user ()->removeUpdatedSetting (456);
    std::vector <int> standard2 = {123, 789};
    EXPECT_EQ (standard2, request.database_config_user ()->getUpdatedSettings ());
  }
  
  // Testing the Sprint month and trimming it.
  // It should get today's month.
  {
    int month = filter::date::numerical_month (filter::date::seconds_since_epoch ());
    EXPECT_EQ (month, request.database_config_user ()->getSprintMonth ());
    // Set the sprint month to another month value: It should get this value back from the database.
    int newmonth = 123;
    request.database_config_user ()->setSprintMonth (newmonth);
    EXPECT_EQ (newmonth, request.database_config_user ()->getSprintMonth ());
    // Trim: The sprint month should not be reset.
    request.database_config_user ()->trim ();
    EXPECT_EQ (newmonth, request.database_config_user ()->getSprintMonth ());
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
    EXPECT_EQ (month, request.database_config_user ()->getSprintMonth ());
  }
  
  // Test boolean setting.
  EXPECT_EQ (false, request.database_config_user ()->getSubscribeToConsultationNotesEditedByMe ());
  request.database_config_user ()->setSubscribeToConsultationNotesEditedByMe (true);
  EXPECT_EQ (true, request.database_config_user ()->getSubscribeToConsultationNotesEditedByMe ());
  
  // Test integer setting.
  EXPECT_EQ (1, request.database_config_user ()->getConsultationNotesPassageSelector ());
  request.database_config_user ()->setConsultationNotesPassageSelector (11);
  EXPECT_EQ (11, request.database_config_user ()->getConsultationNotesPassageSelector ());
  
  // Test string setting.
  EXPECT_EQ (std::string(), request.database_config_user ()->getConsultationNotesAssignmentSelector ());
  request.database_config_user ()->setConsultationNotesAssignmentSelector ("test");
  EXPECT_EQ ("test", request.database_config_user ()->getConsultationNotesAssignmentSelector ());

  // Sprint year.
  EXPECT_EQ (filter::date::numerical_year (filter::date::seconds_since_epoch ()), request.database_config_user ()->getSprintYear ());
  
  // Test getting a Bible that does not exist: It creates one.
  EXPECT_EQ (demo_sample_bible_name (), request.database_config_user ()->getBible ());
  
  // Test that after removing a user, the setting reverts to its default value.
  EXPECT_EQ (0, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());
  request.database_config_user ()->setConsultationNotesTextInclusionSelector (1);
  EXPECT_EQ (1, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());
  request.database_config_user ()->remove (username);
  EXPECT_EQ (0, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());

  // Test setting privileges for a user, and the user retrieving them.
  {
    // Privilege is on by default.
    EXPECT_EQ (true, request.database_config_user ()->getPrivilegeUseAdvancedMode ());
    // Privilege is on for another user also.
    std::string anotheruser = "anotheruser";
    EXPECT_EQ (true, request.database_config_user ()->getPrivilegeUseAdvancedModeForUser (anotheruser));
    // Set it off for the other user.
    request.database_config_user ()->setPrivilegeUseAdvancedModeForUser (anotheruser, false);
    EXPECT_EQ (false, request.database_config_user ()->getPrivilegeUseAdvancedModeForUser (anotheruser));
    // The privilege is still on for the current user.
    EXPECT_EQ (true, request.database_config_user ()->getPrivilegeUseAdvancedMode ());
  }
  
  // Filter allowed journal entries.
  refresh_sandbox (true, {"Creating sample Bible", "Sample Bible was created"});
}

#endif


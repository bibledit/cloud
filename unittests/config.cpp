/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/config.h>
#include <unittests/utilities.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <filter/string.h>
#include <filter/date.h>
#include <filter/url.h>
#include <database/state.h>
#include <database/login.h>
#include <demo/logic.h>


void test_database_config_general ()
{
  trace_unit_tests (__func__);
  
  evaluate (__LINE__, __func__, "Cloud", Database_Config_General::getSiteMailName ());
  
  string value = "unittest";
  Database_Config_General::setSiteMailName (value);
  evaluate (__LINE__, __func__, value, Database_Config_General::getSiteMailName ());

  Database_Config_General::setSiteMailName ("");
  evaluate (__LINE__, __func__, "", Database_Config_General::getSiteMailName ());

  evaluate (__LINE__, __func__, "", Database_Config_General::getMailStorageProtocol ());
}


void test_database_config_bible ()
{
  trace_unit_tests (__func__);
  
  string value = Database_Config_Bible::getVersificationSystem ("phpunit");
  evaluate (__LINE__, __func__, english (), value);
  
  value = Database_Config_Bible::getVersificationSystem ("x");
  evaluate (__LINE__, __func__, english (), value);

  Database_Config_Bible::setVersificationSystem ("phpunit", "VersificatioN");
  value = Database_Config_Bible::getVersificationSystem ("phpunit");
  evaluate (__LINE__, __func__, "VersificatioN", value);

  // Check default value for Bible.
  string bible = "A Bible";
  string standard = ", ;";
  string suffix = " suffix";
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  evaluate (__LINE__, __func__, standard, value);
  // Change value and check it.
  Database_Config_Bible::setSentenceStructureMiddlePunctuation (bible, standard + suffix);
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  evaluate (__LINE__, __func__, standard + suffix, value);
  // Remove that Bible and check that the value is back to default.
  Database_Config_Bible::remove (bible);
  value = Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible);
  evaluate (__LINE__, __func__, standard, value);
}


// Test the user configuration database.
void test_database_config_user ()
{
  trace_unit_tests (__func__);

  // Set it up.
  refresh_sandbox (true);
  Webserver_Request request;
  Database_State::create ();
  Database_Login::create ();
  Database_Users database_users;
  database_users.create ();
  database_users.upgrade ();
  string username = "username";
  string password = "password";
  database_users.add_user (username, password, 5, "");
  request.session_logic ()->attempt_login (username, password, true);

  // Testing setList, getList, plus add/removeUpdatedSetting.
  {
    evaluate (__LINE__, __func__, {}, request.database_config_user ()->getUpdatedSettings ());
    
    vector <int> standard1 = {123, 456};
    request.database_config_user ()->setUpdatedSettings (standard1);
    evaluate (__LINE__, __func__, standard1, request.database_config_user ()->getUpdatedSettings ());
    
    request.database_config_user ()->addUpdatedSetting (789);
    standard1.push_back (789);
    evaluate (__LINE__, __func__, standard1, request.database_config_user ()->getUpdatedSettings ());
    
    request.database_config_user ()->removeUpdatedSetting (456);
    vector <int> standard2 = {123, 789};
    evaluate (__LINE__, __func__, standard2, request.database_config_user ()->getUpdatedSettings ());
  }
  
  // Testing the Sprint month and trimming it.
  // It should get today's month.
  {
    int month = filter::date::numerical_month (filter::date::seconds_since_epoch ());
    evaluate (__LINE__, __func__, month, request.database_config_user ()->getSprintMonth ());
    // Set the sprint month to another month value: It should get this value back from the database.
    int newmonth = 123;
    request.database_config_user ()->setSprintMonth (newmonth);
    evaluate (__LINE__, __func__, newmonth, request.database_config_user ()->getSprintMonth ());
    // Trim: The sprint month should not be reset.
    request.database_config_user ()->trim ();
    evaluate (__LINE__, __func__, newmonth, request.database_config_user ()->getSprintMonth ());
    // Set the modification time of the sprint month record to more than two days ago:
    // Trimming resets the sprint month to the current month.
    string filename = filter_url_create_path ({testing_directory, "databases", "config", "user", "username", "sprint-month"});
    struct stat foo;
    struct utimbuf new_times;
    stat (filename.c_str(), &foo);
    new_times.actime = filter::date::seconds_since_epoch () - (2 * 24 * 3600) - 10;
    new_times.modtime = filter::date::seconds_since_epoch () - (2 * 24 * 3600) - 10;
    utime (filename.c_str(), &new_times);
    request.database_config_user ()->trim ();
    evaluate (__LINE__, __func__, month, request.database_config_user ()->getSprintMonth ());
  }
  
  // Test boolean setting.
  evaluate (__LINE__, __func__, false, request.database_config_user ()->getSubscribeToConsultationNotesEditedByMe ());
  request.database_config_user ()->setSubscribeToConsultationNotesEditedByMe (true);
  evaluate (__LINE__, __func__, true, request.database_config_user ()->getSubscribeToConsultationNotesEditedByMe ());
  
  // Test integer setting.
  evaluate (__LINE__, __func__, 1, request.database_config_user ()->getConsultationNotesPassageSelector ());
  request.database_config_user ()->setConsultationNotesPassageSelector (11);
  evaluate (__LINE__, __func__, 11, request.database_config_user ()->getConsultationNotesPassageSelector ());
  
  // Test string setting.
  evaluate (__LINE__, __func__, "", request.database_config_user ()->getConsultationNotesAssignmentSelector ());
  request.database_config_user ()->setConsultationNotesAssignmentSelector ("test");
  evaluate (__LINE__, __func__, "test", request.database_config_user ()->getConsultationNotesAssignmentSelector ());

  // Sprint year.
  evaluate (__LINE__, __func__, filter::date::numerical_year (filter::date::seconds_since_epoch ()), request.database_config_user ()->getSprintYear ());
  
  // Test getting a Bible that does not exist: It creates one.
  evaluate (__LINE__, __func__, demo_sample_bible_name (), request.database_config_user ()->getBible ());
  
  // Test that after removing a user, the setting reverts to its default value.
  evaluate (__LINE__, __func__, 0, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());
  request.database_config_user ()->setConsultationNotesTextInclusionSelector (1);
  evaluate (__LINE__, __func__, 1, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());
  request.database_config_user ()->remove (username);
  evaluate (__LINE__, __func__, 0, request.database_config_user ()->getConsultationNotesTextInclusionSelector ());

  // Test setting privileges for a user, and the user retrieving them.
  {
    // Privilege is on by default.
    evaluate (__LINE__, __func__, true, request.database_config_user ()->getPrivilegeUseAdvancedMode ());
    // Privilege is on for another user also.
    string anotheruser = "anotheruser";
    evaluate (__LINE__, __func__, true, request.database_config_user ()->getPrivilegeUseAdvancedModeForUser (anotheruser));
    // Set it off for the other user.
    request.database_config_user ()->setPrivilegeUseAdvancedModeForUser (anotheruser, false);
    evaluate (__LINE__, __func__, false, request.database_config_user ()->getPrivilegeUseAdvancedModeForUser (anotheruser));
    // The privilege is still on for the current user.
    evaluate (__LINE__, __func__, true, request.database_config_user ()->getPrivilegeUseAdvancedMode ());
  }
  
  // Filter allowed journal entries.
  refresh_sandbox (true, {"Creating sample Bible", "Sample Bible was created"});
}

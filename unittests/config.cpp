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
  
  evaluate (__LINE__, __func__, "Bibledit Cloud", Database_Config_General::getSiteMailName ());
  
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
  
  // Versification / Mapping
  {
    string versification = Database_Config_Bible::getVersificationSystem ("phpunit");
    evaluate (__LINE__, __func__, english (), versification);
    string mapping = Database_Config_Bible::getVerseMapping ("phpunit");
    evaluate (__LINE__, __func__, english (), mapping);
    versification = Database_Config_Bible::getVersificationSystem ("x");
    evaluate (__LINE__, __func__, english (), versification);
    mapping = Database_Config_Bible::getVerseMapping ("x");
    evaluate (__LINE__, __func__, english (), mapping);
    Database_Config_Bible::setVersificationSystem ("phpunit", "VersificatioN");
    versification = Database_Config_Bible::getVersificationSystem ("phpunit");
    evaluate (__LINE__, __func__, "VersificatioN", versification);
    Database_Config_Bible::setVerseMapping ("phpunit", "VersificatioN");
    mapping = Database_Config_Bible::getVerseMapping ("phpunit");
    evaluate (__LINE__, __func__, "VersificatioN", mapping);
  }
}


void test_database_config_user ()
{
  trace_unit_tests (__func__);
  
  // Tests for Database_Config_User.
  {
    // Setup.
    refresh_sandbox (true);
    Webserver_Request request = Webserver_Request ();
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    database_users.add_user ("username", "password", 5, "");
    request.session_logic ()->attemptLogin ("username", "password", true);
    
    // Testing setList, getList, plus add/removeUpdatedSetting.
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
    
    // Testing the Sprint month and its trim () function.
    // It should get today's month.
    int month = filter_date_numerical_month (filter_date_seconds_since_epoch ());
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
    string filename = filter_url_create_path (testing_directory, "databases", "config", "user", "username", "sprint-month");
    struct stat foo;
    struct utimbuf new_times;
    stat (filename.c_str(), &foo);
    new_times.actime = filter_date_seconds_since_epoch () - (2 * 24 * 3600) - 10;
    new_times.modtime = filter_date_seconds_since_epoch () - (2 * 24 * 3600) - 10;
    utime (filename.c_str(), &new_times);
    request.database_config_user ()->trim ();
    evaluate (__LINE__, __func__, month, request.database_config_user ()->getSprintMonth ());
    
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
    
    evaluate (__LINE__, __func__, filter_date_numerical_year (filter_date_seconds_since_epoch ()), request.database_config_user ()->getSprintYear ());
    
    // Test getting a Bible that does not exist: It creates one.
    evaluate (__LINE__, __func__, demo_sample_bible_name (), request.database_config_user ()->getBible ());
    
    // Filter allowed journal entries.
    refresh_sandbox (true, {"Creating sample Bible", "Sample Bible was created"});
  }
}



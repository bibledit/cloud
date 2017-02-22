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


#include <unittests/databases.h>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <filter/shell.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/sqlite.h>
#include <database/users.h>
#include <database/books.h>
#include <database/bibleactions.h>
#include <database/check.h>
#include <database/localization.h>
#include <database/confirm.h>
#include <database/ipc.h>
#include <database/jobs.h>
#include <database/kjv.h>
#include <database/oshb.h>
#include <database/sblgnt.h>
#include <database/sprint.h>
#include <database/mail.h>
#include <database/navigation.h>
#include <database/usfmresources.h>
#include <database/mappings.h>
#include <database/noteactions.h>
#include <database/versifications.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <database/volatile.h>
#include <database/state.h>
#include <database/imageresources.h>
#include <database/noteassignment.h>
#include <database/strong.h>
#include <database/morphgnt.h>
#include <database/etcbc4.h>
#include <database/hebrewlexicon.h>
#include <database/cache.h>
#include <database/login.h>
#include <database/privileges.h>
#include <database/git.h>
#include <database/userresources.h>
#include <database/statistics.h>
#include <bible/logic.h>
#include <notes/logic.h>
#include <sync/logic.h>
#include <styles/logic.h>
#include <resource/external.h>
#include <changes/logic.h>
#include <demo/logic.h>


void test_database_config_general ()
{
  trace_unit_tests (__func__);

  // Tests for Database_Config_General.
  evaluate (__LINE__, __func__, "Bibledit Cloud", Database_Config_General::getSiteMailName ());
  
  string ref = "unittest";
  Database_Config_General::setSiteMailName (ref);
  evaluate (__LINE__, __func__, ref, Database_Config_General::getSiteMailName ());
  
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


void test_database_logs ()
{
  trace_unit_tests (__func__);
  
  // Tests for Database_Logs.
  {
    refresh_sandbox (true);
    // Log some items.
    Database_Logs::log ("description1", 2);
    Database_Logs::log ("description2", 3);
    Database_Logs::log ("description3", 4);
    // Rotate the items.
    Database_Logs::rotate ();
    // Get the items from the SQLite database.
    string lastfilename;
    vector <string> result = Database_Logs::get (lastfilename);
    evaluate (__LINE__, __func__, 3, result.size ());
    refresh_sandbox (false);
  }
  {
    // Test huge journal entry.
    refresh_sandbox (true);
    string huge (60000, 'x');
    Database_Logs::log (huge);
    Database_Logs::rotate ();
    string s = "0";
    vector <string> result = Database_Logs::get (s);
    if (result.size () == 1) {
      s = result [0];
      string path = filter_url_create_path (Database_Logs::folder (), s);
      string contents = filter_url_file_get_contents (path);
      evaluate (__LINE__, __func__, 50006, contents.find ("This entry was too large and has been truncated: 60000 bytes"));
    } else {
      evaluate (__LINE__, __func__, 1, (int)result.size ());
    }
    refresh_sandbox (true, {"This entry was too large and has been truncated"});
  }
  {
    // Test the getNext function of the Journal.
    refresh_sandbox (true);
    Database_Logs::log ("description");
    int second = filter_date_seconds_since_epoch ();
    string filename = convert_to_string (second) + "00000000";
    // First time: getNext gets the logged entry.
    string s;
    s = Database_Logs::next (filename);
    if (s == "") evaluate (__LINE__, __func__, "...description", s);
    // Since variable "filename" is updated and set to the last filename,
    // next time function getNext gets nothing.
    s = Database_Logs::next (filename);
    evaluate (__LINE__, __func__, "", s);
    refresh_sandbox (false);
  }
}


void test_database_users ()
{
  trace_unit_tests (__func__);
  
  // Tests for Database_Users.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();

    string username = "unit test";
    string password = "pazz";
    int level = 10;
    string email = "email@site.nl";

    database_users.optimize ();
    database_users.trim ();
    
    database_users.add_user (username, password, level, email);

    evaluate (__LINE__, __func__, true, database_users.matchUserPassword (username, password));
    evaluate (__LINE__, __func__, false, database_users.matchUserPassword (username, "wrong password"));

    evaluate (__LINE__, __func__, true, database_users.matchEmailPassword (email, password));
    evaluate (__LINE__, __func__, false, database_users.matchEmailPassword (email, "wrong password"));

    // No matches for a disabled account.
    database_users.set_enabled (username, false);
    evaluate (__LINE__, __func__, false, database_users.matchUserPassword (username, password));
    evaluate (__LINE__, __func__, false, database_users.matchEmailPassword (email, password));

    string ref = "INSERT INTO users (username, password, level, email) VALUES ('unit test', '014877e71841e82d44ce524d66dcc732', 10, 'email@site.nl');";
    string act = database_users.add_userQuery (username, password, level, email);
    evaluate (__LINE__, __func__, ref, act);

    evaluate (__LINE__, __func__, username, database_users.getEmailToUser (email));
    evaluate (__LINE__, __func__, "", database_users.getEmailToUser ("wrong email"));

    evaluate (__LINE__, __func__, email, database_users.get_email (username));
    evaluate (__LINE__, __func__, "", database_users.get_email ("wrong username"));

    evaluate (__LINE__, __func__, true, database_users.usernameExists (username));
    evaluate (__LINE__, __func__, false, database_users.usernameExists ("invalid username"));

    evaluate (__LINE__, __func__, true, database_users.emailExists (email));
    evaluate (__LINE__, __func__, false, database_users.emailExists ("invalid email"));

    evaluate (__LINE__, __func__, level, database_users.get_level (username));
    evaluate (__LINE__, __func__, Filter_Roles::guest (), database_users.get_level ("invalid username"));

    level = 7;
    database_users.set_level (username, level);
    evaluate (__LINE__, __func__, level, database_users.get_level (username));

    database_users.removeUser (username);
    evaluate (__LINE__, __func__, false, database_users.usernameExists (username));
    
    evaluate (__LINE__, __func__, " UPDATE users SET email =  'email@site.nl'  WHERE username =  'unit test'  ; ", database_users.updateEmailQuery (username, email));
  }

  // Test administrators and updating email.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();

    // Test data for two admins.
    string username1 = "unit test1";
    string username2 = "unit test2";
    string password = "pazz";
    int level = Filter_Roles::admin();
    string email = "email@site";

    database_users.add_user (username1, password, level, email);
    vector <string> admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 1, (int)admins.size());
    if (!admins.empty()) evaluate (__LINE__, __func__, username1, admins [0]);
    
    database_users.add_user (username2, password, level, email);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 2, (int)admins.size());
    
    // Check that a disabled admin account is not included in the number of administrators.
    database_users.set_enabled (username1, false);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 1, (int)admins.size());

    // Check that once an account is enabled, it is included again in the number of administrators.
    database_users.set_enabled (username1, true);
    admins = database_users.getAdministrators ();
    evaluate (__LINE__, __func__, 2, (int)admins.size());
    
    email = "new@email.address";
    database_users.updateUserEmail (username1, email);
    evaluate (__LINE__, __func__, email, database_users.get_email (username1));
    
    vector <string> users = database_users.getUsers ();
    evaluate (__LINE__, __func__, 2, (int)users.size());
    
    evaluate (__LINE__, __func__, md5 (password), database_users.get_md5 (username1));
  }
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();

    // LDAP should be off initially.
    string user = "unittest";
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
    database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));

    // Test LDAP on.
    database_users.set_ldap (user + "x", true);
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
    database_users.set_ldap (user, true);
    evaluate (__LINE__, __func__, true, database_users.get_ldap (user));
    
    // Test LDAP off.
    database_users.set_ldap (user, false);
    evaluate (__LINE__, __func__, false, database_users.get_ldap (user));
  }
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    database_users.upgrade ();
    
    // Non-existing account is disabled.
    string user = "unittest";
    evaluate (__LINE__, __func__, false, database_users.get_enabled (user));
    
    // Account should be enabled initially.
    database_users.add_user (user, "password", Filter_Roles::consultant(), "email@site");
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
    
    // Test disable account of other user.
    database_users.set_enabled (user + "x", false);
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
    
    // Test disable account.
    database_users.set_enabled (user, false);
    evaluate (__LINE__, __func__, false, database_users.get_enabled (user));
    
    // Test enable account.
    database_users.set_enabled (user, true);
    evaluate (__LINE__, __func__, true, database_users.get_enabled (user));
  }
}


// Tests for the Database_Books object.
void test_database_books ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  evaluate (__LINE__, __func__, 69, (int)Database_Books::getIDs ().size());
  evaluate (__LINE__, __func__, 2, Database_Books::getIdFromEnglish ("Exodus"));
  evaluate (__LINE__, __func__, 0, Database_Books::getIdFromEnglish ("exodus"));
  evaluate (__LINE__, __func__, "Leviticus", Database_Books::getEnglishFromId (3));
  evaluate (__LINE__, __func__, "NUM", Database_Books::getUsfmFromId (4));
  evaluate (__LINE__, __func__, "Deu", Database_Books::getBibleworksFromId (5));
  evaluate (__LINE__, __func__, 22, Database_Books::getIdFromUsfm ("SNG"));
  evaluate (__LINE__, __func__, 13, Database_Books::getIdFromOsis ("1Chr"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdFromBibleworks ("2Ki"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdLikeText ("2Ki"));
  evaluate (__LINE__, __func__, 14, Database_Books::getIdLikeText ("2Chron"));
  evaluate (__LINE__, __func__, 1, Database_Books::getIdLikeText ("Genes"));
  evaluate (__LINE__, __func__, 12, Database_Books::getIdFromOnlinebible ("2Ki"));
  evaluate (__LINE__, __func__, "De", Database_Books::getOnlinebibleFromId (5));
  evaluate (__LINE__, __func__, "5", Database_Books::getSequenceFromId (5));
  evaluate (__LINE__, __func__, "nt", Database_Books::getType (40));
  evaluate (__LINE__, __func__, "ot", Database_Books::getType (39));
  evaluate (__LINE__, __func__, "", Database_Books::getType (0));
}


void test_database_bibleactions ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_BibleActions database_bibleactions;
  database_bibleactions.create ();
  
  database_bibleactions.optimize ();
  
  vector <string> bibles = database_bibleactions.getBibles ();
  evaluate (__LINE__, __func__, { }, bibles);

  database_bibleactions.record ("phpunit1", 1, 2, "data112");
  database_bibleactions.record ("phpunit1", 1, 3, "data113");
  database_bibleactions.record ("phpunit1", 2, 4, "data124");
  database_bibleactions.record ("phpunit2", 5, 6, "data256");
  database_bibleactions.record ("phpunit2", 5, 6, "data256: Not to be stored");

  bibles = database_bibleactions.getBibles ();
  evaluate (__LINE__, __func__, {"phpunit1", "phpunit2"}, bibles);

  vector <int> books = database_bibleactions.getBooks ("phpunit1");
  evaluate (__LINE__, __func__, {1, 2}, books);

  vector <int> chapters = database_bibleactions.getChapters ("phpunit1", 1);
  evaluate (__LINE__, __func__, {2, 3}, chapters);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, {4}, chapters);
  
  database_bibleactions.erase ("phpunit1", 2, 3);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, {4}, chapters);
  
  database_bibleactions.erase ("phpunit1", 2, 4);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, { }, chapters);
  
  string usfm = database_bibleactions.getUsfm ("phpunit2", 5, 5);
  evaluate (__LINE__, __func__, "", usfm);
  
  usfm = database_bibleactions.getUsfm ("phpunit2", 5, 6);
  evaluate (__LINE__, __func__, "data256", usfm);
}


void test_database_check ()
{
  trace_unit_tests (__func__);
  
  {
    // Test Optimize
    refresh_sandbox (true);
    Database_Check database_check = Database_Check ();
    database_check.optimize ();
  }
  {
    // Test Record Get Truncate.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();

    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, (int)hits.size());
  
    database_check.recordOutput ("phpunit", 1, 2, 3, "test");
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    
    database_check.truncateOutput ("");
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, (int)hits.size());
  }
  {
    // Test getting details.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 5, 2, 3, "test");
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    evaluate (__LINE__, __func__, "phpunit", hits [0].bible);
    evaluate (__LINE__, __func__, 5, hits [0].book);
    evaluate (__LINE__, __func__, 2, hits [0].chapter);
    evaluate (__LINE__, __func__, "test", hits [0].data);
  }
  {
    // Test approvals.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();

    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
  
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
  
    int id = hits [0].rowid;
    database_check.approve (id);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    
    vector <Database_Check_Hit> suppressions = database_check.getSuppressions ();
    evaluate (__LINE__, __func__, 1, (int)suppressions.size());
    
    id = suppressions [0].rowid;
    
    database_check.release (1);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
  }
  {
    // Test delete.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
    int id = hits [0].rowid;
    database_check.erase (id);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
  }
  {
    // Test passage.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 6, 7, 8, "test2");
    Passage passage = database_check.getPassage (2);
    evaluate (__LINE__, __func__, 6, passage.book);
    evaluate (__LINE__, __func__, 7, passage.chapter);
    evaluate (__LINE__, __func__, "8", passage.verse);
  }
  {
    // Test same checks overflow.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "once");
    for (unsigned int i = 0; i < 100; i++) {
      database_check.recordOutput ("phpunit", i, i, i, "multiple");
    }
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 12, (int)hits.size());
  }
}


void test_database_localization ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  string file_po = filter_url_create_root_path ("unittests", "tests", "nl.po");
  Database_Localization database_localization = Database_Localization ("nl");
  database_localization.create (file_po);

  string msgid = "phpunit";
  string msgstr = "phpunit";
  string result = database_localization.translate (msgid);
  evaluate (__LINE__, __func__, msgstr, result);
  result = database_localization.backtranslate (msgstr);
  evaluate (__LINE__, __func__, msgid, result);

  msgid = "When this workbench will be opened, it will display all the notes that refer to the focused passage.";
  msgstr = "Als de werkbank geopend wordt, dan toont het alle aantekeningen die betrekking hebben op de gefocuste passage.";
  result = database_localization.translate (msgid);
  evaluate (__LINE__, __func__, msgstr, result);
  result = database_localization.backtranslate (msgstr);
  evaluate (__LINE__, __func__, msgid, result);
}


void test_database_confirm ()
{
#ifdef HAVE_CLOUD
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_Confirm database_confirm = Database_Confirm ();
  database_confirm.create ();

  database_confirm.optimize ();
  database_confirm.trim ();

  // New ID generation test.
  int id = database_confirm.getNewID ();
  if (id < 10000) evaluate (__LINE__, __func__, "Should be greater than 10000", id);
  
  // Store data for the ID.
  database_confirm.store (id, "SELECT x, y, z FROM a;", "email", "subject", "body");
  
  // Search for this ID based on subject.
  int id2 = database_confirm.searchID ("Subject line CID" + convert_to_string (id) + " Re:");
  evaluate (__LINE__, __func__, id, id2);

  // Retrieve data for the ID.
  string query = database_confirm.getQuery (id);
  evaluate (__LINE__, __func__,"SELECT x, y, z FROM a;", query);

  string to = database_confirm.getMailTo (id);
  evaluate (__LINE__, __func__,"email", to);

  string subject = database_confirm.getSubject (id);
  evaluate (__LINE__, __func__,"subject", subject);

  string body = database_confirm.getBody (id);
  evaluate (__LINE__, __func__,"body", body);

  // Delete this ID.
  database_confirm.erase (id);
  query = database_confirm.getQuery (id);
  evaluate (__LINE__, __func__,"", query);
#endif
}


void test_database_ipc ()
{
  trace_unit_tests (__func__);
  
  // Test Trim
  {
    refresh_sandbox (true);
    Database_Ipc database_ipc = Database_Ipc (NULL);
    database_ipc.trim ();
  }
  // Test Store Retrieve
  {
    refresh_sandbox (true);
    Webserver_Request request = Webserver_Request ();
    Database_Ipc database_ipc = Database_Ipc (&request);

    int id = 1;
    string user = "phpunit";
    string channel = "channel";
    string command = "command";
    string message = "message";
  
    database_ipc.storeMessage (user, channel, command, message);
   
    Database_Ipc_Message data = database_ipc.retrieveMessage (id, user, channel, command);
    evaluate (__LINE__, __func__, 0, data.id);
  
    database_ipc.storeMessage (user, channel, command, message);
  
    data = database_ipc.retrieveMessage (id, user, channel, command);
    evaluate (__LINE__, __func__, 2, data.id);
    evaluate (__LINE__, __func__, message, data.message);
  }
  // Test Delete
  {
    refresh_sandbox (true);
    Webserver_Request request = Webserver_Request ();
    Database_Ipc database_ipc = Database_Ipc (&request);

    int id = 1;
    string user = "phpunit";
    string channel = "channel";
    string command = "command";
    string message = "message";
    
    database_ipc.storeMessage (user, channel, command, message);
    
    Database_Ipc_Message data = database_ipc.retrieveMessage (0, user, channel, command);
    evaluate (__LINE__, __func__, id, data.id);
    
    database_ipc.deleteMessage (id);
  
    data = database_ipc.retrieveMessage (0, user, channel, command);
    evaluate (__LINE__, __func__, 0, data.id);
  }
  // Test Get Focus
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Ipc database_ipc = Database_Ipc (&request);

    string user = "phpunit";
    request.session_logic ()->setUsername (user);
    string channel = "channel";
    string command = "focus";
  
    string passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, "1.1.1", passage);
  
    string message = "2.3.4";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, message, passage);
  
    message = "5.6.7";
    database_ipc.storeMessage (user, channel, command, message);
    passage = database_ipc.getFocus ();
    evaluate (__LINE__, __func__, message, passage);
  }
  // Test Get Note.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    string user = "phpunit";
    request.session_logic ()->setUsername (user);
    string channel = "channel";
    string command = "opennote";
  
    Database_Ipc_Message note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, 0, note.id);
  
    string message = "12345";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, message, note.message);
  
    message = "54321";
    database_ipc.storeMessage (user, channel, command, message);
    note = database_ipc.getNote ();
    evaluate (__LINE__, __func__, message, note.message);
  }
  // Test Notes Alive.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Ipc database_ipc = Database_Ipc (&request);
    
    string user = "phpunit";
    request.session_logic ()->setUsername (user);
    string channel = "channel";
    string command = "notesalive";
  
    bool alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, false, alive);
  
    string message = "1";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, convert_to_bool (message), alive);
  
    message = "0";
    database_ipc.storeMessage (user, channel, command, message);
    alive = database_ipc.getNotesAlive ();
    evaluate (__LINE__, __func__, convert_to_bool (message), alive);
  }
}


void test_database_jobs ()
{
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    Database_Jobs database_jobs = Database_Jobs ();
    database_jobs.create ();

    // Test Optimize
    database_jobs.optimize ();

    // Test Identifiers
    int id = database_jobs.getNewId ();
    bool exists = database_jobs.idExists (id);
    evaluate (__LINE__, __func__, true, exists);

    // Test Level.
    id = database_jobs.getNewId ();
    int level = database_jobs.getLevel (id);
    evaluate (__LINE__, __func__, 0, level);
    database_jobs.setLevel (id, 123);
    level = database_jobs.getLevel (id);
    evaluate (__LINE__, __func__, 123, level);

    // Test Start
    id = database_jobs.getNewId ();
    string start = database_jobs.getStart (id);
    evaluate (__LINE__, __func__, "", start);
    database_jobs.setStart (id, "start");
    start = database_jobs.getStart (id);
    evaluate (__LINE__, __func__, "start", start);
    
    // Test Progress
    id = database_jobs.getNewId ();
    string progress = database_jobs.getProgress (id);
    evaluate (__LINE__, __func__, "", progress);
    database_jobs.setProgress (id, "progress");
    progress = database_jobs.getProgress (id);
    evaluate (__LINE__, __func__, "progress", progress);
    
    // Test Percentage
    id = database_jobs.getNewId ();
    string percentage = database_jobs.getPercentage (id);
    evaluate (__LINE__, __func__, "", percentage);
    database_jobs.setPercentage (id, 55);
    percentage = database_jobs.getPercentage (id);
    evaluate (__LINE__, __func__, "55", percentage);
    
    // Test Result.
    id = database_jobs.getNewId ();
    string result = database_jobs.getResult (id);
    evaluate (__LINE__, __func__, "", result);
    database_jobs.setResult (id, "result");
    result = database_jobs.getResult (id);
    evaluate (__LINE__, __func__, "result", result);
  }
}


void test_database_kjv ()
{
  trace_unit_tests (__func__);
  
  Database_Kjv database_kjv = Database_Kjv ();

  {
    vector <Database_Kjv_Item> data = database_kjv.getVerse (43, 11, 35);
    evaluate (__LINE__, __func__, 6, (int)data.size());
    
    evaluate (__LINE__, __func__, "G3588", data[0].strong);
    evaluate (__LINE__, __func__, "Jesus", data[0].english);
    
    // There's a slash (/) between "Jesus" and "Jesus", to separate the words, so they are not joined.
    
    evaluate (__LINE__, __func__, "G2424", data[2].strong);
    evaluate (__LINE__, __func__, "Jesus", data[2].english);
    
    evaluate (__LINE__, __func__, "G1145", data[4].strong);
    evaluate (__LINE__, __func__, "wept",  data[4].english);
  }
  
  {
    // Testing space between the end of the canonical text and a note following it.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 1, 5);
    evaluate (__LINE__, __func__, 23, (int)data.size());
    evaluate (__LINE__, __func__, " [And the evening…: Heb. And the evening was, and the morning was etc.]", data[22].english);
  }
  
  {
    // Testing proper parsing of the <q> element in Luke 13.2.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (42, 13, 2);
    evaluate (__LINE__, __func__, 40, (int)data.size());
    evaluate (__LINE__, __func__, "Suppose ye", data[12].english);
  }
  
  {
    // Check parsing of <inscription> in Exodus 28.36.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (2, 28, 36);
    evaluate (__LINE__, __func__, 23, (int)data.size());
    evaluate (__LINE__, __func__, "HOLINESS", data[18].english);
  }
  
  {
    // Check parsing of <divineName> in Genesis 2.4.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 2, 4);
    evaluate (__LINE__, __func__, 25, (int)data.size());
    evaluate (__LINE__, __func__, "Lord", data[15].english);
  }

  {
    vector <Passage> passages = database_kjv.searchStrong ("G909");
    evaluate (__LINE__, __func__, 4, (int)passages.size());
    
    evaluate (__LINE__, __func__, 41,   passages[0].book);
    evaluate (__LINE__, __func__, 7,    passages[0].chapter);
    evaluate (__LINE__, __func__, "4",  passages[0].verse);
    
    evaluate (__LINE__, __func__, 41,   passages[1].book);
    evaluate (__LINE__, __func__, 7,    passages[1].chapter);
    evaluate (__LINE__, __func__, "8",  passages[1].verse);
    
    evaluate (__LINE__, __func__, 58,   passages[2].book);
    evaluate (__LINE__, __func__, 6,    passages[2].chapter);
    evaluate (__LINE__, __func__, "2",  passages[2].verse);
    
    evaluate (__LINE__, __func__, 58,   passages[3].book);
    evaluate (__LINE__, __func__, 9,    passages[3].chapter);
    evaluate (__LINE__, __func__, "10", passages[3].verse);
  }
}


void test_database_oshb ()
{
  trace_unit_tests (__func__);
  
  Database_OsHb database_oshb = Database_OsHb ();

  // Job 3:2.
  vector <string> data = database_oshb.getVerse (18, 3, 2);
  vector <string> standard = {
    "וַיַּ֥עַן"
    ,
    " "
    ,
    "אִיּ֗וֹב"
    ,
    " "
    ,
    "וַיֹּאמַֽר"
    ,
    "׃"
  };
  evaluate (__LINE__, __func__, standard, data);
  
  vector <Passage> passages = database_oshb.searchHebrew ("יָדְע֥וּ");
  evaluate (__LINE__, __func__, 2, (int)passages.size());
  
  evaluate (__LINE__, __func__, 19,   passages[0].book);
  evaluate (__LINE__, __func__, 95,   passages[0].chapter);
  evaluate (__LINE__, __func__, "10", passages[0].verse);

  evaluate (__LINE__, __func__, 30,   passages[1].book);
  evaluate (__LINE__, __func__, 3,    passages[1].chapter);
  evaluate (__LINE__, __func__, "10", passages[1].verse);

  // Job 3:2.
  vector <int> items = database_oshb.rowids (18, 3, 2);
  evaluate (__LINE__, __func__, 6, (int)items.size());
  
  evaluate (__LINE__, __func__, "c/6030 b", database_oshb.lemma (items[0]));
  evaluate (__LINE__, __func__, "347", database_oshb.lemma (items[2]));
}


void test_database_sblgnt ()
{
  trace_unit_tests (__func__);
  
  Database_Sblgnt database_sblgnt = Database_Sblgnt ();

  vector <string> data = database_sblgnt.getVerse (43, 11, 35);
  evaluate (__LINE__, __func__, { "ἐδάκρυσεν", "ὁ", "Ἰησοῦς" }, data);

  vector <Passage> passages = database_sblgnt.searchGreek ("βαπτισμῶν");
  evaluate (__LINE__, __func__, 1,   (int)passages.size());
  evaluate (__LINE__, __func__, 58,  passages[0].book);
  evaluate (__LINE__, __func__, 6,   passages[0].chapter);
  evaluate (__LINE__, __func__, "2", passages[0].verse);
}


void test_database_sprint ()
{
#ifdef HAVE_CLOUD
  trace_unit_tests (__func__);
  
  // Maintenance.
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.optimize ();
  }
  // Store Task / GetTask
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, {}, ids);
    database.storeTask ("phpunit", 2014, 1, "phpunit");
    ids = database.getTasks ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, 1, (int)ids.size ());
    ids = database.getTasks ("phpunit", 2014, 2);
    evaluate (__LINE__, __func__, 0, (int)ids.size ());
    ids = database.getTasks ("phpunit2", 2014, 1);
    evaluate (__LINE__, __func__, 0, (int)ids.size ());
  }
  // GetTitle
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.storeTask ("phpunit", 2014, 1, "xyz");
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    string title = database.getTitle (ids[0]);
    evaluate (__LINE__, __func__, "xyz", title);
  }
  // Complete
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.storeTask ("phpunit", 2014, 1, "phpunit");
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    int id = ids[0];
    int complete = database.getComplete (id);
    evaluate (__LINE__, __func__, 0, complete);
    database.updateComplete (id, 95);
    complete = database.getComplete (id);
    evaluate (__LINE__, __func__, 95, complete);
  }
  // History
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();

    // Expect no history at all for January 2014.
    vector <Database_Sprint_Item> history = database.getHistory ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, 0, (int)history.size());

    // Log values for January 2014, and check that the database returns those values.
    database.logHistory ("phpunit", 2014, 1, 10, 15, 50);
    history = database.getHistory ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, 10, history[0].day);
    evaluate (__LINE__, __func__, 15, history[0].tasks);
    evaluate (__LINE__, __func__, 50, history[0].complete);

    // Log values for February 2014, and don't expect them when requesting the history for January ...
    database.logHistory ("phpunit", 2014, 2, 10, 15, 51);
    history = database.getHistory ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, 10, history[0].day);
    evaluate (__LINE__, __func__, 15, history[0].tasks);
    evaluate (__LINE__, __func__, 50, history[0].complete);

    // ... but get those values when requesting history for February.
    history = database.getHistory ("phpunit", 2014, 2);
    evaluate (__LINE__, __func__, 10, history[0].day);
    evaluate (__LINE__, __func__, 15, history[0].tasks);
    evaluate (__LINE__, __func__, 51, history[0].complete);

    // Log another history entry for January 2014, and expect two correct entries for this month.
    database.logHistory ("phpunit", 2014, 1, 11, 16, 55);
    history = database.getHistory ("phpunit", 2014, 1);
    evaluate (__LINE__, __func__, 10, history[0].day);
    evaluate (__LINE__, __func__, 15, history[0].tasks);
    evaluate (__LINE__, __func__, 50, history[0].complete);
    evaluate (__LINE__, __func__, 11, history[1].day);
    evaluate (__LINE__, __func__, 16, history[1].tasks);
    evaluate (__LINE__, __func__, 55, history[1].complete);
  }
#endif
}


void test_database_mail ()
{
  trace_unit_tests (__func__);
  
  // Optimize / Trim.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    database_mail.optimize ();
    database_mail.trim ();
  }
  // Empty.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    request.session_logic ()->setUsername ("phpunit");

    int count = database_mail.getMailCount ();
    evaluate (__LINE__, __func__, 0, count);
    
    vector <Database_Mail_User> mails = database_mail.getMails ();
    evaluate (__LINE__, __func__, 0, (int)mails.size());
    
    vector <int> mails_to_send = database_mail.getMailsToSend ();
    evaluate (__LINE__, __func__, {}, mails_to_send);
  }
  // Normal Cycle
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    request.session_logic ()->setUsername ("phpunit");

    database_mail.send ("phpunit", "subject", "body");

    int count = database_mail.getMailCount ();
    evaluate (__LINE__, __func__, 1, count);

    vector <Database_Mail_User> mails = database_mail.getMails ();
    evaluate (__LINE__, __func__, "subject", mails [0].subject);
    
    Database_Mail_Item mail = database_mail.get (1);
    evaluate (__LINE__, __func__, "phpunit", mail.username);
    evaluate (__LINE__, __func__, "body", mail.body);

    database_mail.erase (1);

    count = database_mail.getMailCount ();
    evaluate (__LINE__, __func__, 0, count);
  }
  // Normal Postpone
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request = Webserver_Request ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();
    request.session_logic ()->setUsername ("phpunit");

    database_mail.send ("phpunit", "subject", "body");

    vector <int> mails = database_mail.getMailsToSend ();
    evaluate (__LINE__, __func__, 1, (int)mails.size ());
    
    database_mail.postpone (1);
    mails = database_mail.getMailsToSend ();
    evaluate (__LINE__, __func__, 0, (int)mails.size ());
  }
}


void test_database_navigation ()
{
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    Database_Navigation database = Database_Navigation ();
    database.create ();
    database.trim ();
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();

    // Use current time.
    int time = filter_date_seconds_since_epoch ();

    // Record one entry. As a result there should be no previous entry.
    database.record (time, "phpunit", 1, 2, 3);
    bool previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);

    // Record another entry, with the same time.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);

    // Record another entry 4 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 4;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);

    // Record another entry 5 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 5;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);

    // Record another entry 6 seconds later.
    // This should not remove the already existing entry.
    // As a result there should be a previous entry.
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, true, previous);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record one entry, and another 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Get previous entry, which should be the first one entered.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record one entry, and another 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Get previous entry for another user: It should not be there.
    Passage passage = database.getPrevious ("phpunit2");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record three entries, each one 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    time += 6;
    database.record (time, "phpunit", 7, 8, 9);
    // Get previous entry, which should be the second one entered.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record five entries, each one 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    time += 6;
    database.record (time, "phpunit", 7, 8, 9);
    time += 6;
    database.record (time, "phpunit", 10, 11, 12);
    time += 6;
    database.record (time, "phpunit", 13, 14, 15);
    // Get previous entry, which should be the last but one passage recorded.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 10, passage.book);
    evaluate (__LINE__, __func__, 11, passage.chapter);
    evaluate (__LINE__, __func__, "12", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // There should be no next passage.
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record several entries, all spaced apart by 6 seconds.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Record two entries at an interval.
    int time = filter_date_seconds_since_epoch ();
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Next entry is not there.
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
    // Previous entry should be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
    // Previous entry should be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
    // Previous entry before previous entry should not be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
    // The entry next to the next entry should not be there.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
}


void test_database_resources ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  vector <string> names = resource_external_names ();
  bool hit = false;
  for (auto & name : names) if (name == "Statenbijbel GBS") hit = true;
  evaluate (__LINE__, __func__, true, hit);
}


void test_database_usfmresources ()
{
  trace_unit_tests (__func__);
  
  // Empty
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    evaluate (__LINE__, __func__, false, hit);
  }
  // Names
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "");
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    evaluate (__LINE__, __func__, true, hit);
  }
  // Store Get Chapter
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    string usfm = database_usfmresources.getUsfm ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, "usfm", usfm);
    usfm = database_usfmresources.getUsfm ("bibledit", 2, 4);
    evaluate (__LINE__, __func__, "", usfm);
  }
  // Books
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    database_usfmresources.storeChapter ("bibledit", 3, 4, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 1, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 2, "usfm");
    vector <int> books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {1, 2, 3}, books);
  }
  // Chapters
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    database_usfmresources.storeChapter ("bibledit", 3, 4, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 1, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 2, "usfm");
    vector <int> chapters = database_usfmresources.getChapters ("bibledit", 1);
    evaluate (__LINE__, __func__, {1, 2}, chapters);
    chapters = database_usfmresources.getChapters ("bibledit", 2);
    evaluate (__LINE__, __func__, {3}, chapters);
  }
  // Sizes
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();

    int size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 0, size);

    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 4, size);
    
    database_usfmresources.storeChapter ("bibledit", 2, 3, "chapter");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 7, size);
  }
  // Delete Book
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    vector <int> books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {2}, books);
    database_usfmresources.deleteBook ("bibledit", 2);
    books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {}, books);
  }
}


void test_database_mappings ()
{
  trace_unit_tests (__func__);
  
  // Setup
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.defaults ();
    database_mappings.create2 ();
    database_mappings.optimize ();
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Dutch Traditional", english (), "French Louise Segond", "Hebrew Greek", "Russian Canonical", "Russian Orthodox", "Russian Protestant", "Spanish", "Vulgate"}, names);
  }
  // Import Export
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = 
      "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("phpunit", import);
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Hebrew Greek", "phpunit"}, names);
    string output = database_mappings.output ("phpunit");
    evaluate (__LINE__, __func__, filter_string_trim (import), filter_string_trim (output));
  }
  // Create
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.create ("phpunit");
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Hebrew Greek", "phpunit"}, names);
  }
  // Translate Same
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    vector <Passage> passages = database_mappings.translate ("ABC", "ABC", 14, 14, 15);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "15");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    passages = database_mappings.translate ("--X", "--X", 15, 16, 17);
    standard = Passage ("", 15, 16, "17");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = 
      "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
      "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "15");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
      "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
      "2 Chronicles 14:13 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate Double Result.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
      "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
      "2 Chronicles 14:12 = 2 Chronicles 14:14\n"
      "2 Chronicles 14:13 = 2 Chronicles 14:14\n"
      "Nehemiah 4:1 = Nehemiah 3:33\n"
      "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }
  // Translate From Original
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate From Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
      "2 Chronicles 14:12 = 2 Chronicles 14:14\n"
      "2 Chronicles 14:13 = 2 Chronicles 14:14\n";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }
  // Translate From Original No Mapping
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 15, 14);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 15, "14");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate To Original
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("ABA", import);
    vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "14");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  // Translate To Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
      "2 Chronicles 14:12 = 2 Chronicles 14:13\n"
      "2 Chronicles 14:12 = 2 Chronicles 14:14\n";
    database_mappings.import ("ABA", import);
    vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "14");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }

}


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
    database.record ("phpunit", 2, 3, "content");
    vector <int> notes = database.getNotes ();
    evaluate (__LINE__, __func__, {2}, notes);
  }
  // Get Notes
  {
    refresh_sandbox (true);
    Database_NoteActions database = Database_NoteActions ();
    database.create ();
    database.record ("phpunit", 2, 3, "content");
    database.record ("phpunit", 2, 4, "content");
    database.record ("phpunit", 3, 3, "content");
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
    database.record ("phpunit", 2, 3, "content");
    database.record ("phpunit", 2, 4, "content");
    database.record ("phpunit", 3, 3, "content");
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


void test_database_versifications ()
{
  trace_unit_tests (__func__);
  
  // Basic operations, create, delete.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.optimize ();
    int id = database_versifications.createSystem ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    id = database_versifications.getID ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    vector <string> systems = database_versifications.getSystems ();
    evaluate (__LINE__, __func__, {"phpunit"}, systems);
    database_versifications.erase ("phpunit");
    systems = database_versifications.getSystems ();
    evaluate (__LINE__, __func__, {}, systems);
  }
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();

    // GetID
    int id = database_versifications.getID (english ());
    evaluate (__LINE__, __func__, 4 , id);

    // Test books.
    vector <int> books = database_versifications.getBooks (english ());
    vector <int> standard;
    for (unsigned int i = 1; i <= 66; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, books);

    // Test chapters.
    vector <int> chapters = database_versifications.getChapters (english (), 1);
    standard.clear ();
    for (unsigned int i = 1; i <= 50; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);
    chapters = database_versifications.getChapters (english (), 1, true);
    standard.clear ();
    for (unsigned int i = 0; i <= 50; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);

    // Test verses.
    vector <int> verses = database_versifications.getVerses (english (), 1, 2);
    standard.clear ();
    for (unsigned int i = 0; i <= 25; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, verses);

    // Verses in chapter 0.
    verses = database_versifications.getVerses (english (), 1, 0);
    evaluate (__LINE__, __func__, {0}, verses);

    // Books Chapters Verses.
    vector <Passage> data = database_versifications.getBooksChaptersVerses (english ());
    evaluate (__LINE__, __func__, 1189, (int)data.size());
    evaluate (__LINE__, __func__, "31", data [0].verse);
    
    // Maximum number of books.
    books = database_versifications.getMaximumBooks ();
    standard = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 70, 71, 73, 74, 75, 80, 81 };
    evaluate (__LINE__, __func__, standard, books);
    
    // Maximum number of chapters.
    chapters = database_versifications.getMaximumChapters (5);
    standard.clear ();
    for (unsigned int i = 0; i <= 34; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);

    // Maximum number of verses.
    verses = database_versifications.getMaximumVerses (1, 2);
    standard.clear ();
    for (unsigned int i = 0; i <= 25; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, verses);
  }
  // Import Export
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    string input = 
      "Genesis 1:31\n"
      "Genesis 2:25\n";
    database_versifications.input (input, "phpunit");
    int id = database_versifications.getID ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    vector <Passage> data = database_versifications.getBooksChaptersVerses ("phpunit");
    evaluate (__LINE__, __func__, 2, (int)data.size ());
    evaluate (__LINE__, __func__, "25", data [1].verse);
    string output = database_versifications.output ("phpunit");
    evaluate (__LINE__, __func__, filter_string_trim (input), filter_string_trim (output));
  }
}


void test_database_modifications_user ()
{
  trace_unit_tests (__func__);
  
  // Create, erase, clear.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.erase ();
    database_modifications.create ();
    database_modifications.clearUserUser ("phpunit");
  }
  // Usernames
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <string> users = database_modifications.getUserUsernames ();
    evaluate (__LINE__, __func__, {}, users);
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 4, "new");
    users = database_modifications.getUserUsernames ();
    evaluate (__LINE__, __func__, {"phpunit1"}, users);
    database_modifications.recordUserSave ("phpunit2", "bible", 1, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit3", "bible", 1, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 7, "new");
    database_modifications.recordUserSave ("phpunit2", "bible", 1, 2, 3, "old", 8, "new");
    database_modifications.recordUserSave ("phpunit3", "bible", 1, 2, 3, "old", 9, "new");
    users = database_modifications.getUserUsernames ();
    evaluate (__LINE__, __func__, {"phpunit1", "phpunit2", "phpunit3"}, users);
  }
  // Bibles
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <string> bibles = database_modifications.getUserBibles ("phpunit1");
    evaluate (__LINE__, __func__, {}, bibles);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 7, "new");
    bibles = database_modifications.getUserBibles ("phpunit1");
    evaluate (__LINE__, __func__, {"bible1"}, bibles);
  }
  // Books
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <int> books = database_modifications.getUserBooks ("phpunit1", "bible1");
    evaluate (__LINE__, __func__, {}, books);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 2, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 3, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible2", 4, 2, 3, "old", 7, "new");
    books = database_modifications.getUserBooks ("phpunit1", "bible1");
    evaluate (__LINE__, __func__, {1, 2, 3}, books);
  }
  // Chapters
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <int> chapters = database_modifications.getUserChapters ("phpunit1", "bible1", 1);
    evaluate (__LINE__, __func__, {}, chapters);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 3, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 4, 3, "old", 6, "new");
    chapters = database_modifications.getUserChapters ("phpunit1", "bible1", 1);
    evaluate (__LINE__, __func__, {2, 3, 4}, chapters);
  }
  // Identifiers.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 4, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 5, "old", 6, "new");
    vector <Database_Modifications_Id> identifiers = database_modifications.getUserIdentifiers ("phpunit1", "bible", 1, 2);
    evaluate (__LINE__, __func__, 3, (int)identifiers.size());
    evaluate (__LINE__, __func__, 3, identifiers[0].oldid);
    evaluate (__LINE__, __func__, 4, identifiers[0].newid);
    evaluate (__LINE__, __func__, 4, identifiers[1].oldid);
    evaluate (__LINE__, __func__, 5, identifiers[1].newid);
    evaluate (__LINE__, __func__, 5, identifiers[2].oldid);
    evaluate (__LINE__, __func__, 6, identifiers[2].newid);
  }
  // Chapter
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old1", 4, "new1");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 4, "old2", 5, "new2");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 5, "old3", 6, "new3");
    Database_Modifications_Text chapter = database_modifications.getUserChapter ("phpunit1", "bible", 1, 2, 4);
    evaluate (__LINE__, __func__, "old1", chapter.oldtext);
    evaluate (__LINE__, __func__, "new1", chapter.newtext);
    chapter = database_modifications.getUserChapter ("phpunit1", "bible", 1, 2, 5);
    evaluate (__LINE__, __func__, "old2", chapter.oldtext);
    evaluate (__LINE__, __func__, "new2", chapter.newtext);
  }
  // Timestamp
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old1", 4, "new1");
    int time = database_modifications.getUserTimestamp ("phpunit1", "bible", 1, 2, 4);
    int currenttime = filter_date_seconds_since_epoch ();
    if ((time < currenttime - 1) || (time > currenttime + 1)) evaluate (__LINE__, __func__, currenttime, time);
  }
}


void test_database_modifications_team ()
{
  trace_unit_tests (__func__);
  
  // Basics.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");
  }
  // Team Existence
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");
 
    // Initially no diff should exist.
    bool exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, false, exists);

    // After storing a chapter there should be a diff.
    bible_logic_store_chapter ("phpunit", 1, 2, "chapter text one");
    exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, true, exists);

    // After storing a chapter for the second time, the diff should still exist.
    bible_logic_store_chapter ("none", 1, 2, "chapter text two");
    exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, true, exists);
    
    // The diff data should not exist for another chapter.
    exists = database_modifications.teamDiffExists ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, false, exists);
  }
  // Team Book
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 2, 1));
    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 2, 2));
    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 2, 3));

    bible_logic_store_chapter ("phpunit", 2, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 2, 2, "chapter text");
    bible_logic_store_chapter ("phpunit", 2, 3, "chapter text");

    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 2));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 3));

    database_modifications.truncateTeams ();

    // Test function storeTeamDiffBook.
    database_modifications.storeTeamDiffBook ("phpunit", 2);
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 2));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 2, 3));
  }
  // Team Bible
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 3, 1));
    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 4, 1));
    evaluate (__LINE__, __func__, false, database_modifications.teamDiffExists ("phpunit", 5, 1));

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 4, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 5, 1, "chapter text");

    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 3, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 4, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 5, 1));

    database_modifications.truncateTeams ();

    // Test function storeTeamDiffBible. It stores diff data for the whole bible.
    database_modifications.storeTeamDiffBible ("phpunit");

    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 3, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 4, 1));
    evaluate (__LINE__, __func__, true, database_modifications.teamDiffExists ("phpunit", 5, 1));
  }
  // Team Get Diff
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    string diff = database_modifications.getTeamDiff ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, "", diff);

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    database_modifications.truncateTeams ();
    bible_logic_store_chapter ("phpunit", 3, 1, "longer chapter text");
    diff = database_modifications.getTeamDiff ("phpunit", 3, 1);
    evaluate (__LINE__, __func__, "chapter text", diff);

    diff = database_modifications.getTeamDiff ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, "", diff);

    bible_logic_store_chapter ("phpunit", 5, 5, "chapter text");
    database_modifications.truncateTeams ();
    bible_logic_store_chapter ("phpunit", 5, 5, "longer chapter text");
    diff = database_modifications.getTeamDiff ("phpunit", 5, 5);
    evaluate (__LINE__, __func__, "chapter text", diff);
  }
  // Team Get Diff Chapters
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 5, "chapter text");

    vector <int> chapters = database_modifications.getTeamDiffChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, {}, chapters);

    chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    evaluate (__LINE__, __func__, {1, 3, 5}, chapters);

    chapters = database_modifications.getTeamDiffChapters ("phpunit2", 3);
    evaluate (__LINE__, __func__, {}, chapters);
  }
  // Team Diff Bible
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 5, "chapter text");
    
    vector <string> bibles = database_modifications.getTeamDiffBibles ();
    evaluate (__LINE__, __func__, {"phpunit"}, bibles);
    
    database_modifications.deleteTeamDiffBible ("phpunit2");

    bibles = database_modifications.getTeamDiffBibles ();
    evaluate (__LINE__, __func__, {"phpunit"}, bibles);

    database_modifications.deleteTeamDiffBible ("phpunit");

    bibles = database_modifications.getTeamDiffBibles ();
    evaluate (__LINE__, __func__, {}, bibles);
  }
  // Team Diff Chapter.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 5, "chapter text");
    
    vector <int> chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    evaluate (__LINE__, __func__, {1, 3, 5}, chapters);

    database_modifications.deleteTeamDiffChapter ("phpunit", 3, 1);

    chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    evaluate (__LINE__, __func__, {3, 5}, chapters);
  }
  // Team Diff Book
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic_store_chapter ("phpunit", 4, 5, "chapter text");

    vector <int> books = database_modifications.getTeamDiffBooks ("phpunit");
    evaluate (__LINE__, __func__, {3, 4}, books);

    books = database_modifications.getTeamDiffBooks ("phpunit2");
    evaluate (__LINE__, __func__, {}, books);
  }
  // Get Team Diff Count
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    database_bibles.createBible ("phpunit2");

    bible_logic_store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic_store_chapter ("phpunit2", 3, 3, "chapter text");
    bible_logic_store_chapter ("phpunit2", 3, 5, "chapter text");

    int count = database_modifications.getTeamDiffCount ("phpunit");
    evaluate (__LINE__, __func__, 1, count);

    count = database_modifications.getTeamDiffCount ("phpunit2");
    evaluate (__LINE__, __func__, 2, count);

    count = database_modifications.getTeamDiffCount ("phpunit3");
    evaluate (__LINE__, __func__, 0, count);
  }
}


void test_database_modifications_notifications ()
{
  trace_unit_tests (__func__);
  
  // Basics.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    for (auto id : ids) {
      database_modifications.deleteNotification (id);
    }
  }
  // Trim.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();

    // Record two entries.
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {1, 2}, ids);
    
    // After trimming the two entries should still be there.
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {1, 2}, ids);

    // Set the time back, re-index, filter_string_trim, and check one entry's gone.
    database_modifications.indexTrimAllNotifications ();
    database_modifications.notificationUpdateTime (1, filter_date_seconds_since_epoch () - 7776001);
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {2}, ids);
  }
  // Next Identifier.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    int identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    evaluate (__LINE__, __func__, 1, identifier);
    database_modifications.recordNotification ({"phpunit1"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    evaluate (__LINE__, __func__, 2, identifier);
    database_modifications.deleteNotification (1);
    identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    evaluate (__LINE__, __func__, 1, identifier);
  }
  // Record Details Retrieval.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();

    // Start with no identifiers.
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {}, ids);

    // Record three notifications and reindex.
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit2", "phpunit3"}, "A", "1", 4, 5, 6, "old2", "mod2", "new2");
    database_modifications.recordNotification ({"phpunit3", "phpunit4"}, changes_bible_category (), "1", 7, 8, 9, "old3", "mod3", "new3");
    database_modifications.indexTrimAllNotifications ();

    // There should be six notifications now: Two users per recordNotification call.
    ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {1, 2, 3, 4, 5, 6}, ids);
    
    // Test notifications per user.
    ids = database_modifications.getNotificationIdentifiers ("phpunit1");
    evaluate (__LINE__, __func__, {1}, ids);
    ids = database_modifications.getNotificationIdentifiers ("phpunit3");
    evaluate (__LINE__, __func__, {4, 5}, ids);
  }
  // Timestamps
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();

    int timestamp = database_modifications.getNotificationTimeStamp (0);
    int currenttime = filter_date_seconds_since_epoch ();
    if ((timestamp < currenttime) || (timestamp > currenttime + 1)) evaluate (__LINE__, __func__, currenttime, timestamp);

    int time = filter_date_seconds_since_epoch () - 21600;
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    timestamp = database_modifications.getNotificationTimeStamp (1);
    if ((timestamp < time) || (timestamp > time + 1)) evaluate (__LINE__, __func__, time, timestamp);
  }
  // Category
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string category = database_modifications.getNotificationCategory (1);
    evaluate (__LINE__, __func__, "A", category);
    category = database_modifications.getNotificationCategory (2);
    evaluate (__LINE__, __func__, "", category);
  }
  // Bible.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string bible = database_modifications.getNotificationBible (1);
    evaluate (__LINE__, __func__, "1", bible);
    bible = database_modifications.getNotificationBible (3);
    evaluate (__LINE__, __func__, "", bible);
  }
  // Passage
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    Passage passage = database_modifications.getNotificationPassage (1);
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
    passage = database_modifications.getNotificationPassage (3);
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  // OldText
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string old = database_modifications.getNotificationOldText (1);
    evaluate (__LINE__, __func__, "old1", old);
    old = database_modifications.getNotificationOldText (3);
    evaluate (__LINE__, __func__, "", old);
  }
  // Modification.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string modification = database_modifications.getNotificationModification (1);
    evaluate (__LINE__, __func__, "mod1", modification);
    modification = database_modifications.getNotificationOldText (3);
    evaluate (__LINE__, __func__, "", modification);
  }
  // New Text
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string newtext = database_modifications.getNotificationNewText (1);
    evaluate (__LINE__, __func__, "new1", newtext);
    newtext = database_modifications.getNotificationNewText (3);
    evaluate (__LINE__, __func__, "", newtext);
  }
  // Clear User
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit1", "phpunit2", "phpunit3"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, 3, (int)ids.size ());

    database_modifications.clearNotificationsUser ("phpunit2");

    ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, 2, (int)ids.size ());

    ids = database_modifications.getNotificationIdentifiers ("phpunit2");
    evaluate (__LINE__, __func__, 0, (int)ids.size ());
  }
  // Clear Matches One
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, changes_personal_category (), "1", 2, 3, 4, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit"}, "T", "1", 2, 3, 4, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, 2, (int)ids.size ());
    database_modifications.clearNotificationMatches ("phpunit", changes_personal_category (), "T");
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, 0, (int)ids.size ());
  }
  // Notification Personal Identifiers
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit2", "phpunit1"}, changes_bible_category (), "1", 1, 2, 3, "old2", "mod2", "new2");
    database_modifications.recordNotification ({"phpunit3", "phpunit4"}, changes_bible_category (), "1", 7, 8, 9, "old3", "mod3", "new3");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationPersonalIdentifiers ("phpunit1", "A");
    evaluate (__LINE__, __func__, {1, 4}, ids);
  }
  // Notification Team Identifiers
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit2", "phpunit1"}, changes_bible_category (), "1", 1, 2, 3, "old2", "mod2", "new2");
    database_modifications.recordNotification ({"phpunit3", "phpunit4"}, changes_bible_category (), "1", 7, 8, 9, "old3", "mod3", "new3");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", "A");
    evaluate (__LINE__, __func__, {1}, ids);
    ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", changes_bible_category ());
    evaluate (__LINE__, __func__, {4}, ids);
  }
  // Record on client.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.storeClientNotification (3, "phpunit", "A", "bible", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.storeClientNotification (5, "phpunit", "A", "bible", 1, 2, 3, "old1", "mod1", "new1");
    vector <int> ids = database_modifications.getNotificationIdentifiers ();
    evaluate (__LINE__, __func__, {3, 5}, ids);
  }
}


void test_database_notes ()
{
  trace_unit_tests (__func__);
  
  // DatabasePath
  {
    refresh_sandbox (true);
    Webserver_Request request;
    Database_Notes database_notes (&request);
    string path = database_notes.database_path ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes.sqlite"), path);
    path = database_notes.checksums_database_path ();
    evaluate (__LINE__, __func__, filter_url_create_root_path ("databases", "notes_checksums.sqlite"), path);
  }
  // TrimOptimize
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
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "", "", false);
    database_notes.erase (identifier);
    database_notes.trim ();
    database_notes.trim_server ();
    // Since the logbook will have an entry about "Deleting empty notes folder",
    // erase that entry here.
    refresh_sandbox (false);
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

    identifier = database_notes.getNewUniqueIdentifier ();
    if ((identifier < 100000000) || (identifier > 999999999)) evaluate (__LINE__, __func__, "Out of bounds", convert_to_string (identifier));
    evaluate (__LINE__, __func__, false, database_notes.identifierExists (identifier));

    identifier = database_notes.storeNewNote ("", 0, 0, 0, "", "", false);
    evaluate (__LINE__, __func__, true, database_notes.identifierExists (identifier));
    database_notes.erase (identifier);
    evaluate (__LINE__, __func__, false, database_notes.identifierExists (identifier));
  }
  // SummaryContents
  {
    refresh_sandbox (true);
    Database_State::create ();
    Database_Login::create ();
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
    Database_Notes database_notes (&request);
    database_notes.create ();
    // Test inserting data for both summary and contents.
    string summary = "Summary";
    string contents = "Contents";
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, summary, contents, false);
    string value = database_notes.getSummary (identifier);
    evaluate (__LINE__, __func__, summary, value);
    value = database_notes.getContents (identifier);
    vector <string> values = filter_string_explode (value, '\n');
    if (values.size () > 2) value = values[2];
    evaluate (__LINE__, __func__, "<p>Contents</p>", value);
    // Test that if the summary is not given, it is going to be the first line of the contents.
    contents = "This is a note.\nLine two.";
    identifier = database_notes.storeNewNote ("", 0, 0, 0, "", contents, false);
    value = database_notes.getSummary (identifier);
    evaluate (__LINE__, __func__, "This is a note.", value);
    value = database_notes.getContents (identifier);
    values = filter_string_explode (value, '\n');
    if (values.size () > 3) value = values[3];
    evaluate (__LINE__, __func__, "<p>Line two.</p>", value);
    // Test setSummary function.
    database_notes.setSummary (identifier, "summary1");
    value = database_notes.getSummary (identifier);
    evaluate (__LINE__, __func__, "summary1", value);
    // Test setContents function.
    database_notes.setContents (identifier, "contents1");
    value = database_notes.getContents (identifier);
    evaluate (__LINE__, __func__, "contents1", value);
    // Test adding comment.
    value = database_notes.getContents (identifier);
    int length = value.length ();
    database_notes.addComment (identifier, "comment1");
    value = database_notes.getContents (identifier);
    if (value.length () < (size_t) (length + 30)) evaluate (__LINE__, __func__, "Should be larger than length + 30", convert_to_string ((int)value.length()));
    size_t pos = value.find ("comment1");
    if (pos == string::npos) evaluate (__LINE__, __func__, "Should contain 'comment1'", value);
  }
  // Subscriptions.
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
    // But since this PHPUnit test runs without sessions, it would have subscribed an empty user.
    request.session_logic()->setUsername ("");
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> subscribers = database_notes.getSubscribers (identifier);
    evaluate (__LINE__, __func__, {}, subscribers);

    // Create a note again, but this time set the session variable to a certain user.
    database_users.add_user ("phpunit", "", 5, "");
    request.session_logic()->setUsername ("phpunit");
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (true);
    identifier = database_notes.storeNewNote ("", 1, 1, 1, "Summary", "Contents", false);
    notes_logic.handlerNewNote (identifier);
    subscribers = database_notes.getSubscribers (identifier);
    evaluate (__LINE__, __func__, {"phpunit"}, subscribers);
    evaluate (__LINE__, __func__, true, database_notes.isSubscribed (identifier, "phpunit"));
    request.database_config_user()->setSubscribeToConsultationNotesEditedByMe (false);
    // Test various other subscription related functions.
    evaluate (__LINE__, __func__, false, database_notes.isSubscribed (identifier, "phpunit_phpunit"));
    database_notes.unsubscribe (identifier);
    evaluate (__LINE__, __func__, false, database_notes.isSubscribed (identifier, "phpunit"));
    database_notes.subscribeUser (identifier, "phpunit_phpunit_phpunit");
    evaluate (__LINE__, __func__, true, database_notes.isSubscribed (identifier, "phpunit_phpunit_phpunit"));
    database_notes.unsubscribeUser (identifier, "phpunit_phpunit_phpunit");
    evaluate (__LINE__, __func__, false, database_notes.isSubscribed (identifier, "phpunit_phpunit_phpunit"));
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
    database_notes.create ();
    Database_Mail database_mail = Database_Mail (&request);
    database_mail.create ();

    request.session_logic()->setUsername ("PHPUnit2");

    // Create a note and check that it was not assigned to anybody.
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "Summary", "Contents", false);
    vector <string> assignees = database_notes.getAssignees (identifier);
    evaluate (__LINE__, __func__, {}, assignees);

    // Assign the note to a user, and check that this reflects in the list of assignees.
    database_notes.assignUser (identifier, "PHPUnit");
    assignees = database_notes.getAssignees (identifier);
    evaluate (__LINE__, __func__, {"PHPUnit"}, assignees);

    // Test the setAssignees function.
    database_notes.setAssignees (identifier, {"PHPUnit"});
    assignees = database_notes.getAssignees (identifier);
    evaluate (__LINE__, __func__, {"PHPUnit"}, assignees);
        
    // Assign note to second user, and check it reflects.
    database_notes.assignUser (identifier, "PHPUnit2");
    assignees = database_notes.getAssignees (identifier);
    evaluate (__LINE__, __func__, {"PHPUnit", "PHPUnit2"}, assignees);

    // Based on the above, check the isAssigned function.
    evaluate (__LINE__, __func__, true, database_notes.isAssigned (identifier, "PHPUnit"));
    evaluate (__LINE__, __func__, true, database_notes.isAssigned (identifier, "PHPUnit2"));
    evaluate (__LINE__, __func__, false, database_notes.isAssigned (identifier, "PHPUnit3"));
    
    // Based on the above, test getAllAssignees().
    assignees = database_notes.getAllAssignees ({""});
    evaluate (__LINE__, __func__, {"PHPUnit", "PHPUnit2"}, assignees);
    
    // Based on the above, test the unassignUser function.
    database_notes.unassignUser (identifier, "PHPUnit");
    assignees = database_notes.getAssignees (identifier);
    evaluate (__LINE__, __func__, {"PHPUnit2"}, assignees);
    database_notes.unassignUser (identifier, "PHPUnit2");
    assignees = database_notes.getAssignees (identifier);
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
    database_notes.create ();

    request.session_logic()->setUsername ("PHPUnit");
    int identifier = database_notes.storeNewNote ("PHPUnit", 0, 0, 0, "Summary", "Contents", false);
    string bible = database_notes.getBible (identifier);
    evaluate (__LINE__, __func__, "PHPUnit", bible);
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
    database_notes.create ();

    request.session_logic()->setUsername ("PHPUnit");

    // Create note for a certain passage.
    int identifier = database_notes.storeNewNote ("", 10, 9, 8, "Summary", "Contents", false);
    
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
    database_notes.create ();

    request.session_logic()->setUsername ("PHPUnit");

    // Create note.
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "Summary", "Contents", false);

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
    database_notes.create ();

    request.session_logic()->setUsername ("PHPUnit");

    // Create note.
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "Summary", "Contents", false);

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
    database_notes.create ();

    request.session_logic()->setUsername ("PHPUnit");
    int time = filter_date_seconds_since_epoch ();

    // Create note.
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "Summary", "Contents", false);
    
    // Test getModified.
    int value = database_notes.getModified (identifier);
    if ((value < time) || (value > time + 1)) evaluate (__LINE__, __func__, time, value);
    // Test setModified.
    time = 123456789;
    database_notes.setModified (identifier, time);
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
    database_notes.create ();

    request.session_logic()->setUsername ("phpunit");
    
    // Create a few notes.
    vector <int> standardids;
    for (unsigned int i = 0; i < 3; i++) {
      int identifier = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
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
    database_notes.create ();

    // Create note.
    request.session_logic()->setUsername ("phpunit");
    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
    
    // Contents of the note.
    string originalContents = database_notes.getContents (identifier);
    if (originalContents.length () <= 20) evaluate (__LINE__, __func__, "Should be greater than 20", convert_to_string ((int)originalContents.length ()));
    
    // Checksum of the note.
    string originalChecksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, 32, (int)originalChecksum.length());
    
    // Change the identifier.
    int newId = 1234567;
    database_notes.setIdentifier (identifier, newId);
    
    // Check old and new identifier.
    string contents = database_notes.getContents (identifier);
    evaluate (__LINE__, __func__, "", contents);
    contents = database_notes.getContents (newId);
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
    database_notes.create ();

    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
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
    database_notes.create ();

    int identifier = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
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
    database_notes.create ();

    int identifier1 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
    int identifier3 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
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
    database_notes.create ();

    int identifier1 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
    int identifier2 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
    int identifier3 = database_notes.storeNewNote ("", 0, 0, 0, "summary", "contents", false);
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
    database_notes.create ();

    // Create note to work with.
    int identifier = database_notes.storeNewNote ("bible", 1, 2, 3, "summary", "contents", false);

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
    database_notes.setModified (identifier, 1234567);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());

    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setSubscribers (identifier, {"subscribers"});
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());

    database_notes.setChecksum (identifier, "");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setBible (identifier, "phpunit");
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
    database_notes.setSummary (identifier, "new");
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, false, checksum.empty());

    database_notes.deleteChecksum (identifier);
    checksum = database_notes.getChecksum (identifier);
    evaluate (__LINE__, __func__, "", checksum);
    database_notes.setContents (identifier, "new");
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
    database_notes.create ();

    // Create notes to work with.
    vector <int> identifiers;
    identifiers.push_back (database_notes.storeNewNote ("bible1", 1, 2, 3, "summary1", "contents1", false));
    identifiers.push_back (database_notes.storeNewNote ("bible2", 2, 3, 4, "summary2", "contents2", false));
    identifiers.push_back (database_notes.storeNewNote ("bible3", 3, 4, 5, "summary3", "contents3", false));

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
    database_notes.create ();

    // Create a couple of notes to work with.
    int identifier = database_notes.storeNewNote ("bible1", 1, 2, 3, "summary", "contents", false);
    int identifier1 = 100000000;
    database_notes.setIdentifier (identifier, identifier1);

    identifier = database_notes.storeNewNote ("bible2", 1, 2, 3, "summary", "contents", false);
    int identifier2 = 500000000;
    database_notes.setIdentifier (identifier, identifier2);

    identifier = database_notes.storeNewNote ("", 1, 2, 3, "summary", "contents", false);
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
    database_notes.create ();

    // Create a couple of notes to work with.
    int identifier1 = database_notes.storeNewNote ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.storeNewNote ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.storeNewNote ("bible3", 1, 2, 3, "summary3", "contents3", false);

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
  }
  // ResilienceChecksumsNotes.
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
    int identifier1 = database_notes.storeNewNote ("bible1", 1, 2, 3, "summary1", "contents1", false);
    int identifier2 = database_notes.storeNewNote ("bible2", 1, 2, 3, "summary2", "contents2", false);
    int identifier3 = database_notes.storeNewNote ("bible3", 1, 2, 3, "summary3", "contents3", false);
    
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
      int identifier = database_notes.storeNewNote (bible, book, chapter, verse, summary, contents, false);
      database_notes.setContents (identifier, contents);
      // Additional fields for the note.
      string assigned = "assigned" + offset;
      filter_url_file_put_contents (database_notes.assignedFile (identifier), assigned);
      int modified = 2 * i;
      filter_url_file_put_contents (database_notes.modifiedFile (identifier), convert_to_string (modified));
      string passage = "passage" + offset;
      filter_url_file_put_contents (database_notes.passageFile (identifier), passage);
      int severity = 4 * i;
      filter_url_file_put_contents (database_notes.severityFile (identifier), convert_to_string (severity));
      string status = "status" + offset;
      filter_url_file_put_contents (database_notes.statusFile (identifier), status);
      string subscriptions = "subscriptions" + offset;
      filter_url_file_put_contents (database_notes.subscriptionsFile (identifier), subscriptions);
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
      database_notes.erase (identifier);
      evaluate (__LINE__, __func__, "", database_notes.getSummary (identifier));
      evaluate (__LINE__, __func__, "", database_notes.getContents (identifier));
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
      string assigned = filter_url_file_get_contents (database_notes.assignedFile (identifier));
      evaluate (__LINE__, __func__, v_assigned [i], assigned);
      string bible = database_notes.getBible (identifier);
      evaluate (__LINE__, __func__, v_bible [i], bible);
      string contents = database_notes.getContents (identifier);
      evaluate (__LINE__, __func__, v_contents [i], contents);
      int modified = database_notes.getModified (identifier);
      evaluate (__LINE__, __func__, v_modified [i], modified);
      string passage = database_notes.getRawPassage (identifier);
      evaluate (__LINE__, __func__, v_passage [i], passage);
      int severity = database_notes.getRawSeverity (identifier);
      evaluate (__LINE__, __func__, v_severity [i], severity);
      string status = database_notes.getRawStatus (identifier);
      evaluate (__LINE__, __func__, v_status [i], status);
      string subscriptions = filter_url_file_get_contents (database_notes.subscriptionsFile (identifier));
      evaluate (__LINE__, __func__, v_subscriptions [i], subscriptions);
      string summary = database_notes.getSummary (identifier);
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


void test_database_volatile ()
{
  trace_unit_tests (__func__);
  
  // No value initially.
  string value = Database_Volatile::getValue (1, "key");
  evaluate (__LINE__, __func__, "", value);
  
  // Store value and retrieve it.
  Database_Volatile::setValue (2, "key2", "value2");
  value = Database_Volatile::getValue (2, "key2");
  evaluate (__LINE__, __func__, "value2", value);
  
  // Another key should retrieve nothing.
  value = Database_Volatile::getValue (2, "key1");
  evaluate (__LINE__, __func__, "", value);
}


void test_database_state ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_State::create ();
  // Test notes checksums.
  {
    // No checksum yet.
    evaluate (__LINE__, __func__, "",  Database_State::getNotesChecksum (100, 1000));
    
    // Store and retrieve checksum in a defined range.
    Database_State::putNotesChecksum (100, 1000, "phpunit");
    evaluate (__LINE__, __func__, "phpunit",  Database_State::getNotesChecksum (100, 1000));
    // Store it again, with a different value.
    Database_State::putNotesChecksum (100, 1000, "phpunit2");
    evaluate (__LINE__, __func__, "phpunit2",  Database_State::getNotesChecksum (100, 1000));
    
    // Erase a note within the defined range, which should erase that range.
    Database_State::eraseNoteChecksum (100);
    evaluate (__LINE__, __func__, "",  Database_State::getNotesChecksum (100, 1000));
    
    // Define a few ranges, store checksums, and erase one note within that range, and test it.
    Database_State::putNotesChecksum (100, 1000, "100-1000");
    Database_State::putNotesChecksum (200, 1100, "200-1100");
    Database_State::putNotesChecksum (300, 900,  "300-900");
    Database_State::putNotesChecksum (2000, 9000, "2000-9000");
    evaluate (__LINE__, __func__, "100-1000",   Database_State::getNotesChecksum (100,  1000));
    evaluate (__LINE__, __func__, "200-1100",   Database_State::getNotesChecksum (200,  1100));
    evaluate (__LINE__, __func__, "300-900",    Database_State::getNotesChecksum (300,  900));
    evaluate (__LINE__, __func__, "2000-9000",  Database_State::getNotesChecksum (2000, 9000));
    Database_State::eraseNoteChecksum (500);
    evaluate (__LINE__, __func__, "",   Database_State::getNotesChecksum (100,  1000));
    evaluate (__LINE__, __func__, "",   Database_State::getNotesChecksum (200,  1100));
    evaluate (__LINE__, __func__, "",    Database_State::getNotesChecksum (300,  900));
    evaluate (__LINE__, __func__, "2000-9000",  Database_State::getNotesChecksum (2000, 9000));
  }
  // Test export flagging.
  {
    // Flag some data for export.
    Database_State::setExport ("1", 2, 3);
    Database_State::setExport ("4", 5, 6);
    Database_State::setExport ("7", 8, 9);

    // Test the data.
    evaluate (__LINE__, __func__, true,  Database_State::getExport ("1", 2, 3));
    evaluate (__LINE__, __func__, true,  Database_State::getExport ("4", 5, 6));
    evaluate (__LINE__, __func__, false,  Database_State::getExport ("1", 2, 1));
    
    // Clear flag.
    Database_State::clearExport ("1", 2, 3);
    
    // Test the data.
    evaluate (__LINE__, __func__, false,  Database_State::getExport ("1", 2, 3));
    evaluate (__LINE__, __func__, true,  Database_State::getExport ("4", 5, 6));
    evaluate (__LINE__, __func__, false,  Database_State::getExport ("1", 2, 1));
  }
}


void test_database_imageresources ()
{
  trace_unit_tests (__func__);
  
  Database_ImageResources database_imageresources;
  string image = filter_url_create_root_path ("unittests", "tests", "Genesis-1-1-18.gif");

  // Empty
  {
    refresh_sandbox (true);
    vector <string> resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 0, resources.size());
  }
  
  // Create, names, erase.
  {
    refresh_sandbox (true);

    database_imageresources.create ("unittest");
    vector <string> resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 1, resources.size());
    bool hit = false;
    for (auto & resource : resources) if (resource == "unittest") hit = true;
    evaluate (__LINE__, __func__, true, hit);
    
    database_imageresources.erase ("none-existing");
    resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 1, resources.size());

    database_imageresources.erase ("unittest");
    resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 0, resources.size());
  }
  
  // Store, get, erase images.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
  
    string path = "/tmp/unittest.jpg";
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_unlink (path);

    vector <string> images = database_imageresources.get ("unittest");
    evaluate (__LINE__, __func__, images, {"unittest.jpg", "unittest0.jpg"});
    
    database_imageresources.erase ("unittest", "unittest.jpg");

    images = database_imageresources.get ("unittest");
    evaluate (__LINE__, __func__, images, {"unittest0.jpg"});
  }
  // Assign passage and get image based on passage.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");

    for (int i = 10; i < 20; i++) {
      string image = "unittest" + convert_to_string (i) + ".jpg";
      string path = "/tmp/" + image;
      filter_url_file_cp (image, path);
      database_imageresources.store ("unittest", path);
      filter_url_unlink (path);
      database_imageresources.assign ("unittest", image, i, i, i, i, i, i+10);
    }
    
    vector <string> images = database_imageresources.get ("unittest", 11, 11, 13);
    evaluate (__LINE__, __func__, images, {"unittest11.jpg"});
    
    images = database_imageresources.get ("unittest", 11, 11, 100);
    evaluate (__LINE__, __func__, images, {});
  }
  // Assign passage to image, and retrieve it.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    string image = "unittest.jpg";
    string path = "/tmp/" + image;
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_unlink (path);
    database_imageresources.assign ("unittest", image, 1, 2, 0, 1, 2, 10);

    int book1, chapter1, verse1, book2, chapter2, verse2;
    database_imageresources.get ("unittest", "none-existing",
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    evaluate (__LINE__, __func__, book1, 0);
    evaluate (__LINE__, __func__, chapter1, 0);

    database_imageresources.get ("unittest", image,
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    evaluate (__LINE__, __func__, book1, 1);
    evaluate (__LINE__, __func__, chapter1, 2);
    evaluate (__LINE__, __func__, verse1, 0);
    evaluate (__LINE__, __func__, book2, 1);
    evaluate (__LINE__, __func__, chapter2, 2);
    evaluate (__LINE__, __func__, verse2, 10);
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


void test_database_strong ()
{
  trace_unit_tests (__func__);
  
  Database_Strong database;

  string result = database.definition ("G0");
  evaluate (__LINE__, __func__, "", result);
  
  result = database.definition ("G1");
  int length_h = result.length ();

  evaluate (__LINE__, __func__, true, length_h > 100);

  vector <string> results = database.strong ("χρηστοσ");
  evaluate (__LINE__, __func__, 1, results.size ());
  if (!results.empty ()) {
    evaluate (__LINE__, __func__, "G5543", results[0]);
  }
}


void test_database_morphgnt ()
{
  trace_unit_tests (__func__);
  
  Database_MorphGnt database;
  
  vector <int> results;
  
  results = database.rowids (0, 1, 2);
  evaluate (__LINE__, __func__, 0, results.size ());

  results = database.rowids (20, 3, 4);
  evaluate (__LINE__, __func__, 0, results.size ());
  
  results = database.rowids (40, 5, 6);
  evaluate (__LINE__, __func__, 10, results.size ());
  
  results = database.rowids (66, 7, 8);
  evaluate (__LINE__, __func__, 16, results.size ());
}


void test_database_etcbc4 ()
{
  trace_unit_tests (__func__);
  
  Database_Etcbc4 database;
  
  vector <int> rowids = database.rowids (1, 1, 1);
  evaluate (__LINE__, __func__, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, rowids);

  rowids = database.rowids (2, 3, 4);
  evaluate (__LINE__, __func__, {
    29690,
    29691,
    29692,
    29693,
    29694,
    29695,
    29696,
    29697,
    29698,
    29699,
    29700,
    29701,
    29702,
    29703,
    29704,
    29705,
    29706,
    29707,
    29708,
    29709,
    29710,
    29711
  }, rowids);
  
  string result;
  
  result = database.word (2);
  evaluate (__LINE__, __func__, "רֵאשִׁ֖ית", result);

  result = database.word (1001);
  evaluate (__LINE__, __func__, "טֹ֛וב", result);
  
  result = database.vocalized_lexeme (2);
  evaluate (__LINE__, __func__, "רֵאשִׁית", result);
  
  result = database.vocalized_lexeme (1001);
  evaluate (__LINE__, __func__, "טֹוב", result);

  result = database.consonantal_lexeme (2);
  evaluate (__LINE__, __func__, "ראשׁית", result);

  result = database.consonantal_lexeme (1001);
  evaluate (__LINE__, __func__, "טוב", result);
  
  result = database.gloss (2);
  evaluate (__LINE__, __func__, "beginning", result);
  
  result = database.gloss (1001);
  evaluate (__LINE__, __func__, "good", result);
  
  result = database.pos (2);
  evaluate (__LINE__, __func__, "subs", result);
  
  result = database.pos (1001);
  evaluate (__LINE__, __func__, "adjv", result);
  
  result = database.subpos (2);
  evaluate (__LINE__, __func__, "none", result);
  
  result = database.subpos (1001);
  evaluate (__LINE__, __func__, "none", result);
  
  result = database.gender (2);
  evaluate (__LINE__, __func__, "f", result);
  
  result = database.gender (1001);
  evaluate (__LINE__, __func__, "m", result);
  
  result = database.number (4);
  evaluate (__LINE__, __func__, "pl", result);
  
  result = database.number (1001);
  evaluate (__LINE__, __func__, "sg", result);
  
  result = database.person (3);
  evaluate (__LINE__, __func__, "p3", result);
  
  result = database.person (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.state (2);
  evaluate (__LINE__, __func__, "a", result);
  
  result = database.state (1001);
  evaluate (__LINE__, __func__, "a", result);
  
  result = database.tense (3);
  evaluate (__LINE__, __func__, "perf", result);
  
  result = database.tense (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.stem (3);
  evaluate (__LINE__, __func__, "qal", result);
  
  result = database.stem (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_function (2);
  evaluate (__LINE__, __func__, "Time", result);
  
  result = database.phrase_function (1001);
  evaluate (__LINE__, __func__, "PreC", result);
  
  result = database.phrase_type (2);
  evaluate (__LINE__, __func__, "PP", result);
  
  result = database.phrase_type (1001);
  evaluate (__LINE__, __func__, "AdjP", result);
  
  result = database.phrase_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_a_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_a_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.clause_text_type (2);
  evaluate (__LINE__, __func__, "?", result);
  
  result = database.clause_text_type (1001);
  evaluate (__LINE__, __func__, "NQ", result);
  
  result = database.clause_type (2);
  evaluate (__LINE__, __func__, "xQtX", result);
  
  result = database.clause_type (1001);
  evaluate (__LINE__, __func__, "AjCl", result);
  
  result = database.clause_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.clause_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
}


void test_database_hebrewlexicon ()
{
  trace_unit_tests (__func__);
  
  Database_HebrewLexicon database;
  string result;

  result = database.getaug ("1");
  evaluate (__LINE__, __func__, "aac", result);

  result = database.getaug ("10");
  evaluate (__LINE__, __func__, "aai", result);

  result = database.getbdb ("a.aa.aa");
  evaluate (__LINE__, __func__, 160, result.length ());
  
  result = database.getbdb ("a.ac.ac");
  evaluate (__LINE__, __func__, 424, result.length ());
  
  result = database.getmap ("aaa");
  evaluate (__LINE__, __func__, "a.aa.aa", result);
  
  result = database.getmap ("aaj");
  evaluate (__LINE__, __func__, "a.ac.af", result);
  
  result = database.getpos ("a");
  evaluate (__LINE__, __func__, "adjective", result);
  
  result = database.getpos ("x");
  evaluate (__LINE__, __func__, "indefinite pronoun", result);
  
  result = database.getstrong ("H0");
  evaluate (__LINE__, __func__, "", result);
  
  result = database.getstrong ("H1");
  evaluate (__LINE__, __func__, 303, result.length ());
  
  result = database.getstrong ("H2");
  evaluate (__LINE__, __func__, 149, result.length ());
}


void test_database_cache ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);

  // Initially the database should not exist.
  bool exists = Database_Cache::exists ("");
  evaluate (__LINE__, __func__, false, exists);
  exists = Database_Cache::exists ("unittests");
  evaluate (__LINE__, __func__, false, exists);

  // Copy an old cache database in place.
  // It contains cached data in the old layout.
  // Test that it now exists and contains data.
  string testdatapath = filter_url_create_root_path ("unittests", "tests", "cache_resource_test.sqlite");
  string databasepath = filter_url_create_root_path ("databases",  "cache_resource_unittests.sqlite");
  string out_err;
  filter_shell_run ("cp " + testdatapath + " " + databasepath, out_err);
  size_t count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 1, count);
  exists = Database_Cache::exists ("unittests", 8, 1, 16);
  evaluate (__LINE__, __func__, true, exists);
  string value = Database_Cache::retrieve ("unittests", 8, 1, 16);
  evaluate (__LINE__, __func__, "And Ruth said, Entreat me not to leave you, or to return from following you; for wherever you go, I will go, and wherever you lodge, I will lodge; your people shall be my people, and your God my God.", value);

  // Now remove the (old) cache and verify that it no longer exists or contains data.
  Database_Cache::remove ("unittests");
  exists = Database_Cache::exists ("unittests");
  evaluate (__LINE__, __func__, false, exists);
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 0, count);
  
  // Create a cache for one book.
  Database_Cache::create ("unittests", 10);
  // It should exists for the correct book, but not for another book.
  exists = Database_Cache::exists ("unittests", 10);
  evaluate (__LINE__, __func__, true, exists);
  exists = Database_Cache::exists ("unittests", 11);
  evaluate (__LINE__, __func__, false, exists);
  // The cache should have one book.
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 1, count);
  
  // Cache and retrieve value.
  Database_Cache::create ("unittests", 1);
  Database_Cache::cache ("unittests", 1, 2, 3, "cached");
  value = Database_Cache::retrieve ("unittests", 1, 2, 3);
  evaluate (__LINE__, __func__, "cached", value);
  
  // Book count check.
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 2, count);
  
  // Cache does not exist for one passage, but does exist for the other passage.
  exists = Database_Cache::exists ("unittests", 1, 2, 4);
  evaluate (__LINE__, __func__, false, exists);
  exists = Database_Cache::exists ("unittests", 1, 2, 3);
  evaluate (__LINE__, __func__, true, exists);
  
  // Excercise book cache removal.
  Database_Cache::remove ("unittests");
  exists = Database_Cache::exists ("unittests", 1);
  evaluate (__LINE__, __func__, false, exists);
  
  // Excercise the file-based cache.
  {
    string url = "https://netbible.org/bible/1/2/3";
    string contents = "Bible contents";
    evaluate (__LINE__, __func__, false, database_filebased_cache_exists (url));
    evaluate (__LINE__, __func__, "", database_filebased_cache_get (url));
    database_filebased_cache_put (url, contents);
    evaluate (__LINE__, __func__, true, database_filebased_cache_exists (url));
    evaluate (__LINE__, __func__, contents, database_filebased_cache_get (url));
    database_cache_trim ();
  }

  // Excercise the ready-flag.
  {
    string bible = "ready";
    int book = 11;
    Database_Cache::create (bible, book);
    
    bool ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, false, ready);
    
    Database_Cache::ready (bible, book, false);
    ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, false, ready);
    
    Database_Cache::ready (bible, book, true);
    ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, true, ready);
  }

  // Check the file size function.
  {
    string bible = "size";
    int book = 12;
    Database_Cache::create (bible, book);
    
    int size = Database_Cache::size (bible, book);
    if ((size < 10000) || (size > 15000)) {
      evaluate (__LINE__, __func__, "between 3072 and 5120", convert_to_string (size));
    }

    size = Database_Cache::size (bible, book + 1);
    evaluate (__LINE__, __func__, 0, size);
  }
  
  // Check file naming for downloading a cache.
  {
    evaluate (__LINE__, __func__, "cache_resource_", Database_Cache::fragment ());
    evaluate (__LINE__, __func__, "databases/cache_resource_download_23.sqlite", Database_Cache::path ("download", 23));
  }
  
  refresh_sandbox (true);
}


// Tests for Database_Bibles.
void test_database_bibles ()
{
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    vector <string> standard;
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, standard, bibles);
  }
  {
    // Test whether optimizing works without errors.
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 2, 3, "a");
    database_bibles.storeChapter ("phpunit", 2, 3, "b");
    database_bibles.storeChapter ("phpunit", 2, 3, "c");
    database_bibles.storeChapter ("phpunit", 2, 3, "d");
    database_bibles.storeChapter ("phpunit", 2, 3, "e");
    database_bibles.storeChapter ("phpunit", 2, 3, "f");
    database_bibles.storeChapter ("phpunit", 2, 3, "g");
    database_bibles.optimize ();
    string usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "g", usfm);
  }
  {
    // Test whether optimizing removes files with 0 size.
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 2, 3, "a");
    database_bibles.storeChapter ("phpunit", 2, 3, "b");
    database_bibles.storeChapter ("phpunit", 2, 3, "c");
    database_bibles.storeChapter ("phpunit", 2, 3, "d");
    database_bibles.storeChapter ("phpunit", 2, 3, "e");
    database_bibles.storeChapter ("phpunit", 2, 3, "f");
    database_bibles.storeChapter ("phpunit", 2, 3, "");
    string usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "", usfm);
    database_bibles.optimize ();
    usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "f", usfm);
  }
  // Test create / get / delete Bibles.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {"phpunit"}, bibles);
    
    database_bibles.deleteBible ("phpunit");
    
    bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {}, bibles);
  }
  // Test storeChapter / getChapter
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    string usfm = "\\c 1\n\\p\n\\v 1 Verse 1";
    database_bibles.storeChapter ("phpunit", 2, 1, usfm);
    string result = database_bibles.getChapter ("phpunit", 2, 1);
    evaluate (__LINE__, __func__, usfm, result);
    result = database_bibles.getChapter ("phpunit2", 2, 1);
    evaluate (__LINE__, __func__, "", result);
    result = database_bibles.getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, "", result);
  }
  // Test books
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    vector <int> books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { }, books);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1 }, books);
    
    database_bibles.storeChapter ("phpunit", 2, 3, "\\c 0");
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook ("phpunit", 3);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook ("phpunit", 1);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 2 }, books);
    
    database_bibles.deleteBook ("phpunit2", 2);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 2 }, books);
  }
  // Test chapters ()
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    vector <int> chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2 }, chapters);
    
    chapters = database_bibles.getChapters ("phpunit", 2);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter ("phpunit", 1, 3, "\\c 1");
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 3, 3);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 1, 2);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 1, 3);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { }, chapters);
  }
  // Test chapter IDs
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    int id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000001, id);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000002, id);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
    
    database_bibles.storeChapter ("phpunit", 2, 3, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
  }
}


void test_database_login ()
{
  trace_unit_tests (__func__);

  {
    refresh_sandbox (true);
    Database_Login::create ();
    string path = database_sqlite_file (Database_Login::database ());
    filter_url_file_put_contents (path, "damaged database");
    evaluate (__LINE__, __func__, false, Database_Login::healthy ());
    Database_Login::optimize ();
    evaluate (__LINE__, __func__, true, Database_Login::healthy ());
    refresh_sandbox (false);
  }
  
  refresh_sandbox (true);
  Database_Login::create ();
  Database_Login::optimize ();
  
  string username = "unittest";
  string username2 = "unittest2";
  string address = "192.168.1.0";
  string agent = "Browser's user agent";
  string fingerprint = "ԴԵԶԸ";
  string cookie = "abcdefghijklmnopqrstuvwxyz";
  string cookie2 = "abcdefghijklmnopqrstuvwxyzabc";
  bool daily;

  // Testing whether setting tokens and reading the username, and removing the tokens works.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie, daily));
  Database_Login::removeTokens (username);
  evaluate (__LINE__, __func__, "", Database_Login::getUsername (cookie, daily));

  // Testing whether a persistent login gets removed after about a year.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie, daily));
  Database_Login::testTimestamp ();
  Database_Login::trim ();
  evaluate (__LINE__, __func__, "", Database_Login::getUsername (cookie, daily));

  // Testing whether storing touch enabled
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, true, Database_Login::getTouchEnabled (cookie));
  Database_Login::removeTokens (username);
  evaluate (__LINE__, __func__, false, Database_Login::getTouchEnabled (cookie));
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, false, Database_Login::getTouchEnabled (cookie + "x"));

  // Testing that removing tokens for one set does not remove all tokens for a user.
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie, daily));
  Database_Login::setTokens (username, address, agent, fingerprint, cookie2, true);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie2, daily));
  Database_Login::removeTokens (username, cookie2);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie, daily));
  evaluate (__LINE__, __func__, "", Database_Login::getUsername (cookie2, daily));
  
  // Test moving tokens to a new username.
  Database_Login::removeTokens (username);
  Database_Login::setTokens (username, address, agent, fingerprint, cookie, true);
  evaluate (__LINE__, __func__, username, Database_Login::getUsername (cookie, daily));
  Database_Login::renameTokens (username, username2, cookie);
  evaluate (__LINE__, __func__, username2, Database_Login::getUsername (cookie, daily));
}


void test_database_privileges ()
{
  trace_unit_tests (__func__);

  // Test creation, automatic repair of damages.
  refresh_sandbox (true);
  Database_Privileges::create ();
  string path = database_sqlite_file (Database_Privileges::database ());
  filter_url_file_put_contents (path, "damaged database");
  evaluate (__LINE__, __func__, false, Database_Privileges::healthy ());
  Database_Privileges::optimize ();
  evaluate (__LINE__, __func__, true, Database_Privileges::healthy ());
  refresh_sandbox (false);
  
  Database_Privileges::create ();
  
  // Upgrade routine should not give errors.
  Database_Privileges::upgrade ();
  
  string username = "phpunit";
  string bible = "bible";
  
  // Initially there's no privileges for a Bible book.
  bool read;
  bool write;
  Database_Privileges::getBibleBook (username, bible, 2, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);

  // Set privileges and read them.
  Database_Privileges::setBibleBook (username, bible, 3, false);
  Database_Privileges::getBibleBook (username, bible, 3, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  
  Database_Privileges::setBibleBook (username, bible, 4, true);
  Database_Privileges::getBibleBook (username, bible, 4, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  
  Database_Privileges::removeBibleBook (username, bible, 4);
  Database_Privileges::getBibleBook (username, bible, 4, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);

  // Test Bible book entry count.
  Database_Privileges::setBibleBook (username, bible, 6, true);
  int count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 2, count);
  
  // Test removing book zero, that it removes entries for all books.
  Database_Privileges::removeBibleBook (username, bible, 0);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  
  // Enter a privilege for book = 1, and a different privilege for book 0,
  // and then test that the privilege for book 1 has preference.
  Database_Privileges::setBibleBook (username, bible, 1, false);
  Database_Privileges::getBibleBook (username, bible, 1, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  Database_Privileges::setBibleBook (username, bible, 0, true);
  Database_Privileges::setBibleBook (username, bible, 1, false);
  Database_Privileges::getBibleBook (username, bible, 1, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);

  // Start afresh to not depend too much on previous tests.
  refresh_sandbox (true);
  Database_Privileges::create ();
  
  // Test whether an entry for a book exists.
  bool exists = Database_Privileges::getBibleBookExists (username, bible, 0);
  evaluate (__LINE__, __func__, false, exists);
  Database_Privileges::setBibleBook (username, bible, 1, false);
  // Test the record for the correct book.
  exists = Database_Privileges::getBibleBookExists (username, bible, 1);
  evaluate (__LINE__, __func__, true, exists);
  // The record should also exist for book 0.
  exists = Database_Privileges::getBibleBookExists (username, bible, 0);
  evaluate (__LINE__, __func__, true, exists);
  // The record should not exist for another book.
  exists = Database_Privileges::getBibleBookExists (username, bible, 2);
  evaluate (__LINE__, __func__, false, exists);

  // Start afresh to not depend on the outcome of previous tests.
  refresh_sandbox (true);
  Database_Privileges::create ();

  // Test no read access to entire Bible.
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);
  // Set Bible read-only and test it.
  Database_Privileges::setBible (username, bible, false);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  // Set Bible read-write, and test it.
  Database_Privileges::setBible (username, bible, true);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  // Set one book read-write and test that this applies to entire Bible.
  Database_Privileges::setBible (username, bible, false);
  Database_Privileges::setBibleBook (username, bible, 1, true);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  
  // A feature is off by default.
  bool enabled = Database_Privileges::getFeature (username, 123);
  evaluate (__LINE__, __func__, false, enabled);

  // Test setting a feature on and off.
  Database_Privileges::setFeature (username, 1234, true);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
  Database_Privileges::setFeature (username, 1234, false);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, false, enabled);
  
  // Test bulk privileges removal.
  refresh_sandbox (true);
  Database_Privileges::create ();
  // Set privileges for user for Bible.
  Database_Privileges::setBible (username, bible, false);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 1, count);
  // Remove privileges for a Bible and check on them.
  Database_Privileges::removeBible (bible);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  // Again, set privileges, and remove them by username.
  Database_Privileges::setBible (username, bible, false);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 1, count);
  Database_Privileges::removeUser (username);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  
  // Set features for user.
  Database_Privileges::setFeature (username, 1234, true);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
  // Remove features by username.
  Database_Privileges::removeUser (username);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, false, enabled);
  
  // Test privileges transfer through a text file.
  refresh_sandbox (true);
  Database_Privileges::create ();
  // Set privileges.
  Database_Privileges::setBibleBook (username, bible, 1, true);
  Database_Privileges::setFeature (username, 1234, true);
  // Check the transfer text file.
  string privileges =
    "bibles_start\n"
    "bible\n"
    "1\n"
    "on\n"
    "bibles_end\n"
    "features_start\n"
    "1234\n"
    "features_start";
  evaluate (__LINE__, __func__, privileges, Database_Privileges::save (username));
  // Transfer the privileges to another user.
  string clientuser = username + "client";
  Database_Privileges::load (clientuser, privileges);
  // Check the privileges for that other user.
  Database_Privileges::getBible (clientuser, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
}


void test_database_git ()
{
#ifdef HAVE_CLOUD
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  // Create the database.
  Database_Git::create ();

  string user = "user";
  string bible = "bible";
  
  // Store one chapter, and check there's one rowid as a result.
  Database_Git::store_chapter (user, bible, 1, 2, "old", "new");
  vector <int> rowids = Database_Git::get_rowids (user, "");
  evaluate (__LINE__, __func__, {}, rowids);
  rowids = Database_Git::get_rowids ("", bible);
  evaluate (__LINE__, __func__, {}, rowids);
  rowids = Database_Git::get_rowids (user, bible);
  evaluate (__LINE__, __func__, {1}, rowids);

  // Store some more chapters to get more rowids in the database.
  Database_Git::store_chapter (user, bible, 2, 5, "old2", "new5");
  Database_Git::store_chapter (user, bible, 3, 6, "old3", "new6");
  Database_Git::store_chapter (user, bible, 4, 7, "old4", "new7");

  // Retrieve and check a certain rowid whether it has the correct values.
  string user2, bible2;
  int book, chapter;
  string oldusfm, newusfm;
  bool get = Database_Git::get_chapter (1, user2, bible2, book, chapter, oldusfm, newusfm);
  evaluate (__LINE__, __func__, true, get);
  evaluate (__LINE__, __func__, user, user2);
  evaluate (__LINE__, __func__, bible, bible2);
  evaluate (__LINE__, __func__, 1, book);
  evaluate (__LINE__, __func__, 2, chapter);
  evaluate (__LINE__, __func__, "old", oldusfm);
  evaluate (__LINE__, __func__, "new", newusfm);
  
  // Erase a rowid, and check that the remaining ones in the database are correct.
  Database_Git::erase_rowid (2);
  rowids = Database_Git::get_rowids (user, bible);
  evaluate (__LINE__, __func__, {1, 3, 4}, rowids);

  // Getting a non-existent rowid should fail.
  get = Database_Git::get_chapter (2, user, bible, book, chapter, oldusfm, newusfm);
  evaluate (__LINE__, __func__, false, get);
  
  // Update the timestamps and check that expired entries get removed and recent ones remain.
  rowids = Database_Git::get_rowids ("user", "bible");
  evaluate (__LINE__, __func__, 3, rowids.size ());
  Database_Git::optimize ();
  rowids = Database_Git::get_rowids (user, bible);
  evaluate (__LINE__, __func__, 3, rowids.size ());
  Database_Git::touch_timestamps (filter_date_seconds_since_epoch () - 432000 - 1);
  Database_Git::optimize ();
  rowids = Database_Git::get_rowids (user, bible);
  evaluate (__LINE__, __func__, 0, rowids.size ());
  
  // Test that it reads distinct users.
  Database_Git::store_chapter (user, bible, 2, 5, "old", "new");
  Database_Git::store_chapter (user, bible, 2, 5, "old", "new");
  Database_Git::store_chapter ("user2", bible, 2, 5, "old", "new");
  vector <string> users = Database_Git::get_users (bible);
  evaluate (__LINE__, __func__, {user, "user2"}, users);
#endif
}


void test_database_userresources ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  vector <string> names;
  string name = "unit//test";
  string url = "https://website.org/resources/<book>/<chapter>/<verse>.html";
  int book = 99;
  string abbrev = "Book 99";
  string fragment;
  string value;
  vector <string> specialnames = { "abc\\def:ghi", name };
  
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, {}, names);

  Database_UserResources::url (name, url);
  value = Database_UserResources::url (name);
  evaluate (__LINE__, __func__, url, value);
  
  for (auto name : specialnames) {
    Database_UserResources::url (name, name + url);
  }
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, specialnames, names);

  for (auto name : specialnames) {
    Database_UserResources::remove (name);
  }
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, {}, names);

  Database_UserResources::book (name, book, abbrev);
  fragment = Database_UserResources::book (name, book);
  evaluate (__LINE__, __func__, abbrev, fragment);

  fragment = Database_UserResources::book (name + "x", book);
  evaluate (__LINE__, __func__, "", fragment);

  fragment = Database_UserResources::book (name, book + 1);
  evaluate (__LINE__, __func__, "", fragment);
}


void test_database_statistics ()
{
#ifdef HAVE_CLOUD
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    
    Database_Statistics::create ();
    Database_Statistics::optimize ();
    
    int one_thousand = 1000;
    int two_thousand = 2000;
    int now = filter_date_seconds_since_epoch ();
    int now_plus_one = now + 1;
    
    // Record some statistics.
    Database_Statistics::store_changes (one_thousand, "zero", 0);
    Database_Statistics::store_changes (one_thousand, "one", 10);
    Database_Statistics::store_changes (two_thousand, "one", 20);
    Database_Statistics::store_changes (now, "one", 30);
    Database_Statistics::store_changes (now_plus_one, "one", 40);
    Database_Statistics::store_changes (1100, "two", 11);
    Database_Statistics::store_changes (2100, "two", 21);
    Database_Statistics::store_changes (now - one_thousand, "two", 31);
    Database_Statistics::store_changes (now - two_thousand, "two", 41);

    // Check all available users.
    vector <string> users = Database_Statistics::get_users ();
    evaluate (__LINE__, __func__, {"one", "two"}, users);

    // The changes for all available users for no more than a year ago.
    vector <pair <int, int>> changes = Database_Statistics::get_changes ("");
    evaluate (__LINE__, __func__, 4, changes.size ());

    // A known amount of change statistics records for a known user for no more than a year ago.
    changes = Database_Statistics::get_changes ("two");
    unsigned int size = 2;
    evaluate (__LINE__, __func__, size, changes.size ());
    
    // Sort the change statistics most recent first.
    if (changes.size () == size) {
      evaluate (__LINE__, __func__, now - one_thousand, changes[0].first);
      evaluate (__LINE__, __func__, 31, changes[0].second);
      evaluate (__LINE__, __func__, now - two_thousand, changes[1].first);
      evaluate (__LINE__, __func__, 41, changes[1].second);
    }
  }
#endif
}


void test_database_develop ()
{
  trace_unit_tests (__func__);
}

/*
Copyright (©) 2003-2018 Teus Benschop.

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


#include <database/mail.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <database/logs.h>
#include <filter/date.h>


// Handles mail sent from Bibledit to the users.
// Database resilience: It stores the emails to be sent shortly after.
// The database is not very often written to.
// The risk of corruption is low and acceptable.


Database_Mail::Database_Mail (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


Database_Mail::~Database_Mail ()
{
}


sqlite3 * Database_Mail::connect ()
{
  return database_sqlite_connect ("mail");
}


void Database_Mail::create ()
{
  sqlite3 * db = connect ();
  string sql = 
    "CREATE TABLE IF NOT EXISTS mail ("
    "  username text,"
    "  timestamp integer,"
    "  subject text,"
    "  body text,"
    "  retry integer"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_Mail::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM mail;");
  database_sqlite_disconnect (db);
}


void Database_Mail::trim ()
{
  int time = filter_date_seconds_since_epoch () - 2592000; // Remove entries after 30 days.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM mail WHERE timestamp <");
  sql.add (time);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_exec (db, "DELETE FROM mail WHERE retry > 10;");
  database_sqlite_disconnect (db);
}


// Send email.
// to: The email address.
// subject: The subject.
// body: The body.
// time: Normally not given, but if given, it indicates the time stamp for sending this email.
void Database_Mail::send (string to, string subject, string body, int time)
{
  if (time == 0) time = filter_date_seconds_since_epoch ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO mail VALUES (");
  sql.add (to);
  sql.add (",");
  sql.add (time);
  sql.add (",");
  sql.add (subject);
  sql.add (",");
  sql.add (body);
  sql.add (", 0);");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Get number of mails for the current user.
int Database_Mail::getMailCount ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic ()->currentUser();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT count(*) FROM mail WHERE username =");
  sql.add (user);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["count(*)"];
  database_sqlite_disconnect (db);
  if (!result.empty ()) {
    return convert_to_int (result [0]);
  }
  return 0;
}


// Get the mails of the current user.
vector <Database_Mail_User> Database_Mail::getMails ()
{
  vector <Database_Mail_User> mails;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string user = request->session_logic ()->currentUser();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid, timestamp, subject FROM mail WHERE username =");
  sql.add (user);
  sql.add ("ORDER BY timestamp DESC;");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> rowids = result ["rowid"];
  vector <string> timestamps = result ["timestamp"];
  vector <string> subjects = result ["subject"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    Database_Mail_User mail = Database_Mail_User ();
    mail.rowid = convert_to_int (rowids [i]);
    mail.timestamp = convert_to_int (timestamps [i]);
    mail.subject = subjects [i];
    mails.push_back (mail);
  }
  return mails;
}


// Delete a mail.
void Database_Mail::erase (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM mail WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Get a mail.
Database_Mail_Item Database_Mail::get (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT username, subject, body FROM mail WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  Database_Mail_Item item = Database_Mail_Item ();
  if (!result.empty ()) {
    item.username = result ["username"] [0];
    item.subject = result ["subject"] [0];
    item.body = result ["body"] [0];
  }
  return item;
}


// Get ids of all mails ready for sending.
vector <int> Database_Mail::getMailsToSend ()
{
  vector <int> ids;
  int timestamp = filter_date_seconds_since_epoch ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM mail WHERE timestamp <=");
  sql.add (timestamp);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  for (auto & id : result) {
    ids.push_back (convert_to_int (id));
  }
  return ids;
}


// Postpones a mail till later.
// Used for retrying after failure to send the mail.  
void Database_Mail::postpone (int id)
{
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("UPDATE mail SET retry = retry + 1 WHERE rowid =");
  sql1.add (id);
  sql1.add (";");
  SqliteSQL sql2 = SqliteSQL ();
  sql2.add ("UPDATE mail SET timestamp = timestamp + retry * 900 WHERE rowid =");
  sql2.add (id);
  sql2.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  database_sqlite_disconnect (db);

}

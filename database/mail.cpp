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


constexpr const auto mail {"mail"};


Database_Mail::Database_Mail (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
}


Database_Mail::~Database_Mail ()
{
}


void Database_Mail::create ()
{
  SqliteDatabase sql (mail);
  sql.add ("CREATE TABLE IF NOT EXISTS mail ("
           " username text,"
           " timestamp integer,"
           " subject text,"
           " body text,"
           " retry integer"
           ");");
  sql.execute ();
}


void Database_Mail::optimize ()
{
  SqliteDatabase sql (mail);
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_Mail::trim ()
{
  const int time = filter::date::seconds_since_epoch () - 2592000; // Remove entries after 30 days.
  SqliteDatabase sql (mail);
  sql.add ("DELETE FROM mail WHERE timestamp <");
  sql.add (time);
  sql.add (";");
  sql.execute ();
  sql.set_sql ("DELETE FROM mail WHERE retry > 10;");
  sql.execute();
}


// Send email.
// to: The email address.
// subject: The subject.
// body: The body.
// time: Normally not given, but if given, it indicates the time stamp for sending this email.
void Database_Mail::send (std::string to, std::string subject, std::string body, int time)
{
  if (time == 0) time = filter::date::seconds_since_epoch ();
  SqliteDatabase sql (mail);
  sql.add ("INSERT INTO mail VALUES (");
  sql.add (to);
  sql.add (",");
  sql.add (time);
  sql.add (",");
  sql.add (subject);
  sql.add (",");
  sql.add (body);
  sql.add (", 0);");
  sql.execute ();
}


// Get number of mails for the current user.
int Database_Mail::getMailCount ()
{
  const std::string& user = m_webserver_request.session_logic ()->get_username();
  SqliteDatabase sql (mail);
  sql.add ("SELECT count(*) FROM mail WHERE username =");
  sql.add (user);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["count(*)"];
  if (!result.empty ()) {
    return filter::strings::convert_to_int (result.at(0));
  }
  return 0;
}


// Get the mails of the current user.
std::vector <Database_Mail_User> Database_Mail::getMails ()
{
  std::vector <Database_Mail_User> mails;
  const std::string& user = m_webserver_request.session_logic ()->get_username();
  SqliteDatabase sql (mail);
  sql.add ("SELECT rowid, timestamp, subject FROM mail WHERE username =");
  sql.add (user);
  sql.add ("ORDER BY timestamp DESC;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> rowids = result ["rowid"];
  const std::vector <std::string> timestamps = result ["timestamp"];
  const std::vector <std::string> subjects = result ["subject"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    Database_Mail_User db_mail = Database_Mail_User ();
    db_mail.rowid = filter::strings::convert_to_int (rowids [i]);
    db_mail.timestamp = filter::strings::convert_to_int (timestamps [i]);
    db_mail.subject = subjects [i];
    mails.push_back (db_mail);
  }
  return mails;
}


// Delete a mail.
void Database_Mail::erase (int id)
{
  SqliteDatabase sql (mail);
  sql.add ("DELETE FROM mail WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


// Get a mail.
Database_Mail_Item Database_Mail::get (int id)
{
  SqliteDatabase sql (mail);
  sql.add ("SELECT username, subject, body FROM mail WHERE rowid =");
  sql.add (id);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  Database_Mail_Item item = Database_Mail_Item ();
  if (!result.empty ()) {
    item.username = result ["username"] [0];
    item.subject = result ["subject"] [0];
    item.body = result ["body"] [0];
  }
  return item;
}


// Get ids of all mails ready for sending.
std::vector <int> Database_Mail::getMailsToSend ()
{
  std::vector <int> ids;
  const int timestamp = filter::date::seconds_since_epoch ();
  SqliteDatabase sql (mail);
  sql.add ("SELECT rowid FROM mail WHERE timestamp <=");
  sql.add (timestamp);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["rowid"];
  for (const auto& id : result) {
    ids.push_back (filter::strings::convert_to_int (id));
  }
  return ids;
}


// Postpones a mail till later.
// Used for retrying after failure to send the mail.  
void Database_Mail::postpone (int id)
{
  SqliteDatabase sql (mail);
  sql.add ("UPDATE mail SET retry = retry + 1 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute();
  sql.clear();
  sql.add ("UPDATE mail SET timestamp = timestamp + retry * 900 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


// Get the row IDs of all mails in the database.
std::vector <int> Database_Mail::getAllMails ()
{
  std::vector <int> rowids {};
  SqliteDatabase sql (mail);
  sql.add ("SELECT rowid FROM mail;");
  const std::vector <std::string> result = sql.query () ["rowid"];
  for (const auto& rowid : result) {
    const int id = filter::strings::convert_to_int (rowid);
    rowids.push_back (id);
  }
  return rowids;
}

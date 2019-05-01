/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <confirm/worker.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <database/confirm.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <email/send.h>


#ifdef HAVE_CLOUD


Confirm_Worker::Confirm_Worker (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


// Sets up a confirmation cycle in order to change something in the database.
// If for example a user requests the email address to be changed, 
// an initial email will be sent, which the user should confirm.
// to                : Email address for the initial email and the response.
// initial_subject   : The subject of the initial email message.
// initial_body      : The body of the initial email message.
// query             : The query to be executed on the database if the user confirms the email successfully.
// subsequent_subject: The subject of the email to send upon user confirmation.
// subsequent_body   : The body of the email to send upon user confirmation.
void Confirm_Worker::setup (string to, string initial_subject, string initial_body, string query, string subsequent_subject, string subsequent_body)
{
  Database_Confirm database_confirm;
  int confirmation_id = database_confirm.getNewID ();
  initial_subject += " " + convert_to_string (confirmation_id);
  initial_body += "\n\n";
  initial_body += translate ("Please confirm this request by replying to this email. There is a confirmation number in the subject line. Your reply should have this same confirmation number in the subject line.");
  email_schedule (to, initial_subject, initial_body);
  database_confirm.store (confirmation_id, query, to, subsequent_subject, subsequent_body);
}


// Handles a confirmation email received "from" with "subject" and "body".
// Returns true if the mail was handled, else false.
bool Confirm_Worker::handleEmail (string from, string subject, string body)
{
  (void) from;
  // Find out in the confirmation database whether the subject line contains an active ID.
  // If not, bail out.
  Database_Confirm database_confirm;
  int id = database_confirm.searchID (subject);
  if (id == 0) {
    return false;
  }
  // An active ID was found: Execute the associated database query.
  string query = database_confirm.getQuery (id);
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  request->database_users()->execute (query);
  // Send confirmation mail.
  string mailto = database_confirm.getMailTo (id);
  subject = database_confirm.getSubject (id);
  body = database_confirm.getBody (id);
  email_schedule (mailto, subject, body);
  // Delete the confirmation record.
  database_confirm.erase (id);
  // Notify managers.
  informManagers (mailto, body);
  // Job done.
  return true;
}


// Inform the managers about an account change.
void Confirm_Worker::informManagers (string email, string body)
{
  Database_Users database_users;
  vector <string> users = database_users.getUsers ();
  for (auto & user : users) {
    int level = database_users.get_level (user);
    if (level >= Filter_Roles::manager ()) {
      string mailto = database_users.get_email (user);
      string subject = translate ("User account change");
      string newbody = translate ("A user account was changed.");
      newbody.append (" ");
      newbody.append (translate ("Email address:"));
      newbody.append (" ");
      newbody.append (email);
      newbody.append (". ");
      newbody.append (translate ("The following email was sent to this user:"));
      newbody.append (" ");
      newbody.append (body);
      email_schedule (mailto, subject, newbody);
    }
  }
}


#endif

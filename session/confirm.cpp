/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <session/confirm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <session/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <confirm/worker.h>
#include <email/send.h>


class Verification
{
public:
  string question;
  string answer;
  string passage;
};


const char * session_confirm_url ()
{
  return "session/confirm";
}


bool session_confirm_acl (void * webserver_request)
{
  // Find the level of the user.
  // This confirmation page only allows access if the user is not yet logged in.
  // Such a situation produces level 1, that is the guest level.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int level = request->session_logic ()->currentLevel ();
  return (level = Filter_Roles::guest());
}


string session_confirm (void * webserver_request)
{
  (void) webserver_request;
  
  string page;

#ifdef HAVE_CLOUD

  Confirm_Worker confirm_worker = Confirm_Worker (webserver_request);
  bool valid = confirm_worker.handleLink ();
  cout << "valid " << valid << endl; // Todo
  

//  Webserver_Request * request = (Webserver_Request *) webserver_request;


//  (void) from;
//  // Find out in the confirmation database whether the subject line contains an active ID.
//  // If not, bail out.
//  Database_Confirm database_confirm;
//  int id = database_confirm.searchID (subject);
//  if (id == 0) {
//    return false;
//  }
//  // An active ID was found: Execute the associated database query.
//  string query = database_confirm.getQuery (id);
//  Webserver_Request * request = (Webserver_Request *) webserver_request;
//  request->database_users()->execute (query);
//  // Send confirmation mail.
//  string mailto = database_confirm.getMailTo (id);
//  subject = database_confirm.getSubject (id);
//  body = database_confirm.getBody (id);
//  email_schedule (mailto, subject, body);
//  // Delete the confirmation record.
//  database_confirm.erase (id);
//  // Notify managers.
//  informManagers (mailto, body);
//  // Job done.
//  return true;

  
  
  
#ifdef DEFAULT_BIBLEDIT_CONFIGURATION
#endif
#ifdef HAVE_INDONESIANCLOUDFREE
#endif

#endif

  return page;
}

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


#include <unittests/mail.h>
#include <unittests/utilities.h>
#include <database/users.h>
#include <webserver/request.h>
#include <database/mail.h>


void test_database_mail ()
{
  trace_unit_tests (__func__);
  
  // Optimize / trim.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
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
    Webserver_Request request;
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
  
  // Normal cycle.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
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

  // Normal postpone.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request request;
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

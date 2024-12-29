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
#include <database/users.h>
#include <webserver/request.h>
#include <database/mail.h>
#include <filter/url.h>
#include <filter/mail.h>


TEST (database, mail)
{
#ifdef HAVE_CLOUD
  
  // Optimize / trim.
  {
    refresh_sandbox (false);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    database_mail.optimize ();
    database_mail.trim ();
  }
  
  // Empty.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    webserver_request.session_logic ()->set_username ("phpunit");
    
    const int count = database_mail.getMailCount ();
    EXPECT_EQ (0, count);
    
    const std::vector <Database_Mail_User> mails = database_mail.getMails ();
    EXPECT_EQ (0, static_cast<int>(mails.size()));
    
    const std::vector <int> mails_to_send = database_mail.getMailsToSend ();
    EXPECT_EQ (std::vector <int>{}, mails_to_send);
  }
  
  // Normal cycle.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    webserver_request.session_logic ()->set_username ("phpunit");
    
    database_mail.send ("phpunit", "subject", "body");
    
    int count = database_mail.getMailCount ();
    EXPECT_EQ (1, count);
    
    std::vector <Database_Mail_User> mails = database_mail.getMails ();
    EXPECT_EQ ("subject", mails [0].subject);
    
    Database_Mail_Item mail = database_mail.get (1);
    EXPECT_EQ ("phpunit", mail.username);
    EXPECT_EQ ("body", mail.body);
    
    database_mail.erase (1);
    
    count = database_mail.getMailCount ();
    EXPECT_EQ (0, count);
  }

  // Normal postpone.
  {
    refresh_sandbox (true);
    Database_Users database_users;
    database_users.create ();
    Webserver_Request webserver_request;
    Database_Mail database_mail (webserver_request);
    database_mail.create ();
    webserver_request.session_logic ()->set_username ("phpunit");
    
    database_mail.send ("phpunit", "subject", "body");
    
    std::vector <int> mails = database_mail.getMailsToSend ();
    EXPECT_EQ (1, static_cast <int>(mails.size ()));
    
    database_mail.postpone (1);
    mails = database_mail.getMailsToSend ();
    EXPECT_EQ (0, static_cast <int>(mails.size ()));
  }
#endif
}


TEST (filter, mail)
{
#ifdef HAVE_CLOUD
  
  const std::string datafolder = filter_url_create_root_path ({"unittests", "tests", "emails"});
  
  // Standard mimetic library's test message.
  {
    const std::string msgpath = filter_url_create_path ({datafolder, "email1.msg"});
    const std::string msg = filter_url_file_get_contents (msgpath);
    std::string from, subject, plaintext;
    filter_mail_dissect (msg, from, subject, plaintext);
    const std::string txtpath = filter_url_create_path ({datafolder, "email1.txt"});
    const std::string txt = filter_url_file_get_contents (txtpath);
    EXPECT_EQ ("stefano@codesink.org", from);
    EXPECT_EQ ("My picture!", subject);
    EXPECT_EQ (txt, plaintext);
  }
   
  // A plain text message, that is, not a MIME message.
  {
    const std::string msgpath = filter_url_create_path ({datafolder, "email2.msg"});
    const std::string msg = filter_url_file_get_contents (msgpath);
    std::string from, subject, plaintext;
    filter_mail_dissect (msg, from, subject, plaintext);
    const std::string txtpath = filter_url_create_path ({datafolder, "email2.txt"});
    const std::string txt = filter_url_file_get_contents (txtpath);
    EXPECT_EQ ("developer@device.localdomain (Developer)", from);
    EXPECT_EQ ("plain text", subject);
    EXPECT_EQ (txt, plaintext);
  }
  
  // A UTF-8 quoted-printable message.
  {
    const std::string msgpath = filter_url_create_path ({datafolder, "email3.msg"});
    const std::string msg = filter_url_file_get_contents (msgpath);
    std::string from, subject, plaintext;
    filter_mail_dissect (msg, from, subject, plaintext);
    const std::string txtpath = filter_url_create_path ({datafolder, "email3.txt"});
    const std::string txt = filter_url_file_get_contents (txtpath);
    EXPECT_EQ ("Sender <sender@domain.net>", from);
    EXPECT_EQ ("Message encoded with quoted-printable", subject);
    EXPECT_EQ (txt, plaintext);
  }

  // A UTF-8 base64 encoded message.
  {
    const std::string msgpath = filter_url_create_path ({datafolder, "email4.msg"});
    const std::string msg = filter_url_file_get_contents (msgpath);
    std::string from, subject, plaintext;
    filter_mail_dissect (msg, from, subject, plaintext);
    const std::string txtpath = filter_url_create_path ({datafolder, "email4.txt"});
    const std::string txt = filter_url_file_get_contents (txtpath);
    EXPECT_EQ ("Sender <sender@domain.net>", from);
    EXPECT_EQ ("Message encoded in base64", subject);
    EXPECT_EQ (txt, plaintext);
  }
  
  // Test the collection of sample mails.
  {
    const std::vector <std::string> files = filter_url_scandir (datafolder);
    for (const auto& messagefile : files) {
      if (messagefile.find ("m") != 0) continue;
      if (filter_url_get_extension (messagefile) != "msg") continue;
      std::string path = filter_url_create_path ({datafolder, messagefile});
      std::string contents = filter_url_file_get_contents (path);
      std::string from, subject, plaintext;
      filter_mail_dissect (contents, from, subject, plaintext);
      path += ".txt";
      contents = filter_url_file_get_contents (path);
      EXPECT_EQ (contents, plaintext);
    }
  }
#endif
}


#endif

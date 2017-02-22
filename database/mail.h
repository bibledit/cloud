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


#ifndef INCLUDED_DATABASE_MAIL_H
#define INCLUDED_DATABASE_MAIL_H


#include <config/libraries.h>


class Database_Mail_User
{
public:
  int rowid;
  int timestamp;
  string subject;
};


class Database_Mail_Item
{
public:
  string username;
  string subject;
  string body;
};


class Database_Mail
{
public:
  Database_Mail (void * webserver_request_in);
  ~Database_Mail ();
  void create ();
  void optimize ();
  void trim ();
  void send (string to, string subject, string body, int time = 0);
  int getMailCount ();
  vector <Database_Mail_User> getMails ();
  void erase (int id);
  Database_Mail_Item get (int id);
  vector <int> getMailsToSend ();
  void postpone (int id);
private:
  sqlite3 * connect ();
  void * webserver_request;
};


#endif

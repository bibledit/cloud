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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

struct Database_Mail_User
{
  int rowid {0};
  int timestamp {0};
  std::string subject {};
};

struct Database_Mail_Item
{
  std::string username {};
  std::string subject {};
  std::string body {};
};

class Database_Mail
{
public:
  Database_Mail (Webserver_Request& webserver_request);
  ~Database_Mail ();
  Database_Mail(const Database_Mail&) = delete;
  Database_Mail operator=(const Database_Mail&) = delete;
  void create ();
  void optimize ();
  void trim ();
  void send (std::string to, std::string subject, std::string body, int time = 0);
  int getMailCount ();
  std::vector <Database_Mail_User> getMails ();
  void erase (int id);
  Database_Mail_Item get (int id);
  std::vector <int> getMailsToSend ();
  void postpone (int id);
  std::vector <int> getAllMails ();
private:
  Webserver_Request& m_webserver_request;
};

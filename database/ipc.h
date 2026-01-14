/*
Copyright (©) 2003-2026 Teus Benschop.

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


struct Database_Ipc_Item
{
  std::string file {};
  int rowid {0};
  std::string user {};
  std::string channel {};
  std::string command {};
};


struct Database_Ipc_Message
{
  int id {0};
  std::string channel {};
  std::string command {};
  std::string message {};
};


class Database_Ipc
{
public:
  Database_Ipc (Webserver_Request& webserver_request);
  void trim ();
  void storeMessage (std::string user, std::string channel, std::string command, std::string message);
  Database_Ipc_Message retrieveMessage (int id, std::string user, std::string channel, std::string command);
  void deleteMessage (int id);
  Database_Ipc_Message getNote ();
  bool getNotesAlive ();
private:
  Webserver_Request& m_webserver_request;
  std::string folder ();
  std::string file (std::string file);
  std::vector <Database_Ipc_Item> readData ();
  void writeRecord (int rowid, std::string user, std::string channel, std::string command, std::string message);
  int getNextId (const std::vector <Database_Ipc_Item> & data);
};



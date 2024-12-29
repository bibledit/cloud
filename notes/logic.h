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
#include <filter/passage.h>

class Webserver_Request;

class Notes_Logic
{
public:
  Notes_Logic (Webserver_Request& webserver_request);
  static constexpr int lowNoteIdentifier  = 100000000;
  static constexpr int highNoteIdentifier = 999999999;
  static constexpr int notifyNoteNew = 1;
  static constexpr int notifyNoteComment = 2;
  static constexpr int notifyNoteDelete = 3;
  static constexpr int notifyMarkNoteForDeletion = 4;
  int createNote (std::string bible, int book, int chapter, int verse, std::string summary, std::string contents, bool raw);
  void setContent (int identifier, const std::string& content);
  void addComment (int identifier, const std::string& comment);
  void set_summary (int identifier, const std::string& summary);
  void subscribe (int identifier);
  void unsubscribe (int identifier);
  void assignUser (int identifier, const std::string& user);
  void unassignUser (int identifier, const std::string& user);
  void setStatus (int identifier, const std::string& status);
  void setPassages (int identifier, const std::vector <Passage> & passages);
  void setRawSeverity (int identifier, int severity);
  void setBible (int identifier, const std::string& bible);
  void markForDeletion (int identifier);
  void unmarkForDeletion (int identifier);
  void erase (int identifier);
  void handlerNewNote (int identifier);
  void handlerAddComment (int identifier);
  void handlerAssignNote (int identifier, const std::string& user);
  void handlerMarkNoteForDeletion (int identifier);
  void handlerDeleteNote (int identifier);
  bool handleEmailNew (std::string from, std::string subject, std::string body);
  bool handleEmailComment (std::string from, std::string subject, std::string body);
  std::string generalBibleName ();
private:
  Webserver_Request& m_webserver_request;
  void notifyUsers (int identifier, int notification);
  void emailUsers (int identifier, const std::string& label, std::string bible, const std::vector <std::string> & users, bool postpone);
};

void notes_logic_maintain_note_assignees (bool force);

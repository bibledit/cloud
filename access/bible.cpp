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


#include <access/bible.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <database/privileges.h>
#include <client/logic.h>
#include <filter/roles.h>


// Returns true if the $user has read access to the $bible.
// If no $user is given, it takes the currently logged-in user.
bool access_bible_read (void * webserver_request, const string & bible, string user)
{
  // Client: User has access to all Bibles.
#ifdef HAVE_CLIENT
  return true;
#endif

  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int level = 0;
  if (user.empty ()) {
    // Current user.
    user = request->session_logic ()->currentUser ();
    level = request->session_logic ()->currentLevel ();
  } else {
    // Take level belonging to user.
    level = request->database_users ()->get_level (user);
  }

  // Managers and higher have read access.
  if (level >= Filter_Roles::manager ()) {
    return true;
  }

  // Read privileges for the user.
  bool read, write;
  Database_Privileges::getBible (user, bible, read, write);
  if (read) {
    return true;
  }

  // No Bibles assigned: Consultant can view any Bible.
  if (level >= Filter_Roles::consultant ()) {
    int privileges_count = Database_Privileges::getBibleBookCount ();
    if (privileges_count == 0) {
      return true;
    }
  }

  // Default.
  return false;
}


// Returns true if the user has write access to the $bible.
bool access_bible_write (void * webserver_request, const string & bible, string user)
{
#ifdef HAVE_CLIENT
  // Client: When not yet connected to the Cloud, the user has access to all Bibles.
  // When connected to the Cloud, this no longer applies,
  // since the client now receives the privileges from the Cloud.
  if (!client_logic_client_enabled ()) {
    return true;
  }
#endif
  
  int level = 0;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  if (user.empty ()) {
    user = request->session_logic ()->currentUser ();
    level = request->session_logic ()->currentLevel ();
  }
  if (level == 0) {
    // Take level belonging to user.
    level = request->database_users ()->get_level (user);
  }
  
  // Managers and higher always have write access.
  if (level >= Filter_Roles::manager ()) {
    return true;
  }
  
  // Read the privileges for the user.
  bool read, write;
  Database_Privileges::getBible (user, bible, read, write);
  if (write) {
    return true;
  }
  
  // No Bibles assigned: Translator can write to any bible.
  if (level >= Filter_Roles::translator ()) {
    int privileges_count = Database_Privileges::getBibleBookCount ();
    if (privileges_count == 0) {
      return true;
    }
  }
  
  // Default.
  return false;
}


// Returns true if the $user has write access to the $bible and the $book.
// If no user is given, it takes the currently logged-in user.
// If the user has read-only access to even one book of the $bible,
// then the user is considered not to have write access to the entire $bible.
bool access_bible_book_write (void * webserver_request, string user, const string & bible, int book)
{
#ifdef HAVE_CLIENT
  // Client: When not yet connected to the Cloud, the user has access to the book.
  // When connected to the Cloud, this no longer applies,
  // since the client now receives the privileges from the Cloud.
  if (!client_logic_client_enabled ()) {
    return true;
  }
#endif

  int level = 0;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  if (user.empty ()) {
    user = request->session_logic ()->currentUser ();
    level = request->session_logic ()->currentLevel ();
  }
  if (level == 0) {
    // Take level belonging to user.
    level = request->database_users ()->get_level (user);
  }

  // Managers and higher always have write access.
  if (level >= Filter_Roles::manager ()) {
    return true;
  }

  // Read the privileges for the user.
  bool read, write;
  Database_Privileges::getBibleBook (user, bible, book, read, write);
  if (write) {
    return true;
  }

  // No Bibles assigned: Translator can write to any bible.
  if (level >= Filter_Roles::translator ()) {
    int privileges_count = Database_Privileges::getBibleBookCount ();
    if (privileges_count == 0) {
      return true;
    }
  }
  
  // Default.
  return false;
}


// Returns an array of Bibles the user has read access to.
// If no user is given, it takes the currently logged-in user.
vector <string> access_bible_bibles (void * webserver_request, string user)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <string> allbibles = request->database_bibles ()->getBibles ();
  vector <string> bibles;
  for (auto & bible : allbibles) {
    if (access_bible_read (webserver_request, bible, user)) {
      bibles.push_back (bible);
    }
  }
  return bibles;
}


// This function clamps bible.
// It returns the $bible if the currently logged-in user has access to it.
// Else it returns another accessible bible or nothing.
string access_bible_clamp (void * webserver_request, string bible)
{
  if (!access_bible_read (webserver_request, bible)) {
    bible = "";
    vector <string> bibles = access_bible_bibles (webserver_request);
    if (!bibles.empty ()) bible = bibles [0];
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    request->database_config_user ()->setBible (bible);
  }
  return bible;
}


// This function checks whether the user in the $webserver_request
// has $read or $write access to one or more Bibles.
void access_a_bible (void * webserver_request, bool & read, bool & write)
{
  read = false;
  write = false;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <string> bibles = request->database_bibles ()->getBibles ();
  for (auto & bible : bibles) {
    if (access_bible_read (webserver_request, bible)) read = true;
    if (access_bible_write (webserver_request, bible)) write = true;
  }
}

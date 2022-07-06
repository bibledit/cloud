/*
Copyright (©) 2003-2022 Teus Benschop.

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
#include <filter/indonesian.h>


// Returns true if the $user has read access to the $bible.
// If no $user is given, it takes the currently logged-in user.
bool AccessBible::Read (void * webserver_request, const string & bible, string user)
{
  // Client: User has access to all Bibles.
#ifdef HAVE_CLIENT
  (void) webserver_request;
  (void) bible;
  (void) user;
  return true;
#endif

#ifdef HAVE_CLOUD
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // Get the level, that is the role, of the given user.
  int role_level { 0 };
  if (user.empty ()) {
    // Current user.
    user = request->session_logic ()->currentUser ();
    role_level = request->session_logic ()->currentLevel ();
  } else {
    // Take level belonging to user.
    role_level = request->database_users ()->get_level (user);
  }

  // Indonesian Cloud Free.
  if (config_logic_indonesian_cloud_free ()) {
    // A free guest account has a role of "Consultant".
    if (role_level == Filter_Roles::consultant()) {
      // This level/role has access to:
      // 1. AlkitabKita / Everyone's Translation.
      if (bible == filter::indonesian::ourtranslation ()) return true;
      // 2. Terjemahanku <user> (My Translation <user>).
      else if (bible == filter::indonesian::mytranslation (user)) return true;
      // If the Bible is none of the above, the free guest account does not have access to it.
      else return false;
    }
  }

  // Managers and higher have read access.
  if (role_level >= Filter_Roles::manager ()) {
    return true;
  }

  // Read privileges for the user.
  auto [ read, write ] = Database_Privileges::getBible (user, bible);
  if (read) {
    return true;
  }

  // No Bibles assigned: Consultant can view any Bible.
  if (role_level >= Filter_Roles::consultant ()) {
    if (int privileges_count = Database_Privileges::getBibleBookCount (); privileges_count == 0) {
      return true;
    }
  }
#endif
  
  // Default.
  return false;
}


// Returns true if the user has write access to the $bible.
bool AccessBible::Write (void * webserver_request, const string & bible, string user)
{
#ifdef HAVE_CLIENT
  // Client: When not yet connected to the Cloud, the user has access to all Bibles.
  // When connected to the Cloud, this no longer applies,
  // since the client now receives the privileges from the Cloud.
  if (!client_logic_client_enabled ()) {
    return true;
  }
#endif
  // Indonesian Cloud Free: Access all Bibles.
  if (config_logic_indonesian_cloud_free ()) {
    return true;
  }

  int level = 0;
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
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
  auto [ read, write ] = Database_Privileges::getBible (user, bible);
  if (write) {
    return true;
  }
  
  // No Bibles assigned: Translator can write to any bible.
  if (level >= Filter_Roles::translator ()) {
    if (int privileges_count = Database_Privileges::getBibleBookCount (); privileges_count == 0) {
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
bool AccessBible::BookWrite (void * webserver_request, string user, const string & bible, int book)
{
#ifdef HAVE_CLIENT
  // Client: When not yet connected to the Cloud, the user has access to the book.
  // When connected to the Cloud, this no longer applies,
  // since the client now receives the privileges from the Cloud.
  if (!client_logic_client_enabled ()) {
    return true;
  }
#endif

  // Get the user level (role).
  int level = 0;
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  if (user.empty ()) {
    user = request->session_logic ()->currentUser ();
    level = request->session_logic ()->currentLevel ();
  }
  if (level == 0) {
    // Take level belonging to user.
    level = request->database_users ()->get_level (user);
  }

  // Indonesian Cloud Free.
  // The free guest account has a role of Consultant.
  // The consultant has write access to his/her own Bible.
  // The consultant has read-access to the "AlkitabKita" Bible.
  if (config_logic_indonesian_cloud_free ()) {
    if (level == Filter_Roles::consultant()) {
      if (bible == filter::indonesian::ourtranslation()) {
        return false;
      }
    }
    return true;
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
    if (int privileges_count = Database_Privileges::getBibleBookCount (); privileges_count == 0) {
      return true;
    }
  }
  
  // Default.
  return false;
}


// Returns an array of Bibles the user has read access to.
// If no user is given, it takes the currently logged-in user.
vector <string> AccessBible::Bibles (void * webserver_request, string user)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <string> allbibles = request->database_bibles ()->getBibles ();
  vector <string> bibles;
  for (auto & bible : allbibles) {
    if (AccessBible::Read (webserver_request, bible, user)) {
      bibles.push_back (bible);
    }
  }
  return bibles;
}


// This function clamps bible.
// It returns the $bible if the currently logged-in user has access to it.
// Else it returns another accessible bible or nothing.
string AccessBible::Clamp (void * webserver_request, string bible)
{
  if (!AccessBible::Read (webserver_request, bible)) {
    bible = string();
    vector <string> bibles = AccessBible::Bibles (webserver_request);
    if (!bibles.empty ()) bible = bibles [0];
    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
    request->database_config_user ()->setBible (bible);
  }
  return bible;
}


// This function checks whether the user in the $webserver_request
// has $read or $write access to one or more Bibles.
// It returns a tuple <read, write>.
tuple<bool, bool> AccessBible::Any (void * webserver_request)
{
  bool read = false;
  bool write = false;
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <string> bibles = request->database_bibles ()->getBibles ();
  for (auto & bible : bibles) {
    if (AccessBible::Read (webserver_request, bible)) read = true;
    if (AccessBible::Write (webserver_request, bible)) write = true;
  }
  if (config_logic_indonesian_cloud_free ()) {
    if (int level = request->session_logic ()->currentLevel ();
        level >= Filter_Roles::consultant())
    {
      read = true;
      write = true;
    } else {
      read = false;
      write = false;
    }
  }
  // The results consists of <read, write>.
  return make_tuple(read, write);
}



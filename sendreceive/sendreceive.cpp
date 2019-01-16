/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#include <sendreceive/sendreceive.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/git.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <webserver/request.h>
#include <locale/translate.h>


string sendreceive_tag ()
{
  return "Git repository: ";
}


void sendreceive_sendreceive (string bible)
{
#ifdef HAVE_CLIENT
  (void) bible;
#endif
#ifdef HAVE_CLOUD
  // Check on Bible.
  if (bible.empty ()) {
    Database_Logs::log (sendreceive_tag () + "No Bible to send and receive", Filter_Roles::translator ());
    return;
  }
  

  // The git repository directory for this object.
  string directory = filter_git_directory (bible);
  
  
  bool read_from_git = Database_Config_Bible::getReadFromGit (bible);
  

  // Check that the repository directory is there.
  if (!file_or_dir_exists (directory)) {
    string msg = "Cannot send ";
    if (read_from_git) msg.append ("and receive ");
    msg.append ("because the local git repository was not found.");
    Database_Logs::log (sendreceive_tag () + msg);
    return;
  }
  
  
  if (read_from_git) {
    Database_Logs::log (sendreceive_tag () + sendreceive_sendreceive_sendreceive_text () + bible, Filter_Roles::translator ());
  } else {
    Database_Logs::log (sendreceive_tag () + sendreceive_sendreceive_send_text () + bible, Filter_Roles::translator ());
  }
  Webserver_Request request;
  bool success = true;
  string error;
  
  
  // Configure the repository to prevent errors, just to be sure.
  filter_git_config (directory);

  
  // Separate commits for each user to the local git repository.
  filter_git_sync_modifications_to_git (bible, directory);

  
  // Synchronize the Bible from the database to the local git repository.
  filter_git_sync_bible_to_git (&request, bible, directory);
  

  // Log the status of the repository: "git status".
  // Set a flag indicating whether there are local changes available.
  bool localchanges = false;
  if (success) {
    vector <string> lines = filter_git_status (directory);
    for (auto & line : lines) {
      Passage passage = filter_git_get_passage (line);
      if (passage.book) {
        Database_Logs::log (sendreceive_tag () + line, Filter_Roles::translator ());
        localchanges = true;
      }
    }
  }
  
  
  // In case of local changes, add / remove the updated data to the index.
  if (success && localchanges) {
    success = filter_git_add_remove_all (directory, error);
    if (!success) {
      Database_Logs::log (sendreceive_tag () + error, Filter_Roles::translator ());
    }
  }
  

  // In case of local changes, commit the index to the repository.
  if (success && localchanges) {
    vector <string> messages;
    success = filter_git_commit (directory, "", translate ("Changes made in Bibledit"), messages, error);
    if (!success) {
      Database_Logs::log (sendreceive_tag () + error, Filter_Roles::translator ());
    }
  }

  
  // Pull changes from the remote repository.
  // Record the pull messages to see which chapter has changes.
  // Record the conflicting passages, to see which chapters to update.
  vector <string> pull_messages;
  vector <string> paths_resolved_conflicts;
  if (success) {
    vector <string> logs;
    bool conflict = false;
    success = filter_git_pull (directory, pull_messages);
    for (auto & line : pull_messages) {
      logs.push_back (line);
      if (line.find ("CONFLICT") != string::npos) conflict = true;
      if (line.find ("MERGE_HEAD") != string::npos) conflict = true;
    }
    if (!success || conflict || logs.size () > 1) {
      for (auto & log : logs) {
        if (log.find ("Updating") == 0) continue;
        if (log.find ("Fast-forward") == 0) continue;
        if (log.find ("file changed") != string::npos) continue;
        if (log.find ("From ") == 0) continue;
        if (log.find ("origin/master") != string::npos) continue;
        Database_Logs::log (sendreceive_tag () + "receive: " + log, Filter_Roles::translator ());
      }
    }
    if (conflict) {
      Database_Logs::log (sendreceive_tag () + translate ("Bibledit will resolve the conflicts"), Filter_Roles::translator ());
      filter_git_resolve_conflicts (directory, paths_resolved_conflicts, error);
      if (!error.empty ()) Database_Logs::log (error, Filter_Roles::translator ());
      vector <string> messages;
      string error;
      filter_git_commit (directory, "", translate ("Bibledit resolved the conflicts"), messages, error);
      for (auto & msg : messages) Database_Logs::log (sendreceive_tag () + "conflict resolution: " + msg, Filter_Roles::translator ());
      // The above "git pull" operation failed due to the conflict(s).
      // Since the conflicts have now been resolved, set "success" to true again.
      // This enables the subsequent git operations to run successfully.
      success = true;
    }
  }
  
  
  // Push any local changes to the remote repository.
  // Or changes due to automatic merge and/or conflict resolution.
  if (success) {
    vector <string> messages;
    success = filter_git_push (directory, messages);
    if (!success || messages.size() > 1) {
      for (auto & msg : messages) Database_Logs::log (sendreceive_tag () + "send: " + msg, Filter_Roles::translator ());
    }
  }
  
  
  // Record the changes from the collaborators into the Bible database.
  // The changes will be taken from the standard "git pull" messages,
  // plus the paths of the files that have resolved conflicts.
  // This is normally off due to some race conditions that came up now and then,
  // where a change made by a user was committed was reverted by the system.
  // So having it off by default is the safest thing one can do.
  if (Database_Config_Bible::getReadFromGit (bible)) {
    if (success) {
      pull_messages.insert (pull_messages.end (), paths_resolved_conflicts.begin (), paths_resolved_conflicts.end());
      for (auto & pull_message : pull_messages) {
        Passage passage = filter_git_get_passage (pull_message);
        if (passage.book) {
          int book = passage.book;
          int chapter = passage.chapter;
          filter_git_sync_git_chapter_to_bible (directory, bible, book, chapter);
        }
      }
    }
  }

  
  // Done.
  if (!success) {
    string msg = "Failure during sending";
    if (read_from_git) msg.append ("and receiving");
    Database_Logs::log (sendreceive_tag () + msg, Filter_Roles::translator ());
  }
  {
    string msg;
    if (read_from_git) {
      msg = sendreceive_sendreceive_sendreceive_ready_text ();
    } else {
      msg = sendreceive_sendreceive_send_ready_text ();
    }
    Database_Logs::log (sendreceive_tag () + msg + " " + bible, Filter_Roles::translator ());
  }
#endif
}


string sendreceive_sendreceive_sendreceive_text ()
{
  return "Send/receive Bible ";
}


string sendreceive_sendreceive_send_text ()
{
  return "Send Bible ";
}


string sendreceive_sendreceive_sendreceive_ready_text ()
{
  return "Ready sending and receiving Bible";
}


string sendreceive_sendreceive_send_ready_text ()
{
  return "Ready sending Bible";
}

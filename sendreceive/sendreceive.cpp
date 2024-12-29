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


#include <sendreceive/sendreceive.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/git.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <webserver/request.h>
#include <locale/translate.h>


std::string sendreceive_tag ()
{
  return "Git repository: ";
}


void sendreceive_sendreceive ([[maybe_unused]] std::string bible)
{
#ifdef HAVE_CLOUD
  // Check on Bible.
  if (bible.empty ()) {
    Database_Logs::log (sendreceive_tag () + "No Bible to send and receive", Filter_Roles::translator ());
    return;
  }
  

  // The git repository directory for this object.
  std::string directory = filter_git_directory (bible);
  
  
  bool read_from_git = database::config::bible::get_read_from_git (bible);
  

  // Check that the repository directory is there.
  if (!file_or_dir_exists (directory)) {
    std::string msg = "Cannot send ";
    if (read_from_git) msg.append ("and receive ");
    msg.append ("because the local git repository was not found.");
    Database_Logs::log (sendreceive_tag () + msg);
    return;
  }
  
  
  // Log what this is going to do.
  if (read_from_git) {
    Database_Logs::log (sendreceive_tag () + sendreceive_sendreceive_sendreceive_text () + bible, Filter_Roles::translator ());
  } else {
    Database_Logs::log (sendreceive_tag () + sendreceive_sendreceive_send_text () + bible, Filter_Roles::translator ());
  }
  
  
  Webserver_Request webserver_request;
  bool success = true;
  std::string error;
  
  
  // Configure the repository to prevent errors, just to be sure.
  filter_git_config (directory);

  
  // Separate commits for each user to the local git repository.
  filter_git_sync_modifications_to_git (bible, directory);

  
  // Synchronize the Bible from the database to the local git repository.
  filter_git_sync_bible_to_git (bible, directory);
  

  // Log the status of the repository: "git status".
  // Set a flag indicating whether there are local changes available.
  bool localchanges = false;
  if (success) {
    std::vector <std::string> lines = filter_git_status (directory, true);
    for (auto & line : lines) {
      Passage passage = filter_git_get_passage (line);
      if (passage.m_book) {
        Database_Logs::log (sendreceive_tag () + line, Filter_Roles::translator ());
        localchanges = true;
      }
    }
  }
  
  
  // In case of local changes, add/remove the updated data to/from the index.
  if (success && localchanges) {
    success = filter_git_add_remove_all (directory, error);
    if (!success) {
      Database_Logs::log (sendreceive_tag () + error, Filter_Roles::translator ());
    }
  }
  

  // In case of local changes, commit the index to the repository.
  if (success && localchanges) {
    std::vector <std::string> messages;
    success = filter_git_commit (directory, "", translate ("Changes made in Bibledit"), messages, error);
    if (!success) {
      Database_Logs::log (sendreceive_tag () + error, Filter_Roles::translator ());
    }
  }

  
  // Pull changes from the remote repository.
  // Record the pull messages to see which chapter has changes.
  // Record the conflicting passages, to see which chapters to update.
  std::vector <std::string> pull_messages;
  std::vector <std::string> paths_resolved_conflicts;
  if (success) {
    std::vector <std::string> logs;
    bool conflict = false;
    success = filter_git_pull (directory, pull_messages);
    for (auto & line : pull_messages) {
      logs.push_back (line);
      if (line.find ("CONFLICT") != std::string::npos) conflict = true;
      if (line.find ("MERGE_HEAD") != std::string::npos) conflict = true;
    }
    if (!success || conflict || logs.size () > 1) {
      for (auto & log : logs) {
        if (log.find ("Updating") == 0) continue;
        if (log.find ("Fast-forward") == 0) continue;
        if (log.find ("file changed") != std::string::npos) continue;
        if (log.find ("From ") == 0) continue;
        if (log.find ("origin/master") != std::string::npos) continue;
        Database_Logs::log (sendreceive_tag () + "receive: " + log, Filter_Roles::translator ());
      }
    }
    if (conflict) {
      Database_Logs::log (sendreceive_tag () + translate ("Bibledit will resolve the conflicts"), Filter_Roles::translator ());
      filter_git_resolve_conflicts (directory, paths_resolved_conflicts, error);
      if (!error.empty ()) Database_Logs::log (error, Filter_Roles::translator ());
      std::vector <std::string> messages;
      std::string tmp_error;
      std::string no_user {};
      filter_git_commit (directory, no_user, translate ("Bibledit resolved the conflicts"), messages, tmp_error);
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
    std::vector <std::string> messages;
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
  if (database::config::bible::get_read_from_git (bible)) {
    if (success) {
      pull_messages.insert (pull_messages.end (), paths_resolved_conflicts.begin (), paths_resolved_conflicts.end());
      for (auto & pull_message : pull_messages) {
        Passage passage = filter_git_get_passage (pull_message);
        if (passage.m_book) {
          int book = passage.m_book;
          int chapter = passage.m_chapter;
          filter_git_sync_git_chapter_to_bible (directory, bible, book, chapter);
        }
      }
    }
  }

  
  // Done.
  if (!success) {
    std::string msg = "Failure during sending";
    if (read_from_git) msg.append ("and receiving");
    Database_Logs::log (sendreceive_tag () + msg, Filter_Roles::translator ());
  }
  {
    std::string msg;
    if (read_from_git) {
      msg = sendreceive_sendreceive_sendreceive_ready_text ();
    } else {
      msg = sendreceive_sendreceive_send_ready_text ();
    }
    Database_Logs::log (sendreceive_tag () + msg + " " + bible, Filter_Roles::translator ());
  }
#endif
}


std::string sendreceive_sendreceive_sendreceive_text ()
{
  return "Send/receive Bible ";
}


std::string sendreceive_sendreceive_send_text ()
{
  return "Send Bible ";
}


std::string sendreceive_sendreceive_sendreceive_ready_text ()
{
  return "Ready sending and receiving Bible";
}


std::string sendreceive_sendreceive_send_ready_text ()
{
  return "Ready sending Bible";
}

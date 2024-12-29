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


#include <sendreceive/logic.h>
#include <filter/url.h>
#include <filter/date.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <client/logic.h>
#include <config/globals.h>


void sendreceive_queue_bible (std::string bible)
{
  tasks_logic_queue (task::send_receive_bibles, {bible});
}


// If the $minute is negative, it syncs right now.
// If the $minute is zero or higher, it determines from the settings whether and what to sync.
void sendreceive_queue_sync (int minute, int second)
{
  // The flags for which data to synchronize now.
  bool sync_bibles = false;
  bool sync_rest = false;
  
  // Deal with a numerical minute to find out whether it's time to automatically sync.
  if (minute >= 0) {

    int repeat = database::config::general::get_repeat_send_receive ();

    // Sync everything every hour.
    if ((repeat == 1) && (second == 0)) {
      minute = minute % 60;
      if (minute == 0) {
        sync_bibles = true;
        sync_rest = true;
      }
    }

    // Sync everything every five minutes.
    if ((repeat == 2) || (repeat == 3)) {
      if (second == 0) {
        minute = minute % 5;
        if (minute == 0) {
          sync_bibles = true;
          sync_rest = true;
        }
      }
    }

  }

  
  // Send and receive manually.
  if (minute < 0) {
    sync_bibles = true;
    sync_rest = true;
  }
  
  
  // Send / receive only works in Client mode.
  if (client_logic_client_enabled ()) {

    // Only queue a sync task if it is not running at the moment.
    if (sync_bibles) {
      if (tasks_logic_queued (task::sync_bibles)) {
        Database_Logs::log ("About to start synchronizing Bibles");
      } else {
        tasks_logic_queue (task::sync_bibles);
      }
    }
    if (sync_rest) {
      if (tasks_logic_queued (task::sync_notes)) {
        Database_Logs::log ("About to start synchronizing Notes");
      } else {
        tasks_logic_queue (task::sync_notes);
      }
      if (tasks_logic_queued (task::sync_settings)) {
        Database_Logs::log ("About to start synchronizing Settings");
      } else {
        tasks_logic_queue (task::sync_settings);
      }
      if (tasks_logic_queued (task::sync_changes)) {
        Database_Logs::log ("About to start synchronizing Changes");
      } else {
        tasks_logic_queue (task::sync_changes);
      }
      if (tasks_logic_queued (task::sync_files)) {
        Database_Logs::log ("About to start synchronizing Files");
      } else {
        tasks_logic_queue (task::sync_files);
      }
      // Sync resources always, because it checks on its own whether to do something.
      tasks_logic_queue (task::sync_resources);
    }

    if (sync_bibles || sync_rest) {
      // Store the most recent time that the sync action ran.
      database::config::general::set_last_send_receive (filter::date::seconds_since_epoch ());
    }
  }
}


// Checks whether any of the sync tasks has been queued.
// Returns the result as a boolean.
bool sendreceive_sync_queued ()
{
  if (tasks_logic_queued (task::sync_notes)) return true;
  if (tasks_logic_queued (task::sync_bibles)) return true;
  if (tasks_logic_queued (task::sync_settings)) return true;
  if (tasks_logic_queued (task::sync_changes)) return true;
  if (tasks_logic_queued (task::sync_files)) return true;
  return false;
}


// Queues Paratext sync.
void sendreceive_queue_paratext (tasks::enums::paratext_sync method)
{
#ifdef HAVE_PARATEXT
  if (sendreceive_paratext_queued ()) {
    Database_Logs::log ("About to start synchronizing with Paratext");
  } else {
    tasks_logic_queue (task::sync_paratext, { std::to_string(static_cast<int>(method)) });
  }
#endif
  (void) method;
}


// Checks whether the Paratext sync task has been queued.
// Returns the result as a boolean.
bool sendreceive_paratext_queued ()
{
  if (tasks_logic_queued (task::sync_paratext)) return true;
  return false;
}


void sendreceive_queue_all (bool now)
{
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  for (auto & bible : bibles) {
    if (database::config::bible::get_remote_repository_url (bible) != "") {
      if (database::config::bible::get_repeat_send_receive (bible) || now) {
        sendreceive_queue_bible (bible);
      }
    }
  }
}


// Function that looks if, at app startup, it needs to queue sync operations in the client.
void sendreceive_queue_startup ()
{
  // Next second when it is supposed to sync.
  int next_second = database::config::general::get_last_send_receive ();

  // Check how often to repeat the sync action.
  int repeat = database::config::general::get_repeat_send_receive ();
  if (repeat == 1) {
    // Repeat every hour.
    next_second += 3600;
  }
  else if (repeat == 2) {
    // Repeat every five minutes.
    next_second += 300;
  } else {
    // No repetition.
    return;
  }
  
  // When the current time is past the next time it is supposed to sync, start the sync.
  if (filter::date::seconds_since_epoch () >= next_second) {
    sendreceive_queue_sync (-1, 0);
  }
}


// Returns true if any prioritized task is now active.
bool sendreceive_logic_prioritized_task_is_active ()
{
  if (   config_globals_syncing_bibles
      || config_globals_syncing_notes
      || config_globals_syncing_settings
      || config_globals_syncing_changes) {
    return true;
  }
  return false;
}


// Returns true if Bibledit Cloud has been linked to an external git repository.
bool sendreceive_git_repository_linked (std::string bible)
{
  std::string url = database::config::bible::get_remote_repository_url (bible);
  return !url.empty ();
}

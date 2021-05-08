/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <timer/index.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/state.h>
#include <config/globals.h>
#include <filter/string.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <tasks/run.h>
#include <sendreceive/logic.h>
#include <client/logic.h>
#include <changes/logic.h>
#include <checks/logic.h>
#include <export/logic.h>
#include <developer/logic.h>
#include <setup/logic.h>
#include <journal/logic.h>


// CPU-intensive actions run at night.
// This keeps the site more responsive during the day.


// The order in which the following nightly scripts run is important.
// Any of those scripts may influence the subsequent ones.
// The order is such that all information generated is as recent as possible.
// More important tasks are done first, and the less important ones at the end.
// This leads to an order as visible in the code below.


void timer_index ()
{
  int previous_second = -1;
  int previous_minute = -1;
  int previous_fraction = -1;
  while (config_globals_webserver_running) {

    try {

      // Wait shortly.
      this_thread::sleep_for (chrono::milliseconds (100));
      
      // Wait till the data structures have been initialized.
      setup_wait_till_main_folders_present ();
      if (!config_globals_data_initialized) continue;
      
      // The current time, localized.
      int local_seconds = filter_date_local_seconds (filter_date_seconds_since_epoch ());
      int second = filter_date_numerical_second (local_seconds);
      int minute = filter_date_numerical_minute (local_seconds);
      int hour = filter_date_numerical_hour (local_seconds);
      int weekday = filter_date_numerical_week_day (local_seconds);
      
      // Run once per second.
      if (second == previous_second) continue;
      previous_second = second;

      // Every second: Deal with queued and/or active tasks.
      tasks_run_check ();
      
      // Every second:
      // Check whether client sends/receives Bibles and Consultation Notes and other stuff.
      sendreceive_queue_sync (minute, second);
      // Log any connections that have come in.
      developer_logic_log_network_write ();

      // Run the part below every so many seconds.
      int fraction = second / 5;
      if (fraction != previous_fraction) {
        previous_fraction = fraction;
      }
      
      // Run the part below once per minute.
      if (minute == previous_minute) continue;
      previous_minute = minute;

      // Every minute send out queued email.
      tasks_logic_queue (SENDEMAIL);

#ifdef HAVE_CLOUD
      // Check for new mail every five minutes.
      // Do not check more often with gmail else the account may be shut down.
      if ((minute % 5) == 0) {
        tasks_logic_queue (RECEIVEEMAIL);
      }
#endif

      // At the ninth minute after every full hour rotate the journal.
      // The nine is chosen, because the journal rotation will summarize the send/receive messages.
      // In case send/receive happens every five minutes, it is expected that under normal circumstances
      // the whole process of sending/receivning will be over, so summarization can then be done.
      if (minute == 9) tasks_logic_queue (ROTATEJOURNAL);
      
      // Sending and receiving Bibles to and from the git repository.
      // On a production website running on an inexpensive virtual private server,
      // with 512 Mbyte of memory and a fast network connection,
      // sending and receiving two Bibles takes more than 15 minutes when there are many changes.
      bool sendreceive = ((hour == 0) && (minute == 5));
      bool repeat = ((minute % 5) == 0);
      if (sendreceive || repeat) {
        sendreceive_queue_all (sendreceive);
      }
      
#ifdef HAVE_CLOUD
      // Deal with the changes in the Bible made per user.
      // Deal with notifications for the daily changes in the Bibles.
      // This takes a few minutes on a production machine with two Bibles and changes in several chapters.
      // It runs in a server configuration, not on a client.
      if ((hour == 0) && (minute == 20)) {
        changes_logic_start ();
      }
#endif

#ifdef HAVE_CLOUD
      // Run the checks on the Bibles.
      // This takes 15 minutes on a production machine with two Bibles.
      if ((hour == 0) && (minute == 30)) {
        checks_logic_start_all ();
      }
#endif
      
      // Database maintenance and trimming.
      // It takes a few minutes on a production machine.
      if ((hour == 0) && (minute == 50)) {
        tasks_logic_queue (MAINTAINDATABASE);
      }

#ifdef HAVE_CLOUD
      // File cache trimming.
      // https://github.com/bibledit/cloud/issues/364
      // This used to be done once a day, and the trimming left files for multiple days.
      // This way of doing things led to a full disk when disk space was tight.
      // The new way of trimming on the Cloud is to do the trimming every hour.
      // And to leave files in the files cache for only a couple of hours.
      if (minute == 10) {
        tasks_logic_queue (TRIMCACHES);
      }

#endif

#ifdef HAVE_CLOUD
      // Export the Bibles to the various output formats.
      // This may take an hour on a production machine.
      // This hour was in PHP. In C++ it is much faster.
      if ((hour == 1) && (minute == 10)) {
        Export_Logic::scheduleAll ();
      }
#endif
      
      // Delete expired temporal files.
      if ((hour == 2) && (minute == 0)) {
        tasks_logic_queue (CLEANTMPFILES);
      }
      
      // Re-index Bibles and notes.
      // Only update missing indexes.
      if ((hour == 2) && (minute == 0)) {
        Database_State::create ();
        Database_Config_General::setIndexBibles (true);
        tasks_logic_queue (REINDEXBIBLES);
        Database_Config_General::setIndexNotes (true);
        tasks_logic_queue (REINDEXNOTES);
      }
      
      // Actions for a demo installation.
      if (minute == 10) {
        if (config_logic_demo_enabled ()) {
          tasks_logic_queue (CLEANDEMO);
        }
      }
      
#ifdef HAVE_CLOUD
      // Sprint burndown.
      // It runs every hour in the Cloud.
      // The script itself determines what to do at which hour of the day or day of the week or day of the month.
      if (minute == 5) {
        tasks_logic_queue (SPRINTBURNDOWN);
      }
#endif

#ifdef HAVE_CLOUD
      // Bibledit Cloud quits at midnight.
      // This is to be sure that any memory leaks don't accumulate too much
      // in case Bibledit Cloud would run for months and years.
      // The shell script notices that the binary has quit, and restarts the binary again.
      if (hour == 0) {
        if (minute == 1) {
          if (!Database_Config_General::getJustStarted ()) {
            if (tasks_run_active_count ()) {
              Database_Logs::log ("Server is due to restart itself but does not because of active jobs");
            } else {
              Database_Logs::log ("Server restarts itself");
              exit (0);
            }
          }
        }
        // Clear flag in preparation of restart next minute.
        // This flag also has the purpose of ensuring the server restarts once during that minute,
        // rather than restarting repeatedly many times during that minute.
        if (minute == 0) {
          Database_Config_General::setJustStarted (false);
        }
      }
#endif
      
#ifdef HAVE_CLOUD
      // Email notes statistics to the users.
      if ((hour == 3) && (minute == 0)) {
        tasks_logic_queue (NOTESSTATISTICS);
      }
#endif

      
#ifdef HAVE_CLOUD
      // In the free Indonesian Cloud configuration, expire free users after so many days.
      if (config_logic_indonesian_cloud_free ()) {
        if ((hour == 3) && (minute == 10)) {
          tasks_logic_queue (EXPIREINDONESIANFREEUSERS);
        }
      }
#endif

	
#ifdef HAVE_CLOUD
      // Update SWORD modules and other web resources once a week.
      if (weekday == 1) {
        // Refresh.
        if ((hour == 3) && (minute == 5)) {
          tasks_logic_queue (REFRESHSWORDMODULES);
          tasks_logic_queue (REFRESHWEBRESOURCES);
        }
        // Update installed SWORD modules, shortly after the module list has been refreshed.
        if ((hour == 3) && (minute == 15)) {
          tasks_logic_queue (UPDATESWORDMODULES);
        }
      }
#endif

#ifdef HAVE_CLOUD
      // The Cloud updates the list of USFM resources once a week.
      if (weekday == 1) {
        if ((hour == 3) && (minute == 10)) {
          tasks_logic_queue (LISTUSFMRESOURCES);
        }
      }
#endif

      // Suppress compiler warning on client.
      (void) weekday;
      
    } catch (exception & e) {
      Database_Logs::log (e.what ());
    } catch (exception * e) {
      Database_Logs::log (e->what ());
    } catch (...) {
      Database_Logs::log ("A general internal error occurred in the timers");
    }
  }
}

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


#include <search/renotes.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <database/logs.h>
#include <webserver/request.h>
#include <database/notes.h>
#include <database/state.h>
#include <database/config/general.h>


static bool search_reindex_notes_running = false;


void search_reindex_notes ()
{
  if (!database::config::general::getIndexNotes ()) return;
  
  
  // One simultaneous instance.
  if (search_reindex_notes_running) {
    Database_Logs::log ("Still indexing Consultation Notes", roles::manager);
    return;
  }
  search_reindex_notes_running = true;
  
  
  // This does not run as a result of a webserver request, so create one.
  Webserver_Request webserver_request;


  // Database.
  Database_Notes database_notes (webserver_request);


  // Set a flag indicating that the notes databases are not available to clients and other parties.
  database_notes.set_availability (false);
  
  
  // Delay shortly to give existing processes a change to complete.
  // Without this delay, "locked database" errors have been seen.
  std::this_thread::sleep_for (std::chrono::seconds (1));


  // Check on health of the databases, and optionally recreate them.
  bool recreate = database_notes.checkup ();
  if (recreate) {
    Database_Logs::log ("The Consultation Notes main index was damaged and is being recreated", roles::manager);
  }
  recreate = database_notes.checkup_checksums ();
  if (recreate) {
    Database_Logs::log ("The Consultation Notes checksums database was damaged and is being recreated", roles::manager);
  }
  
  
  // Synchronize the notes databases with the notes on the filesystem,
  // because it may have happened that certain values at certain times 
  // could not be written to the database because of errors. 
  // Then the notes index would not match the notes data.
  // Syncing ensures the index matches the notes data in the filesystem.
  Database_Logs::log ("Updating Consultation Notes databases", roles::manager);
  database_notes.sync ();
  
  
  // Set the availability flag so that clients and other parties access the notes databases again.
  database_notes.set_availability (true);
  
  
  Database_Logs::log ("Updating Consultation Notes databases ready", roles::manager);
  database::config::general::setIndexNotes (false);
  search_reindex_notes_running = false;
}

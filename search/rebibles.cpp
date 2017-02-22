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


#include <search/rebibles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/config/general.h>
#include <search/logic.h>


bool search_reindex_bibles_running = false;


void search_reindex_bibles (bool force)
{
  if (!Database_Config_General::getIndexBibles ()) return;
  
  
  // One simultaneous instance.
  if (search_reindex_bibles_running) {
    Database_Logs::log ("Still indexing Bibles", Filter_Roles::manager ());
    return;
  }
  search_reindex_bibles_running = true;
  
  
  // This checks whether the data in the search index exists for all chapters in all Bibles.
  // If it does not exist for a certain chapter, the index will be created.
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibles.getBibles ();
  for (auto & bible : bibles) {
    Database_Logs::log ("Indexing Bible: Checking " + bible, Filter_Roles::manager ());
    vector <int> books = database_bibles.getBooks (bible);
    for (auto book : books) {
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        string index = search_logic_chapter_file (bible, book, chapter);
        if (!file_or_dir_exists (index) || force) {
          string msg = "Indexing Bible: " + bible + " " + filter_passage_display (book, chapter, "");
          Database_Logs::log (msg, Filter_Roles::manager ());
          search_logic_index_chapter (bible, book, chapter);
        }
      }
    }
  }
  
  
  Database_Logs::log ("Indexing Bible: Ready", Filter_Roles::manager ());
  Database_Config_General::setIndexBibles (false);
  search_reindex_bibles_running = false;
}

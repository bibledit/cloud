/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#include <export/index.h>
#include <export/web.h>
#include <export/logic.h>
#include <database/bibles.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>


void export_index ()
{
  Database_Bibles database_bibles;
  std::vector <std::string> bibles = database_bibles.get_bibles ();
  
  
  // Go through all sub directories of the exports directory.
  // Remove subdirectories if their corresponding Bible no longer exists in the system.
  std::string directory = export_logic::main_directory ();
  std::vector <std::string> files = filter_url_scandir (directory);
  for (auto & file : files) {
    if (in_array (file, bibles)) continue;
    filter_url_rmdir (filter_url_create_path ({directory, file}));
    Database_Logs::log ("Removing exported Bible " + file, Filter_Roles::translator ());
  }
  
  
  // Schedule the relevant Bibles for export.
  for (auto bible : bibles) {
    
    if (Database_State::getExport (bible, 0, export_logic::export_needed)) {
      
      Database_State::clearExport (bible, 0, export_logic::export_needed);
      std::vector <int> books = database_bibles.get_books (bible);
      // Book 0 flags export of the whole Bible (this is not relevant to all export types).
      books.push_back (0);
      for (auto book : books) {
        for (int format = export_logic::export_needed + 1; format < export_logic::export_end; format++) {
          Database_State::setExport (bible, book, format);
        }
      }

      Database_Logs::log ("Exporting Bible " + bible, Filter_Roles::translator ());

      if (Database_Config_Bible::getExportWebDuringNight (bible)) {
        export_logic::schedule_web (bible, false);
        export_logic::schedule_web_index (bible, false);
      }

      if (Database_Config_Bible::getExportHtmlDuringNight (bible)) {
        export_logic::schedule_html (bible, false);
      }
      
      if (Database_Config_Bible::getExportUsfmDuringNight (bible)) {
        export_logic::schedule_usfm (bible, false);
      }
      
      if (Database_Config_Bible::getExportTextDuringNight (bible)) {
        export_logic::schedule_text_and_basic_usfm (bible, false);
      }
      
      if (Database_Config_Bible::getExportOdtDuringNight (bible)) {
        export_logic::schedule_open_document (bible, false);
      }
      
      if (Database_Config_Bible::getGenerateInfoDuringNight (bible)) {
        export_logic::schedule_info (bible, false);
      }
      
      if (Database_Config_Bible::getExportESwordDuringNight (bible)) {
        export_logic::schedule_e_sword (bible, false);
      }

      if (Database_Config_Bible::getExportOnlineBibleDuringNight (bible)) {
        export_logic::schedule_online_bible (bible, false);
      }
      
    }
  }
}

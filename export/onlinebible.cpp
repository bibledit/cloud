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


#include <export/onlinebible.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <onlinebible/text.h>


void export_onlinebible (string bible, bool log)
{
  string directory = filter_url_create_path (Export_Logic::bibleDirectory (bible), "onlinebible");
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  string filename = filter_url_create_path (directory, "bible.exp");

  
  Database_Bibles database_bibles;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  Filter_Text filter_text_bible = Filter_Text (bible);
  filter_text_bible.onlinebible_text = new OnlineBible_Text ();
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string chapter_data = database_bibles.getChapter (bible, book, chapter);
      chapter_data = filter_string_trim (chapter_data);
      filter_text_bible.addUsfmCode (chapter_data);
    }
  }
  filter_text_bible.run (stylesheet);
  filter_text_bible.onlinebible_text->save (filename);

  
  Database_State::clearExport (bible, 0, Export_Logic::export_online_bible);

  
  if (log) Database_Logs::log (translate("Exported to Online Bible") + " " + bible, Filter_Roles::translator ());
}

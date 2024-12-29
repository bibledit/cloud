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


#include <export/esword.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/state.h>
#include <database/config/bible.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_esword (const std::string& bible, bool log)
{
  const std::string directory = filter_url_create_path ({export_logic::bible_directory (bible), "esword"});
  if (!file_or_dir_exists (directory))
    filter_url_mkdir (directory);
  
  
  const std::string filename = filter_url_create_path ({directory, "bible.bblx"});

  
  if (file_or_dir_exists (filename)) 
    filter_url_unlink (filename);
  
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  
  Filter_Text filter_text_bible = Filter_Text (bible);
  filter_text_bible.esword_text = new Esword_Text (bible);
  const std::vector <int> books = database::bibles::get_books (bible);
  for (const auto book : books) {
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (const auto chapter : chapters) {
      const std::string chapter_data = database::bibles::get_chapter (bible, book, chapter);
      filter_text_bible.add_usfm_code (chapter_data);
    }
  }
  filter_text_bible.run (stylesheet);
  filter_text_bible.esword_text->finalize ();
  filter_text_bible.esword_text->createModule (filename);

  
  Database_State::clearExport (bible, 0, export_logic::export_esword);

  
  if (log) 
    Database_Logs::log (translate("Exported to e-Sword") + " " + bible, Filter_Roles::translator ());
}

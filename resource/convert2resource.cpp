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


#include <resource/convert2resource.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <resource/logic.h>
#include <database/bibles.h>
#include <database/usfmresources.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/privileges.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <tasks/logic.h>


void convert_bible_to_resource (std::string bible)
{
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  
  
  Database_Logs::log (translate("Converting Bible to USFM Resource") + ": " + bible, Filter_Roles::manager ());
  
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto & book : books) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    Database_Logs::log (bookname, Filter_Roles::manager ());
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto & chapter : chapters) {
      std::string usfm = database::bibles::get_chapter (bible, book, chapter);
      database_usfmresources.storeChapter (bible, book, chapter, usfm);
      database::bibles::delete_chapter (bible, book, chapter);
    }
  }
  database::bibles::delete_bible (bible);
  DatabasePrivileges::remove_bible (bible);
  database::config::bible::remove (bible);
  
  
  // The Cloud updates the list of available USFM resources for the clients.
  tasks_logic_queue (LISTUSFMRESOURCES);

  
  Database_Logs::log (translate("Completed"), Filter_Roles::manager ());
}

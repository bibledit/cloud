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


#include <resource/convert2bible.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <resource/logic.h>
#include <database/bibles.h>
#include <database/usfmresources.h>
#include <database/books.h>
#include <database/logs.h>
#include <locale/translate.h>
#include <tasks/logic.h>


void convert_resource_to_bible (std::string resource)
{
  database::logs::log (translate("Converting USFM Resource to Bible") + ": " + resource);
  
  
  database::bibles::create_bible (resource);
  std::vector <int> books = database::usfm_resources::get_books (resource);
  for (auto & book : books) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    database::logs::log (bookname);
    std::vector <int> chapters = database::usfm_resources::get_chapters (resource, book);
    for (auto & chapter : chapters) {
      std::string usfm = database::usfm_resources::get_usfm (resource, book, chapter);
      database::bibles::store_chapter (resource, book, chapter, usfm);
    }
  }
  database::usfm_resources::delete_resource (resource);
  
  
  // The Cloud updates the list of available USFM resources for the clients.
  tasks_logic_queue (task::list_usfm_resources);

  
  database::logs::log (translate("Conversion completed"));
}

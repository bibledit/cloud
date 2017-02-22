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


void convert_resource_to_bible (string resource)
{
  Database_Bibles database_bibles;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  
  
  Database_Logs::log (translate("Converting USFM Resource to Bible") + ": " + resource);
  
  
  database_bibles.createBible (resource);
  vector <int> books = database_usfmresources.getBooks (resource);
  for (auto & book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    Database_Logs::log (bookname);
    vector <int> chapters = database_usfmresources.getChapters (resource, book);
    for (auto & chapter : chapters) {
      string usfm = database_usfmresources.getUsfm (resource, book, chapter);
      database_bibles.storeChapter (resource, book, chapter, usfm);
    }
  }
  database_usfmresources.deleteResource (resource);
  
  
  // The Cloud updates the list of available USFM resources for the clients.
  tasks_logic_queue (LISTUSFMRESOURCES);

  
  Database_Logs::log (translate("Conversion completed"));
}

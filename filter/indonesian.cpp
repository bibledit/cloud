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


#include <filter/indonesian.h>
#include <database/logs.h>
#include <database/books.h>
#include <database/bibles.h>
#include <search/logic.h>
#include <book/create.h>


string filter_indonesian_alkitabkita_ourtranslation_name ()
{
  return "AlkitabKita";
}


string filter_indonesian_terjemahanku_mytranslation_name (const string & user)
{
  string name = "Terjemahanku";
  if (!user.empty()) {
    name.append (" ");
    name.append (user);
  }
  return name;
}


void filter_indonesian_create_mytranslation (const string & user) // Todo
{
  // The name of the Bible to create for this user.
  string bible = filter_indonesian_terjemahanku_mytranslation_name (user);
  
  Database_Logs::log ("Creating My Translation " + user);
  
  // Remove and create the sample Bible.
  Database_Bibles database_bibles;
  database_bibles.deleteBible (bible);
  database_bibles.createBible (bible);
  
  // Remove index for the sample Bible.
  search_logic_delete_bible (bible);

  // In the Indonesian Cloud Free, the request is:
  // to create books with blank verses for the OT and NT.
  vector <int> books = Database_Books::getIDs ();
  for (auto book : books) {
    string type = Database_Books::getType (book);
    if ((type == "ot") || (type == "nt")) {
      vector <string> feedback;
      book_create (bible, book, -1, feedback);
    }
  }
  
  Database_Logs::log ("My Translation " + user + " was created");
}

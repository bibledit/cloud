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


#include <xrefs/interpret.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/abbreviations.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/volatile.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <ipc/focus.h>
#include <xrefs/translate.h>


string xrefs_interpret_url ()
{
  return "xrefs/interpret";
}


bool xrefs_interpret_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_interpret (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string bible = request->database_config_user()->getSourceXrefBible ();
  
  
  // Save abbreviation / book pair.
  if (request->post.count ("save")) {
    string abbreviation = request->post ["abbreviation"];
    string fullname = request->post ["fullname"];
    string abbreviations = Database_Config_Bible::getBookAbbreviations (bible);
    abbreviations = filter_abbreviations_add (abbreviations, fullname, abbreviation);
    Database_Config_Bible::setBookAbbreviations (bible, abbreviations);
  }
  
  
  // Storage identifier, based on the user"s name.
  int identifier = filter_string_user_identifier (webserver_request);
  
  
  // Retrieve all notes from the database.
  string s_allnotes = Database_Volatile::getValue (identifier, "sourcexrefs");
  vector <string> allnotes = filter_string_explode (s_allnotes, '\n');
  
  
  // Retrieve all abbreviations, sort them, longest first.
  // The replace routines replaces the longer strings first,
  // to be sure that no partial book abbreviation is replaced.
  string abbreviationstring = Database_Config_Bible::getBookAbbreviations (bible);
  vector <pair <int, string> > abbreviations = filter_abbreviations_read (abbreviationstring);
  abbreviations = filter_abbreviations_sort (abbreviations);
  

  vector <string> unknown_abbreviations;
  
  
  // Go through notes, do the replacement, collect unknown abbreviations.
  for (unsigned int i = 0; i < allnotes.size () - 2; i += 3) {
    string note = allnotes [i + 2];
    note = filter_string_str_replace ("\\x*", "", note);
    note = filter_string_str_replace ("\\x", "", note);
    for (auto & element : abbreviations) {
      note = filter_string_str_replace (element.second, "", note);
    }
    vector <string> v_note = filter_string_explode (note, ' ');
    for (auto fragment : v_note) {
      if (fragment.length () <= 1) continue;
      if (convert_to_int (fragment) > 0) continue;
      unknown_abbreviations.push_back (fragment);
    }
  }
  
  
  if (unknown_abbreviations.empty ()) {
    redirect_browser (request, xrefs_translate_url ());
    return "";
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  page = header.run ();
  Assets_View view;
  
  
  view.set_variable ("abbreviation", unknown_abbreviations [0]);
  

  string booksblock;
  vector <int> books = Database_Books::getIDs ();
  for (auto book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    booksblock.append ("<option value=\"" + bookname + "\">" + bookname + "</option>\n");
  }
  view.set_variable ("booksblock", booksblock);

  
  unknown_abbreviations = array_unique (unknown_abbreviations);
  view.set_variable ("remaining", convert_to_string (unknown_abbreviations.size() - 1));
  
  
  page += view.render ("xrefs", "interpret");
  page += Assets_Page::footer ();
  return page;
}

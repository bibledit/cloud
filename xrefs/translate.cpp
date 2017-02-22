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


#include <xrefs/translate.h>
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
#include <database/books.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <ipc/focus.h>
#include <xrefs/clear.h>


string xrefs_translate_url ()
{
  return "xrefs/translate";
}


bool xrefs_translate_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_translate (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string sourceBible = request->database_config_user()->getSourceXrefBible ();
  string targetBible = request->database_config_user()->getTargetXrefBible ();
  
  
  // Save book / abbreviation pair.
  if (request->post.count ("save")) {
    string fullname = request->post ["fullname"];
    string abbreviation = request->post ["abbreviation"];
    string abbreviations = Database_Config_Bible::getBookAbbreviations (targetBible);
    abbreviations = filter_abbreviations_add (abbreviations, fullname, abbreviation);
    Database_Config_Bible::setBookAbbreviations (targetBible, abbreviations);
  }
  
  
  string abbrevs = Database_Config_Bible::getBookAbbreviations (sourceBible);
  vector <pair <int, string> > sourceAbbreviations = filter_abbreviations_read (abbrevs);
  vector <int> sourceBooks;
  for (auto element : sourceAbbreviations) sourceBooks.push_back (element.first);

  
  abbrevs = Database_Config_Bible::getBookAbbreviations (targetBible);
  vector <pair <int, string> > targetAbbreviations = filter_abbreviations_read (abbrevs);
  vector <int> targetBooks;
  for (auto element : targetAbbreviations) targetBooks.push_back (element.first);
  
  
  vector <int> unknown_books = filter_string_array_diff (sourceBooks, targetBooks);
  unknown_books = array_unique (unknown_books);
  
  
  if (unknown_books.empty ()) {
    redirect_browser (request, xrefs_clear_url ());
    return "";
  }


  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  page = header.run ();
  Assets_View view;
  
  
  view.set_variable ("remaining", convert_to_string (unknown_books.size () - 1));
                      
  
  string bookname = Database_Books::getEnglishFromId (unknown_books [0]);
  view.set_variable ("bookname", bookname);
  
  
  page += view.render ("xrefs", "translate");
  page += Assets_Page::footer ();
  return page;
}

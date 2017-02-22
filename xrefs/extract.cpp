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


#include <xrefs/extract.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/url.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/volatile.h>
#include <database/logs.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <ipc/focus.h>
#include <xrefs/interpret.h>


string xrefs_extract_url ()
{
  return "xrefs/extract";
}


bool xrefs_extract_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_extract (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string bible = request->database_config_user()->getSourceXrefBible ();
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  
  // Container to hold information of all the xrefs in the chapter.
  // It consists of a repeating pattern of lines each line holding one item of information:
  // 0: the verse number
  // 1: xref offset within the verse
  // 2: xref USFM fragment
  vector <string> allxrefs;
  
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  vector <int> verses = usfm_get_verse_numbers (usfm);
  for (auto verse : verses) {
    string verse_usfm = usfm_get_verse_text (usfm, verse);
    vector <UsfmNote> xrefs = usfm_extract_notes (verse_usfm, {"x"});
    for (auto xref : xrefs) {
      allxrefs.push_back (convert_to_string (verse));
      allxrefs.push_back (convert_to_string (xref.offset));
      allxrefs.push_back (xref.data);
    }
  }
  
  
  int identifier = filter_string_user_identifier (webserver_request);
  string value = filter_string_implode (allxrefs, "\n");
  Database_Volatile::setValue (identifier, "sourcexrefs", value);
  

  if (!allxrefs.empty ()) {
    redirect_browser (request, xrefs_interpret_url ());
    return "";
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  page = header.run ();
  Assets_View view;
  page += view.render ("xrefs", "extract");
  page += Assets_Page::footer ();
  return page;
}

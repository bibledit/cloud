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


#include <xrefs/clear.h>
#include <xrefs/insert.h>
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
#include <bible/logic.h>


string xrefs_clear_url ()
{
  return "xrefs/clear";
}


bool xrefs_clear_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_clear (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string bible = request->database_config_user()->getTargetXrefBible ();
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  
  if (request->query.count ("overwrite")) {
    usfm = usfm_remove_notes (usfm, {"x"});
    bible_logic_store_chapter (bible, book, chapter, usfm);
  }
  
  
  // Count the cross references in this chapter.
  vector <UsfmNote> xrefs = usfm_extract_notes (usfm, {"x"});
  
  
  // Count the number of xref openers / closers in this chapter.
  string opener = usfm_get_opening_usfm ("x");
  int openers = 0;
  filter_string_str_replace (opener, "", usfm, &openers);
  string closer = usfm_get_closing_usfm ("x");
  int closers = 0;
  filter_string_str_replace (closer, "", usfm, &closers);
  
  
  if (xrefs.empty ()) {
    redirect_browser (request, xrefs_insert_url ());
    return "";
  }

  
  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  page = header.run ();
  Assets_View view;
  
  
  view.set_variable ("count", convert_to_string (xrefs.size ()));
  if (openers == closers) {
    view.enable_zone ("overwrite");
  } else {
    view.enable_zone ("tag_error");
  }
  

  page += view.render ("xrefs", "clear");
  page += Assets_Page::footer ();
  return page;
}

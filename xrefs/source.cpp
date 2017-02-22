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


#include <xrefs/source.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <editor/usfm2html.h>


string xrefs_source_url ()
{
  return "xrefs/source";
}


bool xrefs_source_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_source (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
 
  
  string stylesheet = request->database_config_user()->getStylesheet ();
  string bible = request->database_config_user()->getSourceXrefBible ();
  
  
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int verse = convert_to_int (request->query ["verse"]);
  
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  usfm = usfm_get_verse_text (usfm, verse);
  
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  string html = editor_usfm2html.get ();
  
  
  // The source text has notes and cross references.
  // The target text eventually gets them too.
  // The links like <a href="#note1" id="citation1" ...  are the same in both.
  // That applies to <a href="#citation1" id="note1"> ... too.
  // Therefore clicking a link in the source text may take the user to the target
  // text, and vice versa.
  // Fix the source text, because the source text can be modified without trouble, since it won't be saved.
  for (int i = 1; i < 1000; i++) {
    html = filter_string_str_replace ("note" + convert_to_string (i) + "\"", "note" + convert_to_string (i + 1000) + "\"", html);
    html = filter_string_str_replace ("citation" + convert_to_string (i) + "\"", "citation" + convert_to_string (i + 1000) + "\"", html);
  }
  
  
  return  html;
}

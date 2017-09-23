/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <xrefs/target.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <editor/usfm2html.h>
#include <styles/logic.h>


string xrefs_target_url ()
{
  return "xrefs/target";
}


bool xrefs_target_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_target (void * webserver_request) // Todo update it.
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string stylesheet = request->database_config_user()->getStylesheet ();
  string bible = request->database_config_user()->getTargetXrefBible ();
  
  
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  int verse = convert_to_int (request->query ["verse"]);
  
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  usfm = usfm_get_verse_text (usfm, verse);
  
  
  // Remove the footnotes end endnotes from the USFM.
  // This way they won't be visible and mix up the cross references placement.
  /* Currently this disables xrefs movement by the user
  Database_Styles database_styles;
  vector <string> markers = database_styles.getMarkers (stylesheet);
  for (auto marker : markers) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    if (style.type == StyleTypeFootEndNote) {
      string opener = usfm_get_opening_usfm (marker);
      string closer = usfm_get_closing_usfm (marker);
      filter_string_replace_between (usfm, opener, closer, "");
    }
  }
   */

  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  string html = editor_usfm2html.get ();
  
  
  return html;
}

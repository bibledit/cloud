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


#include <editold/offset.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <editor/usfm2html.h>
#include <access/bible.h>


string editold_offset_url ()
{
  return "editold/offset";
}


bool editold_offset_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string editold_offset (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);

  
  // At first the browser used the rangy library to get the offset of the caret.
  // But the rangy library provides the offset relative to the element that contains the caret,
  // not relative to the main editor element.
  // Therefore a pure Javascript implementation was Googled for and implemented.
  // This provides the offset of the caret relative to the <div id="editor">.
  size_t offset = convert_to_int (request->query ["offset"]);

  
  string stylesheet = request->database_config_user()->getStylesheet ();
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();

  
  // The caret offset should be in the main text body.
  // If it is in a note body, skip the verse updating.
  if (offset > editor_usfm2html.textLength) return "";


  // Get the number of verses in the USFM.
  // This covers combined verses also.
  int last_offset = 0;
  vector <int> verses = usfm_get_verse_numbers (usfm);
  for (size_t i = 0; i < verses.size (); i++) {
    if (editor_usfm2html.verseStartOffsets.count (i)) {
      last_offset = editor_usfm2html.verseStartOffsets [i];
    } else {
      editor_usfm2html.verseStartOffsets [i] = last_offset;
    }
  }
  
  
  // Get the starting offsets for each verse.
  vector <size_t> starting_offsets;
  for (size_t i = 0; i < verses.size (); i++) {
    starting_offsets.push_back (editor_usfm2html.verseStartOffsets [i]);
  }
  starting_offsets.push_back (editor_usfm2html.textLength);

  
  // Get the ending offsets for each verse.
  vector <size_t> ending_offsets;
  for (size_t i = 0; i < verses.size (); i++) {
    size_t offset = starting_offsets [i];
    for (size_t i2 = 0; i2 < starting_offsets.size (); i2++) {
      if (starting_offsets [i2] > offset) {
        offset = starting_offsets [i2];
        break;
      }
    }
    ending_offsets.push_back (offset);
  }
  
  
  // If the offset is between the focused verse's min and max values, then do nothing.
  int verse = Ipc_Focus::getVerse (request);
  for (size_t i = 0; i < verses.size (); i++) {
    if (verse == verses[i]) {
      if ((size_t) offset >= starting_offsets [i]) {
        if ((size_t) offset <= ending_offsets [i]) {
          return "";
        }
      }
    }
  }
  
  
  // Look for the verse that matches the offset.
  verse = -1;
  for (auto & element : editor_usfm2html.verseStartOffsets) {
    int key = element.first;
    size_t value = element.second;
    if (offset >= value) {
      // A verse number was found.
      verse = key;
    }
  }
  

  // Only act if a verse was found
  if (verse >= 0) {
    // Only update navigation in case the verse changed.
    // This avoids unnecessary focus operations in the clients.
    if (verse != Ipc_Focus::getVerse (request)) {
      Ipc_Focus::set (request, book, chapter, verse);
    }
    // The editor should scroll the verse into view,
    // because the caret is in the Bible text.
    return convert_to_string (verse);
    // If the caret were in the notes area,
    // then the editor should not scroll the verse into view.
  }
  
  
  return "";
}

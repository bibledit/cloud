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


#include <resource/get.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <resource/logic.h>
#include <database/config/bible.h>
#include <database/versifications.h>
#include <access/logic.h>


string resource_get_url ()
{
  return "resource/get";
}


bool resource_get_acl (void * webserver_request)
{
  return access_logic_privilege_view_resources (webserver_request);
}


string resource_get (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  vector <string> bits;

  
  string s_resource = request->query["resource"];
  string s_book = request->query["book"];
  string s_chapter = request->query["chapter"];
  string s_verse = request->query["verse"];

  
  if (!s_resource.empty () && !s_book.empty () && !s_chapter.empty () && !s_verse.empty ()) {


    unsigned int resource = convert_to_int (s_resource);
    int book = convert_to_int (s_book);
    int chapter = convert_to_int (s_chapter);
    int verse = convert_to_int (s_verse);

    
    // In JavaScript the resource identifier starts at 1 instead of at 0.
    resource--;
    vector <string> resources = request->database_config_user()->getActiveResources ();
    if (resource <= resources.size ()) {
      s_resource = resources [resource];


      // Handle a divider.
      if (resource_logic_is_divider (s_resource)) return resource_logic_get_divider (s_resource);
      
      
      string bible = request->database_config_user ()->getBible ();
      string versification = Database_Config_Bible::getVersificationSystem (bible);
      Database_Versifications database_versifications;
      vector <int> chapters = database_versifications.getChapters (versification, book);
      
      
      // Whether to add extra verse numbers, for clarity in case of viewing more than one verse.
      bool add_verse_numbers = false;
      int context_before = request->database_config_user ()->getResourceVersesBefore ();
      if (context_before) add_verse_numbers = true;
      int context_after = request->database_config_user ()->getResourceVersesAfter ();
      if (context_after) add_verse_numbers = true;
      
      
      // Context before the focused verse.
      vector <int> chapters_before;
      vector <int> verses_before;
      if (context_before > 0) {
        for (int ch = chapter - 1; ch <= chapter; ch++) {
          if (in_array (ch, chapters)) {
            vector <int> verses = database_versifications.getVerses (versification, book, ch);
            for (size_t vs = 0; vs < verses.size (); vs++) {
              int vs2 = verses [vs];
              if ((ch < chapter) || (vs2 < verse)) {
                if (vs2 > 0) {
                  chapters_before.push_back (ch);
                  verses_before.push_back (verses[vs]);
                }
              }
            }
          }
        }
        while ((int)chapters_before.size () > context_before) {
          chapters_before.erase (chapters_before.begin ());
          verses_before.erase (verses_before.begin ());
        }
      }
      for (unsigned int i = 0; i < chapters_before.size (); i++) {
        bits.push_back (resource_logic_get_html (request, s_resource, book, chapters_before[i], verses_before[i], add_verse_numbers));
      }
      

      // Focused verse.
      bits.push_back (resource_logic_get_html (request, s_resource, book, chapter, verse, add_verse_numbers));

    
      // Context after the focused verse.
      vector <int> chapters_after;
      vector <int> verses_after;
      if (context_after > 0) {
        for (int ch = chapter; ch <= chapter + 1; ch++) {
          if (in_array (ch, chapters)) {
            vector <int> verses = database_versifications.getVerses (versification, book, ch);
            for (size_t vs = 0; vs < verses.size (); vs++) {
              int vs2 = verses [vs];
              if ((ch > chapter) || (vs2 > verse)) {
                if (vs2 > 0) {
                  chapters_after.push_back (ch);
                  verses_after.push_back (verses[vs]);
                }
              }
            }
          }
        }
        while ((int)chapters_after.size () > context_after) {
          chapters_after.pop_back ();
          verses_after.pop_back ();
        }
      }
      for (unsigned int i = 0; i < chapters_after.size (); i++) {
        bits.push_back (resource_logic_get_html (request, s_resource, book, chapters_after[i], verses_after[i], add_verse_numbers));
      }
    }
  }
  
  
  string page = filter_string_implode (bits, ""); // <br>
  return page;
}

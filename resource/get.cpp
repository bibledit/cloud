/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


std::string resource_get_url ()
{
  return "resource/get";
}


bool resource_get_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_get (Webserver_Request& webserver_request)
{
  std::vector <std::string> bits;

  
  std::string s_resource = webserver_request.query["resource"];
  std::string s_book = webserver_request.query["book"];
  std::string s_chapter = webserver_request.query["chapter"];
  std::string s_verse = webserver_request.query["verse"];

  
  if (!s_resource.empty () && !s_book.empty () && !s_chapter.empty () && !s_verse.empty ()) {


    unsigned int resource = static_cast<unsigned>(filter::string::convert_to_int (s_resource));
    int book = filter::string::convert_to_int (s_book);
    int chapter = filter::string::convert_to_int (s_chapter);
    int verse = filter::string::convert_to_int (s_verse);

    
    // In JavaScript the resource identifier starts at 1.
    // In the C++ Bibledit kernel it starts at 0.
    resource--;
    std::vector <std::string> resources = webserver_request.database_config_user()->get_active_resources ();
    if (resource < resources.size ()) {
      s_resource = resources [resource];


      // Handle a divider.
      if (resource_logic_is_divider (s_resource)) {
        std::string text = resource_logic_get_divider (s_resource);
        return text;
      }
      
      
      std::string bible = webserver_request.database_config_user ()->get_bible ();
      std::string versification = database::config::bible::get_versification_system (bible);
      Database_Versifications database_versifications;
      std::vector <int> chapters = database_versifications.getChapters (versification, book);
      
      
      // Whether to add extra verse numbers, for clarity in case of viewing more than one verse.
      bool add_verse_numbers = false;
      int context_before = webserver_request.database_config_user ()->get_resource_verses_before ();
      if (context_before) add_verse_numbers = true;
      int context_after = webserver_request.database_config_user ()->get_resource_verses_after ();
      if (context_after) add_verse_numbers = true;
      
      
      // Context before the focused verse.
      std::vector <int> chapters_before;
      std::vector <int> verses_before;
      if (context_before > 0) {
        for (int ch = chapter - 1; ch <= chapter; ch++) {
          if (filter::string::in_array (ch, chapters)) {
            std::vector <int> verses = database_versifications.getVerses (versification, book, ch);
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
        while (static_cast<int>(chapters_before.size()) > context_before) {
          chapters_before.erase (chapters_before.begin ());
          verses_before.erase (verses_before.begin ());
        }
      }
      for (unsigned int i = 0; i < chapters_before.size (); i++) {
        bits.push_back (resource_logic_get_html (webserver_request, s_resource, book, chapters_before[i], verses_before[i], add_verse_numbers));
      }
      

      // Focused verse.
      bits.push_back (resource_logic_get_html (webserver_request, s_resource, book, chapter, verse, add_verse_numbers));

    
      // Context after the focused verse.
      std::vector <int> chapters_after;
      std::vector <int> verses_after;
      if (context_after > 0) {
        for (int ch = chapter; ch <= chapter + 1; ch++) {
          if (filter::string::in_array (ch, chapters)) {
            std::vector <int> verses = database_versifications.getVerses (versification, book, ch);
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
        while (static_cast<int>(chapters_after.size()) > context_after) {
          chapters_after.pop_back ();
          verses_after.pop_back ();
        }
      }
      for (unsigned int i = 0; i < chapters_after.size (); i++) {
        bits.push_back (resource_logic_get_html (webserver_request, s_resource, book, chapters_after[i], verses_after[i], add_verse_numbers));
      }
    }
  }
  
  
  std::string page = filter::string::implode (bits, ""); // <br>
  return page;
}

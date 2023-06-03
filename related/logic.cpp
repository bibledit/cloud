/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <related/logic.h>
#include <database/books.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
using namespace std;
using namespace pugi;


// Internal function that searches related passages in the XML DOM.
void related_logic_search_related (const string & bookname, int input_chapter, const string & input_verse,
                                   const xml_node & node, vector <int> & passages)
{
  for (xml_node set : node.children ()) {
    bool match = false;
    for (xml_node reference : set.children ()) {
      
      // If a match was found, skip further processing.
      if (match) continue;
      
      // Match on book.
      string book = reference.attribute ("book").value ();
      match = (book == bookname);
      
      // Match on chapter.
      if (match) {
        int chapter = filter::strings::convert_to_int (reference.attribute ("chapter").value ());
        match = (chapter == input_chapter);
      }
      
      // Match on verse(s).
      if (match) {
        string verse = reference.attribute ("verse").value ();
        vector <int> verses;
        if (filter::usfm::handle_verse_range (verse, verses)) {
          match = in_array (filter::strings::convert_to_int (input_verse), verses);
        } else {
          match = (verse == input_verse);
        }
      }
      
      // Store all related passages.
      if (match) {
        for (xml_node passage_node : set.children ()) {
          string related_bookname = passage_node.attribute ("book").value ();
          book_id related_book = database::books::get_id_from_english (related_bookname);
          int related_chapter = filter::strings::convert_to_int (passage_node.attribute ("chapter").value ());
          string verse = passage_node.attribute ("verse").value ();
          vector <int> verses {};
          if (filter::usfm::handle_verse_range (verse, verses));
          else verses.push_back (filter::strings::convert_to_int (verse));
          for (auto related_verse : verses) {
            if ((related_book != book_id::_unknown) && related_chapter) {
              Passage passage (string(), static_cast<int>(related_book), related_chapter, filter::strings::convert_to_string (related_verse));
              int i = filter_passage_to_integer (passage);
              // No duplicate passages to be included.
              if (!in_array (i, passages)) {
                passages.push_back (i);
              }
            }
          }
        }
      }
    }
  }
}


// This fetches related verses.
// It takes the passages from $input, and returns them plus their related passages, if there's any.
vector <Passage> related_logic_get_verses (const vector <Passage> & input)
{
  vector <int> related_passages;
  
  
  if (!input.empty ()) {


    // Get details about the book in the passage.
    // It assumes all input passages refer to the same book.
    string bookname = database::books::get_english_from_id (static_cast<book_id>(input[0].m_book));
    book_type booktype = database::books::get_type (static_cast<book_id>(input[0].m_book));
    bool is_ot = (booktype == book_type::old_testament);
    bool is_nt = (booktype == book_type::new_testament);
    
    
    // Load the parallel passages and the quotations.
    xml_document parallel_document;
    xml_document quotation_document;
    if (is_ot || is_nt) {
      string book_type_name = database::books::book_type_to_string(booktype);
      string path = filter_url_create_root_path ({"related", "parallel-passages-" + book_type_name + ".xml"});
      parallel_document.load_file (path.c_str());
      path = filter_url_create_root_path ({"related", "ot-quotations-in-nt.xml"});
      quotation_document.load_file (path.c_str());
    }

    
    // Deal with all of the input passages.
    for (auto & input_passage : input) {
      // Search for parallel passages.
      for (xml_node passages : parallel_document.children ()) {
        for (xml_node section : passages.children ()) {
          related_logic_search_related (bookname, input_passage.m_chapter, input_passage.m_verse, section, related_passages);
        }
      }
      // Search for quotes.
      for (xml_node passages : quotation_document.children ()) {
        related_logic_search_related (bookname, input_passage.m_chapter, input_passage.m_verse, passages, related_passages);
      }
    }
  }

  
  // Sort the passages and convert them.
  vector <Passage> output;
  sort (related_passages.begin (), related_passages.end ());
  for (auto & related_passage : related_passages) {
    Passage passage = filter_integer_to_passage (related_passage);
    output.push_back (passage);
  }

  
  // Done.
  return output;
}

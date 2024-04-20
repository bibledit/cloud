/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#include <sources/oshb.h>
#include <database/oshb.h>
#include <filter/string.h>
#include <filter/passage.h>
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


// Local declarations.
void sources_morphhb_parse_w_element (Database_OsHb * database_oshb, int book, int chapter, int verse, pugi::xml_node node);
void sources_morphhb_parse_unhandled_node (int book, int chapter, int verse, pugi::xml_node node);
void sources_morphhb_parse ();


void sources_morphhb_parse_w_element (Database_OsHb * database_oshb, int book, int chapter, int verse, pugi::xml_node node)
{
  std::string lemma = node.attribute ("lemma").value ();
  std::string word = node.child_value ();
  word = filter::strings::replace ("/", "", word);
  database_oshb->store (book, chapter, verse, lemma, word, "");
}


void sources_morphhb_parse_unhandled_node (int book, int chapter, int verse, pugi::xml_node node)
{
  std::string passage = filter_passage_display (book, chapter, filter::strings::convert_to_string (verse));
  std::string text = node.child_value ();
  std::cerr << "Unhandled " << node.name () << " at " << passage << ": " << text << std::endl;
}


void sources_morphhb_parse ()
{
  std::cout << "Starting" << std::endl;
  Database_OsHb database_oshb;
  database_oshb.create ();

  std::vector <std::string> books = {
    "Gen",
    "Exod",
    "Lev",
    "Num",
    "Deut",
    "Josh",
    "Judg",
    "Ruth",
    "1Sam",
    "2Sam",
    "1Kgs",
    "2Kgs",
    "1Chr",
    "2Chr",
    "Ezra",
    "Neh",
    "Esth",
    "Job",
    "Ps",
    "Prov",
    "Eccl",
    "Song",
    "Isa",
    "Jer",
    "Lam",
    "Ezek",
    "Dan",
    "Hos",
    "Joel",
    "Amos",
    "Obad",
    "Jonah",
    "Mic",
    "Nah",
    "Hab",
    "Zeph",
    "Hag",
    "Zech",
    "Mal"
  };

  for (size_t bk = 0; bk < books.size (); bk++) {
    
    std::string file = "sources/morphhb/" + books[bk] + ".xml";
    std::cout << file << std::endl;

    int book = static_cast<int>(bk + 1);

    pugi::xml_document document;
    document.load_file (file.c_str());
    pugi::xml_node osis_node = document.first_child ();
    pugi::xml_node osisText_node = osis_node.child ("osisText");
    pugi::xml_node div_book_node = osisText_node.child ("div");
    for (pugi::xml_node chapter_node : div_book_node.children()) {
      for (pugi::xml_node verse_node : chapter_node.children ()) {
        std::string node_name = verse_node.name ();
        if (node_name != "verse") continue;

        // Get the passage.
        std::string osisID = verse_node.attribute ("osisID").value ();
        std::vector <std::string> bits = filter::strings::explode (osisID, '.');
        int chapter = filter::strings::convert_to_int (bits[1]);
        int verse = filter::strings::convert_to_int (bits[2]);

        bool word_stored = false;
        
        // Most of the nodes will be "w" but there's more nodes as well, see the source XML file.
        for (pugi::xml_node node : verse_node.children ()) {

          if (word_stored) database_oshb.store (book, chapter, verse, "", " ", "");

          std::string child_node_name = node.name ();

          if (child_node_name == "w") {
            sources_morphhb_parse_w_element (&database_oshb, book, chapter, verse, node);
          }
          
          else if (child_node_name == "seg") {
            std::string word = node.child_value ();
            database_oshb.store (book, chapter, verse, "", word, "");
          }
          
          else if (child_node_name == "note") {
            for (pugi::xml_node variant_node : node.children ()) {
              std::string variant_node_name = variant_node.name ();
              if (variant_node_name == "catchWord") {
                sources_morphhb_parse_w_element (&database_oshb, book, chapter, verse, node);
              } else if (variant_node_name == "rdg") {
                for (pugi::xml_node w_node : variant_node.children ()) {
                  database_oshb.store (book, chapter, verse, "", "/", "");
                  sources_morphhb_parse_w_element (&database_oshb, book, chapter, verse, w_node);
                }
              } else {
                sources_morphhb_parse_unhandled_node (book, chapter, verse, node);
              }
            }
          }
          
          else {
            sources_morphhb_parse_unhandled_node (book, chapter, verse, node);
          }
          
          word_stored = true;
        }
        
      }
    }
  }

  database_oshb.optimize ();
  std::cout << "Completed" << std::endl;
}

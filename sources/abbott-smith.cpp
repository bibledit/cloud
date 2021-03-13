/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <sources/abbott-smith.h>
#include <database/abbottsmith.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


int entry_element_count = 0;


void sources_abbott_smith_parse_entry_element (Database_AbbottSmith * database_abbottsmith,
                                               xml_node & node)
{
  string entry = "entry";
  if (node.name() != entry) return;
  entry_element_count++;
  
//  string lemma = node.attribute ("lemma").value ();
//  string word = node.child_value ();
//  word = filter_string_str_replace ("/", "", word);
  //database_abbottsmith->store (book, chapter, verse, lemma, word, "");
}


void sources_abbott_smith_parse_unhandled_node (int book, int chapter, int verse, xml_node node)
{
  string passage = filter_passage_display (book, chapter, convert_to_string (verse));
  string text = node.child_value ();
  cerr << "Unhandled " << node.name () << " at " << passage << ": " << text << endl;
}


void sources_abbott_smith_parse ()
{
  cout << "Starting" << endl;
  Database_AbbottSmith database_abbottsmith;
  database_abbottsmith.create ();
    
  string file = "sources/abbott-smith/abbott-smith.tei_lemma.xml";
  
  xml_document document;
  document.load_file (file.c_str());
  xml_node TEI_node = document.first_child ();
  // Do a deep parsing.
  // The depth as used below was found out empirically in March 2021.
  // The number of <entry> elements was 6153 when counted in a text editor.
  // And this same number of elements was found when parsing as deep as is done below.
  for (xml_node node1 : TEI_node.children()) {
    sources_abbott_smith_parse_entry_element (&database_abbottsmith, node1);
    for (xml_node node2 : node1.children()) {
      sources_abbott_smith_parse_entry_element (&database_abbottsmith, node2);
      for (xml_node node3 : node2.children()) {
        sources_abbott_smith_parse_entry_element (&database_abbottsmith, node3);
        for (xml_node node4 : node3.children()) {
          sources_abbott_smith_parse_entry_element (&database_abbottsmith, node4);
          for (xml_node node5 : node4.children()) {
            sources_abbott_smith_parse_entry_element (&database_abbottsmith, node5);
          }
        }
      }
    }
  }
//  xml_node osisText_node = osis_node.child ("osisText");
//  xml_node div_book_node = osisText_node.child ("div");
//  for (xml_node chapter_node : div_book_node.children()) {
//    for (xml_node verse_node : chapter_node.children ()) {
//      string node_name = verse_node.name ();
//      if (node_name != "verse") continue;
//
//      // Get the passage.
//      string osisID = verse_node.attribute ("osisID").value ();
//      vector <string> bits = filter_string_explode (osisID, '.');
//      int chapter = convert_to_int (bits[1]);
//      int verse = convert_to_int (bits[2]);
//
//      bool word_stored = false;
//
//      // Most of the nodes will be "w" but there's more nodes as well, see the source XML file.
//      for (xml_node node : verse_node.children ()) {
//
//        //if (word_stored) database_abbottsmith.store (book, chapter, verse, "", " ", "");
//
//        string node_name = node.name ();
//
//        if (node_name == "w") {
//          sources_abbott_smith_parse_w_element (&database_abbottsmith, book, chapter, verse, node);
//        }
//
//        else if (node_name == "seg") {
//          string word = node.child_value ();
//          //database_abbottsmith.store (book, chapter, verse, "", word, "");
//        }
//
//        else if (node_name == "note") {
//          for (xml_node variant_node : node.children ()) {
//            string node_name = variant_node.name ();
//            if (node_name == "catchWord") {
//              sources_abbott_smith_parse_w_element (&database_abbottsmith, book, chapter, verse, node);
//            } else if (node_name == "rdg") {
//              for (xml_node w_node : variant_node.children ()) {
//                //database_abbottsmith.store (book, chapter, verse, "", "/", "");
//                sources_abbott_smith_parse_w_element (&database_abbottsmith, book, chapter, verse, w_node);
//              }
//            } else {
//              sources_abbott_smith_parse_unhandled_node (book, chapter, verse, node);
//            }
//          }
//        }
//
//        else {
//          sources_abbott_smith_parse_unhandled_node (book, chapter, verse, node);
//        }
//
//        word_stored = true;
//      }
//
//    }
//  }
  cout << entry_element_count << " entry elements parsed" << endl;
  database_abbottsmith.optimize ();
  cout << "Completed" << endl;
}

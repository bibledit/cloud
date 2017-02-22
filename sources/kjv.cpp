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


#include <sources/kjv.h>
#include <database/logs.h>
#include <database/kjv.h>
#include <database/sqlite.h>
#include <filter/string.h>
#include <filter/url.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


void sources_kjv_store (int book, int chapter, int verse, string lemma, string english)
{
  Database_Kjv database_kjv;
  vector <string> lemmas = filter_string_explode (lemma, ' ');
  bool output_done = false;
  for (auto & strong : lemmas) {
    if (strong.find ("strong") == string::npos) continue;
    if (output_done) {
      // Two separate two identical words.
      database_kjv.store (book, chapter, verse, "", " / ");
    }
    strong = filter_string_str_replace ("strong:", "", strong);
    database_kjv.store (book, chapter, verse, strong, english);
    output_done = true;
  }
  if (lemmas.empty ()) {
    database_kjv.store (book, chapter, verse, "", english);
  }
}


void sources_kjv_parse_loop (xml_node element,
                             int & book, int & chapter, int & verse,
                             bool & within_verse, string & lemma)
{
  string element_name = element.name ();
  if (element_name == "verse") {
    string sID = element.attribute ("sID").value ();
    if (!sID.empty ()) {
      verse++;
      within_verse = true;
    }
    string eID = element.attribute ("eID").value ();
    if (!eID.empty ()) {
      within_verse = false;
    }
  } else if (element_name == "w") {
    if (within_verse) {
      lemma = element.attribute ("lemma").value ();
      lemma = filter_string_trim (lemma);
      for (xml_node child : element.children ()) {
        sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
      }
    }
  } else if (element_name.empty ()) {
    if (within_verse) {
      string english = element.value ();
      sources_kjv_store (book, chapter, verse, lemma, english);
    }
  } else if (element_name == "note") {
    xml_node textnode = element.first_child ();
    string english = textnode.text ().get ();
    english.insert (0, " [");
    english.append ("]");
    sources_kjv_store (book, chapter, verse, "", english);
  } else if (element_name == "milestone") {
  } else if (element_name == "transChange") {
    xml_node textnode = element.first_child ();
    string english = textnode.text ().get ();
    english.insert (0, "<span style=\"font-style:italic;\">");
    english.append ("</span>");
    sources_kjv_store (book, chapter, verse, "", english);
  } else if (element_name == "inscription") {
    for (xml_node child : element.children ()) {
      string lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
    }
  } else if (element_name == "q") {
    for (xml_node child : element.children ()) {
      string lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
    }
  } else if (element_name == "divineName") {
    for (xml_node child : element.children ()) {
      string lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
    }
  } else if (element_name == "title") {
    for (xml_node child : element.children ()) {
      string lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
    }
  } else if (element_name == "foreign") {
    for (xml_node child : element.children ()) {
      string lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
    }
  } else {
    if (within_verse) {
      xml_node textnode = element.first_child ();
      string english = textnode.text ().get ();
      string location = filter_passage_display (book, chapter, convert_to_string (verse));
      Database_Logs::log (location + ": Failed to parse element " + element_name + " with value " + english);
    }
  }
}


// Parses the XML data from kjv.xml.
void sources_kjv_parse ()
{
  Database_Logs::log ("Parsing data from Crosswire's KJV XML file");
  Database_Kjv database_kjv;
  database_kjv.create ();

  int book = 0;
  int chapter = 0;
  int verse = 0;
  bool within_verse = false;

  xml_document document;
  document.load_file ("sources/kjv.xml", parse_ws_pcdata);
  for (xml_node osis : document.children ()) {
    for (xml_node osisText : osis.children ()) {
      for (xml_node divbook : osisText.children ()) {
        if (strcmp (divbook.name (), "div") == 0) {
          book++;
          Database_Logs::log ("Book " + convert_to_string (book));
          chapter = 0;
          verse = 0;
          for (xml_node chapter_element : divbook.children ()) {
            if (strcmp (chapter_element.name (), "chapter") == 0) {
              chapter++;
              verse = 0;
              within_verse = false;
              for (xml_node element : chapter_element.children ()) {
                string lemma;
                sources_kjv_parse_loop (element, book, chapter, verse, within_verse, lemma);
              }
            }
          }
        }
      }
    }
  }

  database_kjv.optimize ();
  Database_Logs::log ("Finished parsing data from the KJV XML file");
}

/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


// Declarations local functions.
void sources_kjv_store (int book, int chapter, int verse, std::string lemma, std::string english);
void sources_kjv_parse_loop (pugi::xml_node element, int & book, int & chapter, int & verse, bool & within_verse, std::string & lemma);


void sources_kjv_store (int book, int chapter, int verse, std::string lemma, std::string english)
{
  Database_Kjv database_kjv;
  std::vector <std::string> lemmas = filter::strings::explode (lemma, ' ');
  bool output_done = false;
  for (auto & strong : lemmas) {
    if (strong.find ("strong") == std::string::npos) continue;
    if (output_done) {
      // Two separate two identical words.
      database_kjv.store (book, chapter, verse, "", " / ");
    }
    strong = filter::strings::replace ("strong:", "", strong);
    database_kjv.store (book, chapter, verse, strong, english);
    output_done = true;
  }
  if (lemmas.empty ()) {
    database_kjv.store (book, chapter, verse, "", english);
  }
}


void sources_kjv_parse_loop (pugi::xml_node element,
                             int & book, int & chapter, int & verse,
                             bool & within_verse, std::string & lemma)
{
  std::string element_name = element.name ();
  if (element_name == "verse") {
    std::string sID = element.attribute ("sID").value ();
    if (!sID.empty ()) {
      verse++;
      within_verse = true;
    }
    std::string eID = element.attribute ("eID").value ();
    if (!eID.empty ()) {
      within_verse = false;
    }
  } else if (element_name == "w") {
    if (within_verse) {
      lemma = element.attribute ("lemma").value ();
      lemma = filter::strings::trim (lemma);
      for (pugi::xml_node child : element.children ()) {
        sources_kjv_parse_loop (child, book, chapter, verse, within_verse, lemma);
      }
    }
  } else if (element_name.empty ()) {
    if (within_verse) {
      std::string english = element.value ();
      sources_kjv_store (book, chapter, verse, lemma, english);
    }
  } else if (element_name == "note") {
    pugi::xml_node textnode = element.first_child ();
    std::string english = textnode.text ().get ();
    english.insert (0, " [");
    english.append ("]");
    sources_kjv_store (book, chapter, verse, "", english);
  } else if (element_name == "milestone") {
  } else if (element_name == "transChange") {
    pugi::xml_node textnode = element.first_child ();
    std::string english = textnode.text ().get ();
    english.insert (0, "<span style=\"font-style:italic;\">");
    english.append ("</span>");
    sources_kjv_store (book, chapter, verse, "", english);
  } else if (element_name == "inscription") {
    for (pugi::xml_node child : element.children ()) {
      std::string tmp_lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, tmp_lemma);
    }
  } else if (element_name == "q") {
    for (pugi::xml_node child : element.children ()) {
      std::string tmp_lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, tmp_lemma);
    }
  } else if (element_name == "divineName") {
    for (pugi::xml_node child : element.children ()) {
      std::string tmp_lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, tmp_lemma);
    }
  } else if (element_name == "title") {
    for (pugi::xml_node child : element.children ()) {
      std::string tmp_lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, tmp_lemma);
    }
  } else if (element_name == "foreign") {
    for (pugi::xml_node child : element.children ()) {
      std::string tmp_lemma;
      sources_kjv_parse_loop (child, book, chapter, verse, within_verse, tmp_lemma);
    }
  } else {
    if (within_verse) {
      pugi::xml_node textnode = element.first_child ();
      std::string english = textnode.text ().get ();
      std::string location = filter_passage_display (book, chapter, std::to_string (verse));
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

  pugi::xml_document document;
  document.load_file ("sources/kjv.xml", pugi::parse_ws_pcdata);
  for (pugi::xml_node osis : document.children ()) {
    for (pugi::xml_node osisText : osis.children ()) {
      for (pugi::xml_node divbook : osisText.children ()) {
        if (strcmp (divbook.name (), "div") == 0) {
          book++;
          Database_Logs::log ("Book " + std::to_string (book));
          chapter = 0;
          for (pugi::xml_node chapter_element : divbook.children ()) {
            if (strcmp (chapter_element.name (), "chapter") == 0) {
              chapter++;
              verse = 0;
              within_verse = false;
              for (pugi::xml_node element : chapter_element.children ()) {
                std::string lemma;
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

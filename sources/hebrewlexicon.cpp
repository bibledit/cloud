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


#include <sources/hebrewlexicon.h>
#include <database/logs.h>
#include <database/hebrewlexicon.h>
#include <filter/string.h>


void sources_hebrewlexicon_parse ()
{
  Database_Logs::log ("Start parsing Open Scriptures's Hebrew Lexicon");
  Database_HebrewLexicon database_hebrewlexicon;
  database_hebrewlexicon.create ();
  /* To redo this with pugixml

  {
    Database_Logs::log ("AugIndex.xml");
    xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/hebrewlexicon/AugIndex.xml");
    
    std::string aug;
    std::string target;
    
    while ((xmlTextReaderRead(reader) == 1)) {
      switch (xmlTextReaderNodeType (reader)) {
        case XML_READER_TYPE_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName (reader);
          if (element == "w") {
            aug = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "aug");
            target = (char *) xmlTextReaderReadInnerXml (reader);
          }
          break;
        }
        case XML_READER_TYPE_END_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName(reader);
          if (element == "w") {
            std::string xmlns = " xmlns=\"http://openscriptures.github.com/morphhb/namespace\"";
            target = filter::string::replace (xmlns, "", target);
            target = filter::string::trim (target);
            database_hebrewlexicon.setaug (aug, target);
            aug.clear ();
            target.clear ();
          }
          break;
        }
      }
    }
  }
  
  {
    Database_Logs::log ("BrownDriverBriggs.xml");
    xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/hebrewlexicon/BrownDriverBriggs.xml");
    
    std::string id;
    std::string definition;
    
    while ((xmlTextReaderRead(reader) == 1)) {
      switch (xmlTextReaderNodeType (reader)) {
        case XML_READER_TYPE_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName (reader);
          if (element == "entry") {
            id = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "id");
            definition = (char *) xmlTextReaderReadInnerXml (reader);
          }
          break;
        }
        case XML_READER_TYPE_END_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName(reader);
          if (element == "entry") {
            std::string xmlns = " xmlns=\"http://openscriptures.github.com/morphhb/namespace\"";
            definition = filter::string::replace (xmlns, "", definition);
            definition = filter::string::convert_xml_character_entities_to_characters (definition);
            std::vector <std::string> lines = filter::string::explode (definition, '\n');
            for (auto & line : lines) {
              if (line.find ("</status>") != std::string::npos) line.clear ();
              line = filter::string::trim (line);
            }
            definition = filter::string::implode (lines, "\n");
            definition = filter::string::trim (definition);
            database_hebrewlexicon.setbdb (id, definition);
            id.clear ();
            definition.clear ();
          }
          break;
        }
      }
    }
  }

  {
    Database_Logs::log ("LexicalIndex.xml");
    xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/hebrewlexicon/LexicalIndex.xml");
    
    std::string id;
    std::string bdb;
    
    while ((xmlTextReaderRead(reader) == 1)) {
      switch (xmlTextReaderNodeType (reader)) {
        case XML_READER_TYPE_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName (reader);
          if (element == "entry") {
            char * attribute = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "id");
            if (attribute) id = attribute;
          }
          if (element == "xref") {
            char * attribute = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "bdb");
            if (attribute) bdb = attribute;
          }
          break;
        }
        case XML_READER_TYPE_END_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName(reader);
          if (element == "entry") {
            std::string xmlns = " xmlns=\"http://openscriptures.github.com/morphhb/namespace\"";
            bdb = filter::string::replace (xmlns, "", bdb);
            bdb = filter::string::trim (bdb);
            database_hebrewlexicon.setmap (id, bdb);
            id.clear ();
            bdb.clear ();
          }
          break;
        }
      }
    }
  }
  
  {
    Database_Logs::log ("HebrewStrong.xml");
    xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/hebrewlexicon/HebrewStrong.xml");
    
    std::string id;
    std::string definition;
    
    while ((xmlTextReaderRead(reader) == 1)) {
      switch (xmlTextReaderNodeType (reader)) {
        case XML_READER_TYPE_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName (reader);
          if (element == "entry") {
            id = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "id");
            definition = (char *) xmlTextReaderReadInnerXml (reader);
          }
          break;
        }
        case XML_READER_TYPE_END_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName(reader);
          if (element == "entry") {
            std::string xmlns = " xmlns=\"http://openscriptures.github.com/morphhb/namespace\"";
            definition = filter::string::replace (xmlns, "", definition);
            definition = filter::string::convert_xml_character_entities_to_characters (definition);
            definition = filter::string::replace ("'", "''", definition);
            definition = filter::string::trim (definition);
            database_hebrewlexicon.setstrong (id, definition);
            id.clear ();
            definition.clear ();
          }
          break;
        }
      }
    }
  }

  {
    Database_Logs::log ("PartsOfSpeech.xml");
    xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/hebrewlexicon/PartsOfSpeech.xml");
    
    std::string code;
    std::string name;
    
    while ((xmlTextReaderRead(reader) == 1)) {
      switch (xmlTextReaderNodeType (reader)) {
        case XML_READER_TYPE_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName (reader);
          if (element == "Code") {
            code = (char *) xmlTextReaderReadInnerXml (reader);
          }
          if (element == "Name") {
            name = (char *) xmlTextReaderReadInnerXml (reader);
          }
          break;
        }
        case XML_READER_TYPE_END_ELEMENT:
        {
          std::string element = (char *) xmlTextReaderName(reader);
          if (element == "POS") {
            name = filter::string::unicode_string_casefold (name);
            database_hebrewlexicon.setpos (code, name);
            code.clear ();
            name.clear ();
          }
          break;
        }
      }
    }
  }
   */
  
  Database_Logs::log ("Finished parsing Open Scriptures's Hebrew Lexicon");
}

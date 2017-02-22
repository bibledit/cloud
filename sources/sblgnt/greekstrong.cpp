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


#include "greekstrong.h"
#include <unicode/ustdio.h>
#include <unicode/normlzr.h>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>


string str_replace (string search, string replace, string subject)
{
  size_t offposition = subject.find (search);
  while (offposition != string::npos) {
    subject.replace (offposition, search.length (), replace);
    offposition = subject.find (search, offposition + replace.length ());
  }
  return subject;
}


string convert_xml_character_entities_to_characters (string data)
{
  bool keep_going = true;
  int iterations = 0;
  size_t pos1 = -1;
  do {
    iterations++;
    pos1 = data.find ("&#x", pos1 + 1);
    if (pos1 == string::npos) {
      keep_going = false;
      continue;
    }
    size_t pos2 = data.find (";", pos1);
    if (pos2 == string::npos) {
      keep_going = false;
      continue;
    }
    string entity = data.substr (pos1 + 3, pos2 - pos1 - 3);
    data.erase (pos1, pos2 - pos1 + 1);
    int codepoint;
    stringstream ss;
    ss << hex << entity;
    ss >> codepoint;
    
    // The following is not available in GNU libstdc++.
    // wstring_convert <codecvt_utf8 <char32_t>, char32_t> conv1;
    // string u8str = conv1.to_bytes (codepoint);
    
    int cp = codepoint;
    // Adapted from: http://www.zedwood.com/article/cpp-utf8-char-to-codepoint.
    char c[5]={ 0x00,0x00,0x00,0x00,0x00 };
    if     (cp<=0x7F) { c[0] = cp;  }
    else if(cp<=0x7FF) { c[0] = (cp>>6)+192; c[1] = (cp&63)+128; }
    else if(0xd800<=cp && cp<=0xdfff) {} //invalid block of utf8
    else if(cp<=0xFFFF) { c[0] = (cp>>12)+224; c[1]= ((cp>>6)&63)+128; c[2]=(cp&63)+128; }
    else if(cp<=0x10FFFF) { c[0] = (cp>>18)+240; c[1] = ((cp>>12)&63)+128; c[2] = ((cp>>6)&63)+128; c[3]=(cp&63)+128; }
    string u8str = string (c);
    
    data.insert (pos1, u8str);
  } while (keep_going & (iterations < 100000));
  return data;
}


string trim (string s)
{
  if (s.length () == 0)
    return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t beg = s.find_first_not_of(" \t\n\r");
  size_t end = s.find_last_not_of(" \t\n\r");
  // No non-spaces
  if (beg == string::npos)
    return "";
  return string (s, beg, end - beg + 1);
}


string normalize (string str)
{
  // UTF-8 std::string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (str));
  
  // Case folding.
  source.foldCase ();
  
  // Transliterate UTF-16 UnicodeString following this rule:
  // decompose, remove diacritics, recompose
  UErrorCode status = U_ZERO_ERROR;
  Transliterator *accentsConverter = Transliterator::createInstance("NFD; [:M:] Remove; NFC", UTRANS_FORWARD, status);
  accentsConverter->transliterate(source);
  
  // UTF-16 UnicodeString -> UTF-8 std::string
  std::string result;
  source.toUTF8String (result);
  
  return result;
}


int main (int argc, char **argv)
{
  unlink ("greekstrong.sqlite");
  sqlite3 *db;
  sqlite3_open ("greekstrong.sqlite", &db);
  sqlite3_exec (db, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);
  sqlite3_exec (db, "CREATE TABLE IF NOT EXISTS greekstrong (id text, lemma text, definition text);", NULL, NULL, NULL);
  
  xmlTextReaderPtr reader = xmlNewTextReaderFilename ("strongsgreek.xml");

  string id;
  string lemma;
  set <string> lemmas;
  string definition;
  
  while ((xmlTextReaderRead(reader) == 1)) {
    int depth = xmlTextReaderDepth (reader);
    switch (xmlTextReaderNodeType (reader)) {
      case XML_READER_TYPE_ELEMENT:
      {
        string element = (char *) xmlTextReaderName (reader);
        if (element == "entry") {
          id = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "strongs");
          int n = atoi (id.c_str());
          ostringstream r;
          r << "G" << n;
          id = r.str();
          cout << id << endl;
          definition = (char *) xmlTextReaderReadInnerXml (reader);
        }
        if (element == "greek") {
          // Take from first <greek> element only: Any subsequent ones contain other information.
          if (lemma.empty ()) {
            lemma = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "unicode");
            // Casefold the lemma: This enables searching on the lemma.
            lemma = normalize (lemma);
            cout << lemma << endl;
            /*
            if (lemmas.find (lemma) != lemmas.end ()) {
              cout << "Double lemma" << endl;
            }
            lemmas.insert (lemma);
             */
          }
        }
        break;
      }
      case XML_READER_TYPE_TEXT:
      {
        xmlChar *text = xmlTextReaderValue(reader);
        break;
      }
      case XML_READER_TYPE_END_ELEMENT:
      {
        string element = (char *) xmlTextReaderName(reader);
        if (element == "entry") {

          definition = convert_xml_character_entities_to_characters (definition);
          definition = str_replace ("'", "''", definition);
          definition = str_replace ("\n", " ", definition);
          definition = str_replace ("  ", " ", definition);
          definition = str_replace ("  ", " ", definition);
          definition = trim (definition);
          
          string sql = "INSERT INTO greekstrong VALUES ('" + id + "', '" + lemma + "', '" + definition + "');";
          char *error = NULL;
          int rc = sqlite3_exec (db, sql.c_str(), NULL, NULL, &error);
          if (rc != SQLITE_OK) {
            cout << sql << endl;
            cout << error << endl;
            return 0;
          }
          id.clear ();
          lemma.clear ();
          definition.clear ();
        }
        break;
      }
    }
  }

  sqlite3_close (db);

  return 0;
}



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


#include <sources/etcbc4.h>
#include <database/logs.h>
#include <database/etcbc4.h>
#include <filter/string.h>
#include <filter/url.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


void sources_etcbc4_download ()
{
  Database_Logs::log ("Start to download the raw Hebrew morphology data from the ETCBC4 database");
  Database_Etcbc4 database_etcbc4;
  database_etcbc4.create ();
  
  // The book names for downloading data.
  vector <string> books = {
    "Genesis",
    "Exodus",
    "Leviticus",
    "Numeri",
    "Deuteronomium",
    "Josua",
    "Judices",
    "Ruth",
    "Samuel_I",
    "Samuel_II",
    "Reges_I",
    "Reges_II",
    "Chronica_I",
    "Chronica_II",
    "Esra",
    "Nehemia",
    "Esther",
    "Iob",
    "Psalmi",
    "Proverbia",
    "Ecclesiastes",
    "Canticum",
    "Jesaia",
    "Jeremia",
    "Threni",
    "Ezechiel",
    "Daniel",
    "Hosea",
    "Joel",
    "Amos",
    "Obadia",
    "Jona",
    "Micha",
    "Nahum",
    "Habakuk",
    "Zephania",
    "Haggai",
    "Sacharia",
    "Maleachi"
  };

  for (unsigned int bk = 0; bk < books.size (); bk++) {

    int book = bk + 1;
    string bookname = books[bk];

    bool book_done = false;
    for (int chapter = 1; chapter <= 150; chapter++) {
      if (book_done) continue;

      for (int verse = 1; verse < 200; verse++) {
        if (book_done) continue;

        string data = database_etcbc4.raw (book, chapter, verse);
        if (!data.empty ()) continue;
        
        string url = "https://shebanq.ancient-data.org/hebrew/verse?version=4b&book=" + bookname + "&chapter=" + convert_to_string (chapter) + "&verse=" + convert_to_string (verse);

        string error;
        string response = filter_url_http_get (url, error, false);
        if (!error.empty ()) {
          Database_Logs::log (error);
          continue;
        }
        if (response.find ("does not exist") != string::npos) {
          if (verse == 1) book_done = true;
          break;
        }
        Database_Logs::log (bookname + " " + convert_to_string (chapter) + "." + convert_to_string (verse));
        database_etcbc4.store (book, chapter, verse, response);
        // Wait a second: Be polite: Do not overload the website.
        this_thread::sleep_for (chrono::seconds (1));
      }
    }
  }

  Database_Logs::log ("Finished downloading from the ETCBC4 database");
}


string sources_etcbc4_clean (string item)
{
  item = filter_string_str_replace ("/", "", item);
  item = filter_string_str_replace ("]", "", item);
  item = filter_string_str_replace ("[", "", item);
  item = filter_string_str_replace ("=", "", item);
  item = filter_string_trim (item);
  return item;
}


// Parses the raw html data as downloaded from the ETCBC4 database.
// The parser is supposed to be ran only by the developers.
void sources_etcbc4_parse ()
{
  Database_Logs::log ("Parsing data from the ETCBC4 database");
  Database_Etcbc4 database_etcbc4;
  database_etcbc4.create ();
  vector <int> books = database_etcbc4.books ();
  for (auto book : books) {
    vector <int> chapters = database_etcbc4.chapters (book);
    for (auto chapter : chapters) {
      Database_Logs::log ("Parsing book " + convert_to_string (book) + " chapter " + convert_to_string (chapter));
      vector <int> verses = database_etcbc4.verses (book, chapter);
      for (auto verse : verses) {
        // The raw data for the verse.
        string data = database_etcbc4.raw (book, chapter, verse);
        if (data.empty ()) continue;
        data = filter_string_str_replace (unicode_non_breaking_space_entity (), "", data);
        // Parse the data.
        xml_document document;
        document.load_string (data.c_str());
        // Iterate through the <table> elements, one element per word or word fragment.
        for (xml_node table : document.children()) {
          // The relevant grammatical information to be extracted from the data.
          string word;
          string vocalized_lexeme;
          string consonantal_lexeme;
          string gloss;
          string pos;
          string subpos;
          string gender;
          string number;
          string person;
          string state;
          string tense;
          string stem;
          string phrase_function;
          string phrase_type;
          string phrase_relation;
          string phrase_a_relation;
          string clause_text_type;
          string clause_type;
          string clause_relation;
          // Iterate through the <tr> elements.
          // Each element contains one or more table cells with information.
          for (xml_node tr : table.children ()) {
            // Iterate through the <td> elements.
            for (xml_node td : tr.children ()) {
              // Iterate through the one or more <span> elements within this table cell.
              // Each <span> elements has a grammatical tag.
              for (xml_node span : td.children ()) {
                // Get the text this <span> contains.
                xml_node txtnode = span.first_child ();
                string value = txtnode.text ().get ();
                value = sources_etcbc4_clean (value);
                // The class of the <span> element indicates what kind of grammatical tag it has.
                string clazz = span.attribute ("class").value ();
                if (clazz == "ht") word = value;
                if (clazz.find ("hl_hlv") != string::npos) vocalized_lexeme = value;
                if (clazz.find ("hl_hlc") != string::npos) consonantal_lexeme = value;
                if (clazz == "gl") gloss = value;
                if (clazz.find ("_pos") != string::npos) pos = value;
                if (clazz.find ("_subpos") != string::npos) subpos = value;
                if (clazz.find ("_gender") != string::npos) gender = value;
                if (clazz.find ("_gnumber") != string::npos) number = value;
                if (clazz.find ("_person") != string::npos) person = value;
                if (clazz.find ("_state") != string::npos) state = value;
                if (clazz.find ("_tense") != string::npos) tense = value;
                if (clazz.find ("_stem") != string::npos) stem = value;
                if (clazz.find ("ph_fun") != string::npos) phrase_function = value;
                if (clazz.find ("ph_typ") != string::npos) phrase_type = value;
                if (clazz.find ("ph_rela") != string::npos) phrase_relation = value;
                if (clazz.find ("ph_arela") != string::npos) phrase_a_relation = value;
                if (clazz.find ("cl_txt") != string::npos) clause_text_type = value;
                if (clazz.find ("cl_typ") != string::npos) clause_type = value;
                if (clazz.find ("cl_rela") != string::npos) clause_relation = value;
              }
            }
          }
          // The table element has been done: Store it.
          database_etcbc4.store (book, chapter, verse,
                                 word, vocalized_lexeme, consonantal_lexeme, gloss, pos, subpos,
                                 gender, number, person,
                                 state, tense, stem,
                                 phrase_function, phrase_type, phrase_relation,
                                 phrase_a_relation, clause_text_type, clause_type, clause_relation);
        }
      }
    }
  }
  
  Database_Logs::log ("Finished parsing data from the ETCBC4 database");
}

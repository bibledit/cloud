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


#include <sources/etcbc4.h>
#include <database/logs.h>
#include <database/etcbc4.h>
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


void sources_etcbc4_download ()
{
  Database_Logs::log ("Start to download the raw Hebrew morphology data from the ETCBC4 database");
  Database_Etcbc4 database_etcbc4;
  database_etcbc4.create ();
  
  // The book names for downloading data.
  std::vector <std::string> books = {
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

  for (size_t bk = 0; bk < books.size (); bk++) {

    int book = static_cast<int>(bk + 1);
    std::string bookname = books[bk];

    bool book_done = false;
    for (int chapter = 1; chapter <= 150; chapter++) {
      if (book_done) continue;

      for (int verse = 1; verse < 200; verse++) {
        if (book_done) continue;

        std::string data = database_etcbc4.raw (book, chapter, verse);
        if (!data.empty ()) continue;
        
        std::string url = "https://shebanq.ancient-data.org/hebrew/verse?version=4b&book=" + bookname + "&chapter=" + filter::strings::convert_to_string (chapter) + "&verse=" + filter::strings::convert_to_string (verse);

        std::string error;
        std::string response = filter_url_http_get (url, error, false);
        if (!error.empty ()) {
          Database_Logs::log (error);
          continue;
        }
        if (response.find ("does not exist") != std::string::npos) {
          if (verse == 1) book_done = true;
          break;
        }
        Database_Logs::log (bookname + " " + filter::strings::convert_to_string (chapter) + "." + filter::strings::convert_to_string (verse));
        database_etcbc4.store (book, chapter, verse, response);
        // Wait a second: Be polite: Do not overload the website.
        std::this_thread::sleep_for (std::chrono::seconds (1));
      }
    }
  }

  Database_Logs::log ("Finished downloading from the ETCBC4 database");
}


std::string sources_etcbc4_clean (std::string item)
{
  item = filter::strings::replace ("/", "", item);
  item = filter::strings::replace ("]", "", item);
  item = filter::strings::replace ("[", "", item);
  item = filter::strings::replace ("=", "", item);
  item = filter::strings::trim (item);
  return item;
}


// Parses the raw html data as downloaded from the ETCBC4 database.
// The parser is supposed to be ran only by the developers.
void sources_etcbc4_parse ()
{
  Database_Logs::log ("Parsing data from the ETCBC4 database");
  Database_Etcbc4 database_etcbc4;
  database_etcbc4.create ();
  std::vector <int> books = database_etcbc4.books ();
  for (auto book : books) {
    std::vector <int> chapters = database_etcbc4.chapters (book);
    for (auto chapter : chapters) {
      Database_Logs::log ("Parsing book " + filter::strings::convert_to_string (book) + " chapter " + filter::strings::convert_to_string (chapter));
      std::vector <int> verses = database_etcbc4.verses (book, chapter);
      for (auto verse : verses) {
        // The raw data for the verse.
        std::string data = database_etcbc4.raw (book, chapter, verse);
        if (data.empty ()) continue;
        data = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), "", data);
        // Parse the data.
        pugi::xml_document document;
        document.load_string (data.c_str());
        // Iterate through the <table> elements, one element per word or word fragment.
        for (pugi::xml_node table : document.children()) {
          // The relevant grammatical information to be extracted from the data.
          std::string word;
          std::string vocalized_lexeme;
          std::string consonantal_lexeme;
          std::string gloss;
          std::string pos;
          std::string subpos;
          std::string gender;
          std::string number;
          std::string person;
          std::string state;
          std::string tense;
          std::string stem;
          std::string phrase_function;
          std::string phrase_type;
          std::string phrase_relation;
          std::string phrase_a_relation;
          std::string clause_text_type;
          std::string clause_type;
          std::string clause_relation;
          // Iterate through the <tr> elements.
          // Each element contains one or more table cells with information.
          for (pugi::xml_node tr : table.children ()) {
            // Iterate through the <td> elements.
            for (pugi::xml_node td : tr.children ()) {
              // Iterate through the one or more <span> elements within this table cell.
              // Each <span> elements has a grammatical tag.
              for (pugi::xml_node span : td.children ()) {
                // Get the text this <span> contains.
                pugi::xml_node txtnode = span.first_child ();
                std::string value = txtnode.text ().get ();
                value = sources_etcbc4_clean (value);
                // The class of the <span> element indicates what kind of grammatical tag it has.
                std::string clazz = span.attribute ("class").value ();
                if (clazz == "ht") word = value;
                if (clazz.find ("hl_hlv") != std::string::npos) vocalized_lexeme = value;
                if (clazz.find ("hl_hlc") != std::string::npos) consonantal_lexeme = value;
                if (clazz == "gl") gloss = value;
                if (clazz.find ("_pos") != std::string::npos) pos = value;
                if (clazz.find ("_subpos") != std::string::npos) subpos = value;
                if (clazz.find ("_gender") != std::string::npos) gender = value;
                if (clazz.find ("_gnumber") != std::string::npos) number = value;
                if (clazz.find ("_person") != std::string::npos) person = value;
                if (clazz.find ("_state") != std::string::npos) state = value;
                if (clazz.find ("_tense") != std::string::npos) tense = value;
                if (clazz.find ("_stem") != std::string::npos) stem = value;
                if (clazz.find ("ph_fun") != std::string::npos) phrase_function = value;
                if (clazz.find ("ph_typ") != std::string::npos) phrase_type = value;
                if (clazz.find ("ph_rela") != std::string::npos) phrase_relation = value;
                if (clazz.find ("ph_arela") != std::string::npos) phrase_a_relation = value;
                if (clazz.find ("cl_txt") != std::string::npos) clause_text_type = value;
                if (clazz.find ("cl_typ") != std::string::npos) clause_type = value;
                if (clazz.find ("cl_rela") != std::string::npos) clause_relation = value;
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

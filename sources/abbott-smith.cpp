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


#include <sources/abbott-smith.h>
#include <database/abbottsmith.h>
#include <filter/string.h>
#include <filter/passage.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#ifdef HAVE_ICU
#include <unicode/ustdio.h>
#include <unicode/normlzr.h>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>
#endif
#pragma GCC diagnostic pop
using namespace std;
#ifdef HAVE_ICU
using namespace icu;
#endif


int entry_element_count {0};


void sources_abbott_smith_parse_entry_element (Database_AbbottSmith * database_abbottsmith,
                                               pugi::xml_node & node)
{
  std::string entry = "entry";
  if (node.name() != entry) return;
  entry_element_count++;
  
  // Example <entry> element and its content:
  
  //<entry lemma="α" strong="G1">
  //  <form><orth>Α, α, ἄλφα</orth> (q.v.), <foreign xml:lang="grc">τό</foreign>, indecl., </form>
  //  <sense><gloss>alpha</gloss>, the first letter of the Greek alphabet. As a numeral, <foreign xml:lang="grc">αʹ</foreign> = 1, <foreign xml:lang="grc">α͵</foreign> = 1000. As a prefix, it appears to have at least two and perhaps three distinct senses:
  //    <sense n="1."><foreign xml:lang="grc">ἀ-</foreign> (before a vowel, <foreign xml:lang="grc">ἀν-</foreign>) <gloss>negative</gloss>, as in <foreign xml:lang="grc">ἄ-γνωστος</foreign>, <foreign xml:lang="grc">ἄ-δικος</foreign>.</sense>
  //    <sense n="2."><foreign xml:lang="grc">ἀ-</foreign>, <foreign xml:lang="grc">ἁ-</foreign><gloss>copulative</gloss>, indicating community and fellowship, as in <foreign xml:lang="grc">ἁ-πλοῦς</foreign>, <foreign xml:lang="grc">ἀ-κολουθέω</foreign>, <foreign xml:lang="grc">ἀ-δελφός</foreign>.</sense>
  //    <sense n="3.">An <gloss>intensive</gloss> force (LS, s. <foreign xml:lang="grc">α</foreign>), as in <foreign xml:lang="grc">ἀ-τενίζω</foreign> is sometimes assumed (but v. Boisacq, s.v.).</sense>
  //  </sense>
  //</entry>

  // Get the lemma, and the Strong's number, and the raw XML of the entry's contents.
  std::string lemma = filter::strings::trim (node.attribute ("lemma").value ());
#ifdef HAVE_ICU
  lemma = filter::strings::icu_string_normalize (lemma, true, true);
#endif
  std::string strong = filter::strings::trim (node.attribute ("strong").value ());
  std::stringstream ss;
  for (pugi::xml_node child : node.children()) child.print(ss, "", pugi::format_raw);
  std::string contents = ss.str ();
  
  // If there's no lemma, or no Strong's number, then there's nothing to store.
  if (lemma.empty () && strong.empty()) return;

  // There's a few cases in the XML file that the lemma looks like this:
  // ἀκριβῶς|G199
  // It means that a Strong's number is added to the lemma.
  // Such a Strong's number should be parsed too, and put at its proper place in the database.
  // And the lemma should be remove from its attached Strong's number.
  std::vector <std::string> strongs = filter::strings::explode (lemma, '|');
  if (strongs.size() >= 2) {
    lemma = strongs[0];
    strongs.erase (strongs.begin());
  } else {
    strongs.clear();
  }

  // Store the original lemma, the casefolded lemma, and the Strong's number,
  // together with the entry's raw XML, into the database.
  std::string lemma_case_folded = filter::strings::unicode_string_casefold (lemma);
  database_abbottsmith->store (lemma, lemma_case_folded, strong, contents);
  
  // If there's more Strong's numbers in the entry, store those too, but without any lemma.
  for (auto strong2 : strongs) {
    database_abbottsmith->store (string(), std::string(), strong2, contents);
  }
}


void sources_abbott_smith_parse ()
{
  std::cout << "Starting" << std::endl;
  Database_AbbottSmith database_abbottsmith;
  database_abbottsmith.create ();
    
  std::string file = "sources/abbott-smith/abbott-smith.tei_lemma.xml";
  
  pugi::xml_document document;
  document.load_file (file.c_str());
  pugi::xml_node TEI_node = document.first_child ();
  // Do a deep parsing.
  // The depth as used below was found out empirically in March 2021.
  // The number of <entry> elements was 6153 when counted in a text editor.
  // And this same number of elements was found when parsing as deep as is done below.
  for (pugi::xml_node node1 : TEI_node.children()) {
    sources_abbott_smith_parse_entry_element (&database_abbottsmith, node1);
    for (pugi::xml_node node2 : node1.children()) {
      sources_abbott_smith_parse_entry_element (&database_abbottsmith, node2);
      for (pugi::xml_node node3 : node2.children()) {
        sources_abbott_smith_parse_entry_element (&database_abbottsmith, node3);
        for (pugi::xml_node node4 : node3.children()) {
          sources_abbott_smith_parse_entry_element (&database_abbottsmith, node4);
          for (pugi::xml_node node5 : node4.children()) {
            sources_abbott_smith_parse_entry_element (&database_abbottsmith, node5);
          }
        }
      }
    }
  }
  std::cout << entry_element_count << " entry elements parsed" << std::endl;
  database_abbottsmith.optimize ();
  std::cout << "Completed" << std::endl;
}

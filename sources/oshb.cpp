/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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
#include <database/books.h>
#include <filter/string.h>
#include <filter/passage.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop
using namespace std;
using namespace pugi;


void sources_oshb_parse ()
{
  cout << "Starting" << endl;

  Database_OsHb database_oshb;
  database_oshb.create ();
  database_oshb.optimize ();

  string file = "sources/oshb.xml";
  unlink (file.c_str());
  cout << file << endl;

  string command = "gunzip sources/oshb.xml.gz";
#ifndef HAVE_IOS
  [[maybe_unused]] auto result = system (command.c_str ());
#endif
  
  map <int, int> mapping = {
    
    // Torah
    
    // Bereshit Genesis
    pair (1, database::books::get_id_from_english_v1 ("Genesis")),
    // Shemot Exodus
    pair (2, database::books::get_id_from_english_v1 ("Exodus")),
    // Vayikra Leviticus
    pair (3, database::books::get_id_from_english_v1 ("Leviticus")),
    // Bəmidbar Numbers
    pair (4, database::books::get_id_from_english_v1 ("Numbers")),
    // Devarim Deuteronomy
    pair (5, database::books::get_id_from_english_v1 ("Deuteronomy")),
    
    // Nevi'im

    // Yĕhôshúa‘ Joshua
    pair (6, database::books::get_id_from_english_v1 ("Joshua")),
    // Shophtim Judges
    pair (7, database::books::get_id_from_english_v1 ("Judges")),
    // 1 Shmû’ēl 1 Samuel
    pair (8, database::books::get_id_from_english_v1 ("1 Samuel")),
    // 2 Shmû’ēl 2 Samuel
    pair (9, database::books::get_id_from_english_v1 ("2 Samuel")),
    // 1 M'lakhim 1 Kings
    pair (10, database::books::get_id_from_english_v1 ("1 Kings")),
    // 2 M'lakhim 2 Kings
    pair (11, database::books::get_id_from_english_v1 ("2 Kings")),
    // Yĕsha‘ăyāhû Isaiah
    pair (12, database::books::get_id_from_english_v1 ("Isaiah")),
    // Yirmyāhû Jeremiah
    pair (13, database::books::get_id_from_english_v1 ("Jeremiah")),
    // Yĕkhezqiēl Ezekiel
    pair (14, database::books::get_id_from_english_v1 ("Ezekiel")),
    // Hôshēa‘ Hosea
    pair (15, database::books::get_id_from_english_v1 ("Hosea")),
    // Yô’ēl Joel
    pair (16, database::books::get_id_from_english_v1 ("Joel")),
    // Āmôs Amos
    pair (17, database::books::get_id_from_english_v1 ("Amos")),
    // Ōvadhyāh Obadiah
    pair (18, database::books::get_id_from_english_v1 ("Obadiah")),
    // Yônāh Jonah
    pair (19, database::books::get_id_from_english_v1 ("Jonah")),
    // Mîkhāh Micah
    pair (20, database::books::get_id_from_english_v1 ("Micah")),
    // Nakḥûm Nahum
    pair (21, database::books::get_id_from_english_v1 ("Nahum")),
    // Khăvhakûk Habakkuk
    pair (22, database::books::get_id_from_english_v1 ("Habakkuk")),
    // Tsĕphanyāh Zephaniah
    pair (23, database::books::get_id_from_english_v1 ("Zephaniah")),
    // Khaggai Haggai
    pair (24, database::books::get_id_from_english_v1 ("Haggai")),
    // Zkharyāh Zechariah
    pair (25, database::books::get_id_from_english_v1 ("Zechariah")),
    // Mal’ākhî Malachi
    pair (26, database::books::get_id_from_english_v1 ("Malachi")),

    // Ketuvim
    
    // Tehillim (Psalms)
    pair (27, database::books::get_id_from_english_v1 ("Psalms")),
    // Mishlei (Book of Proverbs)
    pair (28, database::books::get_id_from_english_v1 ("Proverbs")),
    // Iyyôbh (Book of Job)
    pair (29, database::books::get_id_from_english_v1 ("Job")),
    // Shīr Hashīrīm (Song of Songs) or (Song of Solomon)
    pair (30, database::books::get_id_from_english_v1 ("Song of Solomon")),
    // Rūth (Book of Ruth)
    pair (31, database::books::get_id_from_english_v1 ("Ruth")),
    // Eikhah (Lamentations)
    pair (32, database::books::get_id_from_english_v1 ("Lamentations")),
    // Qōheleth (Ecclesiastes)
    pair (33, database::books::get_id_from_english_v1 ("Ecclesiastes")),
    // Estēr (Book of Esther)
    pair (34, database::books::get_id_from_english_v1 ("Esther")),
    // Dānî’ēl (Book of Daniel)
    pair (35, database::books::get_id_from_english_v1 ("Daniel")),
    // 1 Ezrā (Book of Ezra)
    pair (36, database::books::get_id_from_english_v1 ("Ezra")),
    // 2 Ezrā (Book of Nehemiah)
    pair (37, database::books::get_id_from_english_v1 ("Nehemiah")),
    // 1 Divrei ha-Yamim (1 Chronicles)
    pair (38, database::books::get_id_from_english_v1 ("1 Chronicles")),
    // 2 Divrei ha-Yamim (2 Chronicles)
    pair (39, database::books::get_id_from_english_v1 ("2 Chronicles"))
    
  };
  
  int previous_book = 0;

  xml_document document;
  document.load_file (file.c_str(), parse_ws_pcdata_single);
  xml_node mysqldump_node = document.first_child ();
  xml_node database_node = mysqldump_node.first_child ();
  xml_node table_data_node = database_node.child ("table_data");
  for (xml_node row_node : table_data_node.children()) {
    int book = 0;
    int chapter = 0;
    int verse = 0;
    string word;
    string append;
    string lemma;
    string morph;
    for (xml_node field_node : row_node.children ()) {
      string name = field_node.attribute ("name").value ();
      string value = field_node.child_value ();
      if (name == "bookId") book = mapping [convert_to_int (value)];
      if (name == "chapter") chapter = convert_to_int (value);
      if (name == "verse") verse = convert_to_int (value);
      if (name == "word") word = value;
      if (name == "append") append = value;
      if (name == "lemma") lemma = value;
      if (name == "morph") morph = value;
    }
    if (book != previous_book) {
      previous_book = book;
      cout << database::books::get_english_from_id_v1 (book) << endl;
    }
    word = filter_string_str_replace ("/", "", word);
    database_oshb.store (book, chapter, verse, lemma, word, morph);
    database_oshb.store (book, chapter, verse, "", append, "");
  }

  database_oshb.optimize ();

  cout << "Completed" << endl;
}

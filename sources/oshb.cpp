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
#include <database/books.h>
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
using namespace std;
using namespace pugi;


void sources_oshb_parse ()
{
  std::cout << "Starting" << std::endl;

  Database_OsHb database_oshb;
  database_oshb.create ();
  database_oshb.optimize ();

  string file = "sources/oshb.xml";
  unlink (file.c_str());
  std::cout << file << std::endl;

  string command = "gunzip sources/oshb.xml.gz";
#ifndef HAVE_IOS
  [[maybe_unused]] auto result = system (command.c_str ());
#endif
  
  map <int, book_id> mapping = {
    
    // Torah
    
    // Bereshit Genesis
    pair (1, book_id::_genesis),
    // Shemot Exodus
    pair (2, book_id::_exodus),
    // Vayikra Leviticus
    pair (3, book_id::_leviticus),
    // Bəmidbar Numbers
    pair (4, book_id::_numbers),
    // Devarim Deuteronomy
    pair (5, book_id::_deuteronomy),
    
    // Nevi'im

    // Yĕhôshúa‘ Joshua
    pair (6, book_id::_joshua),
    // Shophtim Judges
    pair (7, book_id::_judges),
    // 1 Shmû’ēl 1 Samuel
    pair (8, book_id::_1_samuel),
    // 2 Shmû’ēl 2 Samuel
    pair (9, book_id::_2_samuel),
    // 1 M'lakhim 1 Kings
    pair (10, book_id::_1_kings),
    // 2 M'lakhim 2 Kings
    pair (11, book_id::_2_kings),
    // Yĕsha‘ăyāhû Isaiah
    pair (12, book_id::_isaiah),
    // Yirmyāhû Jeremiah
    pair (13, book_id::_jeremiah),
    // Yĕkhezqiēl Ezekiel
    pair (14, book_id::_ezekiel),
    // Hôshēa‘ Hosea
    pair (15, book_id::_hosea),
    // Yô’ēl Joel
    pair (16, book_id::_joel),
    // Āmôs Amos
    pair (17, book_id::_amos),
    // Ōvadhyāh Obadiah
    pair (18, book_id::_obadiah),
    // Yônāh Jonah
    pair (19, book_id::_jonah),
    // Mîkhāh Micah
    pair (20, book_id::_micah),
    // Nakḥûm Nahum
    pair (21, book_id::_nahum),
    // Khăvhakûk Habakkuk
    pair (22, book_id::_habakkuk),
    // Tsĕphanyāh Zephaniah
    pair (23, book_id::_zephaniah),
    // Khaggai Haggai
    pair (24, book_id::_haggai),
    // Zkharyāh Zechariah
    pair (25, book_id::_zechariah),
    // Mal’ākhî Malachi
    pair (26, book_id::_malachi),

    // Ketuvim
    
    // Tehillim (Psalms)
    pair (27, book_id::_psalms),
    // Mishlei (Book of Proverbs)
    pair (28, book_id::_proverbs),
    // Iyyôbh (Book of Job)
    pair (29, book_id::_job),
    // Shīr Hashīrīm (Song of Songs) or (Song of Solomon)
    pair (30, book_id::_song_of_solomon),
    // Rūth (Book of Ruth)
    pair (31, book_id::_ruth),
    // Eikhah (Lamentations)
    pair (32, book_id::_lamentations),
    // Qōheleth (Ecclesiastes)
    pair (33, book_id::_ecclesiastes),
    // Estēr (Book of Esther)
    pair (34, book_id::_esther),
    // Dānî’ēl (Book of Daniel)
    pair (35, book_id::_daniel),
    // 1 Ezrā (Book of Ezra)
    pair (36, book_id::_ezra),
    // 2 Ezrā (Book of Nehemiah)
    pair (37, book_id::_nehemiah),
    // 1 Divrei ha-Yamim (1 Chronicles)
    pair (38, book_id::_1_chronicles),
    // 2 Divrei ha-Yamim (2 Chronicles)
    pair (39, book_id::_2_chronicles),
    
  };
  
  book_id previous_book {book_id::_unknown};

  xml_document document;
  document.load_file (file.c_str(), parse_ws_pcdata_single);
  xml_node mysqldump_node = document.first_child ();
  xml_node database_node = mysqldump_node.first_child ();
  xml_node table_data_node = database_node.child ("table_data");
  for (xml_node row_node : table_data_node.children()) {
    book_id book {book_id::_unknown};
    int chapter = 0;
    int verse = 0;
    string word;
    string append;
    string lemma;
    string morph;
    for (xml_node field_node : row_node.children ()) {
      string name = field_node.attribute ("name").value ();
      string value = field_node.child_value ();
      if (name == "bookId") book = mapping [filter::strings::convert_to_int (value)];
      if (name == "chapter") chapter = filter::strings::convert_to_int (value);
      if (name == "verse") verse = filter::strings::convert_to_int (value);
      if (name == "word") word = value;
      if (name == "append") append = value;
      if (name == "lemma") lemma = value;
      if (name == "morph") morph = value;
    }
    if (book != previous_book) {
      previous_book = book;
      std::cout << database::books::get_english_from_id (book) << std::endl;
    }
    word = filter::strings::replace ("/", "", word);
    database_oshb.store (static_cast<int>(book), chapter, verse, lemma, word, morph);
    database_oshb.store (static_cast<int>(book), chapter, verse, "", append, "");
  }

  database_oshb.optimize ();

  std::cout << "Completed" << std::endl;
}

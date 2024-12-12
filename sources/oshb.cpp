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
#include <filter/shell.h>
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


void sources_oshb_parse ()
{
  std::cout << "Starting" << std::endl;

  Database_OsHb database_oshb;
  database_oshb.create ();
  database_oshb.optimize ();

  std::string file = "sources/oshb.xml";
  unlink (file.c_str());
  std::cout << file << std::endl;

  std::string command = std::string(filter::shell::get_executable(filter::shell::Executable::gunzip)) + " sources/oshb.xml.gz";
#ifndef HAVE_IOS
  [[maybe_unused]] auto result = system (command.c_str ());
#endif
  
  std::map <int, book_id> mapping = {
    
    // Torah
    
    // Bereshit Genesis
    std::pair (1, book_id::_genesis),
    // Shemot Exodus
    std::pair (2, book_id::_exodus),
    // Vayikra Leviticus
    std::pair (3, book_id::_leviticus),
    // Bəmidbar Numbers
    std::pair (4, book_id::_numbers),
    // Devarim Deuteronomy
    std::pair (5, book_id::_deuteronomy),
    
    // Nevi'im

    // Yĕhôshúa‘ Joshua
    std::pair (6, book_id::_joshua),
    // Shophtim Judges
    std::pair (7, book_id::_judges),
    // 1 Shmû’ēl 1 Samuel
    std::pair (8, book_id::_1_samuel),
    // 2 Shmû’ēl 2 Samuel
    std::pair (9, book_id::_2_samuel),
    // 1 M'lakhim 1 Kings
    std::pair (10, book_id::_1_kings),
    // 2 M'lakhim 2 Kings
    std::pair (11, book_id::_2_kings),
    // Yĕsha‘ăyāhû Isaiah
    std::pair (12, book_id::_isaiah),
    // Yirmyāhû Jeremiah
    std::pair (13, book_id::_jeremiah),
    // Yĕkhezqiēl Ezekiel
    std::pair (14, book_id::_ezekiel),
    // Hôshēa‘ Hosea
    std::pair (15, book_id::_hosea),
    // Yô’ēl Joel
    std::pair (16, book_id::_joel),
    // Āmôs Amos
    std::pair (17, book_id::_amos),
    // Ōvadhyāh Obadiah
    std::pair (18, book_id::_obadiah),
    // Yônāh Jonah
    std::pair (19, book_id::_jonah),
    // Mîkhāh Micah
    std::pair (20, book_id::_micah),
    // Nakḥûm Nahum
    std::pair (21, book_id::_nahum),
    // Khăvhakûk Habakkuk
    std::pair (22, book_id::_habakkuk),
    // Tsĕphanyāh Zephaniah
    std::pair (23, book_id::_zephaniah),
    // Khaggai Haggai
    std::pair (24, book_id::_haggai),
    // Zkharyāh Zechariah
    std::pair (25, book_id::_zechariah),
    // Mal’ākhî Malachi
    std::pair (26, book_id::_malachi),

    // Ketuvim
    
    // Tehillim (Psalms)
    std::pair (27, book_id::_psalms),
    // Mishlei (Book of Proverbs)
    std::pair (28, book_id::_proverbs),
    // Iyyôbh (Book of Job)
    std::pair (29, book_id::_job),
    // Shīr Hashīrīm (Song of Songs) or (Song of Solomon)
    std::pair (30, book_id::_song_of_solomon),
    // Rūth (Book of Ruth)
    std::pair (31, book_id::_ruth),
    // Eikhah (Lamentations)
    std::pair (32, book_id::_lamentations),
    // Qōheleth (Ecclesiastes)
    std::pair (33, book_id::_ecclesiastes),
    // Estēr (Book of Esther)
    std::pair (34, book_id::_esther),
    // Dānî’ēl (Book of Daniel)
    std::pair (35, book_id::_daniel),
    // 1 Ezrā (Book of Ezra)
    std::pair (36, book_id::_ezra),
    // 2 Ezrā (Book of Nehemiah)
    std::pair (37, book_id::_nehemiah),
    // 1 Divrei ha-Yamim (1 Chronicles)
    std::pair (38, book_id::_1_chronicles),
    // 2 Divrei ha-Yamim (2 Chronicles)
    std::pair (39, book_id::_2_chronicles),
    
  };
  
  book_id previous_book {book_id::_unknown};

  pugi::xml_document document;
  document.load_file (file.c_str(), pugi::parse_ws_pcdata_single);
  pugi::xml_node mysqldump_node = document.first_child ();
  pugi::xml_node database_node = mysqldump_node.first_child ();
  pugi::xml_node table_data_node = database_node.child ("table_data");
  for (pugi::xml_node row_node : table_data_node.children()) {
    book_id book {book_id::_unknown};
    int chapter = 0;
    int verse = 0;
    std::string word;
    std::string append;
    std::string lemma;
    std::string morph;
    for (pugi::xml_node field_node : row_node.children ()) {
      std::string name = field_node.attribute ("name").value ();
      std::string value = field_node.child_value ();
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

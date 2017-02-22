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


#include <sources/oshb.h>
#include <database/oshb.h>
#include <database/books.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <pugixml/pugixml.hpp>


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
  system (command.c_str ());
  
  map <int, int> mapping = {
    
    // Torah
    
    // Bereshit Genesis
    make_pair (1, Database_Books::getIdFromEnglish ("Genesis")),
    // Shemot Exodus
    make_pair (2, Database_Books::getIdFromEnglish ("Exodus")),
    // Vayikra Leviticus
    make_pair (3, Database_Books::getIdFromEnglish ("Leviticus")),
    // Bəmidbar Numbers
    make_pair (4, Database_Books::getIdFromEnglish ("Numbers")),
    // Devarim Deuteronomy
    make_pair (5, Database_Books::getIdFromEnglish ("Deuteronomy")),
    
    // Nevi'im

    // Yĕhôshúa‘ Joshua
    make_pair (6, Database_Books::getIdFromEnglish ("Joshua")),
    // Shophtim Judges
    make_pair (7, Database_Books::getIdFromEnglish ("Judges")),
    // 1 Shmû’ēl 1 Samuel
    make_pair (8, Database_Books::getIdFromEnglish ("1 Samuel")),
    // 2 Shmû’ēl 2 Samuel
    make_pair (9, Database_Books::getIdFromEnglish ("2 Samuel")),
    // 1 M'lakhim 1 Kings
    make_pair (10, Database_Books::getIdFromEnglish ("1 Kings")),
    // 2 M'lakhim 2 Kings
    make_pair (11, Database_Books::getIdFromEnglish ("2 Kings")),
    // Yĕsha‘ăyāhû Isaiah
    make_pair (12, Database_Books::getIdFromEnglish ("Isaiah")),
    // Yirmyāhû Jeremiah
    make_pair (13, Database_Books::getIdFromEnglish ("Jeremiah")),
    // Yĕkhezqiēl Ezekiel
    make_pair (14, Database_Books::getIdFromEnglish ("Ezekiel")),
    // Hôshēa‘ Hosea
    make_pair (15, Database_Books::getIdFromEnglish ("Hosea")),
    // Yô’ēl Joel
    make_pair (16, Database_Books::getIdFromEnglish ("Joel")),
    // Āmôs Amos
    make_pair (17, Database_Books::getIdFromEnglish ("Amos")),
    // Ōvadhyāh Obadiah
    make_pair (18, Database_Books::getIdFromEnglish ("Obadiah")),
    // Yônāh Jonah
    make_pair (19, Database_Books::getIdFromEnglish ("Jonah")),
    // Mîkhāh Micah
    make_pair (20, Database_Books::getIdFromEnglish ("Micah")),
    // Nakḥûm Nahum
    make_pair (21, Database_Books::getIdFromEnglish ("Nahum")),
    // Khăvhakûk Habakkuk
    make_pair (22, Database_Books::getIdFromEnglish ("Habakkuk")),
    // Tsĕphanyāh Zephaniah
    make_pair (23, Database_Books::getIdFromEnglish ("Zephaniah")),
    // Khaggai Haggai
    make_pair (24, Database_Books::getIdFromEnglish ("Haggai")),
    // Zkharyāh Zechariah
    make_pair (25, Database_Books::getIdFromEnglish ("Zechariah")),
    // Mal’ākhî Malachi
    make_pair (26, Database_Books::getIdFromEnglish ("Malachi")),

    // Ketuvim
    
    // Tehillim (Psalms)
    make_pair (27, Database_Books::getIdFromEnglish ("Psalms")),
    // Mishlei (Book of Proverbs)
    make_pair (28, Database_Books::getIdFromEnglish ("Proverbs")),
    // Iyyôbh (Book of Job)
    make_pair (29, Database_Books::getIdFromEnglish ("Job")),
    // Shīr Hashīrīm (Song of Songs) or (Song of Solomon)
    make_pair (30, Database_Books::getIdFromEnglish ("Song of Solomon")),
    // Rūth (Book of Ruth)
    make_pair (31, Database_Books::getIdFromEnglish ("Ruth")),
    // Eikhah (Lamentations)
    make_pair (32, Database_Books::getIdFromEnglish ("Lamentations")),
    // Qōheleth (Ecclesiastes)
    make_pair (33, Database_Books::getIdFromEnglish ("Ecclesiastes")),
    // Estēr (Book of Esther)
    make_pair (34, Database_Books::getIdFromEnglish ("Esther")),
    // Dānî’ēl (Book of Daniel)
    make_pair (35, Database_Books::getIdFromEnglish ("Daniel")),
    // 1 Ezrā (Book of Ezra)
    make_pair (36, Database_Books::getIdFromEnglish ("Ezra")),
    // 2 Ezrā (Book of Nehemiah)
    make_pair (37, Database_Books::getIdFromEnglish ("Nehemiah")),
    // 1 Divrei ha-Yamim (1 Chronicles)
    make_pair (38, Database_Books::getIdFromEnglish ("1 Chronicles")),
    // 2 Divrei ha-Yamim (2 Chronicles)
    make_pair (39, Database_Books::getIdFromEnglish ("2 Chronicles"))
    
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
      cout << Database_Books::getEnglishFromId (book) << endl;
    }
    word = filter_string_str_replace ("/", "", word);
    database_oshb.store (book, chapter, verse, lemma, word, morph);
    database_oshb.store (book, chapter, verse, "", append, "");
  }

  database_oshb.optimize ();

  cout << "Completed" << endl;
}

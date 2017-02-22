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


#include <database/books.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <filter/string.h>
#include <filter/diff.h>


// All data is stored in the code in memory, not in a database on disk.


typedef struct
{
  const char *english;     // English name.
  const char *osis;        // OSIS abbreviation.
  const char *usfm;        // USFM ID.
  const char *bibleworks;  // BibleWorks abbreviation.
  const char *onlinebible; // Online Bible abbreviation.
  int id;                  // Bibledit's internal id.
  const char *type;        // The type of the book.
  bool onechapter;         // The book has one chapter.
} book_record;


/*
This table gives the books Bibledit knows about.
The books are put in the standard order.

A note about this data.

* The id should always be a number higher than 0, because 0 is taken for "not found".
* The id is connected to a book, and is used throughout the databases.
  Therefore, ids are not supposed to be changed; new ones can be added though.
* type: One of the following
    frontback - Frontmatter / Backmatter
    ot        - Old Testament
    nt        - New Testament
    other     - Other matter
    ap        - Apocrypha
*/
book_record books_table [] =
{
  {"Genesis",                    "Gen",     "GEN",  "Gen",  "Ge",    1, "ot",        false}, // ‘1 Moses’ in some Bibles.
  {"Exodus",                     "Exod",    "EXO",  "Exo",  "Ex",    2, "ot",        false}, // ‘2 Moses’ in some Bibles.
  {"Leviticus",                  "Lev",     "LEV",  "Lev",  "Le",    3, "ot",        false}, // ‘3 Moses’ in some Bibles.
  {"Numbers",                    "Num",     "NUM",  "Num",  "Nu",    4, "ot",        false}, // ‘4 Moses’ in some Bibles.
  {"Deuteronomy",                "Deut",    "DEU",  "Deu",  "De",    5, "ot",        false}, // ‘5 Moses’ in some Bibles.
  {"Joshua",                     "Josh",    "JOS",  "Jos",  "Jos",   6, "ot",        false},
  {"Judges",                     "Judg",    "JDG",  "Jdg",  "Jud",   7, "ot",        false},
  {"Ruth",                       "Ruth",    "RUT",  "Rut",  "Ru",    8, "ot",        false},
  {"1 Samuel",                   "1Sam",    "1SA",  "1Sa",  "1Sa",   9, "ot",        false}, // 1 Kings or Kingdoms in Orthodox Bibles. Do not confuse this abbreviation with ISA for Isaiah.
  {"2 Samuel",                   "2Sam",    "2SA",  "2Sa",  "2Sa",  10, "ot",        false}, // 2 Kings or Kingdoms in Orthodox Bibles.
  {"1 Kings",                    "1Kgs",    "1KI",  "1Ki",  "1Ki",  11, "ot",        false}, // 3 Kings or Kingdoms in Orthodox Bibles.
  {"2 Kings",                    "2Kgs",    "2KI",  "2Ki",  "2Ki",  12, "ot",        false}, // 4 Kings or Kingdoms in Orthodox Bibles.
  {"1 Chronicles",               "1Chr",    "1CH",  "1Ch",  "1Ch",  13, "ot",        false}, // 1 Paralipomenon in Orthodox Bibles.
  {"2 Chronicles",               "2Chr",    "2CH",  "2Ch",  "2Ch",  14, "ot",        false}, // 2 Paralipomenon in Orthodox Bibles.
  {"Ezra",                       "Ezra",    "EZR",  "Ezr",  "Ezr",  15, "ot",        false}, // This is for Hebrew Ezra, sometimes called 1 Ezra or 1 Esdras. Also for Ezra-Nehemiah when one book.
  {"Nehemiah",                   "Neh",     "NEH",  "Neh",  "Ne",   16, "ot",        false}, // Sometimes appended to Ezra; called 2 Esdras in the Vulgate.
  {"Esther",                     "Esth",    "EST",  "Est",  "Es",   17, "ot",        false}, // This is for Hebrew Esther. For the longer Greek LXX Esther use ESG.
  {"Job",                        "Job",     "JOB",  "Job",  "Job",  18, "ot",        false},
  {"Psalms",                     "Ps",      "PSA",  "Psa",  "Ps",   19, "ot",        false}, // 150 Psalms in Hebrew, 151 Psalms in Orthodox Bibles, 155 Psalms in West Syriac Bibles. If you put Psalm 151 separately in an Apocrypha use PS2, for Psalms 152-155 use PS3.
  {"Proverbs",                   "Prov",    "PRO",  "Pro",  "Pr",   20, "ot",        false}, // 31 Proverbs, but 24 Proverbs in the Ethiopian Bible.
  {"Ecclesiastes",               "Eccl",    "ECC",  "Ecc",  "Ec",   21, "ot",        false}, // Qoholeth in Catholic Bibles; for Ecclesiasticus use SIR.
  {"Song of Solomon",            "Song",    "SNG",  "Sol",  "So",   22, "ot",        false}, // Song of Solomon, or Canticles of Canticles in Catholic Bibles.
  {"Isaiah",                     "Isa",     "ISA",  "Isa",  "Isa",  23, "ot",        false}, // Do not confuse this abbreviation with 1SA for 1 Samuel.
  {"Jeremiah",                   "Jer",     "JER",  "Jer",  "Jer",  24, "ot",        false}, // The Book of Jeremiah; for the Letter of Jeremiah use LJE.
  {"Lamentations",               "Lam",     "LAM",  "Lam",  "La",   25, "ot",        false}, // The Lamentations of Jeremiah.
  {"Ezekiel",                    "Ezek",    "EZK",  "Eze",  "Eze",  26, "ot",        false},
  {"Daniel",                     "Dan",     "DAN",  "Dan",  "Da",   27, "ot",        false}, // This is for Hebrew Daniel; for the longer Greek LXX Daniel use DAG.
  {"Hosea",                      "Hos",     "HOS",  "Hos",  "Ho",   28, "ot",        false},
  {"Joel",                       "Joel",    "JOL",  "Joe",  "Joe",  29, "ot",        false},
  {"Amos",                       "Amos",    "AMO",  "Amo",  "Am",   30, "ot",        false},
  {"Obadiah",                    "Obad",    "OBA",  "Oba",  "Ob",   31, "ot",        true},
  {"Jonah",                      "Jonah",   "JON",  "Jon",  "Jon",  32, "ot",        false}, // Do not confuse this abbreviation with JHN for John.
  {"Micah",                      "Mic",     "MIC",  "Mic",  "Mic",  33, "ot",        false},
  {"Nahum",                      "Nah",     "NAM",  "Nah",  "Na",   34, "ot",        false},
  {"Habakkuk",                   "Hab",     "HAB",  "Hab",  "Hab",  35, "ot",        false},
  {"Zephaniah",                  "Zeph",    "ZEP",  "Zep",  "Zep",  36, "ot",        false},
  {"Haggai",                     "Hag",     "HAG",  "Hag",  "Hag",  37, "ot",        false},
  {"Zechariah",                  "Zech",    "ZEC",  "Zec",  "Zec",  38, "ot",        false},
  {"Malachi",                    "Mal",     "MAL",  "Mal",  "Mal",  39, "ot",        false},
  {"Matthew",                    "Matt",    "MAT",  "Mat",  "Mt",   40, "nt",        false}, // The Gospel according to Matthew.
  {"Mark",                       "Mark",    "MRK",  "Mar",  "Mr",   41, "nt",        false}, // The Gospel according to Mark.
  {"Luke",                       "Luke",    "LUK",  "Luk",  "Lu",   42, "nt",        false}, // The Gospel according to Luke.
  {"John",                       "John",    "JHN",  "Joh",  "Joh",  43, "nt",        false}, // The Gospel according to John.
  {"Acts",                       "Acts",    "ACT",  "Act",  "Ac",   44, "nt",        false}, // The Acts of the Apostles.
  {"Romans",                     "Rom",     "ROM",  "Rom",  "Ro",   45, "nt",        false}, // The Letter of Paul to the Romans.
  {"1 Corinthians",              "1Cor",    "1CO",  "1Co",  "1Co",  46, "nt",        false}, // The First Letter of Paul to the Corinthians.
  {"2 Corinthians",              "2Cor",    "2CO",  "2Co",  "2Co",  47, "nt",        false}, // The Second Letter of Paul to the Corinthians.
  {"Galatians",                  "Gal",     "GAL",  "Gal",  "Ga",   48, "nt",        false}, // The Letter of Paul to the Galatians.
  {"Ephesians",                  "Eph",     "EPH",  "Eph",  "Eph",  49, "nt",        false}, // The Letter of Paul to the Ephesians.
  {"Philippians",                "Phil",    "PHP",  "Phi",  "Php",  50, "nt",        false}, // The Letter of Paul to the Philippians.
  {"Colossians",                 "Col",     "COL",  "Col",  "Col",  51, "nt",        false}, // The Letter of Paul to the Colossians.
  {"1 Thessalonians",            "1Thess",  "1TH",  "1Th",  "1Th",  52, "nt",        false}, // The First Letter of Paul to the Thessalonians.
  {"2 Thessalonians",            "2Thess",  "2TH",  "2Th",  "2Th",  53, "nt",        false}, // The Second Letter of Paul to the Thessalonians.
  {"1 Timothy",                  "1Tim",    "1TI",  "1Ti",  "1Ti",  54, "nt",        false}, // The First Letter of Paul to Timothy.
  {"2 Timothy",                  "2Tim",    "2TI",  "2Ti",  "2Ti",  55, "nt",        false}, // The Second Letter of Paul to Timothy.
  {"Titus",                      "Titus",   "TIT",  "Tit",  "Tit",  56, "nt",        false}, // The Letter of Paul to Titus.
  {"Philemon",                   "Phlm",    "PHM",  "Phm",  "Phm",  57, "nt",        true},  // The Letter of Paul to Philemon.
  {"Hebrews",                    "Heb",     "HEB",  "Heb",  "Heb",  58, "nt",        false}, // The Letter to the Hebrews.
  {"James",                      "Jas",     "JAS",  "Jam",  "Jas",  59, "nt",        false}, // The Letter of James.
  {"1 Peter",                    "1Pet",    "1PE",  "1Pe",  "1Pe",  60, "nt",        false}, // The First Letter of Peter.
  {"2 Peter",                    "2Pet",    "2PE",  "2Pe",  "2Pe",  61, "nt",        false}, // The Second Letter of Peter.
  {"1 John",                     "1John",   "1JN",  "1Jo",  "1Jo",  62, "nt",        false}, // The First Letter of John.
  {"2 John",                     "2John",   "2JN",  "2Jo",  "2Jo",  63, "nt",        false}, // The Second Letter of John.
  {"3 John",                     "3John",   "3JN",  "3Jo",  "3Jo",  64, "nt",        true},  // The Third Letter of John.
  {"Jude",                       "Jude",    "JUD",  "Jud",  "Jude", 65, "nt",        true},  // The Letter of Jude; do not confuse this abbreviation with JDG for Judges, or JDT for Judith.
  {"Revelation",                 "Rev",     "REV",  "Rev",  "Re",   66, "nt",        false}, // The Revelation to John; called Apocalypse in Catholic Bibles.
  {"Front Matter",               "",        "FRT",  "",     "",     67, "frontback", false},
  {"Back Matter",                "",        "BAK",  "",     "",     68, "frontback", false},
  {"Other Material",             "",        "OTH",  "",     "",     69, "other",     false},
  {"Tobit",                      "Tob",     "TOB",  "Tob",  "",     70, "ap",        false},
  {"Judith",                     "Jdt",     "JDT",  "Jdt",  "",     71, "ap",        false},
  {"Esther (Greek)",             "AddEsth", "ESG",  "EsG",  "",     72, "ap",        false},
  {"Wisdom of Solomon",          "Wis",     "WIS",  "Wis",  "",     73, "ap",        false},
  {"Sirach",                     "Sir",     "SIR",  "Sir",  "",     74, "ap",        false}, // Ecclesiasticus or Jesus son of Sirach.
  {"Baruch",                     "Bar",     "BAR",  "Bar",  "",     75, "ap",        false}, // 5 chapters in Orthodox Bibles (LJE is separate); 6 chapters in Catholic Bibles (includes LJE); called 1 Baruch in Syriac Bibles.
  {"Letter of Jeremiah",         "EpJer",   "LJE",  "LJe",  "",     76, "ap",        true},  // Sometimes included in Baruch; called ‘Rest of Jeremiah’ in Ethiopia.
  {"Song of the Three Children", "PrAzar",  "S3Y",  "S3Y",  "",     77, "ap",        true},  // Includes the Prayer of Azariah; sometimes included in Greek Daniel.
  {"Susanna",                    "Sus",     "SUS",  "Sus",  "",     78, "ap",        true},  // Sometimes included in Greek Daniel.
  {"Bel and the Dragon",         "Bel",     "BEL",  "Bel",  "",     79, "ap",        true},  // Sometimes included in Greek Daniel; called ‘Rest of Daniel’ in Ethiopia.
  {"1 Maccabees",                "1Macc",   "1MA",  "1Ma",  "",     80, "ap",        false}, // Called ‘3 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
  {"2 Maccabees",                "2Macc",   "2MA",  "2Ma",  "",     81, "ap",        false}, // Called ‘1 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
  {"1 Esdras (Greek)",           "1Esd",    "1ES",  "1Es",  "",     82, "ap",        false}, // The 9-chapter book of Greek Ezra in the LXX, called ‘2 Esdras’ in Russian Bibles, and called ‘3 Esdras’ in the Vulgate; when Ezra-Nehemiah is one book use EZR.
  {"Prayer of Manasses",         "PrMan",   "MAN",  "Man",  "",     83, "ap",        true},  // Sometimes appended to 2 Chronicles. Included in Orthodox Bibles.
  {"Psalm 151",                  "Ps151",   "PS2",  "Ps2",  "",     84, "ap",        true},  // An additional Psalm in the Septuagint. Appended to Psalms in Orthodox Bibles.
  {"3 Maccabees",                "3Macc",   "3MA",  "3Ma",  "",     85, "ap",        false}, // Called ‘2 Maccabees’ in some traditions, printed in Orthodox Bibles.
  {"2 Esdras (Latin)",           "2Esd",    "2ES",  "2Es",  "",     86, "ap",        false}, // The 16 chapter book of Latin Esdras called ‘3 Esdras’ in Russian Bibles and called ‘4 Esdras’ in the Vulgate. For the 12 chapter Apocalypse of Ezra use EZA.
  {"4 Maccabees",                "4Macc",   "4MA",  "4Ma",  "",     87, "ap",        false}, // In an appendix to the Greek Bible and in the Georgian Bible.
  {"Daniel (Greek)",             "",        "DAG",  "",     "",     88, "ap",        false}, // The 14-chapter version of Daniel from the Septuagint including Greek additions.
  {"Odes",                       "",        "ODA",  "",     "",     89, "ap",        false}, // Or Odae. A book in some editions of the Septuagint. Odes has different contents in Greek, Russian, and Syriac traditions.
  {"Psalms of Solomon",          "",        "PSS",  "",     "",     90, "ap",        false}, // A book in some editions of the Septuagint, but not printed in modern Bibles.
  {"Ezra Apocalypse",            "",        "EZA",  "",     "",     91, "ap",        false}, // 12-Chapter book of Ezra Apocalypse. Called ‘3 Ezra’ in the Armenian Bible. Called ‘Ezra Shealtiel’ in the Ethiopian Bible. Formerly called 4ES; called ‘2 Esdras’ when it includes 5 Ezra and 6 Ezra.
  {"5 Ezra",                     "",        "5EZ",  "",     "",     92, "ap",        false}, // 2-Chapter Latin preface to Ezra Apocalypse. Formerly called 5ES.
  {"6 Ezra",                     "",        "6EZ",  "",     "",     93, "ap",        false}, // 2-Chapter Latin conclusion to Ezra Apocalypse. Formerly called 6ES.
  {"Psalms 152-155",             "",        "PS3",  "",     "",     94, "ap",        false}, // Additional Psalms 152-155 found in West Syriac manuscripts.
  {"2 Baruch (Apocalypse)",      "",        "2BA",  "",     "",     95, "ap",        false}, // The Apocalypse of Baruch in Syriac Bibles.
  {"Letter of Baruch",           "",        "LBA",  "",     "",     96, "ap",        false}, // Sometimes appended to 2 Baruch. Sometimes separate in Syriac Bibles.
  {"Jubilees",                   "",        "JUB",  "",     "",     97, "ap",        false}, // Ancient Hebrew book used in the Ethiopian Bible.
  {"Enoch",                      "",        "ENO",  "",     "",     98, "ap",        false}, // Sometimes called ‘1 Enoch’. Ancient Hebrew book in the Ethiopian Bible.
  {"1 Meqabyan/Mekabis",         "",        "1MQ",  "",     "",     99, "ap",        false}, // Book of Mekabis of Benjamin in the Ethiopian Bible.
  {"2 Meqabyan/Mekabis",         "",        "2MQ",  "",     "",    100, "ap",        false}, // Book of Mekabis of Moab in the Ethiopian Bible.
  {"3 Meqabyan/Mekabis",         "",        "3MQ",  "",     "",    101, "ap",        false}, // Book of Meqabyan in the Ethiopian Bible.
  {"Reproof",                    "",        "REP",  "",     "",    102, "ap",        false}, // Proverbs part 2. Used in the Ethiopian Bible.
  {"4 Baruch",                   "",        "4BA",  "",     "",    103, "ap",        false}, // Paralipomenon of Jeremiah, called ‘Rest of the Words of Baruch’ in Ethiopia. May include or exclude the Letter of Jeremiah as chapter 1. Used in the Ethiopian Bible.
  {"Letter to the Laodiceans",   "",        "LAO",  "",     "",    104, "ap",        false}, // A Latin Vulgate book, found in the Vulgate and some medieval Catholic translations.
  {"Introduction Matter",        "",        "INT",  "",     "",     89, "frontback", false},
  {"Concordance",                "",        "CNC",  "",     "",     90, "frontback", false},
  {"Glossary / Wordlist",        "",        "GLO",  "",     "",     91, "frontback", false},
  {"Topical Index",              "",        "TDX",  "",     "",     92, "frontback", false},
  {"Names Index",                "",        "NDX",  "",     "",     93, "frontback", false}
};


// For OSIS abbreviations see http://www.crosswire.org/wiki/OSIS_Book_Abbreviations.


vector <int> Database_Books::getIDs ()
{
  vector <int> ids;
  for (unsigned int i = 0; i < data_count (); i++) {
    // Exclude Apocrypha.
    int id = books_table[i].id;
    if (id < 70) ids.push_back (id);
  }
  return ids;
}


int Database_Books::getIdFromEnglish (string english)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (english == books_table[i].english) {
      return books_table[i].id;
    }
  }
  return 0;  
}


string Database_Books::getEnglishFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].english;
    }
  }
  return "Unknown";
}


string Database_Books::getUsfmFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string Database_Books::getBibleworksFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string Database_Books::getOsisFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].osis;
    }
  }
  return "Unknown";
}


int Database_Books::getIdFromUsfm (string usfm)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id;
    }
  }
  return 0;
}


int Database_Books::getIdFromOsis (string osis)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id;
    }
  }
  return 0;
}


int Database_Books::getIdFromBibleworks (string bibleworks)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id;
    }
  }
  return 0;
}


/*
Tries to interprete $text as the name of a Bible book.
Returns the book's identifier if it succeeds.
If it fails, it returns 0.
*/
int Database_Books::getIdLikeText (string text)
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids;
  vector <int> similarities;
  for (unsigned int i = 0; i < data_count(); i++) {
    int id = books_table[i].id;
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].english));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].osis));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].usfm));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].bibleworks));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].onlinebible));
  }
  quick_sort (similarities, ids, 0, ids.size());
  return ids.back ();
}


int Database_Books::getIdFromOnlinebible (string onlinebible)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id;
    }
  }
  return 0;
}


string Database_Books::getOnlinebibleFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].onlinebible;
    }
  }
  return "";
}


int Database_Books::getIdLastEffort (string text)
{
  // Remove all spaces.
  text = filter_string_str_replace (" ", "", text);
  text = unicode_string_casefold (text);
  // Length.
  int length = text.length ();
  // Go through all book data, convert everything to lower case, and remove all spaces, and compare only "length" characters.
  for (unsigned int i = 0; i < data_count(); i++) {
    string book;
    book = unicode_string_casefold (filter_string_str_replace (" ", "", books_table[i].english));
    if (text == book.substr (0, length)) return books_table[i].id;
    book = unicode_string_casefold (filter_string_str_replace (" ", "", books_table[i].osis));
    if (text == book.substr (0, length)) return books_table[i].id;
    book = unicode_string_casefold (filter_string_str_replace (" ", "", books_table[i].usfm));
    if (text == book.substr (0, length)) return books_table[i].id;
    book = unicode_string_casefold (filter_string_str_replace (" ", "", books_table[i].bibleworks));
    if (text == book.substr (0, length)) return books_table[i].id;
    book = unicode_string_casefold (filter_string_str_replace (" ", "", books_table[i].onlinebible));
    if (text == book.substr (0, length)) return books_table[i].id;
  }
  return 0;
}


int Database_Books::getSequenceFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return i;
    }
  }
  return 0;
}


string Database_Books::getType (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].type;
    }
  }
  return "";
}


unsigned int Database_Books::data_count ()
{
  return sizeof (books_table) / sizeof (*books_table);
}

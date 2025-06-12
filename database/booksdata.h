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


// All data is stored in the code in memory, not in a database on disk.


struct book_record
{
  const char *english;     // English name.
  const char *osis;        // OSIS abbreviation.
  const char *usfm;        // USFM ID.
  const char *bibleworks;  // BibleWorks abbreviation.
  const char *onlinebible; // Online Bible abbreviation.
  book_id id;              // Bibledit's internal book identifier as an enum.
  short order;             // The order of the books.
  book_type type;          // The type of the book as an enum.
  bool onechapter;         // Whether the book has one chapter.
};


/*
This table gives the books Bibledit knows about.
The books are put in the standard order.

A note about this data.

* The id should always be a number higher than 0, because 0 is taken for "not found".
* The id is connected to a book, and is used throughout the databases.
  Therefore, ids are not supposed to be changed; new ones can be added though.
* The order describes the order of the books. Books may be assigned an updated order when needed.
* type: One of the following
    frontback - Frontmatter / Backmatter
    ot        - Old Testament
    nt        - New Testament
    other     - Other matter
    ap        - Apocrypha
*/
constexpr const book_record books_table [] =
{
  {"Genesis", "Gen", "GEN", "Gen", "Ge", book_id::_genesis, 3, book_type::old_testament, false}, // ‘1 Moses’ in some Bibles.
  {"Exodus", "Exod", "EXO", "Exo", "Ex", book_id::_exodus, 4, book_type::old_testament, false}, // ‘2 Moses’ in some Bibles.
  {"Leviticus", "Lev", "LEV", "Lev", "Le", book_id::_leviticus, 5, book_type::old_testament, false}, // ‘3 Moses’ in some Bibles.
  {"Numbers", "Num", "NUM", "Num", "Nu", book_id::_numbers, 6, book_type::old_testament, false}, // ‘4 Moses’ in some Bibles.
  {"Deuteronomy", "Deut", "DEU", "Deu", "De", book_id::_deuteronomy, 7, book_type::old_testament, false}, // ‘5 Moses’ in some Bibles.
  {"Joshua", "Josh", "JOS", "Jos", "Jos", book_id::_joshua, 8, book_type::old_testament, false},
  {"Judges", "Judg", "JDG", "Jdg", "Jud", book_id::_judges, 9, book_type::old_testament, false},
  {"Ruth", "Ruth", "RUT", "Rut", "Ru", book_id::_ruth, 10, book_type::old_testament, false},
  {"1 Samuel", "1Sam", "1SA", "1Sa", "1Sa", book_id::_1_samuel, 11, book_type::old_testament, false}, // 1 Kings or Kingdoms in Orthodox Bibles. Do not confuse this abbreviation with ISA for Isaiah.
  {"2 Samuel", "2Sam", "2SA", "2Sa", "2Sa", book_id::_2_samuel, 12, book_type::old_testament, false}, // 2 Kings or Kingdoms in Orthodox Bibles.
  {"1 Kings", "1Kgs", "1KI", "1Ki", "1Ki", book_id::_1_kings, 13, book_type::old_testament, false}, // 3 Kings or Kingdoms in Orthodox Bibles.
  {"2 Kings", "2Kgs", "2KI", "2Ki", "2Ki", book_id::_2_kings, 14, book_type::old_testament, false}, // 4 Kings or Kingdoms in Orthodox Bibles.
  {"1 Chronicles", "1Chr", "1CH", "1Ch", "1Ch", book_id::_1_chronicles, 15, book_type::old_testament, false}, // 1 Paralipomenon in Orthodox Bibles.
  {"2 Chronicles", "2Chr", "2CH", "2Ch", "2Ch", book_id::_2_chronicles, 16, book_type::old_testament, false}, // 2 Paralipomenon in Orthodox Bibles.
  {"Ezra", "Ezra", "EZR", "Ezr", "Ezr", book_id::_ezra, 17, book_type::old_testament, false}, // This is for Hebrew Ezra, sometimes called 1 Ezra or 1 Esdras. Also for Ezra-Nehemiah when one book.
  {"Nehemiah", "Neh", "NEH", "Neh", "Ne", book_id::_nehemiah, 18, book_type::old_testament, false}, // Sometimes appended to Ezra; called 2 Esdras in the Vulgate.
  {"Esther", "Esth", "EST", "Est", "Es", book_id::_esther, 19, book_type::old_testament, false}, // This is for Hebrew Esther. For the longer Greek LXX Esther use ESG.
  {"Job", "Job", "JOB", "Job", "Job", book_id::_job, 20, book_type::old_testament, false},
  {"Psalms", "Ps", "PSA", "Psa", "Ps", book_id::_psalms, 21, book_type::old_testament, false}, // 150 Psalms in Hebrew, 151 Psalms in Orthodox Bibles, 155 Psalms in West Syriac Bibles. If you put Psalm 151 separately in an Apocrypha use PS2, for Psalms 152-155 use PS3.
  {"Proverbs", "Prov", "PRO", "Pro", "Pr", book_id::_proverbs, 22, book_type::old_testament, false}, // 31 Proverbs, but 24 Proverbs in the Ethiopian Bible.
  {"Ecclesiastes", "Eccl", "ECC", "Ecc", "Ec", book_id::_ecclesiastes, 23, book_type::old_testament, false}, // Qoholeth in Catholic Bibles; for Ecclesiasticus use SIR.
  {"Song of Solomon", "Song", "SNG", "Sol", "So", book_id::_song_of_solomon, 24, book_type::old_testament, false}, // Song of Solomon, or Canticles of Canticles in Catholic Bibles.
  {"Isaiah", "Isa", "ISA", "Isa", "Isa", book_id::_isaiah, 25, book_type::old_testament, false}, // Do not confuse this abbreviation with 1SA for 1 Samuel.
  {"Jeremiah", "Jer", "JER", "Jer", "Jer", book_id::_jeremiah, 26, book_type::old_testament, false}, // The Book of Jeremiah; for the Letter of Jeremiah use LJE.
  {"Lamentations", "Lam", "LAM", "Lam", "La", book_id::_lamentations, 27, book_type::old_testament, false}, // The Lamentations of Jeremiah.
  {"Ezekiel", "Ezek", "EZK", "Eze", "Eze", book_id::_ezekiel, 28, book_type::old_testament, false},
  {"Daniel", "Dan", "DAN", "Dan", "Da", book_id::_daniel, 29, book_type::old_testament, false}, // This is for Hebrew Daniel; for the longer Greek LXX Daniel use DAG.
  {"Hosea", "Hos", "HOS", "Hos", "Ho", book_id::_hosea, 30, book_type::old_testament, false},
  {"Joel", "Joel", "JOL", "Joe", "Joe", book_id::_joel, 31, book_type::old_testament, false},
  {"Amos", "Amos", "AMO", "Amo", "Am", book_id::_amos, 32, book_type::old_testament, false},
  {"Obadiah", "Obad", "OBA", "Oba", "Ob", book_id::_obadiah, 33, book_type::old_testament, true},
  {"Jonah", "Jonah", "JON", "Jon", "Jon", book_id::_jonah, 34, book_type::old_testament, false}, // Do not confuse this abbreviation with JHN for John.
  {"Micah", "Mic", "MIC", "Mic", "Mic", book_id::_micah, 35, book_type::old_testament, false},
  {"Nahum", "Nah", "NAM", "Nah", "Na", book_id::_nahum, 36, book_type::old_testament, false},
  {"Habakkuk", "Hab", "HAB", "Hab", "Hab", book_id::_habakkuk, 37, book_type::old_testament, false},
  {"Zephaniah", "Zeph", "ZEP", "Zep", "Zep", book_id::_zephaniah, 38, book_type::old_testament, false},
  {"Haggai", "Hag", "HAG", "Hag", "Hag", book_id::_haggai, 39, book_type::old_testament, false},
  {"Zechariah", "Zech", "ZEC", "Zec", "Zec", book_id::_zechariah, 40, book_type::old_testament, false},
  {"Malachi", "Mal", "MAL", "Mal", "Mal", book_id::_malachi, 41, book_type::old_testament, false},
  {"Matthew", "Matt", "MAT", "Mat", "Mt", book_id::_matthew, 42, book_type::new_testament,  false}, // The Gospel according to Matthew.
  {"Mark", "Mark", "MRK", "Mar", "Mr", book_id::_mark, 43, book_type::new_testament,  false}, // The Gospel according to Mark.
  {"Luke", "Luke", "LUK", "Luk", "Lu", book_id::_luke, 44, book_type::new_testament,  false}, // The Gospel according to Luke.
  {"John", "John", "JHN", "Joh", "Joh", book_id::_john, 45, book_type::new_testament,  false}, // The Gospel according to John.
  {"Acts", "Acts", "ACT", "Act", "Ac", book_id::_acts, 46, book_type::new_testament,  false}, // The Acts of the Apostles.
  {"Romans", "Rom", "ROM", "Rom", "Ro", book_id::_romans, 47, book_type::new_testament,  false}, // The Letter of Paul to the Romans.
  {"1 Corinthians", "1Cor", "1CO", "1Co", "1Co", book_id::_1_corinthians, 48, book_type::new_testament,  false}, // The First Letter of Paul to the Corinthians.
  {"2 Corinthians", "2Cor", "2CO", "2Co", "2Co", book_id::_2_corinthians, 49, book_type::new_testament,  false}, // The Second Letter of Paul to the Corinthians.
  {"Galatians", "Gal", "GAL", "Gal", "Ga", book_id::_galatians, 50, book_type::new_testament,  false}, // The Letter of Paul to the Galatians.
  {"Ephesians", "Eph", "EPH", "Eph", "Eph", book_id::_ephesians, 51, book_type::new_testament,  false}, // The Letter of Paul to the Ephesians.
  {"Philippians", "Phil", "PHP", "Phi", "Php", book_id::_philippians, 52, book_type::new_testament,  false}, // The Letter of Paul to the Philippians.
  {"Colossians", "Col", "COL", "Col", "Col", book_id::_colossians, 53, book_type::new_testament,  false}, // The Letter of Paul to the Colossians.
  {"1 Thessalonians", "1Thess", "1TH", "1Th", "1Th", book_id::_1_thessalonians, 54, book_type::new_testament,  false}, // The First Letter of Paul to the Thessalonians.
  {"2 Thessalonians", "2Thess", "2TH", "2Th", "2Th", book_id::_2_thessalonians, 55, book_type::new_testament,  false}, // The Second Letter of Paul to the Thessalonians.
  {"1 Timothy", "1Tim", "1TI", "1Ti", "1Ti", book_id::_1_timothy, 56, book_type::new_testament,  false}, // The First Letter of Paul to Timothy.
  {"2 Timothy", "2Tim", "2TI", "2Ti", "2Ti", book_id::_2_timothy, 57, book_type::new_testament,  false}, // The Second Letter of Paul to Timothy.
  {"Titus", "Titus", "TIT", "Tit", "Tit", book_id::_titus, 58, book_type::new_testament,  false}, // The Letter of Paul to Titus.
  {"Philemon", "Phlm", "PHM", "Phm", "Phm", book_id::_philemon, 59, book_type::new_testament,  true}, // The Letter of Paul to Philemon.
  {"Hebrews", "Heb", "HEB", "Heb", "Heb", book_id::_hebrews, 60, book_type::new_testament,  false}, // The Letter to the Hebrews.
  {"James", "Jas", "JAS", "Jam", "Jas", book_id::_james, 61, book_type::new_testament,  false}, // The Letter of James.
  {"1 Peter", "1Pet", "1PE", "1Pe", "1Pe", book_id::_1_peter, 62, book_type::new_testament,  false}, // The First Letter of Peter.
  {"2 Peter", "2Pet", "2PE", "2Pe", "2Pe", book_id::_2_peter, 63, book_type::new_testament,  false}, // The Second Letter of Peter.
  {"1 John", "1John", "1JN", "1Jo", "1Jo", book_id::_1_john, 64, book_type::new_testament,  false}, // The First Letter of John.
  {"2 John", "2John", "2JN", "2Jo", "2Jo", book_id::_2_john, 65, book_type::new_testament,  false}, // The Second Letter of John.
  {"3 John", "3John", "3JN", "3Jo", "3Jo", book_id::_3_john, 66, book_type::new_testament,  true}, // The Third Letter of John.
  {"Jude", "Jude", "JUD", "Jud", "Jude", book_id::_jude, 67, book_type::new_testament,  true}, // The Letter of Jude; do not confuse this abbreviation with JDG for Judges, or JDT for Judith.
  {"Revelation", "Rev", "REV", "Rev", "Re", book_id::_revelation, 68, book_type::new_testament,  false}, // The Revelation to John; called Apocalypse in Catholic Bibles.
  {"Front Matter", "", "FRT", "", "", book_id::_front_matter, 1, book_type::front_back, false},
  {"Back Matter", "", "BAK", "", "", book_id::_back_matter, 69, book_type::front_back, false},
  {"Other Material", "", "OTH", "", "", book_id::_other_material, 70, book_type::other, false},
  {"Tobit", "Tob", "TOB", "Tob", "", book_id::_tobit, 71, book_type::apocryphal,  false},
  {"Judith", "Jdt", "JDT", "Jdt", "", book_id::_judith, 72, book_type::apocryphal,  false},
  {"Esther (Greek)", "AddEsth", "ESG", "EsG", "", book_id::_esther_greek, 73, book_type::apocryphal,  false},
  {"Wisdom of Solomon", "Wis", "WIS", "Wis", "", book_id::_wisdom_of_solomon, 74, book_type::apocryphal,  false},
  {"Sirach", "Sir", "SIR", "Sir", "", book_id::_sirach, 75, book_type::apocryphal,  false}, // Ecclesiasticus or Jesus son of Sirach.
  {"Baruch", "Bar", "BAR", "Bar", "", book_id::_baruch, 76, book_type::apocryphal,  false}, // 5 chapters in Orthodox Bibles (LJE is separate); 6 chapters in Catholic Bibles (includes LJE); called 1 Baruch in Syriac Bibles.
  {"Letter of Jeremiah", "EpJer", "LJE", "LJe", "", book_id::_letter_of_jeremiah, 77, book_type::apocryphal,  true}, // Sometimes included in Baruch; called ‘Rest of Jeremiah’ in Ethiopia.
  {"Song of the Three Children", "PrAzar", "S3Y", "S3Y", "", book_id::_song_of_the_three_children, 78, book_type::apocryphal,  true}, // Includes the Prayer of Azariah; sometimes included in Greek Daniel.
  {"Susanna", "Sus", "SUS", "Sus", "", book_id::_susanna, 79, book_type::apocryphal,  true}, // Sometimes included in Greek Daniel.
  {"Bel and the Dragon", "Bel", "BEL", "Bel", "",  book_id::_bel_and_the_dragon, 80, book_type::apocryphal,  true}, // Sometimes included in Greek Daniel; called ‘Rest of Daniel’ in Ethiopia.
  {"1 Maccabees", "1Macc", "1MA", "1Ma", "", book_id::_1_maccabees, 81, book_type::apocryphal,  false}, // Called ‘3 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
  {"2 Maccabees", "2Macc", "2MA", "2Ma", "", book_id::_2_maccabees, 82, book_type::apocryphal,  false}, // Called ‘1 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
  {"1 Esdras (Greek)", "1Esd", "1ES", "1Es", "", book_id::_1_esdras_greek, 83, book_type::apocryphal,  false}, // The 9-chapter book of Greek Ezra in the LXX, called ‘2 Esdras’ in Russian Bibles, and called ‘3 Esdras’ in the Vulgate; when Ezra-Nehemiah is one book use EZR.
  {"Prayer of Manasses", "PrMan", "MAN", "Man", "", book_id::_prayer_of_manasses, 84, book_type::apocryphal,  true}, // Sometimes appended to 2 Chronicles. Included in Orthodox Bibles.
  {"Psalm 151", "Ps151", "PS2", "Ps2", "", book_id::_psalm_151, 85, book_type::apocryphal,  true}, // An additional Psalm in the Septuagint. Appended to Psalms in Orthodox Bibles.
  {"3 Maccabees", "3Macc", "3MA", "3Ma", "", book_id::_3_maccabees, 86, book_type::apocryphal,  false}, // Called ‘2 Maccabees’ in some traditions, printed in Orthodox Bibles.
  {"2 Esdras (Latin)", "2Esd", "2ES", "2Es", "", book_id::_2_esdras_latin, 87, book_type::apocryphal,  false}, // The 16 chapter book of Latin Esdras called ‘3 Esdras’ in Russian Bibles and called ‘4 Esdras’ in the Vulgate. For the 12 chapter Apocalypse of Ezra use EZA.
  {"4 Maccabees", "4Macc", "4MA", "4Ma", "", book_id::_4_maccabees,  88, book_type::apocryphal,  false}, // In an appendix to the Greek Bible and in the Georgian Bible.
  {"Daniel (Greek)", "", "DAG", "", "", book_id::_daniel_greek, 89, book_type::apocryphal,  false}, // The 14-chapter version of Daniel from the Septuagint including Greek additions.
  {"Odes", "", "ODA", "", "", book_id::_odes, 90, book_type::apocryphal,  false}, // Or Odae. A book in some editions of the Septuagint. Odes has different contents in Greek, Russian, and Syriac traditions.
  {"Psalms of Solomon", "", "PSS", "", "", book_id::_psalms_of_solomon, 91, book_type::apocryphal,  false}, // A book in some editions of the Septuagint, but not printed in modern Bibles.
  {"Ezra Apocalypse", "", "EZA", "", "", book_id::_ezra_apocalypse, 92, book_type::apocryphal,  false}, // 12-Chapter book of Ezra Apocalypse. Called ‘3 Ezra’ in the Armenian Bible. Called ‘Ezra Shealtiel’ in the Ethiopian Bible. Formerly called 4ES; called ‘2 Esdras’ when it includes 5 Ezra and 6 Ezra.
  {"5 Ezra", "", "5EZ", "", "", book_id::_5_ezra, 93, book_type::apocryphal,  false}, // 2-Chapter Latin preface to Ezra Apocalypse. Formerly called 5ES.
  {"6 Ezra", "", "6EZ", "", "", book_id::_6_ezra, 94, book_type::apocryphal,  false}, // 2-Chapter Latin conclusion to Ezra Apocalypse. Formerly called 6ES.
  {"Psalms 152-155", "", "PS3", "", "", book_id::_psalms_152_155, 95, book_type::apocryphal,  false}, // Additional Psalms 152-155 found in West Syriac manuscripts.
  {"2 Baruch (Apocalypse)", "", "2BA", "", "", book_id::_2_baruch_apocalypse, 96, book_type::apocryphal,  false}, // The Apocalypse of Baruch in Syriac Bibles.
  {"Letter of Baruch", "", "LBA", "", "", book_id::_letter_of_baruch, 97, book_type::apocryphal,  false}, // Sometimes appended to 2 Baruch. Sometimes separate in Syriac Bibles.
  {"Jubilees", "", "JUB", "", "", book_id::_jubilees, 98, book_type::apocryphal,  false}, // Ancient Hebrew book used in the Ethiopian Bible.
  {"Enoch", "", "ENO", "", "", book_id::_enoch, 99, book_type::apocryphal,  false}, // Sometimes called ‘1 Enoch’. Ancient Hebrew book in the Ethiopian Bible.
  {"1 Meqabyan/Mekabis", "", "1MQ", "", "", book_id::_1_meqabyan_mekabis, 100, book_type::apocryphal,  false}, // Book of Mekabis of Benjamin in the Ethiopian Bible.
  {"2 Meqabyan/Mekabis", "", "2MQ", "", "", book_id::_2_meqabyan_mekabis, 101, book_type::apocryphal,  false}, // Book of Mekabis of Moab in the Ethiopian Bible.
  {"3 Meqabyan/Mekabis", "", "3MQ", "", "", book_id::_3_meqabyan_mekabis, 102, book_type::apocryphal,  false}, // Book of Meqabyan in the Ethiopian Bible.
  {"Reproof", "", "REP", "", "", book_id::_reproof, 103, book_type::apocryphal,  false}, // Proverbs part 2. Used in the Ethiopian Bible.
  {"4 Baruch", "", "4BA", "", "", book_id::_4_baruch, 104, book_type::apocryphal,  false}, // Paralipomenon of Jeremiah, called ‘Rest of the Words of Baruch’ in Ethiopia. May include or exclude the Letter of Jeremiah as chapter 1. Used in the Ethiopian Bible.
  {"Letter to the Laodiceans", "", "LAO", "", "", book_id::_letter_to_the_laodiceans, 105, book_type::apocryphal,  false}, // A Latin Vulgate book, found in the Vulgate and some medieval Catholic translations.
  {"Introduction Matter", "", "INT", "", "", book_id::_introduction_matter, 2, book_type::front_back, false},
  {"Concordance", "", "CNC", "", "", book_id::_concordance, 106, book_type::front_back, false},
  {"Glossary / Wordlist", "", "GLO", "", "", book_id::_glossary_wordlist, 107, book_type::front_back, false},
  {"Topical Index", "", "TDX", "", "", book_id::_topical_index, 108, book_type::front_back, false},
  {"Names Index", "", "NDX", "", "", book_id::_names_index, 109, book_type::front_back, false}
};

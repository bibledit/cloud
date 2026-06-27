/*
Copyright (©) 2003-2026 Teus Benschop.

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


#pragma once

// All data is stored in the code in memory, not in a database on disk.

struct book_record
{
    const char* english; // English name.
    const char* osis; // OSIS abbreviation.
    const char* usfm; // USFM ID.
    const char* bibleworks; // BibleWorks abbreviation.
    const char* onlinebible; // Online Bible abbreviation.
    book_id id; // Bibledit's internal book identifier as an enum.
    short order; // The order of the books.
    book_type type; // The type of the book as an enum.
    bool onechapter; // Whether the book has one chapter.
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
constexpr book_record books_table[] =
{
    {
        .english = "Genesis",
        .osis = "Gen",
        .usfm = "GEN",
        .bibleworks = "Gen",
        .onlinebible = "Ge",
        .id = book_id::_genesis,
        .order = 3,
        .type = book_type::old_testament,
        .onechapter = false
    }, // ‘1 Moses’ in some Bibles.
    {
        .english = "Exodus",
        .osis = "Exod",
        .usfm = "EXO",
        .bibleworks = "Exo",
        .onlinebible = "Ex",
        .id = book_id::_exodus,
        .order = 4,
        .type = book_type::old_testament,
        .onechapter = false
    }, // ‘2 Moses’ in some Bibles.
    {
        .english = "Leviticus",
        .osis = "Lev",
        .usfm = "LEV",
        .bibleworks = "Lev",
        .onlinebible = "Le",
        .id = book_id::_leviticus,
        .order = 5,
        .type = book_type::old_testament,
        .onechapter = false
    }, // ‘3 Moses’ in some Bibles.
    {
        .english = "Numbers",
        .osis = "Num",
        .usfm = "NUM",
        .bibleworks = "Num",
        .onlinebible = "Nu",
        .id = book_id::_numbers,
        .order = 6,
        .type = book_type::old_testament,
        .onechapter = false
    }, // ‘4 Moses’ in some Bibles.
    {
        .english = "Deuteronomy",
        .osis = "Deut",
        .usfm = "DEU",
        .bibleworks = "Deu",
        .onlinebible = "De",
        .id = book_id::_deuteronomy,
        .order = 7,
        .type = book_type::old_testament,
        .onechapter = false
    }, // ‘5 Moses’ in some Bibles.
    {
        .english = "Joshua",
        .osis = "Josh",
        .usfm = "JOS",
        .bibleworks = "Jos",
        .onlinebible = "Jos",
        .id = book_id::_joshua,
        .order = 8,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Judges",
        .osis = "Judg",
        .usfm = "JDG",
        .bibleworks = "Jdg",
        .onlinebible = "Jud",
        .id = book_id::_judges,
        .order = 9,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Ruth",
        .osis = "Ruth",
        .usfm = "RUT",
        .bibleworks = "Rut",
        .onlinebible = "Ru",
        .id = book_id::_ruth,
        .order = 10,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "1 Samuel",
        .osis = "1Sam",
        .usfm = "1SA",
        .bibleworks = "1Sa",
        .onlinebible = "1Sa",
        .id = book_id::_1_samuel,
        .order = 11,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 1 Kings or Kingdoms in Orthodox Bibles. Do not confuse this abbreviation with ISA for Isaiah.
    {
        .english = "2 Samuel",
        .osis = "2Sam",
        .usfm = "2SA",
        .bibleworks = "2Sa",
        .onlinebible = "2Sa",
        .id = book_id::_2_samuel,
        .order = 12,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 2 Kings or Kingdoms in Orthodox Bibles.
    {
        .english = "1 Kings",
        .osis = "1Kgs",
        .usfm = "1KI",
        .bibleworks = "1Ki",
        .onlinebible = "1Ki",
        .id = book_id::_1_kings,
        .order = 13,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 3 Kings or Kingdoms in Orthodox Bibles.
    {
        .english = "2 Kings",
        .osis = "2Kgs",
        .usfm = "2KI",
        .bibleworks = "2Ki",
        .onlinebible = "2Ki",
        .id = book_id::_2_kings,
        .order = 14,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 4 Kings or Kingdoms in Orthodox Bibles.
    {
        .english = "1 Chronicles",
        .osis = "1Chr",
        .usfm = "1CH",
        .bibleworks = "1Ch",
        .onlinebible = "1Ch",
        .id = book_id::_1_chronicles,
        .order = 15,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 1 Paralipomenon in Orthodox Bibles.
    {
        .english = "2 Chronicles",
        .osis = "2Chr",
        .usfm = "2CH",
        .bibleworks = "2Ch",
        .onlinebible = "2Ch",
        .id = book_id::_2_chronicles,
        .order = 16,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 2 Paralipomenon in Orthodox Bibles.
    {
        .english = "Ezra",
        .osis = "Ezra",
        .usfm = "EZR",
        .bibleworks = "Ezr",
        .onlinebible = "Ezr",
        .id = book_id::_ezra,
        .order = 17,
        .type = book_type::old_testament,
        .onechapter = false
    }, // This is for Hebrew Ezra, sometimes called 1 Ezra or 1 Esdras. Also for Ezra-Nehemiah when one book.
    {
        .english = "Nehemiah",
        .osis = "Neh",
        .usfm = "NEH",
        .bibleworks = "Neh",
        .onlinebible = "Ne",
        .id = book_id::_nehemiah,
        .order = 18,
        .type = book_type::old_testament,
        .onechapter = false
    }, // Sometimes appended to Ezra; called 2 Esdras in the Vulgate.
    {
        .english = "Esther",
        .osis = "Esth",
        .usfm = "EST",
        .bibleworks = "Est",
        .onlinebible = "Es",
        .id = book_id::_esther,
        .order = 19,
        .type = book_type::old_testament,
        .onechapter = false
    }, // This is for Hebrew Esther. For the longer Greek LXX Esther use ESG.
    {
        .english = "Job",
        .osis = "Job",
        .usfm = "JOB",
        .bibleworks = "Job",
        .onlinebible = "Job",
        .id = book_id::_job,
        .order = 20,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Psalms",
        .osis = "Ps",
        .usfm = "PSA",
        .bibleworks = "Psa",
        .onlinebible = "Ps",
        .id = book_id::_psalms,
        .order = 21,
        .type = book_type::old_testament,
        .onechapter = false
    },
    // 150 Psalms in Hebrew, 151 Psalms in Orthodox Bibles, 155 Psalms in West Syriac Bibles. If you put Psalm 151 separately in an Apocrypha use PS2, for Psalms 152-155 use PS3.
    {
        .english = "Proverbs",
        .osis = "Prov",
        .usfm = "PRO",
        .bibleworks = "Pro",
        .onlinebible = "Pr",
        .id = book_id::_proverbs,
        .order = 22,
        .type = book_type::old_testament,
        .onechapter = false
    }, // 31 Proverbs, but 24 Proverbs in the Ethiopian Bible.
    {
        .english = "Ecclesiastes",
        .osis = "Eccl",
        .usfm = "ECC",
        .bibleworks = "Ecc",
        .onlinebible = "Ec",
        .id = book_id::_ecclesiastes,
        .order = 23,
        .type = book_type::old_testament,
        .onechapter = false
    }, // Qoholeth in Catholic Bibles; for Ecclesiasticus use SIR.
    {
        .english = "Song of Solomon",
        .osis = "Song",
        .usfm = "SNG",
        .bibleworks = "Sol",
        .onlinebible = "So",
        .id = book_id::_song_of_solomon,
        .order = 24,
        .type = book_type::old_testament,
        .onechapter = false
    }, // Song of Solomon, or Canticles of Canticles in Catholic Bibles.
    {
        .english = "Isaiah",
        .osis = "Isa",
        .usfm = "ISA",
        .bibleworks = "Isa",
        .onlinebible = "Isa",
        .id = book_id::_isaiah,
        .order = 25,
        .type = book_type::old_testament,
        .onechapter = false
    }, // Do not confuse this abbreviation with 1SA for 1 Samuel.
    {
        .english = "Jeremiah",
        .osis = "Jer",
        .usfm = "JER",
        .bibleworks = "Jer",
        .onlinebible = "Jer",
        .id = book_id::_jeremiah,
        .order = 26,
        .type = book_type::old_testament,
        .onechapter = false
    }, // The Book of Jeremiah; for the Letter of Jeremiah use LJE.
    {
        .english = "Lamentations",
        .osis = "Lam",
        .usfm = "LAM",
        .bibleworks = "Lam",
        .onlinebible = "La",
        .id = book_id::_lamentations,
        .order = 27,
        .type = book_type::old_testament,
        .onechapter = false
    }, // The Lamentations of Jeremiah.
    {
        .english = "Ezekiel",
        .osis = "Ezek",
        .usfm = "EZK",
        .bibleworks = "Eze",
        .onlinebible = "Eze",
        .id = book_id::_ezekiel,
        .order = 28,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Daniel",
        .osis = "Dan",
        .usfm = "DAN",
        .bibleworks = "Dan",
        .onlinebible = "Da",
        .id = book_id::_daniel,
        .order = 29,
        .type = book_type::old_testament,
        .onechapter = false
    }, // This is for Hebrew Daniel; for the longer Greek LXX Daniel use DAG.
    {
        .english = "Hosea",
        .osis = "Hos",
        .usfm = "HOS",
        .bibleworks = "Hos",
        .onlinebible = "Ho",
        .id = book_id::_hosea,
        .order = 30,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Joel",
        .osis = "Joel",
        .usfm = "JOL",
        .bibleworks = "Joe",
        .onlinebible = "Joe",
        .id = book_id::_joel,
        .order = 31,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Amos",
        .osis = "Amos",
        .usfm = "AMO",
        .bibleworks = "Amo",
        .onlinebible = "Am",
        .id = book_id::_amos,
        .order = 32,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Obadiah",
        .osis = "Obad",
        .usfm = "OBA",
        .bibleworks = "Oba",
        .onlinebible = "Ob",
        .id = book_id::_obadiah,
        .order = 33,
        .type = book_type::old_testament,
        .onechapter = true
    },
    {
        .english = "Jonah",
        .osis = "Jonah",
        .usfm = "JON",
        .bibleworks = "Jon",
        .onlinebible = "Jon",
        .id = book_id::_jonah,
        .order = 34,
        .type = book_type::old_testament,
        .onechapter = false
    }, // Do not confuse this abbreviation with JHN for John.
    {
        .english = "Micah",
        .osis = "Mic",
        .usfm = "MIC",
        .bibleworks = "Mic",
        .onlinebible = "Mic",
        .id = book_id::_micah,
        .order = 35,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Nahum",
        .osis = "Nah",
        .usfm = "NAM",
        .bibleworks = "Nah",
        .onlinebible = "Na",
        .id = book_id::_nahum,
        .order = 36,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Habakkuk",
        .osis = "Hab",
        .usfm = "HAB",
        .bibleworks = "Hab",
        .onlinebible = "Hab",
        .id = book_id::_habakkuk,
        .order = 37,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Zephaniah",
        .osis = "Zeph",
        .usfm = "ZEP",
        .bibleworks = "Zep",
        .onlinebible = "Zep",
        .id = book_id::_zephaniah,
        .order = 38,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Haggai",
        .osis = "Hag",
        .usfm = "HAG",
        .bibleworks = "Hag",
        .onlinebible = "Hag",
        .id = book_id::_haggai,
        .order = 39,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Zechariah",
        .osis = "Zech",
        .usfm = "ZEC",
        .bibleworks = "Zec",
        .onlinebible = "Zec",
        .id = book_id::_zechariah,
        .order = 40,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Malachi",
        .osis = "Mal",
        .usfm = "MAL",
        .bibleworks = "Mal",
        .onlinebible = "Mal",
        .id = book_id::_malachi,
        .order = 41,
        .type = book_type::old_testament,
        .onechapter = false
    },
    {
        .english = "Matthew",
        .osis = "Matt",
        .usfm = "MAT",
        .bibleworks = "Mat",
        .onlinebible = "Mt",
        .id = book_id::_matthew,
        .order = 42,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Gospel according to Matthew.
    {
        .english = "Mark",
        .osis = "Mark",
        .usfm = "MRK",
        .bibleworks = "Mar",
        .onlinebible = "Mr",
        .id = book_id::_mark,
        .order = 43,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Gospel according to Mark.
    {
        .english = "Luke",
        .osis = "Luke",
        .usfm = "LUK",
        .bibleworks = "Luk",
        .onlinebible = "Lu",
        .id = book_id::_luke,
        .order = 44,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Gospel according to Luke.
    {
        .english = "John",
        .osis = "John",
        .usfm = "JHN",
        .bibleworks = "Joh",
        .onlinebible = "Joh",
        .id = book_id::_john,
        .order = 45,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Gospel according to John.
    {
        .english = "Acts",
        .osis = "Acts",
        .usfm = "ACT",
        .bibleworks = "Act",
        .onlinebible = "Ac",
        .id = book_id::_acts,
        .order = 46,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Acts of the Apostles.
    {
        .english = "Romans",
        .osis = "Rom",
        .usfm = "ROM",
        .bibleworks = "Rom",
        .onlinebible = "Ro",
        .id = book_id::_romans,
        .order = 47,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to the Romans.
    {
        .english = "1 Corinthians",
        .osis = "1Cor",
        .usfm = "1CO",
        .bibleworks = "1Co",
        .onlinebible = "1Co",
        .id = book_id::_1_corinthians,
        .order = 48,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The First Letter of Paul to the Corinthians.
    {
        .english = "2 Corinthians",
        .osis = "2Cor",
        .usfm = "2CO",
        .bibleworks = "2Co",
        .onlinebible = "2Co",
        .id = book_id::_2_corinthians,
        .order = 49,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Second Letter of Paul to the Corinthians.
    {
        .english = "Galatians",
        .osis = "Gal",
        .usfm = "GAL",
        .bibleworks = "Gal",
        .onlinebible = "Ga",
        .id = book_id::_galatians,
        .order = 50,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to the Galatians.
    {
        .english = "Ephesians",
        .osis = "Eph",
        .usfm = "EPH",
        .bibleworks = "Eph",
        .onlinebible = "Eph",
        .id = book_id::_ephesians,
        .order = 51,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to the Ephesians.
    {
        .english = "Philippians",
        .osis = "Phil",
        .usfm = "PHP",
        .bibleworks = "Phi",
        .onlinebible = "Php",
        .id = book_id::_philippians,
        .order = 52,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to the Philippians.
    {
        .english = "Colossians",
        .osis = "Col",
        .usfm = "COL",
        .bibleworks = "Col",
        .onlinebible = "Col",
        .id = book_id::_colossians,
        .order = 53,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to the Colossians.
    {
        .english = "1 Thessalonians",
        .osis = "1Thess",
        .usfm = "1TH",
        .bibleworks = "1Th",
        .onlinebible = "1Th",
        .id = book_id::_1_thessalonians,
        .order = 54,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The First Letter of Paul to the Thessalonians.
    {
        .english = "2 Thessalonians",
        .osis = "2Thess",
        .usfm = "2TH",
        .bibleworks = "2Th",
        .onlinebible = "2Th",
        .id = book_id::_2_thessalonians,
        .order = 55,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Second Letter of Paul to the Thessalonians.
    {
        .english = "1 Timothy",
        .osis = "1Tim",
        .usfm = "1TI",
        .bibleworks = "1Ti",
        .onlinebible = "1Ti",
        .id = book_id::_1_timothy,
        .order = 56,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The First Letter of Paul to Timothy.
    {
        .english = "2 Timothy",
        .osis = "2Tim",
        .usfm = "2TI",
        .bibleworks = "2Ti",
        .onlinebible = "2Ti",
        .id = book_id::_2_timothy,
        .order = 57,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Second Letter of Paul to Timothy.
    {
        .english = "Titus",
        .osis = "Titus",
        .usfm = "TIT",
        .bibleworks = "Tit",
        .onlinebible = "Tit",
        .id = book_id::_titus,
        .order = 58,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of Paul to Titus.
    {
        .english = "Philemon",
        .osis = "Phlm",
        .usfm = "PHM",
        .bibleworks = "Phm",
        .onlinebible = "Phm",
        .id = book_id::_philemon,
        .order = 59,
        .type = book_type::new_testament,
        .onechapter = true
    }, // The Letter of Paul to Philemon.
    {
        .english = "Hebrews",
        .osis = "Heb",
        .usfm = "HEB",
        .bibleworks = "Heb",
        .onlinebible = "Heb",
        .id = book_id::_hebrews,
        .order = 60,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter to the Hebrews.
    {
        .english = "James",
        .osis = "Jas",
        .usfm = "JAS",
        .bibleworks = "Jam",
        .onlinebible = "Jas",
        .id = book_id::_james,
        .order = 61,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Letter of James.
    {
        .english = "1 Peter",
        .osis = "1Pet",
        .usfm = "1PE",
        .bibleworks = "1Pe",
        .onlinebible = "1Pe",
        .id = book_id::_1_peter,
        .order = 62,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The First Letter of Peter.
    {
        .english = "2 Peter",
        .osis = "2Pet",
        .usfm = "2PE",
        .bibleworks = "2Pe",
        .onlinebible = "2Pe",
        .id = book_id::_2_peter,
        .order = 63,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Second Letter of Peter.
    {
        .english = "1 John",
        .osis = "1John",
        .usfm = "1JN",
        .bibleworks = "1Jo",
        .onlinebible = "1Jo",
        .id = book_id::_1_john,
        .order = 64,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The First Letter of John.
    {
        .english = "2 John",
        .osis = "2John",
        .usfm = "2JN",
        .bibleworks = "2Jo",
        .onlinebible = "2Jo",
        .id = book_id::_2_john,
        .order = 65,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Second Letter of John.
    {
        .english = "3 John",
        .osis = "3John",
        .usfm = "3JN",
        .bibleworks = "3Jo",
        .onlinebible = "3Jo",
        .id = book_id::_3_john,
        .order = 66,
        .type = book_type::new_testament,
        .onechapter = true
    }, // The Third Letter of John.
    {
        .english = "Jude",
        .osis = "Jude",
        .usfm = "JUD",
        .bibleworks = "Jud",
        .onlinebible = "Jude",
        .id = book_id::_jude,
        .order = 67,
        .type = book_type::new_testament,
        .onechapter = true
    }, // The Letter of Jude; do not confuse this abbreviation with JDG for Judges, or JDT for Judith.
    {
        .english = "Revelation",
        .osis = "Rev",
        .usfm = "REV",
        .bibleworks = "Rev",
        .onlinebible = "Re",
        .id = book_id::_revelation,
        .order = 68,
        .type = book_type::new_testament,
        .onechapter = false
    }, // The Revelation to John; called Apocalypse in Catholic Bibles.
    {
        .english = "Front Matter",
        .osis = "",
        .usfm = "FRT",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_front_matter,
        .order = 1,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Back Matter",
        .osis = "",
        .usfm = "BAK",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_back_matter,
        .order = 69,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Other Material",
        .osis = "",
        .usfm = "OTH",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_other_material,
        .order = 70,
        .type = book_type::other,
        .onechapter = false
    },
    {
        .english = "Tobit",
        .osis = "Tob",
        .usfm = "TOB",
        .bibleworks = "Tob",
        .onlinebible = "",
        .id = book_id::_tobit,
        .order = 71,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    {
        .english = "Judith",
        .osis = "Jdt",
        .usfm = "JDT",
        .bibleworks = "Jdt",
        .onlinebible = "",
        .id = book_id::_judith,
        .order = 72,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    {
        .english = "Esther (Greek)",
        .osis = "AddEsth",
        .usfm = "ESG",
        .bibleworks = "EsG",
        .onlinebible = "",
        .id = book_id::_esther_greek,
        .order = 73,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    {
        .english = "Wisdom of Solomon",
        .osis = "Wis",
        .usfm = "WIS",
        .bibleworks = "Wis",
        .onlinebible = "",
        .id = book_id::_wisdom_of_solomon,
        .order = 74,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    {
        .english = "Sirach",
        .osis = "Sir",
        .usfm = "SIR",
        .bibleworks = "Sir",
        .onlinebible = "",
        .id = book_id::_sirach,
        .order = 75,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Ecclesiasticus or Jesus son of Sirach.
    {
        .english = "Baruch",
        .osis = "Bar",
        .usfm = "BAR",
        .bibleworks = "Bar",
        .onlinebible = "",
        .id = book_id::_baruch,
        .order = 76,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // 5 chapters in Orthodox Bibles (LJE is separate); 6 chapters in Catholic Bibles (includes LJE); called 1 Baruch in Syriac Bibles.
    {
        .english = "Letter of Jeremiah",
        .osis = "EpJer",
        .usfm = "LJE",
        .bibleworks = "LJe",
        .onlinebible = "",
        .id = book_id::_letter_of_jeremiah,
        .order = 77,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // Sometimes included in Baruch; called ‘Rest of Jeremiah’ in Ethiopia.
    {
        .english = "Song of the Three Children",
        .osis = "PrAzar",
        .usfm = "S3Y",
        .bibleworks = "S3Y",
        .onlinebible = "",
        .id = book_id::_song_of_the_three_children,
        .order = 78,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // Includes the Prayer of Azariah; sometimes included in Greek Daniel.
    {
        .english = "Susanna",
        .osis = "Sus",
        .usfm = "SUS",
        .bibleworks = "Sus",
        .onlinebible = "",
        .id = book_id::_susanna,
        .order = 79,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // Sometimes included in Greek Daniel.
    {
        .english = "Bel and the Dragon",
        .osis = "Bel",
        .usfm = "BEL",
        .bibleworks = "Bel",
        .onlinebible = "",
        .id = book_id::_bel_and_the_dragon,
        .order = 80,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // Sometimes included in Greek Daniel; called ‘Rest of Daniel’ in Ethiopia.
    {
        .english = "1 Maccabees",
        .osis = "1Macc",
        .usfm = "1MA",
        .bibleworks = "1Ma",
        .onlinebible = "",
        .id = book_id::_1_maccabees,
        .order = 81,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Called ‘3 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
    {
        .english = "2 Maccabees",
        .osis = "2Macc",
        .usfm = "2MA",
        .bibleworks = "2Ma",
        .onlinebible = "",
        .id = book_id::_2_maccabees,
        .order = 82,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Called ‘1 Maccabees’ in some traditions, printed in Catholic and Orthodox Bibles.
    {
        .english = "1 Esdras (Greek)",
        .osis = "1Esd",
        .usfm = "1ES",
        .bibleworks = "1Es",
        .onlinebible = "",
        .id = book_id::_1_esdras_greek,
        .order = 83,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // The 9-chapter book of Greek Ezra in the LXX, called ‘2 Esdras’ in Russian Bibles, and called ‘3 Esdras’ in the Vulgate; when Ezra-Nehemiah is one book use EZR.
    {
        .english = "Prayer of Manasses",
        .osis = "PrMan",
        .usfm = "MAN",
        .bibleworks = "Man",
        .onlinebible = "",
        .id = book_id::_prayer_of_manasses,
        .order = 84,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // Sometimes appended to 2 Chronicles. Included in Orthodox Bibles.
    {
        .english = "Psalm 151",
        .osis = "Ps151",
        .usfm = "PS2",
        .bibleworks = "Ps2",
        .onlinebible = "",
        .id = book_id::_psalm_151,
        .order = 85,
        .type = book_type::apocryphal,
        .onechapter = true
    }, // An additional Psalm in the Septuagint. Appended to Psalms in Orthodox Bibles.
    {
        .english = "3 Maccabees",
        .osis = "3Macc",
        .usfm = "3MA",
        .bibleworks = "3Ma",
        .onlinebible = "",
        .id = book_id::_3_maccabees,
        .order = 86,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Called ‘2 Maccabees’ in some traditions, printed in Orthodox Bibles.
    {
        .english = "2 Esdras (Latin)",
        .osis = "2Esd",
        .usfm = "2ES",
        .bibleworks = "2Es",
        .onlinebible = "",
        .id = book_id::_2_esdras_latin,
        .order = 87,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // The 16-chapter book of Latin Esdras called ‘3 Esdras’ in Russian Bibles and called ‘4 Esdras’ in the Vulgate. For the 12 chapter Apocalypse of Ezra use EZA.
    {
        .english = "4 Maccabees",
        .osis = "4Macc",
        .usfm = "4MA",
        .bibleworks = "4Ma",
        .onlinebible = "",
        .id = book_id::_4_maccabees,
        .order = 88,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // In an appendix to the Greek Bible and in the Georgian Bible.
    {
        .english = "Daniel (Greek)",
        .osis = "",
        .usfm = "DAG",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_daniel_greek,
        .order = 89,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // The 14-chapter version of Daniel from the Septuagint including Greek additions.
    {
        .english = "Odes",
        .osis = "",
        .usfm = "ODA",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_odes,
        .order = 90,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // Or Odae. A book in some editions of the Septuagint. Odes has different contents in Greek, Russian, and Syriac traditions.
    {
        .english = "Psalms of Solomon",
        .osis = "",
        .usfm = "PSS",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_psalms_of_solomon,
        .order = 91,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // A book in some editions of the Septuagint, but not printed in modern Bibles.
    {
        .english = "Ezra Apocalypse",
        .osis = "",
        .usfm = "EZA",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_ezra_apocalypse,
        .order = 92,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // 12-Chapter book of Ezra Apocalypse. Called ‘3 Ezra’ in the Armenian Bible. Called ‘Ezra Shealtiel’ in the Ethiopian Bible. Formerly called 4ES; called ‘2 Esdras’ when it includes 5 Ezra and 6 Ezra.
    {
        .english = "5 Ezra",
        .osis = "",
        .usfm = "5EZ",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_5_ezra,
        .order = 93,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // 2-Chapter Latin preface to Ezra Apocalypse. Formerly called 5ES.
    {
        .english = "6 Ezra",
        .osis = "",
        .usfm = "6EZ",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_6_ezra,
        .order = 94,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // 2-Chapter Latin conclusion to Ezra Apocalypse. Formerly called 6ES.
    {
        .english = "Psalms 152-155",
        .osis = "",
        .usfm = "PS3",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_psalms_152_155,
        .order = 95,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Additional Psalms 152-155 found in West Syriac manuscripts.
    {
        .english = "2 Baruch (Apocalypse)",
        .osis = "",
        .usfm = "2BA",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_2_baruch_apocalypse,
        .order = 96,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // The Apocalypse of Baruch in Syriac Bibles.
    {
        .english = "Letter of Baruch",
        .osis = "",
        .usfm = "LBA",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_letter_of_baruch,
        .order = 97,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Sometimes appended to 2 Baruch. Sometimes separate in Syriac Bibles.
    {
        .english = "Jubilees",
        .osis = "",
        .usfm = "JUB",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_jubilees,
        .order = 98,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Ancient Hebrew book used in the Ethiopian Bible.
    {
        .english = "Enoch",
        .osis = "",
        .usfm = "ENO",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_enoch,
        .order = 99,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Sometimes called ‘1 Enoch’. Ancient Hebrew book in the Ethiopian Bible.
    {
        .english = "1 Meqabyan/Mekabis",
        .osis = "",
        .usfm = "1MQ",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_1_meqabyan_mekabis,
        .order = 100,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Book of Mekabis of Benjamin in the Ethiopian Bible.
    {
        .english = "2 Meqabyan/Mekabis",
        .osis = "",
        .usfm = "2MQ",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_2_meqabyan_mekabis,
        .order = 101,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Book of Mekabis of Moab in the Ethiopian Bible.
    {
        .english = "3 Meqabyan/Mekabis",
        .osis = "",
        .usfm = "3MQ",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_3_meqabyan_mekabis,
        .order = 102,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Book of Meqabyan in the Ethiopian Bible.
    {
        .english = "Reproof",
        .osis = "",
        .usfm = "REP",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_reproof,
        .order = 103,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // Proverbs part 2. Used in the Ethiopian Bible.
    {
        .english = "4 Baruch",
        .osis = "",
        .usfm = "4BA",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_4_baruch,
        .order = 104,
        .type = book_type::apocryphal,
        .onechapter = false
    },
    // Paralipomenon of Jeremiah, called ‘Rest of the Words of Baruch’ in Ethiopia. May include or exclude the Letter of Jeremiah as chapter 1. Used in the Ethiopian Bible.
    {
        .english = "Letter to the Laodiceans",
        .osis = "",
        .usfm = "LAO",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_letter_to_the_laodiceans,
        .order = 105,
        .type = book_type::apocryphal,
        .onechapter = false
    }, // A Latin Vulgate book, found in the Vulgate and some medieval Catholic translations.
    {
        .english = "Introduction Matter",
        .osis = "",
        .usfm = "INT",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_introduction_matter,
        .order = 2,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Concordance",
        .osis = "",
        .usfm = "CNC",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_concordance,
        .order = 106,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Glossary / Wordlist",
        .osis = "",
        .usfm = "GLO",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_glossary_wordlist,
        .order = 107,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Topical Index",
        .osis = "",
        .usfm = "TDX",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_topical_index,
        .order = 108,
        .type = book_type::front_back,
        .onechapter = false
    },
    {
        .english = "Names Index",
        .osis = "",
        .usfm = "NDX",
        .bibleworks = "",
        .onlinebible = "",
        .id = book_id::_names_index,
        .order = 109,
        .type = book_type::front_back,
        .onechapter = false
    }
};

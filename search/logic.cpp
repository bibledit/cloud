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


#include <search/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <config/globals.h>
#include <database/bibles.h>
#include <database/config/bible.h>


string search_logic_index_folder ()
{
  return filter_url_create_root_path ("databases", "search");
}


string search_logic_bible_fragment (string bible)
{
  return filter_url_create_path (search_logic_index_folder (), bible + "_");
}


string search_logic_book_fragment (string bible, int book)
{
  return search_logic_bible_fragment (bible) + convert_to_string (book) + "_";
}


string search_logic_chapter_file (string bible, int book, int chapter)
{
  return search_logic_book_fragment (bible, book) + convert_to_string (chapter);
}


string search_logic_verse_separator ()
{
  return "v#e#r#s#e#s#e#p#a#r#a#t#o#r";
}


string search_logic_index_separator ()
{
  return "i#n#d#e#x#s#e#p#a#r#a#t#o#r";
}


#define USFM_RAW 1
#define USFM_LOWER 2
#define PLAIN_RAW 3
#define PLAIN_LOWER 4


// Indexes a $bible $book $chapter for searching.
void search_logic_index_chapter (string bible, int book, int chapter)
{
  Database_Bibles database_bibles;
  
  string usfm = database_bibles.getChapter (bible, book, chapter);
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  vector <string> index;
  
  set <string> already_processed;
  
  vector <int> verses = usfm_get_verse_numbers (usfm);
  
  for (auto verse : verses) {

    string raw_usfm = filter_string_trim (usfm_get_verse_text (usfm, verse));

    // In case of combined verses, the bit of USFM may have been indexed already.
    // Skip it in that case.
    if (already_processed.find (raw_usfm) != already_processed.end ()) continue;
    already_processed.insert (raw_usfm);

    index.push_back (search_logic_verse_separator ());
    index.push_back (convert_to_string (verse));
    index.push_back (search_logic_index_separator ());

    index.push_back (raw_usfm);
    
    string usfm_lower = unicode_string_casefold (raw_usfm);

    index.push_back (search_logic_index_separator ());

    index.push_back (usfm_lower);
    
    // Text filter for getting the plain text.
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.initializeHeadingsAndTextPerVerse (true);
    filter_text.addUsfmCode (raw_usfm);
    filter_text.run (stylesheet);

    string raw_plain;
    // Add the clean verse texts.
    map <int, string> texts = filter_text.getVersesText ();
    for (auto & element : texts) {
      raw_plain.append (element.second + "\n");
    }
    // Add any clean headings.
    map <int, string> headings = filter_text.verses_headings;
    for (auto & element : headings) {
      raw_plain.append (element.second + "\n");
    }
    // Add any footnotes.
    raw_plain.append (filter_text.text_text->getnote ());
    // Clean up.
    raw_plain = filter_string_trim (raw_plain);
    
    index.push_back (search_logic_index_separator ());

    index.push_back (raw_plain);
    
    string plain_lower = unicode_string_casefold (raw_plain);

    index.push_back (search_logic_index_separator ());

    index.push_back (plain_lower);
  }
  
  index.push_back (search_logic_index_separator ());
  
  // Store everything.
  string path = search_logic_chapter_file (bible, book, chapter);
  filter_url_file_put_contents (path, filter_string_implode (index, "\n"));
}


// Searches the text of the Bibles.
// Returns an array with matching passages.
// $search: Contains the text to search for.
// $bibles: Array of Bible names to search in.
vector <Passage> search_logic_search_text (string search, vector <string> bibles)
{
  vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = unicode_string_casefold (search);
  search = filter_string_str_replace (",", "", search);
  
  Database_Bibles database_bibles;
  for (auto bible : bibles) {
    vector <int> books = database_bibles.getBooks (bible);
    for (auto book : books) {
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        string path = search_logic_chapter_file (bible, book, chapter);
        string index = filter_url_file_get_contents (path);
        if (index.find (search) != string::npos) {
          vector <string> lines = filter_string_explode (index, '\n');
          int index_verse = 0;
          bool read_index_verse = false;
          int index_item = 0;
          for (auto & line : lines) {
            if (read_index_verse) {
              index_verse = convert_to_int (line);
              read_index_verse = false;
            } else if (line == search_logic_verse_separator ()) {
              read_index_verse = true;
              index_item = 0;
            } else if (line == search_logic_index_separator ()) {
              index_item++;
            } else if (index_item == PLAIN_LOWER) {
              if (line.find (search) != string::npos) {
                passages.push_back (Passage (bible, book, chapter, convert_to_string (index_verse)));
              }
            }
          }
        }
      }
    }
  }

  return passages;
}


// Performs a case-insensitive search of the text of one $bible.
// Returns an array with the matching passages.
// $search: Contains the text to search for.
vector <Passage> search_logic_search_bible_text (string bible, string search)
{
  vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = unicode_string_casefold (search);
  
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string path = search_logic_chapter_file (bible, book, chapter);
      string index = filter_url_file_get_contents (path);
      if (index.find (search) != string::npos) {
        vector <string> lines = filter_string_explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == PLAIN_LOWER) {
            if (line.find (search) != string::npos) {
              passages.push_back (Passage (bible, book, chapter, convert_to_string (index_verse)));
            }
          }
        }
      }
    }
  }
  
  return passages;
}


// Performs a case-sensitive search of the text of one $bible.
// Returns an array with the rowid's of matching verses.
// $search: Contains the text to search for.
vector <Passage> search_logic_search_bible_text_case_sensitive (string bible, string search)
{
  vector <Passage> passages;
  
  if (search == "") return passages;
  
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string path = search_logic_chapter_file (bible, book, chapter);
      string index = filter_url_file_get_contents (path);
      if (index.find (search) != string::npos) {
        vector <string> lines = filter_string_explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == PLAIN_RAW) {
            if (line.find (search) != string::npos) {
              passages.push_back (Passage (bible, book, chapter, convert_to_string (index_verse)));
            }
          }
        }
      }
    }
  }
  
  return passages;
}


// Performs a case-insensitive search of the USFM of one bible.
// Returns an array with the rowid's of matching verses.
// search: Contains the text to search for.
vector <Passage> search_logic_search_bible_usfm (string bible, string search)
{
  vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = unicode_string_casefold (search);
  
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string path = search_logic_chapter_file (bible, book, chapter);
      string index = filter_url_file_get_contents (path);
      if (index.find (search) != string::npos) {
        vector <string> lines = filter_string_explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == USFM_LOWER) {
            if (line.find (search) != string::npos) {
              passages.push_back (Passage (bible, book, chapter, convert_to_string (index_verse)));
            }
          }
        }
      }
    }
  }
  
  return passages;
}


// Performs a case-sensitive search of the USFM of one $bible.
// Returns an array with the rowid's of matching verses.
// $search: Contains the text to search for.
vector <Passage> search_logic_search_bible_usfm_case_sensitive (string bible, string search)
{
  vector <Passage> passages;
  
  if (search == "") return passages;
  
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string path = search_logic_chapter_file (bible, book, chapter);
      string index = filter_url_file_get_contents (path);
      if (index.find (search) != string::npos) {
        vector <string> lines = filter_string_explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == USFM_RAW) {
            if (line.find (search) != string::npos) {
              passages.push_back (Passage (bible, book, chapter, convert_to_string (index_verse)));
            }
          }
        }
      }
    }
  }
  
  return passages;
}


// Gets the plain raw text for the bible and passage given.
string search_logic_get_bible_verse_text (string bible, int book, int chapter, int verse)
{
  vector <string> texts;
  string path = search_logic_chapter_file (bible, book, chapter);
  string index = filter_url_file_get_contents (path);
  vector <string> lines = filter_string_explode (index, '\n');
  int index_verse = 0;
  bool read_index_verse = false;
  int index_item = 0;
  for (auto & line : lines) {
    if (read_index_verse) {
      index_verse = convert_to_int (line);
      read_index_verse = false;
    } else if (line == search_logic_verse_separator ()) {
      read_index_verse = true;
      index_item = 0;
    } else if (line == search_logic_index_separator ()) {
      index_item++;
    } else if (index_item == PLAIN_RAW) {
      if (verse == index_verse) {
        texts.push_back (line);
      }
    }
  }
  return filter_string_implode (texts, "\n");
}


// Gets the raw USFM for the bible and passage given.
string search_logic_get_bible_verse_usfm (string bible, int book, int chapter, int verse)
{
  vector <string> texts;
  string path = search_logic_chapter_file (bible, book, chapter);
  string index = filter_url_file_get_contents (path);
  vector <string> lines = filter_string_explode (index, '\n');
  int index_verse = 0;
  bool read_index_verse = false;
  int index_item = 0;
  for (auto & line : lines) {
    if (read_index_verse) {
      index_verse = convert_to_int (line);
      read_index_verse = false;
    } else if (line == search_logic_verse_separator ()) {
      read_index_verse = true;
      index_item = 0;
    } else if (line == search_logic_index_separator ()) {
      index_item++;
    } else if (index_item == USFM_RAW) {
      if (verse == index_verse) {
        texts.push_back (line);
      }
    }
  }
  return filter_string_implode (texts, "\n");
}


void search_logic_delete_bible (string bible)
{
  string fragment = search_logic_bible_fragment (bible);
  fragment = filter_url_basename (fragment);
  vector <string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      string path = filter_url_create_path (search_logic_index_folder (), file);
      filter_url_unlink (path);
    }
  }
}


void search_logic_delete_book (string bible, int book)
{
  string fragment = search_logic_book_fragment (bible, book);
  fragment = filter_url_basename (fragment);
  vector <string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      string path = filter_url_create_path (search_logic_index_folder (), file);
      filter_url_unlink (path);
    }
  }
}


void search_logic_delete_chapter (string bible, int book, int chapter)
{
  string fragment = search_logic_chapter_file (bible, book, chapter);
  fragment = filter_url_basename (fragment);
  vector <string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      string path = filter_url_create_path (search_logic_index_folder (), file);
      filter_url_unlink (path);
    }
  }
}


// Returns the total verse count within a $bible.
int search_logic_get_verse_count (string bible)
{
  int verse_count = 0;
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string path = search_logic_chapter_file (bible, book, chapter);
      string index = filter_url_file_get_contents (path);
      vector <string> lines = filter_string_explode (index, '\n');
      for (auto & line : lines) {
        if (line == search_logic_verse_separator ()) {
          verse_count++;
        }
      }
    }
  }
  return verse_count;
}


// Copies the search index of Bible $original to Bible $destination.
void search_logic_copy_bible (string original, string destination)
{
  string original_fragment = search_logic_bible_fragment (original);
  original_fragment = filter_url_basename (original_fragment);
  string destination_fragment = search_logic_bible_fragment (destination);
  destination_fragment = filter_url_basename (destination_fragment);
  vector <string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (original_fragment) == 0) {
      string original_path = filter_url_create_path (search_logic_index_folder (), file);
      string destination_file = destination_fragment + file.substr (original_fragment.length ());
      string destination_path = filter_url_create_path (search_logic_index_folder (), destination_file);
      filter_url_file_cp (original_path, destination_path);
    }
  }
}

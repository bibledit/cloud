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


#include <search/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <config/globals.h>
#include <database/bibles.h>
#include <database/config/bible.h>
#include <database/logic.h>


std::string search_logic_index_folder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "search"});
}


std::string search_logic_bible_fragment (std::string bible)
{
  return filter_url_create_path ({search_logic_index_folder (), bible + "_"});
}


std::string search_logic_book_fragment (std::string bible, int book)
{
  return search_logic_bible_fragment (bible) + std::to_string (book) + "_";
}


std::string search_logic_chapter_file (std::string bible, int book, int chapter)
{
  return search_logic_book_fragment (bible, book) + std::to_string (chapter);
}


std::string search_logic_verse_separator ()
{
  return "v#e#r#s#e#s#e#p#a#r#a#t#o#r";
}


std::string search_logic_index_separator ()
{
  return "i#n#d#e#x#s#e#p#a#r#a#t#o#r";
}


#define USFM_RAW 1
#define USFM_LOWER 2
#define PLAIN_RAW 3
#define PLAIN_LOWER 4


// Indexes a $bible $book $chapter for searching.
void search_logic_index_chapter (std::string bible, int book, int chapter)
{
  std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);

  std::vector <std::string> index;
  
  std::set <std::string> already_processed;
  
  std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
  
  for (auto verse : verses) {

    std::string raw_usfm = filter::strings::trim (filter::usfm::get_verse_text (usfm, verse));

    // In case of combined verses, the bit of USFM may have been indexed already.
    // Skip it in that case.
    if (already_processed.find (raw_usfm) != already_processed.end ()) continue;
    already_processed.insert (raw_usfm);

    index.push_back (search_logic_verse_separator ());
    index.push_back (std::to_string (verse));
    index.push_back (search_logic_index_separator ());

    index.push_back (raw_usfm);
    
    std::string usfm_lower = filter::strings::unicode_string_casefold (raw_usfm);

    index.push_back (search_logic_index_separator ());

    index.push_back (usfm_lower);
    
    // Text filter for getting the plain text.
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.initializeHeadingsAndTextPerVerse (true);
    filter_text.add_usfm_code (raw_usfm);
    filter_text.run (stylesheet);

    std::string raw_plain;
    // Add the clean verse texts.
    std::map <int, std::string> texts = filter_text.getVersesText ();
    for (auto & element : texts) {
      raw_plain.append (element.second + "\n");
    }
    // Add any clean headings.
    std::map <int, std::string> headings = filter_text.verses_headings;
    for (auto & element : headings) {
      raw_plain.append (element.second + "\n");
    }
    // Add any footnotes.
    raw_plain.append (filter_text.text_text->getnote ());
    // Clean up.
    raw_plain = filter::strings::trim (raw_plain);
    
    index.push_back (search_logic_index_separator ());

    index.push_back (raw_plain);
    
    std::string plain_lower = filter::strings::unicode_string_casefold (raw_plain);

    index.push_back (search_logic_index_separator ());

    index.push_back (plain_lower);
  }
  
  index.push_back (search_logic_index_separator ());
  
  // Store everything.
  std::string path = search_logic_chapter_file (bible, book, chapter);
  filter_url_file_put_contents (path, filter::strings::implode (index, "\n"));
}


// Searches the text of the Bibles.
// Returns an array with matching passages.
// $search: Contains the text to search for.
// $bibles: Array of Bible names to search in.
std::vector <Passage> search_logic_search_text (std::string search, std::vector <std::string> bibles)
{
  std::vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = filter::strings::unicode_string_casefold (search);
  search = filter::strings::replace (",", "", search);
  
  for (auto bible : bibles) {
    std::vector <int> books = database::bibles::get_books (bible);
    for (auto book : books) {
      std::vector <int> chapters = database::bibles::get_chapters (bible, book);
      for (auto chapter : chapters) {
        std::string path = search_logic_chapter_file (bible, book, chapter);
        std::string index = filter_url_file_get_contents (path);
        if (index.find (search) != std::string::npos) {
          std::vector <std::string> lines = filter::strings::explode (index, '\n');
          int index_verse = 0;
          bool read_index_verse = false;
          int index_item = 0;
          for (auto & line : lines) {
            if (read_index_verse) {
              index_verse = filter::strings::convert_to_int (line);
              read_index_verse = false;
            } else if (line == search_logic_verse_separator ()) {
              read_index_verse = true;
              index_item = 0;
            } else if (line == search_logic_index_separator ()) {
              index_item++;
            } else if (index_item == PLAIN_LOWER) {
              if (line.find (search) != std::string::npos) {
                passages.push_back (Passage (bible, book, chapter, std::to_string (index_verse)));
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
std::vector <Passage> search_logic_search_bible_text (std::string bible, std::string search)
{
  std::vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = filter::strings::unicode_string_casefold (search);
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string path = search_logic_chapter_file (bible, book, chapter);
      std::string index = filter_url_file_get_contents (path);
      if (index.find (search) != std::string::npos) {
        std::vector <std::string> lines = filter::strings::explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = filter::strings::convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == PLAIN_LOWER) {
            if (line.find (search) != std::string::npos) {
              passages.push_back (Passage (bible, book, chapter, std::to_string (index_verse)));
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
std::vector <Passage> search_logic_search_bible_text_case_sensitive (std::string bible, std::string search)
{
  std::vector <Passage> passages;
  
  if (search == "") return passages;
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string path = search_logic_chapter_file (bible, book, chapter);
      std::string index = filter_url_file_get_contents (path);
      if (index.find (search) != std::string::npos) {
        std::vector <std::string> lines = filter::strings::explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = filter::strings::convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == PLAIN_RAW) {
            if (line.find (search) != std::string::npos) {
              passages.push_back (Passage (bible, book, chapter, std::to_string (index_verse)));
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
std::vector <Passage> search_logic_search_bible_usfm (std::string bible, std::string search)
{
  std::vector <Passage> passages;
  
  if (search == "") return passages;
  
  search = filter::strings::unicode_string_casefold (search);
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string path = search_logic_chapter_file (bible, book, chapter);
      std::string index = filter_url_file_get_contents (path);
      if (index.find (search) != std::string::npos) {
        std::vector <std::string> lines = filter::strings::explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = filter::strings::convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == USFM_LOWER) {
            if (line.find (search) != std::string::npos) {
              passages.push_back (Passage (bible, book, chapter, std::to_string (index_verse)));
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
std::vector <Passage> search_logic_search_bible_usfm_case_sensitive (std::string bible, std::string search)
{
  std::vector <Passage> passages;
  
  if (search == "") return passages;
  
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string path = search_logic_chapter_file (bible, book, chapter);
      std::string index = filter_url_file_get_contents (path);
      if (index.find (search) != std::string::npos) {
        std::vector <std::string> lines = filter::strings::explode (index, '\n');
        int index_verse = 0;
        bool read_index_verse = false;
        int index_item = 0;
        for (auto & line : lines) {
          if (read_index_verse) {
            index_verse = filter::strings::convert_to_int (line);
            read_index_verse = false;
          } else if (line == search_logic_verse_separator ()) {
            read_index_verse = true;
            index_item = 0;
          } else if (line == search_logic_index_separator ()) {
            index_item++;
          } else if (index_item == USFM_RAW) {
            if (line.find (search) != std::string::npos) {
              passages.push_back (Passage (bible, book, chapter, std::to_string (index_verse)));
            }
          }
        }
      }
    }
  }
  
  return passages;
}


// Gets the plain raw text for the bible and passage given.
std::string search_logic_get_bible_verse_text (std::string bible, int book, int chapter, int verse)
{
  std::vector <std::string> texts;
  std::string path = search_logic_chapter_file (bible, book, chapter);
  std::string index = filter_url_file_get_contents (path);
  std::vector <std::string> lines = filter::strings::explode (index, '\n');
  int index_verse = 0;
  bool read_index_verse = false;
  int index_item = 0;
  for (auto & line : lines) {
    if (read_index_verse) {
      index_verse = filter::strings::convert_to_int (line);
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
  return filter::strings::implode (texts, "\n");
}


// Gets the raw USFM for the bible and passage given.
std::string search_logic_get_bible_verse_usfm (std::string bible, int book, int chapter, int verse)
{
  std::vector <std::string> texts;
  std::string path = search_logic_chapter_file (bible, book, chapter);
  std::string index = filter_url_file_get_contents (path);
  std::vector <std::string> lines = filter::strings::explode (index, '\n');
  int index_verse = 0;
  bool read_index_verse = false;
  int index_item = 0;
  for (auto & line : lines) {
    if (read_index_verse) {
      index_verse = filter::strings::convert_to_int (line);
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
  return filter::strings::implode (texts, "\n");
}


void search_logic_delete_bible (std::string bible)
{
  std::string fragment = search_logic_bible_fragment (bible);
  fragment = filter_url_basename (fragment);
  std::vector <std::string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      std::string path = filter_url_create_path ({search_logic_index_folder (), file});
      filter_url_unlink (path);
    }
  }
}


void search_logic_delete_book (std::string bible, int book)
{
  std::string fragment = search_logic_book_fragment (bible, book);
  fragment = filter_url_basename (fragment);
  std::vector <std::string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      std::string path = filter_url_create_path ({search_logic_index_folder (), file});
      filter_url_unlink (path);
    }
  }
}


void search_logic_delete_chapter (std::string bible, int book, int chapter)
{
  std::string fragment = search_logic_chapter_file (bible, book, chapter);
  fragment = filter_url_basename (fragment);
  std::vector <std::string> files = filter_url_scandir (search_logic_index_folder ());
  for (auto & file : files) {
    if (file.find (fragment) == 0) {
      std::string path = filter_url_create_path ({search_logic_index_folder (), file});
      filter_url_unlink (path);
    }
  }
}


// Returns the total verse count within a $bible.
int search_logic_get_verse_count (std::string bible)
{
  int verse_count = 0;
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string path = search_logic_chapter_file (bible, book, chapter);
      std::string index = filter_url_file_get_contents (path);
      std::vector <std::string> lines = filter::strings::explode (index, '\n');
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
void search_logic_copy_bible (std::string original, std::string destination)
{
  std::string original_fragment = search_logic_bible_fragment (original);
  original_fragment = filter_url_basename (original_fragment);
  std::string destination_fragment = search_logic_bible_fragment (destination);
  destination_fragment = filter_url_basename (destination_fragment);
  std::vector <std::string> files = filter_url_scandir (search_logic_index_folder ());
  for (const auto& file : files) {
    if (file.find (original_fragment) == 0) {
      const std::string original_path = filter_url_create_path ({search_logic_index_folder (), file});
      const std::string destination_file = destination_fragment + file.substr (original_fragment.length ());
      const std::string destination_path = filter_url_create_path ({search_logic_index_folder (), destination_file});
      filter_url_file_cp (original_path, destination_path);
    }
  }
}


// This generated the plain text that can be used as a reference during a replace operation.
std::string search_logic_plain_replace_verse_text (std::string usfm)
{
  // Text filter for getting the plain text.
  Filter_Text filter_text = Filter_Text ("");
  filter_text.text_text = new Text_Text ();
  filter_text.initializeHeadingsAndTextPerVerse (true);
  filter_text.add_usfm_code (usfm);
  filter_text.run (styles_logic_standard_sheet ());
  
  // The resulting plain text.
  std::string plain_text;

  // Add the clean verse texts.
  std::map <int, std::string> texts = filter_text.getVersesText ();
  for (auto & element : texts) {
    plain_text.append (element.second + "\n");
  }

  // Add any clean headings.
  std::map <int, std::string> headings = filter_text.verses_headings;
  for (auto & element : headings) {
    plain_text.append (element.second + "\n");
  }

  // Add any footnotes.
  plain_text.append (filter_text.text_text->getnote ());
  
  // Clean up.
  plain_text = filter::strings::trim (plain_text);
  
  // Done.
  return plain_text;
}

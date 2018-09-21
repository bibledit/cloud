/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <export/quickbible.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/shell.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/sheets.h>


// Info about Quick Bible export format:
// http://www.bibleforandroid.com/developer
// https://github.com/yukuku/androidbible


string export_quickbible_tabify (const string & one, const string & two, const string & three, const string & four = "", const string & five = "", const string & six = "")
{
  string result;
  result.append (one);
  if (!two.empty ()) {
    result.append ("\t");
    result.append (two);
  }
  if (!three.empty ()) {
    result.append ("\t");
    result.append (three);
  }
  if (!four.empty ()) {
    result.append ("\t");
    result.append (four);
  }
  if (!five.empty ()) {
    result.append ("\t");
    result.append (five);
  }
  if (!six.empty ()) {
    result.append ("\t");
    result.append (six);
  }
  result.append ("\n");
  return result;
}


void export_quickbible (string bible, bool log)
{
  string directory = filter_url_create_path (Export_Logic::bibleDirectory (bible), "quickbible");
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);

  Database_Bibles database_bibles;
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  string yet_contents;
  
  // Storage for the footnotes.
  // Format: book, chapter, verse, note text.
  vector <tuple <int, int, int, string> > footnotes;
  
  yet_contents.append (export_quickbible_tabify ("info", "shortName", bible));
  yet_contents.append (export_quickbible_tabify ("info", "longName", bible));
  yet_contents.append (export_quickbible_tabify ("info", "description", bible));
  yet_contents.append (export_quickbible_tabify ("info", "locale", "en"));
  
  vector <int> books = database_bibles.getBooks (bible);

  for (auto book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    yet_contents.append (export_quickbible_tabify ("book_name", convert_to_string (book), bookname, ""));
  }
  
  for (auto book : books) {
    // The .yet to .yes converter only handles books > 0.
    if (book) {
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        // The .yet to .yes converter only handles chapters > 0.
        if (chapter) {
          Filter_Text filter_text = Filter_Text (bible);
          filter_text.initializeHeadingsAndTextPerVerse (true);
          string usfm = database_bibles.getChapter (bible, book, chapter);
          usfm = usfm_remove_word_level_attributes (usfm);
          filter_text.addUsfmCode (usfm);
          filter_text.run (stylesheet);
          map <int, string> text = filter_text.getVersesText ();
          for (auto & element : text) {
            int verse = element.first;
            // The .yet to .yes converter only handles verses > 0.
            if (verse) {
              string bk = convert_to_string (book);
              string ch = convert_to_string (chapter);
              string vs = convert_to_string (element.first);
              string tx = element.second;
              if (tx.empty ()) tx = "empty";
              yet_contents.append (export_quickbible_tabify ("verse", bk, ch, vs, tx));
            }
          }
          for (auto & element : filter_text.notes_plain_text) {
            int verse = convert_to_int (element.first);
            string note = element.second;
            footnotes.push_back (make_tuple (book, chapter, verse, note));
          }
        }
      }
    }
  }

  // Both footnotes and cross references are supported.
  // There is no need for any flags to Yet2Yes2.jar to enable it.
  // It has to be included below the "verse" lines, as "footnote" and "xref" lines in the YET file.
  // Example:
  // footnote  66  22  3  1  Greek "bondservants"; also verse 6.
  int previousbook = 0, previouschapter = 0, previousverse = 0;
  int offset = 1;
  for (auto & note : footnotes) {
    int book = get<0>(note);
    int chapter = get<1>(note);
    int verse = get<2>(note);
    string notetext = get<3>(note);
    if ((book != previousbook) || (chapter != previouschapter) || (verse != previousverse)) {
      offset = 1;
    } else {
      offset++;
    }
    yet_contents.append (export_quickbible_tabify ("footnote", convert_to_string (book), convert_to_string (chapter), convert_to_string (verse), convert_to_string (offset), notetext));
    previousbook = verse;
    previouschapter = chapter;
    previousverse = verse;
  }

  string bible_yet = filter_url_create_path (directory, "bible.yet");
  filter_url_file_put_contents (bible_yet, yet_contents);
  
  string quickbible_html_source = filter_url_create_root_path ("export", "quickbb.html");
  string quickbible_html_destination = filter_url_create_path (directory, "readme.html");
  string contents = filter_url_file_get_contents (quickbible_html_source);
  filter_url_file_put_contents (quickbible_html_destination, contents);
  
  Database_State::clearExport (bible, 0, Export_Logic::export_quick_bible);
  
  if (log) Database_Logs::log (translate("Exported to Quick Bible") + " " + bible, Filter_Roles::translator ());
}

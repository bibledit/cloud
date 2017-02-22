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


#include <export/html.h>
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
#include <filter/css.h>
#include <fonts/logic.h>
#include <html/text.h>
#include <html/header.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_html_book (string bible, int book, bool log)
{
  // Create folders for the html export.
  string directory = filter_url_create_path (Export_Logic::bibleDirectory (bible), "html");
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  // Filename for the html file.
  string basename = Export_Logic::baseBookFileName (book);
  string filename = filter_url_create_path (directory, basename + ".html");
  string filecss = filter_url_create_path (directory, "stylesheet.css");
  
  
  Database_Bibles database_bibles;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  // Create stylesheet.
  Styles_Sheets styles_sheets;
  styles_sheets.create (stylesheet, filecss, false, bible);
  
  
  // Copy font to the output directory.
  string font = Fonts_Logic::getTextFont (bible);
  if (!font.empty ()) {
    if (Fonts_Logic::fontExists (font)) {
      string fontpath = Fonts_Logic::getFontPath (font);
      string contents = filter_url_file_get_contents (fontpath);
      fontpath = filter_url_create_path (directory, font);
      filter_url_file_put_contents (fontpath, contents);
    }
  }
  
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new Html_Text (translate("Bible"));
  filter_text.html_text_standard->customClass = Filter_Css::getClass (bible);
  
  
  // Load one book.
  vector <int> chapters = database_bibles.getChapters (bible, book);
  for (auto chapter : chapters) {
    string usfm = database_bibles.getChapter (bible, book, chapter);
    usfm = filter_string_trim (usfm);
    // Use small chunks of USFM at a time for much better performance.
    filter_text.addUsfmCode (usfm);
  }
  
  
  // Convert the USFM.
  filter_text.run (stylesheet);
  
  
  // Save file.
  filter_text.html_text_standard->save (filename);
  
  
  // Clear the flag for this export.
  Database_State::clearExport (bible, book, Export_Logic::export_html);

  
  if (log) Database_Logs::log (translate("Exported to html") + ": " + bible + " " + Database_Books::getEnglishFromId (book), Filter_Roles::translator ());
}

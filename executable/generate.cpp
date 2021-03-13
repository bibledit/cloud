/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <library/bibledit.h>
#include <config/libraries.h>
#include <config/globals.h>
#include <filter/url.h>
#include <filter/string.h>
#include <setup/logic.h>
#include <demo/logic.h>
#include <sources/morphhb.h>
#include <sources/oshb.h>
#include <sources/styles.h>
#include <sources/abbott-smith.h>


int main (int argc, char **argv)
{
  cout << "Data Generator " << config_logic_version () << endl;
  
  if (argc < 2) {
    cerr << "Please pass the document root folder as the first argument" << endl;
    return EXIT_FAILURE;
  }
  config_globals_document_root = argv [1];
  cout << "Document root folder: " << config_globals_document_root << endl;
  if (!file_or_dir_exists (config_globals_document_root)) {
    cerr << "Please pass an existing document root folder" << endl;
    return EXIT_FAILURE;
  }
  
  if (argc < 3) {
    cerr << "Please pass a command as the second argument" << endl;
    return EXIT_FAILURE;
  }
  string command = argv [2];
  
  string locale_command = "locale";
  string sample_bible_command = "samplebible";
  string mappings_command = "mappings";
  string versifications_command = "versifications";
  string morphhb_command = "morphhb";
  string oshb_command = "oshb";
  string stylesheet_command = "styles";
  string abbott_smith_command = "abbott-smith";
  
  if (command == locale_command) {
  
    cout << "Generating locale databases from the *.po files in folder locale" << endl;
    setup_generate_locale_databases (true);
    
  } else if (command == sample_bible_command) {

    cout << "Generating the sample Bible" << endl;
    demo_prepare_sample_bible ();

  } else if (command == mappings_command) {

    cout << "Generating the verse mappings database" << endl;
    setup_generate_verse_mapping_databases ();

  } else if (command == versifications_command) {
    
    cout << "Generating the versifications database" << endl;
    setup_generate_versification_databases ();
    
  } else if (command == morphhb_command) {
    
    cout << "Parsing Open Scriptures Hebrew with limited morphology into the morphhb database" << endl;
    sources_morphhb_parse ();
    
  } else if (command == oshb_command) {
    
    cout << "Parsing Open Scriptures Hebrew Bible with morphology into the oshb database" << endl;
    sources_oshb_parse ();
    
  } else if (command == stylesheet_command) {
    
    cout << "Parsing style values and importing them into the default styles" << endl;
    sources_styles_parse ();

  } else if (command == abbott_smith_command) {

    cout << "Parsing Abbott-Smith's Manual Greek Lexicon into the abbottsmith database" << endl;
    sources_abbott_smith_parse ();
    
  } else {
    
    cerr << "This command is unknown" << endl;
    cerr << "The following commands are supported:" << endl;
    cerr << locale_command << ": Generate locale databases from the *.po files in folder locale" << endl;
    cerr << sample_bible_command << ": Generate the sample Bible" << endl;
    cerr << mappings_command << ": Generate the default verse mappings database" << endl;
    cerr << versifications_command << ": Generate the default versifications database" << endl;
    cerr << morphhb_command << ": Parse Open Scriptures Hebrew with limited morphology into the morphhb database" << endl;
    cerr << oshb_command << ": Parse Open Scriptures Hebrew Bible with morphology into the oshb database" << endl;
    cerr << stylesheet_command << ": Parse style values and import them into the default styles" << endl;
    cout << abbott_smith_command << ": Parse Abbott-Smith's Manual Greek Lexicon into the abbottsmith database" << endl;
    
    return EXIT_FAILURE;
    
  }
  
  return EXIT_SUCCESS;
}

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
#include <i18n/logic.h>


int main (int argc, char **argv)
{
  std::cout << "Data Generator " << config::logic::version () << std::endl;
  
  if (argc < 2) {
    std::cerr << "Please pass the document root folder as the first argument" << std::endl;
    return EXIT_FAILURE;
  }
  config_globals_document_root = argv [1];
  std::cout << "Document root folder: " << config_globals_document_root << std::endl;
  if (!file_or_dir_exists (config_globals_document_root)) {
    std::cerr << "Please pass an existing document root folder" << std::endl;
    return EXIT_FAILURE;
  }
  
  if (argc < 3) {
    std::cerr << "Please pass a command as the second argument" << std::endl;
    return EXIT_FAILURE;
  }
  std::string command = argv [2];

  std::string i18n_command {"i18n"};
  std::string locale_command {"locale"};
  std::string sample_bible_command {"samplebible"};
  std::string mappings_command {"mappings"};
  std::string versifications_command {"versifications"};
  std::string morphhb_command {"morphhb"};
  std::string oshb_command {"oshb"};
  std::string stylesheet_command {"styles"};
  std::string abbott_smith_command {"abbott-smith"};

  if (command == i18n_command) {

    std::cout << "Translating untranslated GUI texts through Google Translate" << std::endl;
    i18n_logic_augment_via_google_translate ();

  } else if (command == locale_command) {
  
    std::cout << "Generating locale databases from the *.po files in folder locale" << std::endl;
    setup_generate_locale_databases (true);
    
  } else if (command == sample_bible_command) {

    std::cout << "Generating the sample Bible" << std::endl;
    demo_prepare_sample_bible ();

  } else if (command == mappings_command) {

    std::cout << "Generating the verse mappings database" << std::endl;
    setup_generate_verse_mapping_databases ();

  } else if (command == versifications_command) {
    
    std::cout << "Generating the versifications database" << std::endl;
    setup_generate_versification_databases ();
    
  } else if (command == morphhb_command) {
    
    std::cout << "Parsing Open Scriptures Hebrew with limited morphology into the morphhb database" << std::endl;
    sources_morphhb_parse ();
    
  } else if (command == oshb_command) {
    
    std::cout << "Parsing Open Scriptures Hebrew Bible with morphology into the oshb database" << std::endl;
    sources_oshb_parse ();
    
  } else if (command == stylesheet_command) {
    
    std::cout << "Parsing style values and importing them into the default styles" << std::endl;
    sources_styles_parse ();

  } else if (command == abbott_smith_command) {

    std::cout << "Parsing Abbott-Smith's Manual Greek Lexicon into the abbottsmith database" << std::endl;
    sources_abbott_smith_parse ();
    
  } else {
    
    std::cerr << "This command is unknown" << std::endl;
    std::cerr << "The following commands are supported:" << std::endl;
    std::cerr << i18n_command << ": Translate untranslated GUI texts through Google Translate" << std::endl;
    std::cerr << locale_command << ": Generate locale databases from the *.po files in folder locale" << std::endl;
    std::cerr << sample_bible_command << ": Generate the sample Bible" << std::endl;
    std::cerr << mappings_command << ": Generate the default verse mappings database" << std::endl;
    std::cerr << versifications_command << ": Generate the default versifications database" << std::endl;
    std::cerr << morphhb_command << ": Parse Open Scriptures Hebrew with limited morphology into the morphhb database" << std::endl;
    std::cerr << oshb_command << ": Parse Open Scriptures Hebrew Bible with morphology into the oshb database" << std::endl;
    std::cerr << stylesheet_command << ": Parse style values and import them into the default styles" << std::endl;
    std::cout << abbott_smith_command << ": Parse Abbott-Smith's Manual Greek Lexicon into the abbottsmith database" << std::endl;
    
    return EXIT_FAILURE;
    
  }
  
  return EXIT_SUCCESS;
}

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


#ifndef INCLUDED_LEXICON_LOGIC_H
#define INCLUDED_LEXICON_LOGIC_H


#include <config/libraries.h>


#define HEBREW_ETCBC4_NAME "Hebrew (University of Amsterdam)"
#define HEBREW_ETCBC4_PREFIX "e"
#define KJV_LEXICON_NAME "King James Bible with Lexicon"
#define KJV_LEXICON_PREFIX "k"
#define OSHB_NAME "Hebrew (Open Scriptures)"
#define OSHB_PREFIX "o"
#define SBLGNT_NAME "Greek (SBL)"
#define SBLGNT_PREFIX "s"
#define BDB_PREFIX "b"


vector <string> lexicon_logic_resource_names ();
string lexicon_logic_get_html (void * webserver_request, string lexicon, int book, int chapter, int verse);
string lexicon_logic_get_script (string prefix);
string lexicon_logic_strong_number_cleanup (string strong);
void lexicon_logic_convert_morphhb_parsing_to_strong (string parsing,
                                                      vector <string>& strongs,
                                                      vector <string>& bdbs);
string lexicon_logic_render_definition (string strong);
string lexicon_logic_render_part_of_speech (string value);
string lexicon_logic_render_part_of_speech_stem (string abbrev);
string lexicon_logic_render_part_of_speech_person (string abbrev);
string lexicon_logic_render_part_of_speech_gender (string abbrev);
string lexicon_logic_render_part_of_speech_number (string abbrev);
string lexicon_logic_render_part_of_speech_state (string abbrev);
string lexicon_logic_define_user_strong (string strong);
string lexicon_logic_render_morphgnt_part_of_speech (string pos);
string lexicon_logic_render_morphgnt_parsing_code (string parsing);
string lexicon_logic_render_etcbc4_morphology (string rowid);
string lexicon_logic_render_bdb_entry (string code);
string lexicon_logic_get_remove_attribute (string & xml, const char * key);
string lexicon_logic_get_text (string & xml);

string lexicon_logic_hebrew_morphology_render (string value);
string lexicon_logic_hebrew_morphology_render_type_verb_conjugation (string & value);
string lexicon_logic_hebrew_morphology_render_type_adjective (string & value);
string lexicon_logic_hebrew_morphology_render_type_noun (string & value);
string lexicon_logic_hebrew_morphology_render_type_pronoun (string & value);
string lexicon_logic_hebrew_morphology_render_type_preposition (string & value);
string lexicon_logic_hebrew_morphology_render_type_suffix (string & value);
string lexicon_logic_hebrew_morphology_render_type_particle (string & value);
string lexicon_logic_hebrew_morphology_render_stem (bool hebrew, bool aramaic, string & value);
string lexicon_logic_hebrew_morphology_render_person (string & value);
string lexicon_logic_hebrew_morphology_render_gender (string & value);
string lexicon_logic_hebrew_morphology_render_number (string & value);
string lexicon_logic_hebrew_morphology_render_state (string & value);


#endif

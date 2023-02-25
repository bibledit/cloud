/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


std::vector <std::string> lexicon_logic_resource_names ();
std::string lexicon_logic_get_html (void * webserver_request, std::string lexicon, int book, int chapter, int verse);
std::string lexicon_logic_get_script (std::string prefix);
std::string lexicon_logic_strong_number_cleanup (std::string strong);
void lexicon_logic_convert_morphhb_parsing_to_strong (std::string parsing,
                                                      std::vector <std::string>& strongs,
                                                      std::vector <std::string>& bdbs);
std::string lexicon_logic_render_strongs_definition (std::string strong);
std::string lexicon_logic_render_strongs_part_of_speech (std::string value);
std::string lexicon_logic_render_strongs_part_of_speech_stem (std::string abbrev);
std::string lexicon_logic_render_strongs_part_of_speech_person (std::string abbrev);
std::string lexicon_logic_render_strongs_part_of_speech_gender (std::string abbrev);
std::string lexicon_logic_render_strongs_part_of_speech_number (std::string abbrev);
std::string lexicon_logic_render_strongs_part_of_speech_state (std::string abbrev);
std::string lexicon_logic_define_user_strong (std::string strong);
std::string lexicon_logic_render_morphgnt_part_of_speech (std::string pos);
std::string lexicon_logic_render_morphgnt_parsing_code (std::string parsing);
std::string lexicon_logic_render_etcbc4_morphology (std::string rowid);
std::string lexicon_logic_render_bdb_entry (std::string code);
std::string lexicon_logic_get_remove_attribute (std::string & xml, const char * key);
std::string lexicon_logic_get_text (std::string & xml);

std::string lexicon_logic_hebrew_morphology_render (std::string value);
std::string lexicon_logic_hebrew_morphology_render_type_verb_conjugation (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_adjective (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_noun (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_pronoun (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_preposition (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_suffix (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_type_particle (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_stem (bool hebrew, bool aramaic, std::string & value);
std::string lexicon_logic_hebrew_morphology_render_person (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_gender (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_number (std::string & value);
std::string lexicon_logic_hebrew_morphology_render_state (std::string & value);

std::string lexicon_logic_render_abbott_smiths_definition (std::string lemma, std::string strong);


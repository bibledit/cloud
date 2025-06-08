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


#include <lexicon/definition.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <lexicon/logic.h>
#include <database/kjv.h>
#include <database/oshb.h>
#include <database/morphgnt.h>
#include <database/strong.h>


std::string lexicon_definition_url ()
{
  return "lexicon/definition";
}


bool lexicon_definition_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string lexicon_definition (Webserver_Request& webserver_request)
{
  // Retrieve the id: It may contain a Strong's number or a lemma.
  std::string id = webserver_request.query["id"];
  
  std::vector <std::string> renderings;
  
  if (!id.empty ()) {
    
    std::string letter = id.substr (0, 1);
    
    // ETCBC4 database.
    if (letter == HEBREW_ETCBC4_PREFIX) {
      renderings.push_back (lexicon_logic_render_etcbc4_morphology (id));
    }
    
    // King James Bible with Strong's numbers.
    else if (letter == KJV_LEXICON_PREFIX) {
      Database_Kjv database_kjv;
      std::string strong = database_kjv.strong (filter::strings::convert_to_int (id.substr (1)));
      std::string rendering = lexicon_logic_render_strongs_definition (strong);
      if (!rendering.empty ()) renderings.push_back (rendering);
      rendering = lexicon_logic_render_abbott_smiths_definition("", strong);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Open Scriptures Hebrew with Strong's numbers and morphology.
    else if (letter == OSHB_PREFIX) {
      int rowid = filter::strings::convert_to_int (id.substr (1));
      Database_OsHb database_oshb;
      std::string morph = database_oshb.morph (rowid);
      renderings.push_back (lexicon_logic_hebrew_morphology_render (morph));
      std::string lemma = database_oshb.lemma (rowid);
      std::vector <std::string> strongs;
      std::vector <std::string> bdbs;
      lexicon_logic_convert_morphhb_parsing_to_strong (lemma, strongs, bdbs);
      for (size_t i = 0; i < strongs.size (); i++) {
        std::string rendering1 = lexicon_logic_render_strongs_definition (strongs[i]);
        if (!rendering1.empty ()) renderings.push_back (rendering1);
        std::stringstream rendering2;
        rendering2 << "<a href=" << std::quoted(BDB_PREFIX + bdbs[i]) << ">Brown Driver Briggs</a>";
        renderings.push_back (rendering2.str());
      }
    }
    
    // SBL Greek New Testament plus morphology.
    else if (letter == SBLGNT_PREFIX) {
      Database_MorphGnt database_morphgnt;
      Database_Strong database_strong;
      int rowid = filter::strings::convert_to_int (id.substr (1));
      // The part of speech.
      std::string pos = database_morphgnt.pos (rowid);
      std::string rendering = lexicon_logic_render_morphgnt_part_of_speech (pos);
      rendering.append (" ");
      // The parsing.
      std::string parsing = database_morphgnt.parsing (rowid);
      rendering.append (lexicon_logic_render_morphgnt_parsing_code (parsing));
      renderings.push_back (rendering);
      // The lemma.
      std::string lemma = database_morphgnt.lemma (rowid);
      std::vector <std::string> strongs = database_strong.strong (lemma);
      for (auto & lexicon_id : strongs) {
        rendering = lexicon_logic_render_strongs_definition (lexicon_id);
        if (!rendering.empty ()) renderings.push_back (rendering);
      }
      rendering = lexicon_logic_render_abbott_smiths_definition(lemma, "");
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Strong's Hebrew.
    else if (letter == "H") {
      std::string rendering = lexicon_logic_render_strongs_definition (id);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Strong's Greek.
    else if (letter == "G") {
      std::string rendering = lexicon_logic_render_strongs_definition (id);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Brown Driver Briggs lexicon.
    else if (letter == BDB_PREFIX) {
      std::string rendering = lexicon_logic_render_bdb_entry (id.substr (1));
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Unknown definition request.
    else {
      renderings.push_back (id);
    }
    
  }
  
  return filter::strings::implode (renderings, "<br>");
}

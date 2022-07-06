/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


string lexicon_definition_url ()
{
  return "lexicon/definition";
}


bool lexicon_definition_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string lexicon_definition (void * webserver_request)
{
  // Retrieve the id: It may contain a Strong's number or a lemma.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string id = request->query["id"];
  
  vector <string> renderings;
  
  if (!id.empty ()) {
    
    string letter = id.substr (0, 1);
    
    // ETCBC4 database.
    if (letter == HEBREW_ETCBC4_PREFIX) {
      renderings.push_back (lexicon_logic_render_etcbc4_morphology (id));
    }
    
    // King James Bible with Strong's numbers.
    else if (letter == KJV_LEXICON_PREFIX) {
      Database_Kjv database_kjv;
      string strong = database_kjv.strong (convert_to_int (id.substr (1)));
      string rendering = lexicon_logic_render_strongs_definition (strong);
      if (!rendering.empty ()) renderings.push_back (rendering);
      rendering = lexicon_logic_render_abbott_smiths_definition("", strong);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Open Scriptures Hebrew with Strong's numbers and morphology.
    else if (letter == OSHB_PREFIX) {
      int rowid = convert_to_int (id.substr (1));
      Database_OsHb database_oshb;
      string morph = database_oshb.morph (rowid);
      renderings.push_back (lexicon_logic_hebrew_morphology_render (morph));
      string lemma = database_oshb.lemma (rowid);
      vector <string> strongs;
      vector <string> bdbs;
      lexicon_logic_convert_morphhb_parsing_to_strong (lemma, strongs, bdbs);
      for (size_t i = 0; i < strongs.size (); i++) {
        string rendering1 = lexicon_logic_render_strongs_definition (strongs[i]);
        if (!rendering1.empty ()) renderings.push_back (rendering1);
        stringstream rendering2;
        rendering2 << "<a href=" << quoted(BDB_PREFIX + bdbs[i]) << ">Brown Driver Briggs</a>";
        renderings.push_back (rendering2.str());
      }
    }
    
    // SBL Greek New Testament plus morphology.
    else if (letter == SBLGNT_PREFIX) {
      Database_MorphGnt database_morphgnt;
      Database_Strong database_strong;
      int rowid = convert_to_int (id.substr (1));
      // The part of speech.
      string pos = database_morphgnt.pos (rowid);
      string rendering = lexicon_logic_render_morphgnt_part_of_speech (pos);
      rendering.append (" ");
      // The parsing.
      string parsing = database_morphgnt.parsing (rowid);
      rendering.append (lexicon_logic_render_morphgnt_parsing_code (parsing));
      renderings.push_back (rendering);
      // The lemma.
      string lemma = database_morphgnt.lemma (rowid);
      vector <string> strongs = database_strong.strong (lemma);
      for (auto & lexicon_id : strongs) {
        rendering = lexicon_logic_render_strongs_definition (lexicon_id);
        if (!rendering.empty ()) renderings.push_back (rendering);
      }
      rendering = lexicon_logic_render_abbott_smiths_definition(lemma, "");
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Strong's Hebrew.
    else if (letter == "H") {
      string rendering = lexicon_logic_render_strongs_definition (id);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Strong's Greek.
    else if (letter == "G") {
      string rendering = lexicon_logic_render_strongs_definition (id);
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Brown Driver Briggs lexicon.
    else if (letter == BDB_PREFIX) {
      string rendering = lexicon_logic_render_bdb_entry (id.substr (1));
      if (!rendering.empty ()) renderings.push_back (rendering);
    }
    
    // Unknown definition request.
    else {
      renderings.push_back (id);
    }
    
  }
  
  return filter_string_implode (renderings, "<br>");
}

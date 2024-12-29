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


#include <lexicon/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/etcbc4.h>
#include <database/kjv.h>
#include <database/oshb.h>
#include <database/morphgnt.h>
#include <database/strong.h>
#include <database/hebrewlexicon.h>
#include <database/abbottsmith.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <webserver/request.h>


#define BETH_STRONG 10000
#define CONJUNCTION_STRONG 10001
#define DEFINITE_ARTICLE_STRONG 10002
#define INDEFINITE_ARTICLE_STRONG 10003
#define KAF_STRONG 10004
#define LAMED_STRONG 10005
#define MEM_STRONG 10006
#define SHIN_STRONG 10007


// Internal function declarations.
std::string lexicon_logic_render_part_of_speech_pop_front (std::vector <std::string> & parts);


// The names of the available lexicon resources.
std::vector <std::string> lexicon_logic_resource_names ()
{
  return {
    HEBREW_ETCBC4_NAME,
    KJV_LEXICON_NAME,
    OSHB_NAME,
    SBLGNT_NAME
  };
}


// Gets the HTMl for displaying the book/chapter/verse of the $lexicon.
std::string lexicon_logic_get_html ([[maybe_unused]] Webserver_Request& webserver_request, std::string lexicon, int book, int chapter, int verse)
{
  std::string html;
  
  if (lexicon == HEBREW_ETCBC4_NAME) {
    std::string prefix = HEBREW_ETCBC4_PREFIX;
    // Data from the ETCBC4 database.
    const std::vector <int> rowids = database::etcbc4::rowids (book, chapter, verse);
    std::stringstream ss;
    if (!rowids.empty ()) {
      std::string id = "lexicontxt" + prefix;
      ss << "<div id=" << std::quoted(id) << ">" << std::endl;
      for (auto rowid : rowids) {
        ss << "<table class='interlinear rtl'>";
        ss << "<tr>";
        ss << "<td class=" << std::quoted ("hebrew") << ">";
        std::string word = database::etcbc4::word (rowid);
        ss << "<a href=" << std::quoted(HEBREW_ETCBC4_PREFIX + std::to_string (rowid)) << ">" << word << "</a>";
        ss << "</td>";
        ss << "</tr>";
        ss << "<tr>";
        ss << "<td>";
        std::string gloss = database::etcbc4::gloss (rowid);
        gloss = filter::strings::escape_special_xml_characters (gloss);
        ss << gloss;
        ss << "</td>";
        ss << "</tr>";
        ss << "</table>";
      }
      ss << "</div>";
      ss << lexicon_logic_get_script (prefix);
    }
    html.append(ss.str());
  }

  if (lexicon == KJV_LEXICON_NAME) {
    std::string prefix = KJV_LEXICON_PREFIX;
    Database_Kjv database_kjv;
    std::vector <int> rowids = database_kjv.rowids (book, chapter, verse);
    if (!rowids.empty ()) {
      std::stringstream ss;
      std::string id = "lexicontxt" + prefix;
      ss << "<div id=" << std::quoted(id) << ">" << std::endl;
      for (size_t i = 0; i < rowids.size (); i++) {
        int rowid = rowids[i];
        std::string english = database_kjv.english (rowid);
        ss << "<a href=" << std::quoted(KJV_LEXICON_PREFIX + std::to_string (rowid)) << ">" << english << "</a>";
      }
      ss << "</div>";
      ss << lexicon_logic_get_script (prefix);
      html.append(ss.str());
    }
  }

  if (lexicon == OSHB_NAME) {
    std::string prefix = OSHB_PREFIX;
    Database_OsHb database_oshb;
    std::vector <int> rowids = database_oshb.rowids (book, chapter, verse);
    if (!rowids.empty ()) {
      std::stringstream ss;
      std::string id = "lexicontxt" + prefix;
      ss << "<div id=" << std::quoted(id) << " class=" << std::quoted("hebrew") << ">" << std::endl;
      for (size_t i = 0; i < rowids.size (); i++) {
        int rowid = rowids[i];
        std::string word = database_oshb.word (rowid);
        // Give more spacing where needed.
        if (word == "׀") word = " ׀ ";
        ss << "<a href=" << std::quoted(OSHB_PREFIX + std::to_string (rowid)) << ">" << word << "</a>";
      }
      ss << "</div>";
      ss << lexicon_logic_get_script (prefix);
      html.append(ss.str());
    }
  }
  
  if (lexicon == SBLGNT_NAME) {
    std::string prefix = SBLGNT_PREFIX;
    Database_MorphGnt database_morphgnt;
    std::vector <int> rowids = database_morphgnt.rowids (book, chapter, verse);
    if (!rowids.empty ()) {
      std::stringstream ss;
      std::string id = "lexicontxt" + prefix;
      ss << "<div id=" << std::quoted(id) << " class=" << std::quoted("greek") << ">" << std::endl;
      for (size_t i = 0; i < rowids.size (); i++) {
        if (i) ss << " ";
        int rowid = rowids[i];
        std::string word = database_morphgnt.word (rowid);
        ss << "<a href=" << std::quoted(SBLGNT_PREFIX + std::to_string (rowid)) << ">" << word << "</a>";
      }
      ss << "</div>";
      ss << lexicon_logic_get_script (prefix);
      html.append (ss.str());
    }
  }
  
  return html;
}


// The script to put into the html for a lexicon's defined $prefix.
std::string lexicon_logic_get_script (std::string prefix)
{
  std::string defid = "lexicondef" + prefix;
  std::string txtid = "lexicontxt" + prefix;

  std::string script = R"(
<div id="defid" style="clear:both"></div>
<script>

  function get_defid (event) {
    leave_defid (event);
    var href = event.target.href;
    href = href.substring (href.lastIndexOf ('/') + 1);
    $.ajax ({
      url: "/lexicon/definition",
      type: "GET",
      data: { id: href },
      success: function (response) {
        var element = $ ("#defid");
        element.empty ();
        element.append (response);
        $("#defid a").hover (enter_defid, leave_defid);
      }
    });
  }
  
  var timer_defid = null;
  
  function click_defid (event) {
    event.preventDefault ();
    get_defid (event);
  }
  
  function enter_defid (event) {
    leave_defid (event);
    timer_defid = setTimeout (function () {
      get_defid (event);
    }, 500);
  }

  function leave_defid (event) {
    if (timer_defid) {
      clearTimeout (timer_defid);
      timer_defid = null;
    }
  }

  $("#txtid, #defid").on ("click", click_defid);
  $("#txtid a").hover (enter_defid, leave_defid);

</script>
  )";

  script = filter::strings::replace ("defid", defid, script);
  script = filter::strings::replace ("txtid", txtid, script);
  
  return script;
}


// Clean up the Strong's number.
std::string lexicon_logic_strong_number_cleanup (std::string strong)
{
  // Remove the leading zero from a Hebrew Strong's number.
  strong = filter::strings::replace ("H0", "H", strong);
  
  return strong;
}


// Converts a parsing from the Open Scriptures Hebrew database to Strong's numbers.
// It also provides the links to call BDB entries.
void lexicon_logic_convert_morphhb_parsing_to_strong (std::string parsing,
                                                      std::vector <std::string>& strongs,
                                                      std::vector <std::string>& bdbs)
{
  strongs.clear ();
  bdbs.clear ();
  std::vector <std::string> bits = filter::strings::explode (parsing, '/');
  for (auto & bit : bits) {
    // Remove the space that is in the parsings, e.g. change "1254 a" to "1254a".
    bit = filter::strings::replace (" ", "", bit);
    bdbs.push_back (bit);
    int strong = filter::strings::convert_to_int (bit);
    if (strong == 0) {
      if (bit == "b") {
        // The Hebrew letter beth ב֖.
        strong = BETH_STRONG;
      }
      else if (bit == "c") {
        // Conjunction וְ.
        strong = CONJUNCTION_STRONG;
      }
      if (bit == "d") {
        // Definite article הַ.
        strong = DEFINITE_ARTICLE_STRONG;
      }
      else if (bit == "i") {
        // The indefinite article הַ.
        strong = INDEFINITE_ARTICLE_STRONG;
      }
      else if (bit == "k") {
        // The Hebrew letter kaf כַּ.
        strong = KAF_STRONG;
      }
      else if (bit == "l") {
        // The Hebrew letter lamed לָ.
        strong = LAMED_STRONG;
      }
      else if (bit == "m") {
        // The Hebrew letter mem מִ.
        strong = MEM_STRONG;
      }
      else if (bit == "s") {
        // The Hebrew letter shin שֶׁ.
        strong = SHIN_STRONG;
      }
    }
    strongs.push_back ("H" + std::to_string (strong));
  }
}


std::string lexicon_logic_render_strongs_definition (std::string strong)
{
  std::vector <std::string> renderings;
  Database_Strong database_strong;
  Database_HebrewLexicon database_hebrewlexicon;
  std::string definition = database_strong.definition (lexicon_logic_strong_number_cleanup (strong));
  if (definition.empty ()) {
    definition = database_hebrewlexicon.getstrong (lexicon_logic_strong_number_cleanup (strong));
  }
  definition = filter::strings::replace ("/>", "/>\n", definition);
  std::vector <std::string> lines = filter::strings::explode (definition, '\n');
  for (auto & line : lines) {
    line = filter::strings::trim (line);
    line = filter::strings::collapse_whitespace (line);
    size_t position;
    // The first <w> element describes it.
    position = line.find ("<w");
    if (position == 0) {
      // Strong's number.
      renderings.push_back ("Strong's " + strong.substr (1));
      // Part of speech.
      std::string pos = lexicon_logic_get_remove_attribute (line, "pos");
      if (!pos.empty ()) {
        pos = lexicon_logic_render_strongs_part_of_speech (pos);
        renderings.push_back (pos);
      }
      // Pronounciation.
      std::string pron = lexicon_logic_get_remove_attribute (line, "pron");
      if (!pron.empty ()) {
        renderings.push_back (pron);
      }
      // Transliteration.
      std::string xlit = lexicon_logic_get_remove_attribute (line, "xlit");
      if (!xlit.empty ()) {
        renderings.push_back (xlit);
      }
      // Original word.
      std::string word = lexicon_logic_get_text (line);
      if (!word.empty ()) {
        renderings.push_back (word);
      }
    } else {
      // Transform link to a source Strong's number.
      line = filter::strings::replace ("<w ", "<a ", line);
      line = filter::strings::replace ("src=", "href=", line);
      line = filter::strings::replace ("</w>", "</a>", line);
      // Elements referring to the source/derivation can be removed.
      line = filter::strings::replace ("<source>", "", line);
      line = filter::strings::replace ("</source>", "", line);
      line = filter::strings::replace ("<strongs_derivation>", "", line);
      line = filter::strings::replace ("</strongs_derivation>", "", line);
      // Remove elements referring to the meaning.
      line = filter::strings::replace ("<meaning>", "", line);
      line = filter::strings::replace ("</meaning>", "", line);
      line = filter::strings::replace ("<strongs_def>", "", line);
      line = filter::strings::replace ("</strongs_def>", "", line);
      // Transform markup for usage in the King James Bible.
      line = filter::strings::replace ("<usage>", "; usage in King James Bible: ", line);
      line = filter::strings::replace ("</usage>", "", line);
      line = filter::strings::replace ("<kjv_def>", "; usage in King James Bible", line);
      line = filter::strings::replace ("</kjv_def>", "", line);
      // Mark the definitions.
      line = filter::strings::replace ("<def>", "<em>", line);
      line = filter::strings::replace ("</def>", "</em>", line);
      // Clarify Strong's number.
      line = filter::strings::replace ("<strongs>", "Strong's ", line);
      line = filter::strings::replace ("</strongs>", "", line);
      // Get the <greek /> line to extract information from it.
      position = line.find ("<greek ");
      if (position != std::string::npos) {
        size_t position2 = line.find ("/>", position);
        if (position2 != std::string::npos) {
          std::string greek = line.substr (position, position2 - position + 2);
          std::string xml = greek;
          // Greek in Unicode.
          std::string unicode = lexicon_logic_get_remove_attribute (xml, "unicode");
          // Greek in transliteration.
          std::string translit = lexicon_logic_get_remove_attribute (xml, "translit");
          // Put the updated fragment back.
          line = filter::strings::replace (greek, unicode + " " + translit, line);
        }
      }
      // Get the <pronunciation /> line to extract information from it.
      position = line.find ("<pronunciation ");
      if (position != std::string::npos) {
        size_t position2 = line.find ("/>", position);
        if (position2 != std::string::npos) {
          std::string pronunciation = line.substr (position, position2 - position + 2);
          std::string xml = pronunciation;
          // Greek in strongs.
          std::string strongs = lexicon_logic_get_remove_attribute (xml, "strongs");
          // Put the updated fragment back.
          line = filter::strings::replace (pronunciation, strongs, line);
        }
      }
      // Get the <see /> line to extract information from it.
      // Do the same for the <strongsref />.
      // The difference is that the entire <see /> line is removed, and the other one converted.
      for (int i = 0; i <= 1; i++) {
        std::string tag = "see";
        if (i) tag = "strongsref";
        position = line.find ("<" + tag + " ");
        if (position != std::string::npos) {
          size_t position2 = line.find ("/>", position);
          if (position2 != std::string::npos) {
            std::string see_strongsref = line.substr (position, position2 - position + 2);
            std::string xml = see_strongsref;
            // Strong's reference.
            std::string strongs = lexicon_logic_get_remove_attribute (xml, "strongs");
            // Language.
            std::string language = lexicon_logic_get_remove_attribute (xml, "language");
            // Put the updated fragment back, with a link.
            std::string replacement;
            if (i) {
              replacement = R"(<a href=")" + language.substr (0, 1) + strongs + R"(">)" + strongs + "</a>";
            }
            line = filter::strings::replace (see_strongsref, replacement, line);
          }
        }
      }
      // Add the updated line to the rendering.
      renderings.push_back (line);
    }
  }
  std::string rendering = filter::strings::implode (renderings, " ");
  rendering = filter::strings::trim (rendering);

  // If no rendering has been found yet, try the user-defined Strong's definitions.
  if (rendering.empty ()) {
    rendering = lexicon_logic_define_user_strong (strong);
  }
  
  // Remove bits.
  rendering = filter::strings::replace ("×", "", rendering);
  
  return rendering;
}


std::string lexicon_logic_render_part_of_speech_pop_front (std::vector <std::string> & parts)
{
  std::string part;
  if (!parts.empty ()) {
    part = parts[0];
    parts.erase (parts.begin ());
  }
  return part;
}


// Render the part of speech.
std::string lexicon_logic_render_strongs_part_of_speech (std::string value)
{
  if (value == filter::strings::unicode_string_casefold (value)) {
    // Deal with Strong's parsings.
    std::vector <std::string> parts = filter::strings::explode (value, ' ');
    value.clear ();
    for (auto part : parts) {
      value.append (" ");
      if (part == "a") {
        value.append ("adjective");
      } else if (part == "a-f") {
        value.append ("adjective feminine");
      } else if (part == "a-m") {
        value.append ("adjective masculine");
      } else if (part == "adv") {
        value.append ("adverb");
      } else if (part == "conj") {
        value.append ("conjunction");
      } else if (part == "d") {
        value.append ("demonstrative");
      } else if (part == "dp") {
        value.append ("demonstrative particle");
      } else if (part == "i") {
        value.append ("interrogative");
      } else if (part == "inj") {
        value.append ("interjection");
      } else if (part == "n") {
        value.append ("noun");
      } else if (part == "n-f") {
        value.append ("noun feminine");
      } else if (part == "n-m") {
        value.append ("noun masculine");
      } else if (part == "n-pr-m") {
        value.append ("noun proper masculine");
      } else if (part == "n-pr") {
        value.append ("noun proper");
      } else if (part == "n-pr-f") {
        value.append ("noun proper feminine");
      } else if (part == "n-pr-loc") {
        value.append ("noun proper location");
      } else if (part == "a-gent") {
        value.append ("adjective gent");
      } else if (part == "np") {
        value.append ("negative particle");
      } else if (part == "p") {
        value.append ("pronoun");
      } else if (part == "prep") {
        value.append ("preposition");
      } else if (part == "pron") {
        value.append ("pron");
      } else if (part == "prt") {
        value.append ("particle");
      } else if (part == "r") {
        value.append ("relative");
      } else if (part == "v") {
        value.append ("verb");
      } else if (part == "x") {
        value.append ("unparsed");
      } else {
        value.append (part);
      }
    }
    
  } else {
    // Deal with the BDB parsings.
    std::vector <std::string> parts = filter::strings::explode (value, '-');
    value.clear ();
    std::string word = lexicon_logic_render_part_of_speech_pop_front (parts);
    value.append (" ");
    // BDB.
    if (word == "A") {
      value.append ("adjective");
    } else if (word == "C") {
      value.append ("conjunction");
    } else if (word == "D") {
      value.append ("adverb");
    } else if (word == "N") {
      value.append ("noun");
    } else if (word == "P") {
      value.append ("pronoun");
    } else if (word == "R") {
      value.append ("preposition");
    } else if (word == "S") {
      value.append ("suffix");
    } else if (word == "T") {
      value.append ("particle");
    } else if (word == "V") {
      value.append ("verb");
    }
  }

  value.append (" ");
  
  return value;
}


std::string lexicon_logic_render_strongs_part_of_speech_stem (std::string abbrev)
{
  return abbrev;
}


std::string lexicon_logic_render_strongs_part_of_speech_person (std::string abbrev)
{
  return abbrev;
}


std::string lexicon_logic_render_strongs_part_of_speech_gender (std::string abbrev)
{
  return abbrev;
}


std::string lexicon_logic_render_strongs_part_of_speech_number (std::string abbrev)
{
  return abbrev;
}


std::string lexicon_logic_render_strongs_part_of_speech_state (std::string abbrev)
{
  return abbrev;
}


// Define user-defined Strong's numbers.
std::string lexicon_logic_define_user_strong (std::string strong)
{
  std::string definition;
  if (!strong.empty ()) {
    if (strong.substr (0, 1) == "H") {
      strong.erase (0, 1);
      int number = filter::strings::convert_to_int (strong);
      if (number == BETH_STRONG) {
        definition = "particle preposition ב: in, at, by, with, among";
      }
      else if (number == CONJUNCTION_STRONG) {
        definition = "particle conjunction ו: and, so, then, when, now, or, but, that";
      }
      else if (number == DEFINITE_ARTICLE_STRONG) {
        definition = "particle definite article ה: the";
      }
      else if (number == INDEFINITE_ARTICLE_STRONG) {
        definition = "particle indefinite article ה: a";
      }
      else if (number == KAF_STRONG) {
        definition = "particle preposition כ: like, as, at, according to, after, when, if";
      }
      else if (number == LAMED_STRONG) {
        definition = "particle preposition ל: to, for, towards, belonging to, in regard to, according to, in";
      }
      else if (number == MEM_STRONG) {
        definition = "particle preposition מ: from, out of, by, by reason of, at, because of, more than";
      }
      else if (number == SHIN_STRONG) {
        definition = "particle relative ש: that";
      }
    }
  }
  return definition;
}


std::string lexicon_logic_render_morphgnt_part_of_speech (std::string pos)
{
  pos = filter::strings::replace ("-", "", pos);
  std::string rendering;
  if (pos == "N") rendering = "noun";
  if (pos == "V") rendering = "verb";
  if (pos == "RA") rendering = "definite article";
  if (pos == "C") rendering = "conjunction";
  if (pos == "RP") rendering = "personal pronoun";
  if (pos == "P") rendering = "preposition";
  if (pos == "RR") rendering = "relative pronoun";
  if (pos == "A") rendering = "adjective";
  if (pos == "D") rendering = "adverb";
  if (pos == "RD") rendering = "demonstrative pronoun";
  if (pos == "X") rendering = "interjection";
  if (pos == "RI") rendering = "indefinite adjective";
  if (pos == "I") rendering = "interjection";
  return rendering;
}


std::string lexicon_logic_render_morphgnt_parsing_code (std::string parsing)
{
  std::vector <std::string> renderings;
  // person.
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "1") renderings.push_back ("first person");
    if (p == "2") renderings.push_back ("second person");
    if (p == "3") renderings.push_back ("third person");
  }
  // tense
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "A") renderings.push_back ("aorist");
    if (p == "F") renderings.push_back ("future");
    if (p == "I") renderings.push_back ("imperfect");
    if (p == "P") renderings.push_back ("present");
    if (p == "X") renderings.push_back ("perfect");
    if (p == "Y") renderings.push_back ("pluperfect");
  }
  // voice
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "A") renderings.push_back ("active");
    if (p == "M") renderings.push_back ("middle");
    if (p == "P") renderings.push_back ("passive");
  }
  // mood
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "D") renderings.push_back ("imperative");
    if (p == "I") renderings.push_back ("indicative");
    if (p == "N") renderings.push_back ("infinitive");
    if (p == "O") renderings.push_back ("optative");
    if (p == "P") renderings.push_back ("participle");
    if (p == "S") renderings.push_back ("subjunctive");
  }
  // case
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "A") renderings.push_back ("accusative");
    if (p == "D") renderings.push_back ("dative");
    if (p == "G") renderings.push_back ("genetive");
    if (p == "N") renderings.push_back ("nominative");
    if (p == "V") renderings.push_back ("vocative");
  }
  // number
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "P") renderings.push_back ("plural");
    if (p == "S") renderings.push_back ("singular");
  }
  // gender
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "F") renderings.push_back ("female");
    if (p == "M") renderings.push_back ("male");
    if (p == "N") renderings.push_back ("neuter");
  }
  // degree
  if (!parsing.empty ()) {
    std::string p = parsing.substr (0, 1);
    parsing = parsing.substr (1);
    if (p == "C") renderings.push_back ("comparative");
    if (p == "S") renderings.push_back ("superlative");
  }
  return filter::strings::implode (renderings, " ");
}


std::string lexicon_logic_render_etcbc4_morphology (std::string rowid)
{
  // The order of the rendered morphological information is such
  // that the pieces of information most relevant to the Bible translator come first,
  // and the remaining bits come after.
  
  std::vector <std::string> renderings;
  int row = filter::strings::convert_to_int (rowid.substr (1));

  std::string pos = database::etcbc4::pos (row);
  if (pos == "art") pos = "article";
  if (pos == "verb") pos = "verb";
  if (pos == "subs") pos = "noun";
  if (pos == "nmpr") pos = "proper noun";
  if (pos == "advb") pos = "adverb";
  if (pos == "prep") pos = "preposition";
  if (pos == "conj") pos = "conjunction";
  if (pos == "prps") pos = "personal pronoun";
  if (pos == "prde") pos = "demonstrative pronoun";
  if (pos == "prin") pos = "interrogative pronoun";
  if (pos == "intj") pos = "interjection";
  if (pos == "nega") pos = "negative particle";
  if (pos == "inrg") pos = "interrogative particle";
  if (pos == "adjv") pos = "adjective";
  //renderings.push_back (";");
  //renderings.push_back ("part of speech:");
  renderings.push_back (pos);
  
  std::string lexical_set = database::etcbc4::subpos (row);
  if (lexical_set == "nmdi") lexical_set = "distributive noun";
  if (lexical_set == "nmcp") lexical_set = "copulative noun";
  if (lexical_set == "padv") lexical_set = "potential adverb";
  if (lexical_set == "afad") lexical_set = "anaphoric adverb";
  if (lexical_set == "ppre") lexical_set = "potential preposition";
  if (lexical_set == "cjad") lexical_set = "conjunctive adverb";
  if (lexical_set == "ordn") lexical_set = "ordinal";
  if (lexical_set == "vbcp") lexical_set = "copulative verb";
  if (lexical_set == "mult") lexical_set = "noun of multitude";
  if (lexical_set == "focp") lexical_set = "focus particle";
  if (lexical_set == "ques") lexical_set = "interrogative particle";
  if (lexical_set == "gntl") lexical_set = "gentilic";
  if (lexical_set == "quot") lexical_set = "quotation verb";
  if (lexical_set == "card") lexical_set = "cardinal";
  if (lexical_set == "none") lexical_set = "";
  if (!lexical_set.empty ()) {
    // renderings.push_back (";");
    // renderings.push_back ("lexical set:");
    renderings.push_back ("(" + lexical_set + ")");
  }

  std::string stem = database::etcbc4::stem (row);
  if (stem == "hif") stem = "hif‘il";
  if (stem == "hit") stem = "hitpa“el";
  if (stem == "hof") stem = "hof‘al";
  if (stem == "nif") stem = "nif‘al";
  if (stem == "piel") stem = "pi“el";
  if (stem == "pual") stem = "pu“al";
  if (stem == "qal") stem = "qal";
  if (stem == "afel") stem = "af‘el";
  if (stem == "etpa") stem = "etpa“al";
  if (stem == "etpe") stem = "etpe‘el";
  if (stem == "haf") stem = "haf‘el";
  if (stem == "hop") stem = "hotpa“al";
  if (stem == "hsht") stem = "hishtaf‘al";
  if (stem == "htpa") stem = "hitpa“al";
  if (stem == "htpe") stem = "hitpe‘el";
  if (stem == "nit") stem = "nitpa“el";
  if (stem == "pael") stem = "pa“el";
  if (stem == "peal") stem = "pe‘al";
  if (stem == "peil") stem = "pe‘il";
  if (stem == "shaf") stem = "shaf‘el";
  if (stem == "tif") stem = "tif‘al";
  if (stem == "pasq") stem = "passiveqal";
  if (stem == "NA") stem.clear ();
  if (!stem.empty ()) {
    //renderings.push_back (";");
    //renderings.push_back ("stem:");
    renderings.push_back (stem);
  }
  
  std::string tense = database::etcbc4::tense (row);
  if (tense == "perf") tense = "perfect";
  if (tense == "impf") tense = "imperfect";
  if (tense == "wayq") tense = "wayyiqtol";
  if (tense == "impv") tense = "imperative";
  if (tense == "infa") tense = "infinitive (absolute)";
  if (tense == "infc") tense = "infinitive (construct)";
  if (tense == "ptca") tense = "participle";
  if (tense == "ptcp") tense = "participle (passive)";
  if (tense == "NA") tense.clear ();
  if (!tense.empty ()) {
    //renderings.push_back (";");
    //renderings.push_back ("tense:");
    renderings.push_back (tense);
  }
  
  std::string person = database::etcbc4::person (row);
  if (person == "p1") person = "first person";
  if (person == "p2") person = "second person";
  if (person == "p3") person = "third person";
  if (person == "NA") person.clear ();
  if (person == "unknown") person = "unknown person";
  if (!person.empty ()) {
    //renderings.push_back (";");
    //renderings.push_back ("person:");
    renderings.push_back (person);
  }
  
  std::string gender = database::etcbc4::gender (row);
  if (gender == "m") gender = "masculine";
  if (gender == "f") gender = "feminine";
  if (gender == "NA") gender.clear ();
  if (gender == "unknown") gender = "unknown gender";
  if (!gender.empty ()) {
    // renderings.push_back (";");
    // renderings.push_back ("gender:");
    renderings.push_back (gender);
  }
  
  std::string number = database::etcbc4::number (row);
  if (number == "sg") number = "singular";
  if (number == "du") number = "dual";
  if (number == "pl") number = "plural";
  if (number == "NA") number.clear ();
  if (number == "unknown") number = "unknown number";
  if (!number.empty ()) {
    // renderings.push_back (";");
    // renderings.push_back ("number:");
    renderings.push_back (number);
  }
  
  std::string state = database::etcbc4::state (row);
  if (state == "a") state = "absolute";
  if (state == "c") state = "construct";
  if (state == "e") state = "emphatic";
  if (state == "NA") state.clear ();
  if (!state.empty ()) {
    // renderings.push_back (";");
    // renderings.push_back ("state:");
    renderings.push_back (state);
  }
  
  std::string gloss = database::etcbc4::gloss (row);
  //renderings.push_back (";");
  //renderings.push_back ("gloss:");
  renderings.push_back ("-");
  renderings.push_back (filter::strings::escape_special_xml_characters (gloss));
  
  renderings.push_back ("<br>");
  
  std::string word = database::etcbc4::word (row);
  renderings.push_back ("word:");
  renderings.push_back (word);

  std::string vocalized_lexeme = database::etcbc4::vocalized_lexeme (row);
  renderings.push_back (";");
  renderings.push_back ("vocalized lexeme:");
  renderings.push_back (vocalized_lexeme);

  std::string consonantal_lexeme = database::etcbc4::consonantal_lexeme (row);
  renderings.push_back (";");
  renderings.push_back ("consonantal lexeme:");
  renderings.push_back (consonantal_lexeme);

  std::string phrase_function = database::etcbc4::phrase_function (row);
  if (phrase_function == "Adju") phrase_function = "adjunct";
  if (phrase_function == "Cmpl") phrase_function = "complement";
  if (phrase_function == "Conj") phrase_function = "conjunction";
  if (phrase_function == "EPPr") phrase_function = "enclitic personal pronoun";
  if (phrase_function == "ExsS") phrase_function = "existence with subject suffix";
  if (phrase_function == "Exst") phrase_function = "existence";
  if (phrase_function == "Frnt") phrase_function = "fronted element";
  if (phrase_function == "Intj") phrase_function = "interjection";
  if (phrase_function == "IntS") phrase_function = "interjection with subject suffix";
  if (phrase_function == "Loca") phrase_function = "locative";
  if (phrase_function == "Modi") phrase_function = "modifier";
  if (phrase_function == "ModS") phrase_function = "modifier with subject suffix";
  if (phrase_function == "NCop") phrase_function = "negative copula";
  if (phrase_function == "NCoS") phrase_function = "negative copula with subject suffix";
  if (phrase_function == "Nega") phrase_function = "negation";
  if (phrase_function == "Objc") phrase_function = "object";
  if (phrase_function == "PrAd") phrase_function = "predicative adjunct";
  if (phrase_function == "PrcS") phrase_function = "predicate complement with subject suffix";
  if (phrase_function == "PreC") phrase_function = "predicate complement";
  if (phrase_function == "Pred") phrase_function = "predicate";
  if (phrase_function == "PreO") phrase_function = "predicate with object suffix";
  if (phrase_function == "PreS") phrase_function = "predicate with subject suffix";
  if (phrase_function == "PtcO") phrase_function = "participle with object suffix";
  if (phrase_function == "Ques") phrase_function = "question";
  if (phrase_function == "Rela") phrase_function = "relative";
  if (phrase_function == "Subj") phrase_function = "subject";
  if (phrase_function == "Supp") phrase_function = "supplementary constituent";
  if (phrase_function == "Time") phrase_function = "time reference";
  if (phrase_function == "Unkn") phrase_function.clear (); // "unknown";
  if (phrase_function == "Voct") phrase_function = "vocative";
  if (!phrase_function.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("phrase function:");
    renderings.push_back (phrase_function);
  }

  std::string phrase_type = database::etcbc4::phrase_type (row);
  if (phrase_type == "VP") phrase_type = "verbal phrase";
  if (phrase_type == "NP") phrase_type = "nominal phrase";
  if (phrase_type == "PrNP") phrase_type = "proper-noun phrase";
  if (phrase_type == "AdvP") phrase_type = "adverbial phrase";
  if (phrase_type == "PP") phrase_type = "prepositional phrase";
  if (phrase_type == "CP") phrase_type = "conjunctive phrase";
  if (phrase_type == "PPrP") phrase_type = "personal pronoun phrase";
  if (phrase_type == "DPrP") phrase_type = "demonstrative pronoun phrase";
  if (phrase_type == "IPrP") phrase_type = "interrogative pronoun phrase";
  if (phrase_type == "InjP") phrase_type = "interjectional phrase";
  if (phrase_type == "NegP") phrase_type = "negative phrase";
  if (phrase_type == "InrP") phrase_type = "interrogative phrase";
  if (phrase_type == "AdjP") phrase_type = "adjective phrase";
  renderings.push_back (";");
  renderings.push_back ("phrase type:");
  renderings.push_back (phrase_type);

  std::string phrase_relation = database::etcbc4::phrase_relation (row);
  if (phrase_relation == "PrAd") phrase_relation = "predicative adjunct";
  if (phrase_relation == "Resu") phrase_relation = "resumption";
  if (phrase_relation == "NA") phrase_relation.clear ();
  if (!phrase_relation.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("phrase relation:");
    renderings.push_back (phrase_relation);
  }
  
  std::string phrase_a_relation = database::etcbc4::phrase_a_relation (row);
  if (phrase_a_relation == "Appo") phrase_a_relation = "apposition";
  if (phrase_a_relation == "Sfxs") phrase_a_relation = "suffix specification";
  if (phrase_a_relation == "Link") phrase_a_relation = "conjunction";
  if (phrase_a_relation == "Spec") phrase_a_relation = "specification";
  if (phrase_a_relation == "Para") phrase_a_relation = "parallel";
  if (phrase_a_relation == "NA") phrase_a_relation.clear ();
  if (!phrase_a_relation.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("phrase atom relation:");
    renderings.push_back (phrase_a_relation);
  }

  std::string clause_text_type = database::etcbc4::clause_text_type (row);
  std::string rendering;
  while (!clause_text_type.empty ()) {
    std::string type = clause_text_type.substr (clause_text_type.length () - 1);
    clause_text_type.erase (clause_text_type.length () - 1, 1);
    if (type == "?") type.clear ();
    if (type == "N") type = "narrative";
    if (type == "D") type = "discursive";
    if (type == "Q") type = "quotation";
    if (!type.empty ()) {
      if (!rendering.empty ()) rendering.append (" in a ");
      rendering.append (type);
    }
  }
  if (!rendering.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("text type:");
    renderings.push_back (rendering);
  }

  std::string clause_type = database::etcbc4::clause_type (row);
  if (clause_type == "AjCl") clause_type = "adjective clause";
  if (clause_type == "CPen") clause_type = "casus pendens";
  if (clause_type == "Defc") clause_type = "defective clause atom";
  if (clause_type == "Ellp") clause_type = "ellipsis";
  if (clause_type == "InfA") clause_type = "infinitive absolute clause";
  if (clause_type == "InfC") clause_type = "infinitive construct clause";
  if (clause_type == "MSyn") clause_type = "macrosyntactic sign";
  if (clause_type == "NmCl") clause_type = "nominal clause";
  if (clause_type == "Ptcp") clause_type = "participle clause";
  if (clause_type == "Reop") clause_type = "reopening";
  if (clause_type == "Unkn") clause_type.clear (); // Unknown";
  if (clause_type == "Voct") clause_type = "vocative clause";
  if (clause_type == "Way0") clause_type = "wayyiqtol-null clause";
  if (clause_type == "WayX") clause_type = "wayyiqtol-X clause";
  if (clause_type == "WIm0") clause_type = "we-imperative-null clause";
  if (clause_type == "WImX") clause_type = "we-imperative-X clause";
  if (clause_type == "WQt0") clause_type = "we-qatal-null clause";
  if (clause_type == "WQtX") clause_type = "we-qatal-X clause";
  if (clause_type == "WxI0") clause_type = "we-x-imperative-null clause";
  if (clause_type == "WXIm") clause_type = "we-X-imperative clause";
  if (clause_type == "WxIX") clause_type = "we-x-imperative-X clause";
  if (clause_type == "WxQ0") clause_type = "we-x-qatal-null clause";
  if (clause_type == "WXQt") clause_type = "we-X-qatal clause";
  if (clause_type == "WxQX") clause_type = "we-x-qatal-X clause";
  if (clause_type == "WxY0") clause_type = "we-x-yiqtol-null clause";
  if (clause_type == "WXYq") clause_type = "we-X-yiqtol clause";
  if (clause_type == "WxYX") clause_type = "we-x-yiqtol-X clause";
  if (clause_type == "WYq0") clause_type = "we-yiqtol-null clause";
  if (clause_type == "WYqX") clause_type = "we-yiqtol-X clause";
  if (clause_type == "xIm0") clause_type = "x-imperative-null clause";
  if (clause_type == "XImp") clause_type = "X-imperative clause";
  if (clause_type == "xImX") clause_type = "x-imperative-X clause";
  if (clause_type == "XPos") clause_type = "extraposition";
  if (clause_type == "xQt0") clause_type = "x-qatal-null clause";
  if (clause_type == "XQtl") clause_type = "X-qatal clause";
  if (clause_type == "xQtX") clause_type = "x-qatal-X clause";
  if (clause_type == "xYq0") clause_type = "x-yiqtol-null clause";
  if (clause_type == "XYqt") clause_type = "X-yiqtol clause";
  if (clause_type == "xYqX") clause_type = "x-yiqtol-X clause";
  if (clause_type == "ZIm0") clause_type = "zero-imperative-null clause";
  if (clause_type == "ZImX") clause_type = "zero-imperative-X clause";
  if (clause_type == "ZQt0") clause_type = "zero-qatal-null clause";
  if (clause_type == "ZQtX") clause_type = "zero-qatal-X clause";
  if (clause_type == "ZYq0") clause_type = "zero-yiqtol-null clause";
  if (clause_type == "ZYqX") clause_type = "zero-yiqtol-X clause";
  if (!clause_type.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("clause type:");
    renderings.push_back (clause_type);
  }

  std::string clause_relation = database::etcbc4::clause_relation (row);
  if (clause_relation == "Adju") clause_relation = "adjunctive clause";
  if (clause_relation == "Attr") clause_relation = "attributive clause";
  if (clause_relation == "Cmpl") clause_relation = "complement clause";
  if (clause_relation == "Coor") clause_relation = "coordinated clause";
  if (clause_relation == "Objc") clause_relation = "object clause";
  if (clause_relation == "PrAd") clause_relation = "predicative adjunct clause";
  if (clause_relation == "PreC") clause_relation = "predicative complement clause";
  if (clause_relation == "ReVo") clause_relation = "referral to the vocative";
  if (clause_relation == "ReSu") clause_relation = "resumptive clause";
  if (clause_relation == "RgRc") clause_relation = "regens/rectum connection";
  if (clause_relation == "Spec") clause_relation = "specification clause";
  if (clause_relation == "Subj") clause_relation = "subject clause";
  if (clause_relation == "NA") clause_relation.clear ();
  if (!clause_relation.empty ()) {
    renderings.push_back (";");
    renderings.push_back ("clause relation:");
    renderings.push_back (clause_relation);
  }

  return filter::strings::implode (renderings, " ");
}


// Converts a code from MorphHb into a rendered BDB entry from the HebrewLexicon.
std::string lexicon_logic_render_bdb_entry (std::string code)
{
  Database_HebrewLexicon database_hebrewlexicon;
  // Get the intermediate map value between the augmented Strong's numbers and the BDB lexicon.
  std::string map = database_hebrewlexicon.getaug (code);
  // Get the BDB entry ID.
  std::string bdb = database_hebrewlexicon.getmap (map);
  // Get the BDB definition.
  std::string definition = database_hebrewlexicon.getbdb (bdb);
  // Remove XML elements.
  filter::strings::replace_between (definition, "<", ">", "");
  // Convert new lines to <br> to retain some formatting.
  definition = filter::strings::replace ("\n\n", "\n", definition);
  definition = filter::strings::replace ("\n\n", "\n", definition);
  definition = filter::strings::replace ("\n", "<br>", definition);
  // Done.
  return definition;
}


// Gets and removes an attribute from $xml, and updates $xml.
// Returns the attribute.
std::string lexicon_logic_get_remove_attribute (std::string & xml, const char * key)
{
  std::string value;
  pugi::xml_document document;
  pugi::xml_parse_result result = document.load_string (xml.c_str(), pugi::parse_ws_pcdata_single);
  if (result) {
    pugi::xml_node child = document.first_child ();
    pugi::xml_attribute attribute = child.attribute (key);
    if (attribute) {
      value = attribute.value ();
      child.remove_attribute (attribute);
      std::stringstream output;
      child.print (output, "", pugi::format_raw);
      xml = output.str ();
    }
  }
  return value;
}


// Gets the text contents of the $xml node.
std::string lexicon_logic_get_text (std::string & xml)
{
  std::string value;
  pugi::xml_document document;
  pugi::xml_parse_result result = document.load_string (xml.c_str(), pugi::parse_ws_pcdata_single);
  if (result) {
    pugi::xml_node child = document.first_child ();
    pugi::xml_text text = child.text ();
    if (text) {
      value = text.get ();
      text.set ("");
      std::stringstream output;
      child.print (output, "", pugi::format_raw);
      xml = output.str ();
    }
  }
  return value;
}


std::string lexicon_logic_hebrew_morphology_render (std::string value)
{
  // No data: bail out.
  if (value.empty ()) return value;

  // A morphology item in the Open Scriptures Hebrew Bible starts with a language code.
  // One language code is prefixed to the entire morphological parsing string, including prefixes, main word and suffixes.
  std::string language = value.substr (0, 1);
  bool hebrew = (language == "H");
  bool aramaic = (language == "A");
  // At times the parser may have ommitted the language code. Take Hebrew in that case.
  if (hebrew || aramaic) value.erase (0, 1);
  if (!hebrew && !aramaic) hebrew = true;
 
  // Description of the Hebrew Morphology Codes:
  // http://openscriptures.github.io/morphhb/parsing/HebrewMorphologyCodes.html
  
  std::vector <std::string> renderings;
  
  // A slash separates morphology items.
  std::vector <std::string> values = filter::strings::explode (value, '/');
  for (auto value2 : values) {

    if (value2.empty ()) continue;
    
    if (!renderings.empty ()) renderings.push_back (" + ");
    
    // Part of Speech
    std::string part_of_speech = value2.substr (0, 1);
    value2.erase (0, 1);
    if (part_of_speech == "A") {
      renderings.push_back ("adjective");
      // type gender number state
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_adjective (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_gender (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_number (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_state (value2));
    }
    else if (part_of_speech == "C") {
      renderings.push_back ("conjunction");
    }
    else if (part_of_speech == "D") {
      renderings.push_back ("adverb");
    }
    else if (part_of_speech == "N") {
      renderings.push_back ("noun");
      // type gender number state
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_noun (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_gender (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_number (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_state (value2));
    }
    else if (part_of_speech == "P") {
      renderings.push_back ("pronoun");
      // type person gender number
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_pronoun (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_person (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_gender (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_number (value2));
    }
    else if (part_of_speech == "R") {
      renderings.push_back ("preposition");
      // type
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_preposition (value2));
    }
    else if (part_of_speech == "S") {
      renderings.push_back ("suffix");
      // type person gender number
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_suffix (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_person (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_gender (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_number (value2));
    }
    else if (part_of_speech == "T") {
      renderings.push_back ("particle");
      // type
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_particle (value2));
    }
    else if (part_of_speech == "V") {
      renderings.push_back ("verb");
      // stem type person gender number state
      renderings.push_back (lexicon_logic_hebrew_morphology_render_stem (hebrew, aramaic, value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_type_verb_conjugation (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_person (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_gender (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_number (value2));
      renderings.push_back (lexicon_logic_hebrew_morphology_render_state (value2));
    }
    else {
      renderings.push_back ("unknown");
    }
    
  }
  
  return filter::strings::implode (renderings, " ");
}


// Verb conjugation types
std::string lexicon_logic_hebrew_morphology_render_type_verb_conjugation (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "p") rendering = "perfect (qatal)";
    else if (code == "q") rendering = "sequential perfect (weqatal)";
    else if (code == "i") rendering = "imperfect (yiqtol)";
    else if (code == "w") rendering = "sequential imperfect (wayyiqtol)";
    else if (code == "h") rendering = "cohortative";
    else if (code == "j") rendering = "jussive";
    else if (code == "v") rendering = "imperative";
    else if (code == "r") rendering = "participle active";
    else if (code == "s") rendering = "participle passive";
    else if (code == "a") rendering = "infinitive absolute";
    else if (code == "c") rendering = "infinitive construct";
    else                  rendering = code;
  }
  return rendering;
}


// Adjective types
std::string lexicon_logic_hebrew_morphology_render_type_adjective (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "a") rendering = "adjective";
    else if (code == "c") rendering = "cardinal number";
    else if (code == "g") rendering = "gentilic";
    else if (code == "o") rendering = "ordinal number";
    else                  rendering = code;
  }
  return rendering;
}


// Noun types
std::string lexicon_logic_hebrew_morphology_render_type_noun (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "c") rendering = "common";
    else if (code == "g") rendering = "gentilic";
    else if (code == "p") rendering = "proper name";
    else                  rendering = code;
  }
  return rendering;
}


// Pronoun types
std::string lexicon_logic_hebrew_morphology_render_type_pronoun (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "d") rendering = "demonstrative";
    else if (code == "f") rendering = "indefinite";
    else if (code == "i") rendering = "interrogative";
    else if (code == "p") rendering = "personal";
    else if (code == "r") rendering = "relative";
    else                  rendering = code;
  }
  return rendering;
}


// Preposition types
std::string lexicon_logic_hebrew_morphology_render_type_preposition (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if (code == "d") rendering = "definite article";
    else             rendering = code;
  }
  return rendering;
}


// Suffix types
std::string lexicon_logic_hebrew_morphology_render_type_suffix (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "d") rendering = "directional he";
    else if (code == "h") rendering = "paragogic he";
    else if (code == "n") rendering = "paragogic nun";
    else if (code == "p") rendering = "pronominal";
    else                  rendering = code;
  }
  return rendering;
}


// Particle types
std::string lexicon_logic_hebrew_morphology_render_type_particle (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "a") rendering = "affirmation";
    else if (code == "d") rendering = "definite article";
    else if (code == "e") rendering = "exhortation";
    else if (code == "i") rendering = "interrogative";
    else if (code == "j") rendering = "interjection";
    else if (code == "m") rendering = "demonstrative";
    else if (code == "n") rendering = "negative";
    else if (code == "o") rendering = "direct object marker";
    else if (code == "r") rendering = "relative";
    else                  rendering = code;
  }
  return rendering;
}


// Render verb stems.
std::string lexicon_logic_hebrew_morphology_render_stem (bool hebrew, bool aramaic, std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if (hebrew) {
      // Verb stems (Hebrew)
      if      (code == "q") rendering = "qal";
      else if (code == "N") rendering = "niphal";
      else if (code == "p") rendering = "piel";
      else if (code == "P") rendering = "pual";
      else if (code == "h") rendering = "hiphil";
      else if (code == "H") rendering = "hophal";
      else if (code == "t") rendering = "hithpael";
      else if (code == "o") rendering = "polel";
      else if (code == "O") rendering = "polal";
      else if (code == "r") rendering = "hithpolel";
      else if (code == "m") rendering = "poel";
      else if (code == "M") rendering = "poal";
      else if (code == "k") rendering = "palel";
      else if (code == "K") rendering = "pulal";
      else if (code == "Q") rendering = "qal passive";
      else if (code == "l") rendering = "pilpel";
      else if (code == "L") rendering = "polpal";
      else if (code == "f") rendering = "hithpalpel";
      else if (code == "D") rendering = "nithpael";
      else if (code == "j") rendering = "pealal";
      else if (code == "i") rendering = "pilel";
      else if (code == "u") rendering = "hothpaal";
      else if (code == "c") rendering = "tiphil";
      else if (code == "v") rendering = "hishtaphel";
      else if (code == "w") rendering = "nithpalel";
      else if (code == "y") rendering = "nithpoel";
      else if (code == "z") rendering = "hithpoel";
      else                  rendering = code;
    }
    else if (aramaic) {
      // Verb stems (Aramaic)
      if      (code == "q") rendering = "peal";
      else if (code == "Q") rendering = "peil";
      else if (code == "u") rendering = "hithpeel";
      else if (code == "p") rendering = "pael";
      else if (code == "P") rendering = "ithpaal";
      else if (code == "M") rendering = "hithpaal";
      else if (code == "a") rendering = "aphel";
      else if (code == "h") rendering = "haphel";
      else if (code == "s") rendering = "saphel";
      else if (code == "e") rendering = "shaphel";
      else if (code == "H") rendering = "hophal";
      else if (code == "i") rendering = "ithpeel";
      else if (code == "t") rendering = "hishtaphel";
      else if (code == "v") rendering = "ishtaphel";
      else if (code == "w") rendering = "hithaphel";
      else if (code == "o") rendering = "polel";
      else if (code == "z") rendering = "ithpoel";
      else if (code == "r") rendering = "hithpolel";
      else if (code == "f") rendering = "hithpalpel";
      else if (code == "b") rendering = "hephal";
      else if (code == "c") rendering = "tiphel";
      else if (code == "m") rendering = "poel";
      else if (code == "l") rendering = "palpel";
      else if (code == "L") rendering = "ithpalpel";
      else if (code == "O") rendering = "ithpolel";
      else if (code == "G") rendering = "ittaphal";
      else                  rendering = code;
    }
    else {
      rendering = code;
    }
  }
  return rendering;
}


// Person.
std::string lexicon_logic_hebrew_morphology_render_person (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "1") rendering = "first person";
    else if (code == "2") rendering = "second person";
    else if (code == "3") rendering = "third person";
    else if (code == "x") ;
    // No rendering found, probable error in morphology, restore the code to not interrupt the subsequent codes.
    else                  value.insert (0, code);
  }
  return rendering;
}


// Gender.
std::string lexicon_logic_hebrew_morphology_render_gender (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "b") rendering = "both (noun)";
    else if (code == "c") rendering = "common (verb)";
    else if (code == "f") rendering = "feminine";
    else if (code == "m") rendering = "masculine";
    else                  rendering = code;
  }
  return rendering;
}


// Number.
std::string lexicon_logic_hebrew_morphology_render_number (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "d") rendering = "dual";
    else if (code == "p") rendering = "plural";
    else if (code == "s") rendering = "singular";
    else                  rendering = code;
  }
  return rendering;
}


// State.
std::string lexicon_logic_hebrew_morphology_render_state (std::string & value)
{
  std::string rendering;
  if (!value.empty ()) {
    std::string code = value.substr (0, 1);
    value.erase (0, 1);
    if      (code == "a") rendering = "absolute";
    else if (code == "c") rendering = "construct";
    else if (code == "d") rendering = "determined";
    else                  rendering = code;
  }
  return rendering;
}


struct abbott_smith_walker: pugi::xml_tree_walker
{
  std::string text {};

  bool text_element_already_handled {false};
  std::string previous_element_name {};

  virtual bool for_each (pugi::xml_node& node) override
  {
    // Details of the current node.
    std::string clas = node.attribute ("class").value ();
    std::string name = node.name ();

    // Handle occurrences count in the New Testament.
    if (name == "note") {
      std::string type = node.attribute("type").value();
      if (type == "occurrencesNT") {
        text.append ("occurs ");
        text.append (node.text().get());
        text.append (" times in the New Testament, ");
        text_element_already_handled = true;
        return true;
      }
    }

    // Handle sense indicators.
    if (name == "sense") {
      if (previous_element_name != "sense") text.append (" sense ");
      std::string n = node.attribute("n").value();
      if (!n.empty()) {
        text.append (n);
        text.append (" ");
      }
    }
    
    // Include node's text content.
    if (name.empty()) {
      if (!text_element_already_handled) {
        text.append (node.value());
      }
      text_element_already_handled = false;
    }

    previous_element_name = name;
    
    // Continue parsing.
    return true;
  }
};


std::string lexicon_logic_render_abbott_smiths_definition (std::string lemma, std::string strong)
{
  std::vector <std::string> renderings;

  const std::string definition = database::abboth_smith::get (lemma, lexicon_logic_strong_number_cleanup (strong));
  
  pugi::xml_document document;
  document.load_string (definition.c_str());
  abbott_smith_walker tree_walker {};
  document.traverse (tree_walker);
  renderings.push_back (tree_walker.text);

  std::string rendering = filter::strings::implode (renderings, " ");
  rendering = filter::strings::trim (rendering);

  // If any rendering is given, then prefix the name of the lexicon.
  if (!rendering.empty ()) {
    rendering.insert(0, "Abbott Smith; ");
  }
   
  // Done.
  return rendering;
}

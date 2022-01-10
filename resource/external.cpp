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


#include <resource/external.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include "assets/view.h"
#include "resource/logic.h"
#include <jsonxx/jsonxx.h>
#include <pugixml/pugixml.hpp>


using namespace jsonxx;
using namespace pugi;


// Local forward declarations:


unsigned int resource_external_count ();
int gbs_digitaal_json_callback (void *userdata, int type, const char *data, uint32_t length);
string bibleserver_processor (string directory, int book, int chapter, int verse);
string resource_external_get_statenbijbel_gbs (int book, int chapter, int verse);
string resource_external_get_statenbijbel_plus_gbs (int book, int chapter, int verse);
string resource_external_get_king_james_version_gbs (int book, int chapter, int verse);
string resource_external_get_king_james_version_plus_gbs (int book, int chapter, int verse);
string resource_external_get_biblehub_interlinear (int book, int chapter, int verse);
string resource_external_get_biblehub_scrivener (int book, int chapter, int verse);
string resource_external_get_biblehub_westminster (int book, int chapter, int verse);
string resource_external_get_net_bible (int book, int chapter, int verse);
string resource_external_get_blue_letter_bible (int book, int chapter, int verse);
string resource_external_get_elberfelder_bibel (int book, int chapter, int verse);
string resource_external_convert_book_biblehub (int book);
string resource_external_convert_book_netbible (int book);
string resource_external_convert_book_blueletterbible (int book);
string resource_external_convert_book_bibleserver (int book);


typedef struct
{
  string name;
  const char *versification;
  const char *mapping;
  const char *type;
  string (* func) (int, int, int);
} resource_record;


#define UNKNOWN "u"
#define ORIGINAL "o"
#define BIBLE "b"


// Implementation:


resource_record resource_table [] =
{
  { "Statenbijbel GBS", "Dutch Traditional", "Dutch Traditional", BIBLE, & resource_external_get_statenbijbel_gbs },
  { "Statenbijbel Plus GBS", "Dutch Traditional", "Dutch Traditional", BIBLE, & resource_external_get_statenbijbel_plus_gbs },
  { "King James Version GBS", english (), english (), BIBLE, & resource_external_get_king_james_version_gbs },
  { "King James Version Plus GBS", english (), english (), BIBLE, & resource_external_get_king_james_version_plus_gbs },
  { resource_external_biblehub_interlinear_name (), english (), english (), ORIGINAL, & resource_external_get_biblehub_interlinear },
  { "Scrivener Greek", english (), english (), ORIGINAL, & resource_external_get_biblehub_scrivener },
  { "Westminster Hebrew", english (), english (), ORIGINAL, & resource_external_get_biblehub_westminster },
  { resource_external_net_bible_name (), english (), english (), BIBLE, & resource_external_get_net_bible },
  { "Blue Letter Bible", english (), english (), ORIGINAL, & resource_external_get_blue_letter_bible },
  { "Elberfelder Bibel", english (), english (), BIBLE, & resource_external_get_elberfelder_bibel },
  { resource_logic_easy_english_bible_name (), english (), english (), BIBLE, & resource_logic_easy_english_bible_get },
};


struct gbs_basic_walker: xml_tree_walker
{
  vector <string> texts;
  bool canonical_text = true;

  virtual bool for_each (xml_node& node)
  {
    xml_node_type nodetype = node.type();
    if (nodetype == node_pcdata) {
      // Handle plain character data.
      string text = node.text().get();
      if (canonical_text) texts.push_back(text);
      // If the text above was a footnote caller,
      // in the GBS layout the following text could be canonical again.
      canonical_text = true;
    } else {
      // Handle the node itself.
      string nodename = node.name ();
      string classname = node.attribute ("class").value ();
      canonical_text = true;
      // Bits to exclude from the canonical text.
      if (classname == "verse-number") canonical_text = false;
      if (classname == "kanttekening") canonical_text = false;
      if (classname == "verwijzing") canonical_text = false;
      if (classname == "sup") canonical_text = false;
      // End of parsing this verse.
      if (classname == "verse-references") return false;
    }

    // Continue parsing.
    return true;
  }
};


// This function displays the canonical text from bijbel-statenvertaling.com.
string gbs_basic_processor (string url, int verse)
{
  string text;
  
  // Get the html from the server.
  string html = resource_logic_web_or_cache_get (url, text);

  // Tidy the html so it can be loaded as xml.
  html = filter_string_tidy_invalid_html (html);

  // Parse the html into a DOM.
  xml_document document;
  document.load_string (html.c_str());

  // Example verse container within the XML:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  string selector;
  if (verse != 0) selector = "//div[contains(@class,'verse-" + convert_to_string (verse) + " ')]";
  else selector = "//p[@class='summary']";
  xpath_node xpathnode = document.select_node(selector.c_str());
  xml_node div_node = xpathnode.node();

  // Extract relevant information.
  gbs_basic_walker walker;
  div_node.traverse (walker);
  for (unsigned int i = 0; i < walker.texts.size(); i++) {
    if (i) text.append (" ");
    text.append (filter_string_trim(walker.texts[i]));
  }
  
  // Done.
  return text;
}


struct gbs_plus_walker: xml_tree_walker
{
  vector <string> texts;
  bool canonical_text = true;
  bool verse_references = false;
  string reference_number;

  virtual bool for_each (xml_node& node)
  {
    xml_node_type nodetype = node.type();
    if (nodetype == node_pcdata) {
      // Handle plain character data.
      if (canonical_text && !verse_references) {
        string text = node.text().get();
        texts.push_back(text);
      }
      // If the text above was a footnote caller,
      // in the GBS layout the following text could be canonical again.
      canonical_text = true;
    } else {
      // Handle the node itself.
      string nodename = node.name ();
      string classname = node.attribute ("class").value ();
      canonical_text = true;
      // Bits to exclude from the canonical text.
      if (classname == "verse-number") canonical_text = false;
      //if (classname == "kanttekening") canonical_text = false;
      //if (classname == "verwijzing") canonical_text = false;
      if (classname == "sup") canonical_text = false;
      // Start of the verse references.
      if (classname == "verse-references") {
        verse_references = true;
      }
      // Add the reference verse text.
      if (classname == "reference") {
        texts.push_back("<br>");
        texts.push_back(reference_number);
        string title = node.attribute("data-title").value();
        texts.push_back(title);
        string content = node.attribute("data-content").value();
        texts.push_back(content);
      }
      // Store the reference caller in the object.
      if (classname == "reference-number") {
        reference_number = node.text().get();
      }
    }

    // Continue parsing.
    return true;
  }
};


struct gbs_annotation_walker: xml_tree_walker
{
  vector <string> texts;
  bool within_annotations = false;

  virtual bool for_each (xml_node& node)
  {
    xml_node_type nodetype = node.type();
    if (nodetype == node_pcdata) {
      if (within_annotations) {
        string text = node.text().get();
        texts.push_back(text);
      }
    } else {
      // Handle the node itself.
      string nodename = node.name ();
      string classname = node.attribute ("class").value ();
      if (classname == "annotationnumber") {
        texts.push_back("<br>");
        within_annotations = true;
      }
      if (classname == "annotationtext") within_annotations = true;
      if (nodename == "hr") within_annotations = false;
    }
    // Continue parsing.
    return true;
  }
};


// This function displays the canonical text from bijbel-statenvertaling.com.
string gbs_plus_processor (string url, int book, [[maybe_unused]] int chapter, int verse)
{
  string text;
  
  // Get the html from the server.
  string html = resource_logic_web_or_cache_get (url, text);

  // Tidy the html so it can be loaded as xml.
  html = filter_string_tidy_invalid_html (html);

  // Parse the html into a DOM.
  xml_document document;
  document.load_string (html.c_str());

  // Example verse container within the XML:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  string selector;
  if (verse != 0) selector = "//div[contains(@class,'verse-" + convert_to_string (verse) + " ')]";
  else selector = "//p[@class='summary']";
  xpath_node xpathnode = document.select_node(selector.c_str());
  xml_node div_node = xpathnode.node();

  // Example text:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change" id="1" onclick="i_toggle_annotation('sv','30217','Hebr.','10','1', '1201')"><span class="verse-number">  1</span><div class="verse-text "><p class="text">      WANT<span class="verwijzing"> a</span><span class="kanttekening">1</span>de wet, hebbende <span class="kanttekening"> 2</span>een schaduw <span class="kanttekening"> 3</span>der toekomende goederen, niet <span class="kanttekening"> 4</span>het beeld zelf der zaken, kan met <span class="kanttekening"> 5</span>dezelfde offeranden die zij alle jaar <span class="kanttekening"> 6</span>geduriglijk opofferen, nimmermeer <span class="kanttekening"> 7</span>heiligen degenen die <span class="kanttekening"> 8</span>daar toegaan.    </p><span class="verse-references"><div class="verse-reference"><span class="reference-number">a </span><a href="/statenvertaling/kolossenzen/2/#17" target="_blank" class="reference" data-title="Kol. 2:17" data-content="Welke zijn een schaduw der toekomende dingen, maar het lichaam is van Christus.">Kol. 2:17</a>. <a href="/statenvertaling/hebreeen/8/#5" target="_blank" class="reference" data-title="Hebr. 8:5" data-content="Welke het voorbeeld en de schaduw der hemelse dingen dienen, gelijk Mozes door Goddelijke aanspraak vermaand was, als hij den tabernakel volmaken zou. Want zie, zegt Hij, dat gij het alles maakt naar de afbeelding die u op den berg getoond is.">Hebr. 8:5</a>.        </div></span></div></div>

  // Extract relevant information.
  gbs_plus_walker walker;
  div_node.traverse (walker);
  for (unsigned int i = 0; i < walker.texts.size(); i++) {
    if (i) text.append (" ");
    text.append (filter_string_trim(walker.texts[i]));
  }
  
  // Get the raw annotations html.
  string annotation_info = div_node.attribute("onclick").value();
  vector <string> bits = filter_string_explode(annotation_info, '\'');
  if (bits.size() >= 13) {
    string url = "https://bijbel-statenvertaling.com/includes/ajax/kanttekening.php";
    map <string, string> post;
    post ["prefix"] = bits[1];
    post ["verse_id"] = bits[3];
    post ["short_bookname"] = bits[5];
    post ["chapter"] = bits[7];
    post ["verse"] = bits[9];
    post ["slug_id"] = bits[11];
    post ["book_id"] = convert_to_string(book);
    string error, html;
    html = filter_url_http_post (url, post, error, false, false);
    if (error.empty()) {
      html = filter_string_tidy_invalid_html (html);
      xml_document document;
      document.load_string (html.c_str());
      string selector = "//body";
      xpath_node xpathnode = document.select_node(selector.c_str());
      xml_node body_node = xpathnode.node();
      stringstream ss;
      body_node.print (ss, "", format_raw);
      gbs_annotation_walker walker;
      body_node.traverse (walker);
      for (auto fragment : walker.texts) {
        text.append(" ");
        text.append (filter_string_trim(fragment));
      }
    } else {
      text.append("<br>");
      text.append(error);
    }
  }
  
  // Done.
  return text;
}


// This filters Bibles from www.bibleserver.com.
string bibleserver_processor (string directory, int book, int chapter, int verse)
{
  string bookname = resource_external_convert_book_bibleserver (book);
  
  string url = "http://www.bibleserver.com/text/" + directory + "/" + bookname + convert_to_string (chapter);
  
  string error;
  string text = resource_logic_web_or_cache_get (url, error);
  string tidy = html_tidy (text);
  vector <string> tidied = filter_string_explode (tidy, '\n');

  text.clear ();
  bool relevant_line = false;
  for (auto & line : tidied) {
    size_t pos = line.find ("noscript");
    if (pos != string::npos) relevant_line = false;
    if (relevant_line) {
      if (!text.empty ()) text.append (" ");
      text.append (line);
    }
    pos = line.find ("no=\"" + convert_to_string (verse) + "," + convert_to_string (verse) + "\"");
    if (pos != string::npos) relevant_line = true;
  }
  filter_string_replace_between (text, "<", ">", "");
  text = filter_string_trim (text);
  
  text += "<p><a href=\"" + url + "\">" + url + "</a></p>\n";
  
  return text;
}


string resource_external_convert_book_gbs_statenbijbel (int book)
{
  switch (book) {
    case 1: return "genesis";
    case 2: return "exodus";
    case 3: return "leviticus";
    case 4: return "numeri";
    case 5: return "deuteronomium";
    case 6: return "jozua";
    case 7: return "richteren";
    case 8: return "ruth";
    case 9: return "1-samuel";
    case 10: return "2-samuel";
    case 11: return "1-koningen";
    case 12: return "2-koningen";
    case 13: return "1-kronieken";
    case 14: return "2-kronieken";
    case 15: return "ezra";
    case 16: return "nehemia";
    case 17: return "esther";
    case 18: return "job";
    case 19: return "psalmen";
    case 20: return "spreuken";
    case 21: return "prediker";
    case 22: return "hooglied";
    case 23: return "jesaja";
    case 24: return "jeremia";
    case 25: return "klaagliederen";
    case 26: return "ezechiel";
    case 27: return "daniel";
    case 28: return "hosea";
    case 29: return "joel";
    case 30: return "amos";
    case 31: return "obadja";
    case 32: return "jona";
    case 33: return "micha";
    case 34: return "nahum";
    case 35: return "habakuk";
    case 36: return "zefanja";
    case 37: return "haggai";
    case 38: return "zacharia";
    case 39: return "maleachi";
    case 40: return "mattheus";
    case 41: return "markus";
    case 42: return "lukas";
    case 43: return "johannes";
    case 44: return "handelingen";
    case 45: return "romeinen";
    case 46: return "1-korinthe";
    case 47: return "2-korinthe";
    case 48: return "galaten";
    case 49: return "efeze";
    case 50: return "filippenzen";
    case 51: return "kolossenzen";
    case 52: return "1-thessalonicenzen";
    case 53: return "2-thessalonicenzen";
    case 54: return "1-timotheus";
    case 55: return "2-timotheus";
    case 56: return "titus";
    case 57: return "filemon";
    case 58: return "hebreeen";
    case 59: return "jakobus";
    case 60: return "1-petrus";
    case 61: return "2-petrus";
    case 62: return "1-johannes";
    case 63: return "2-johannes";
    case 64: return "3-johannes";
    case 65: return "judas";
    case 66: return "openbaring";
    default: return string();
  }
  return string();
}


string resource_external_convert_book_gbs_king_james_bible (int book)
{
  switch (book) {
    case 1: return "genesis";
    case 2: return "exodus";
    case 3: return "leviticus";
    case 4: return "numbers";
    case 5: return "deuteronomy";
    case 6: return "joshua";
    case 7: return "judges";
    case 8: return "ruth";
    case 9: return "1-samuel";
    case 10: return "2-samuel";
    case 11: return "1-kings";
    case 12: return "2-kings";
    case 13: return "1-chronicles";
    case 14: return "2-chronicles";
    case 15: return "ezra";
    case 16: return "nehemiah";
    case 17: return "esther";
    case 18: return "job";
    case 19: return "psalms";
    case 20: return "proverbs";
    case 21: return "ecclesiastes";
    case 22: return "song-of-solomon";
    case 23: return "isaiah";
    case 24: return "jeremiah";
    case 25: return "lamentations";
    case 26: return "ezekiel";
    case 27: return "daniel";
    case 28: return "hosea";
    case 29: return "joel";
    case 30: return "amos";
    case 31: return "obadiah";
    case 32: return "jonah";
    case 33: return "micah";
    case 34: return "nahum";
    case 35: return "habakkuk";
    case 36: return "zephaniah";
    case 37: return "haggai";
    case 38: return "zechariah";
    case 39: return "malachi";
    case 40: return "matthew";
    case 41: return "mark";
    case 42: return "luke";
    case 43: return "john";
    case 44: return "acts";
    case 45: return "romans";
    case 46: return "1-corinthians";
    case 47: return "2-corinthians";
    case 48: return "galatians";
    case 49: return "ephesians";
    case 50: return "philippians";
    case 51: return "colossians";
    case 52: return "1-thessalonians";
    case 53: return "2-thessalonians";
    case 54: return "1-timothy";
    case 55: return "2-timothy";
    case 56: return "titus";
    case 57: return "philemon";
    case 58: return "hebrews";
    case 59: return "james";
    case 60: return "1-peter";
    case 61: return "2-peter";
    case 62: return "1-john";
    case 63: return "2-john";
    case 64: return "3-john";
    case 65: return "jude";
    case 66: return "revelation";
    default: return string();
  }
  return string();
}


// This script fetches the Statenbijbel from the Dutch GBS.
string resource_external_get_statenbijbel_gbs (int book, int chapter, int verse)
{
  // Hebrews 11: https://bijbel-statenvertaling.com/statenvertaling/hebreeen/11/
  string url = "http://bijbel-statenvertaling.com/statenvertaling/" + resource_external_convert_book_gbs_statenbijbel (book) + "/" + convert_to_string(chapter) + "/";
  return gbs_basic_processor (url, verse);
}


// This displays the Statenbijbel from the Dutch GBS.
// It also includes headers, introductions, and notes.
string resource_external_get_statenbijbel_plus_gbs (int book, int chapter, int verse)
{
  // Hebrews 11: https://bijbel-statenvertaling.com/statenvertaling/hebreeen/11/
  string url = "http://bijbel-statenvertaling.com/statenvertaling/" + resource_external_convert_book_gbs_statenbijbel (book) + "/" + convert_to_string(chapter) + "/";
  return gbs_plus_processor (url, book, chapter, verse);
}


// This script displays the King James Bible published by the Dutch GBS.
string resource_external_get_king_james_version_gbs (int book, int chapter, int verse)
{
  string url = "http://bijbel-statenvertaling.com/authorised-version/" + resource_external_convert_book_gbs_king_james_bible (book) + "/" + convert_to_string(chapter) + "/";
  return gbs_basic_processor (url, verse);
}


// This script displays the Statenbijbel from the Dutch GBS.
// It also includes headers, introductions, and notes.
string resource_external_get_king_james_version_plus_gbs (int book, int chapter, int verse)
{
  string url = "http://bijbel-statenvertaling.com/authorised-version/" + resource_external_convert_book_gbs_king_james_bible (book) + "/" + convert_to_string(chapter) + "/";
  return gbs_plus_processor (url, book, chapter, verse);
}


// This displays the interlinear Bible from biblehub.com.
string resource_external_get_biblehub_interlinear (int book, int chapter, int verse)
{
  // Sample URL:
  // http://biblehub.com/interlinear/genesis/1-1.htm
 
  string bookname = resource_external_convert_book_biblehub (book);
  
  string url = "http://biblehub.com/interlinear/" + bookname + "/" + convert_to_string (chapter) + "-" + convert_to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  string error;
  string html = resource_logic_web_or_cache_get (url, error);
  string tidy = html_tidy (html);
  vector <string> tidied = filter_string_explode (tidy, '\n');
  
  vector <string> filtered_lines;
  
  int relevant_line = 0;
  for (auto & line : tidied) {
    if (line.find ("<div") != string::npos) {
      relevant_line = 0;
    }
    if (relevant_line) {
      relevant_line++;
    }
    if (relevant_line > 3) {
      if (line.find ("iframe") == string::npos) {
        filtered_lines.push_back (line);
      }
    }
    if (line.find ("\"vheading\"") != string::npos) {
      relevant_line++;
    }
  }
  
  html = filter_string_implode (filtered_lines, "\n");
  
  html = filter_string_str_replace ("/abbrev.htm", "http://biblehub.com/abbrev.htm", html);
  html = filter_string_str_replace ("/hebrew/", "http://biblehub.com/hebrew/", html);
  html = filter_string_str_replace ("/hebrewparse.htm", "http://biblehub.com/hebrewparse.htm", html);
  html = filter_string_str_replace ("/greek/", "http://biblehub.com/greek/", html);
  html = filter_string_str_replace ("/grammar/", "http://biblehub.com/grammar/", html);
  //html = filter_string_str_replace ("height=\"165\"", "", html);
  html = filter_string_str_replace ("height=\"160\"", "", html);
  html = filter_string_str_replace ("height=\"145\"", "", html);
  html = filter_string_str_replace (unicode_non_breaking_space_entity () + unicode_non_breaking_space_entity (), unicode_non_breaking_space_entity (), html);
  
  // Stylesheet for using web fonts,
  // because installing fonts on some tablets is very hard.
  string stylesheet =
  "<style>\n"
  "span[class*='ref'] {\n"
  "display: none;\n"
  "}\n"
  "</style>\n";
  string output = stylesheet;
  // The following line prevents the Hebrew and Greek from floating around
  // the name of the Resource, which would disturb the order of the words in Hebrew.
  output += "<p></p>\n";
  output += html;
  
  return output;
}


// Filters the Scrivener Greek text from biblehub.com.
string resource_external_get_biblehub_scrivener (int book, int chapter, int verse)
{
  string bookname = resource_external_convert_book_biblehub (book);
  
  string url = "http://biblehub.com/text/" + bookname + "/" + convert_to_string (chapter) + "-" + convert_to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  string error;
  string html = resource_logic_web_or_cache_get (url, error);
  string tidy = html_tidy (html);
  vector <string> tidied = filter_string_explode (tidy, '\n');

  html.clear ();
  int hits = 0;
  for (auto & line : tidied) {
    /* This is the text block we are looking at:
    <p>
    <span class="versiontext">
    <a href="/tr94/matthew/1.htm">Scrivener's Textus Receptus 1894
    </a>
    <br>
    </span>
    <span class="greek">Βίβλος γενέσεως Ἰησοῦ Χριστοῦ, υἱοῦ Δαβὶδ, υἱοῦ Ἀβραάμ.
    </span>
    */
    if (line.find ("Scrivener") != string::npos) hits = 1;
    if (hits == 1) if (line.find ("greek") != string::npos) hits = 2;
    if ((hits == 2) || (hits == 3)) {
      html.append (line);
      hits++;
    }
  }
  
  if (html.empty ()) return html;
  
  string stylesheet =
  "<style>\n"
  "</style>\n";
  
  return stylesheet + "\n" + html;
}


// This displays the Westminster Leningrad Codex from biblehub.com.
string resource_external_get_biblehub_westminster (int book, int chapter, int verse)
{
  // No New Testament in the Westminster Leningrad Codex.
  if (book >= 40) {
    return "";
  }
  
  string bookname = resource_external_convert_book_biblehub (book);
  
  // Sample URL:
  // http://biblehub.com/text/genesis/1-1.htm
  string url = "http://biblehub.com/text/" + bookname + "/" + convert_to_string (chapter) + "-" + convert_to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  string error;
  string html = resource_logic_web_or_cache_get (url, error);
  string tidy = html_tidy (html);
  vector <string> tidied = filter_string_explode (tidy, '\n');
  
  html.clear ();
  int hits = 0;
  for (auto & line : tidied) {
    /* This is the text block we are looking at:
     <div align="right">
     <span class="versiontext">
     <a href="/wlc/genesis/1.htm">בראשית 1:1 Hebrew OT: Westminster Leningrad Codex
     </a>
     <br>
     </span>
     <span class="heb">בְּרֵאשִׁ֖ית בָּרָ֣א אֱלֹהִ֑ים אֵ֥ת הַשָּׁמַ֖יִם וְאֵ֥ת הָאָֽרֶץ׃
     </span>
     <p>
    */
    if (line.find ("Westminster") != string::npos) hits = 1;
    if (hits == 1) if (line.find ("class=\"heb\"") != string::npos) hits = 2;
    if (hits == 2) {
      html.append (line);
      if (line.find ("</span>") != string::npos) hits = 0;
    }
  }
  
  if (html.empty ()) return html;
  
  // Change class "heb" to "hebrew", because that is what Bibledit uses for all Hebrew text.
  html = filter_string_str_replace ("heb", "hebrew", html);
  
  
  // Stylesheet for using web fonts,
  // because installing fonts on some tablets is very hard.
  string stylesheet =
  "<style>\n"
  "</style>\n";

  string output = stylesheet;
  
  // The following line prevents the Hebrew from floating around the name of the Resource,
  // which would disturb the order of the words.
  output += "<p></p>\n";
  
  output += html;

  return output;

  // In Job 24:14, it lacks a space between two words, making it hard to read.
  // In Job 31:36, one letter is joined to the wrong word.
}


// This displays the text and the notes of the NET Bible.
string resource_external_get_net_bible (int book, int chapter, int verse)
{
  string bookname = resource_external_convert_book_netbible (book);
  
  string url = bookname + " " + convert_to_string (chapter) + ":" + convert_to_string (verse);
  url = filter_url_urlencode (url);
  url.insert (0, "https://net.bible.org/resource/netTexts/");
  
  string error;
  string text = resource_logic_web_or_cache_get (url, error);
  
  // Due to an error, the result could include PHP.
  // See https://github.com/bibledit/cloud/issues/579.
  // So if the text contains ".php", then there's that error.
  if (text.find(".php") != string::npos) text.clear();
  
  string output = text;
  
  url = bookname + " " + convert_to_string (chapter) + ":" + convert_to_string (verse);
  url = filter_url_urlencode (url);
  url.insert (0, "https://net.bible.org/resource/netNotes/");
  
  string notes = resource_logic_web_or_cache_get (url, error);
  // If notes fail with an error, don't include the note text.
  if (!error.empty ()) notes.clear ();

  // It the verse contains no notes, the website returns an unusual message.
  if (notes.find ("We are currently offline") != string::npos) notes.clear ();

  // Deal with the following message:
  // Warning Message
  // An error was detected which prevented the loading of this page. If this problem persists, please contact the website administrator.
  // libraries/DB_Bible.php [780]:
  // Invalid argument supplied for foreach()
  // Loaded in 0.0303 seconds, using 0.59MB of memory. Generated by Kohana v2.3.4.
  // This error contains so much additional code, that the entire Bibledit program gets confused.
  if (notes.find ("Warning Message") != string::npos) notes.clear ();

  // The "bibleref" class experiences interference from other resources,
  // so that the reference would become invisible.
  // Remove this class, and the references will remain visible.
  notes = filter_string_str_replace ("class=\"bibleref\"", "", notes);
  
  output += notes;
  
  return output;
}


// Blue Letter Bible.
string resource_external_get_blue_letter_bible (int book, int chapter, int verse)
{
  if (verse) {};
  
  string bookname = resource_external_convert_book_blueletterbible (book);
  
  string output;
  
  string url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=$" + convert_to_string (chapter) + "&t=KJV&ss=1";
  
  output += "<a href=\"" + url + "\">KJV</a>";
  
  output += " | ";
  
  url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=" + convert_to_string (chapter) + "&t=WLC";
  
  output += "<a href=\"" + url + "\">WLC</a>";
  
  output += " | ";
  
  url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=" + convert_to_string (chapter) + "&t=mGNT";
  
  output += "<a href=\"" + url + "\">mGNT</a>";

  return output;
}


// This displays the text of the Elberfelder Bibel.
string resource_external_get_elberfelder_bibel (int book, int chapter, int verse)
{
  // The directory on bibleserver.com
  string directory = "ELB";
  return bibleserver_processor (directory, book, chapter, verse);
}


// The number of available external resource scripts.
unsigned int resource_external_count ()
{
  return sizeof (resource_table) / sizeof (*resource_table);
}


// Gets the names of all the known Web resources.
vector <string> resource_external_names ()
{
  vector <string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    names.push_back (resource_table [i].name);
  }
  sort (names.begin (), names.end ());
  return names;
}


// Get the names of the Web resources which are original language resources.
vector <string> resource_external_get_original_language_resources ()
{
  vector <string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    if (strcmp (resource_table [i].type, ORIGINAL) == 0) {
      names.push_back (resource_table [i].name);
    }
  }
  sort (names.begin (), names.end ());
  return names;
}


// Get the names of the Web resources which are Bibles and notes.
vector <string> resource_external_get_bibles ()
{
  vector <string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    if (strcmp (resource_table [i].type, BIBLE) == 0) {
      names.push_back (resource_table [i].name);
    }
  }
  sort (names.begin (), names.end ());
  return names;
}


// Returns the versification for the resource.
string resource_external_versification (string name)
{
  string versification = english ();
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    string resource = resource_table [i].name;
    if (name == resource) {
      versification = resource_table [i].versification;
    }
  }
  return versification;
}


// Returns the versification for the resource.
string resource_external_mapping (string name)
{
  string versification = english ();
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    string resource = resource_table [i].name;
    if (name == resource) {
      versification = resource_table [i].mapping;
    }
  }
  return versification;
}


// Returns what type the Web resource $name is.
string resource_external_type (string name)
{
  string type = UNKNOWN;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    string resource = resource_table [i].name;
    if (name == resource) {
      type = resource_table [i].type;
    }
  }
  return type;
}


// Fetches the html data for the resource / book / chapter / verse.
// This function runs on the Cloud.
// It fetches data either from the cache or from the web via http(s),
// while optionally updating the cache with the raw web page content.
// It extracts the relevant snipped from the larger http(s) content.
string resource_external_cloud_fetch_cache_extract (string name, int book, int chapter, int verse)
{
  string (* function_name) (int, int, int) = NULL;

  for (unsigned int i = 0; i < resource_external_count (); i++) {
    string resource = resource_table [i].name;
    if (name == resource) {
      function_name = resource_table [i].func;
    }
  }
  
  if (function_name == NULL) return string();

  string result = function_name (book, chapter, verse);
  
  return result;
}


string resource_external_convert_book_biblehub (int book)
{
  // Map Bibledit books to biblehub.com books.
  map <int, string> mapping = {
    pair (1, "genesis"),
    pair (2, "exodus"),
    pair (3, "leviticus"),
    pair (4, "numbers"),
    pair (5, "deuteronomy"),
    pair (6, "joshua"),
    pair (7, "judges"),
    pair (8, "ruth"),
    pair (9, "1_samuel"),
    pair (10, "2_samuel"),
    pair (11, "1_kings"),
    pair (12, "2_kings"),
    pair (13, "1_chronicles"),
    pair (14, "2_chronicles"),
    pair (15, "ezra"),
    pair (16, "nehemiah"),
    pair (17, "esther"),
    pair (18, "job"),
    pair (19, "psalms"),
    pair (20, "proverbs"),
    pair (21, "ecclesiastes"),
    pair (22, "songs"),
    pair (23, "isaiah"),
    pair (24, "jeremiah"),
    pair (25, "lamentations"),
    pair (26, "ezekiel"),
    pair (27, "daniel"),
    pair (28, "hosea"),
    pair (29, "joel"),
    pair (30, "amos"),
    pair (31, "obadiah"),
    pair (32, "jonah"),
    pair (33, "micah"),
    pair (34, "nahum"),
    pair (35, "habakkuk"),
    pair (36, "zephaniah"),
    pair (37, "haggai"),
    pair (38, "zechariah"),
    pair (39, "malachi"),
    pair (40, "matthew"),
    pair (41, "mark"),
    pair (42, "luke"),
    pair (43, "john"),
    pair (44, "acts"),
    pair (45, "romans"),
    pair (46, "1_corinthians"),
    pair (47, "2_corinthians"),
    pair (48, "galatians"),
    pair (49, "ephesians"),
    pair (50, "philippians"),
    pair (51, "colossians"),
    pair (52, "1_thessalonians"),
    pair (53, "2_thessalonians"),
    pair (54, "1_timothy"),
    pair (55, "2_timothy"),
    pair (56, "titus"),
    pair (57, "philemon"),
    pair (58, "hebrews"),
    pair (59, "james"),
    pair (60, "1_peter"),
    pair (61, "2_peter"),
    pair (62, "1_john"),
    pair (63, "2_john"),
    pair (64, "3_john"),
    pair (65, "jude"),
    pair (66, "revelation")
  };
  return mapping [book];
}


string resource_external_convert_book_netbible (int book)
{
  // Map Bibledit books to net.bible.com books as used at the web service.
  map <int, string> mapping = {
    pair (1, "Genesis"),
    pair (2, "Exodus"),
    pair (3, "Leviticus"),
    pair (4, "Numbers"),
    pair (5, "Deuteronomy"),
    pair (6, "Joshua"),
    pair (7, "Judges"),
    pair (8, "Ruth"),
    pair (9, "1 Samuel"),
    pair (10, "2 Samuel"),
    pair (11, "1 Kings"),
    pair (12, "2 Kings"),
    pair (13, "1 Chronicles"),
    pair (14, "2 Chronicles"),
    pair (15, "Ezra"),
    pair (16, "Nehemiah"),
    pair (17, "Esther"),
    pair (18, "Job"),
    pair (19, "Psalms"),
    pair (20, "Proverbs"),
    pair (21, "Ecclesiastes"),
    pair (22, "Song of Solomon"),
    pair (23, "Isaiah"),
    pair (24, "Jeremiah"),
    pair (25, "Lamentations"),
    pair (26, "Ezekiel"),
    pair (27, "Daniel"),
    pair (28, "Hosea"),
    pair (29, "Joel"),
    pair (30, "Amos"),
    pair (31, "Obadiah"),
    pair (32, "Jonah"),
    pair (33, "Micah"),
    pair (34, "Nahum"),
    pair (35, "Habakkuk"),
    pair (36, "Zephaniah"),
    pair (37, "Haggai"),
    pair (38, "Zechariah"),
    pair (39, "Malachi"),
    pair (40, "Matthew"),
    pair (41, "Mark"),
    pair (42, "Luke"),
    pair (43, "John"),
    pair (44, "Acts"),
    pair (45, "Romans"),
    pair (46, "1 Corinthians"),
    pair (47, "2 Corinthians"),
    pair (48, "Galatians"),
    pair (49, "Ephesians"),
    pair (50, "Philippians"),
    pair (51, "Colossians"),
    pair (52, "1 Thessalonians"),
    pair (53, "2 Thessalonians"),
    pair (54, "1 Timothy"),
    pair (55, "2 Timothy"),
    pair (56, "Titus"),
    pair (57, "Philemon"),
    pair (58, "Hebrews"),
    pair (59, "James"),
    pair (60, "1 Peter"),
    pair (61, "2 Peter"),
    pair (62, "1 John"),
    pair (63, "2 John"),
    pair (64, "3 John"),
    pair (65, "Jude"),
    pair (66, "Revelation")
  };
  return mapping [book];
}


string resource_external_convert_book_blueletterbible (int book)
{
  // This array maps the the book identifiers from Bibledit
  // to the book names as used by the blueletterbible.org web service.
  map <int, string> mapping = {
    pair (1, "Gen"),
    pair (2, "Exd"),
    pair (3, "Lev"),
    pair (4, "Num"),
    pair (5, "Deu"),
    pair (6, "Jos"),
    pair (7, "Jdg"),
    pair (8, "Rth"),
    pair (9, "1Sa"),
    pair (10, "2Sa"),
    pair (11, "1Ki"),
    pair (12, "2Ki"),
    pair (13, "1Ch"),
    pair (14, "2Ch"),
    pair (15, "Ezr"),
    pair (16, "Neh"),
    pair (17, "Est"),
    pair (18, "Job"),
    pair (19, "Psa"),
    pair (20, "Pro"),
    pair (21, "Ecc"),
    pair (22, "Sgs"),
    pair (23, "Isa"),
    pair (24, "Jer"),
    pair (25, "Lam"),
    pair (26, "Eze"),
    pair (27, "Dan"),
    pair (28, "Hsa"),
    pair (29, "Joe"),
    pair (30, "Amo"),
    pair (31, "Oba"),
    pair (32, "Jon"),
    pair (33, "Mic"),
    pair (34, "Nah"),
    pair (35, "Hab"),
    pair (36, "Zep"),
    pair (37, "Hag"),
    pair (38, "Zec"),
    pair (39, "Mal"),
    pair (40, "Mat"),
    pair (41, "Mar"),
    pair (42, "Luk"),
    pair (43, "Jhn"),
    pair (44, "Act"),
    pair (45, "Rom"),
    pair (46, "1Cr"),
    pair (47, "2Cr"),
    pair (48, "Gal"),
    pair (49, "Eph"),
    pair (50, "Phl"),
    pair (51, "Col"),
    pair (52, "1Th"),
    pair (53, "2Th"),
    pair (54, "1Ti"),
    pair (55, "2Ti"),
    pair (56, "Tts"),
    pair (57, "Phm"),
    pair (58, "Hbr"),
    pair (59, "Jam"),
    pair (60, "1Pe"),
    pair (61, "2Pe"),
    pair (62, "1Jo"),
    pair (63, "2Jo"),
    pair (64, "3Jo"),
    pair (65, "Jud"),
    pair (66, "Rev")
  };
  return mapping [book];
}


string resource_external_convert_book_bibleserver (int book)
{
  // On bibleserver.com, Genesis is "1.Mose", Exodus is "2.Mose", and so on.
  map <int, string> mapping = {
    pair (1, "1.Mose"),
    pair (2, "2.Mose"),
    pair (3, "3.Mose"),
    pair (4, "4.Mose"),
    pair (5, "5.Mose"),
    pair (6, "Josua"),
    pair (7, "Richter"),
    pair (8, "Rut"),
    pair (9, "1.Samuel"),
    pair (10, "2.Samuel"),
    pair (11, "1.Könige"),
    pair (12, "2.Könige"),
    pair (13, "1.Chronik"),
    pair (14, "2.Chronik"),
    pair (15, "Esra"),
    pair (16, "Nehemia"),
    pair (17, "Esther"),
    pair (18, "Hiob"),
    pair (19, "Psalm"),
    pair (20, "Sprüche"),
    pair (21, "Prediger"),
    pair (22, "Hoheslied"),
    pair (23, "Jesaja"),
    pair (24, "Jeremia"),
    pair (25, "Klagelieder"),
    pair (26, "Hesekiel"),
    pair (27, "Daniel"),
    pair (28, "Hosea"),
    pair (29, "Joel"),
    pair (30, "Amos"),
    pair (31, "Obadja"),
    pair (32, "Jona"),
    pair (33, "Micha"),
    pair (34, "Nahum"),
    pair (35, "Habakuk"),
    pair (36, "Zefanja"),
    pair (37, "Haggai"),
    pair (38, "Sacharja"),
    pair (39, "Maleachi"),
    pair (40, "Matthäus"),
    pair (41, "Markus"),
    pair (42, "Lukas"),
    pair (43, "Johannes"),
    pair (44, "Apostelgeschichte"),
    pair (45, "Römer"),
    pair (46, "1.Korinther"),
    pair (47, "2.Korinther"),
    pair (48, "Galater"),
    pair (49, "Epheser"),
    pair (50, "Philipper"),
    pair (51, "Kolosser"),
    pair (52, "1.Thessalonicher"),
    pair (53, "2.Thessalonicher"),
    pair (54, "1.Timotheus"),
    pair (55, "2.Timotheus"),
    pair (56, "Titus"),
    pair (57, "Philemon"),
    pair (58, "Hebräer"),
    pair (59, "Jakobus"),
    pair (60, "1.Petrus"),
    pair (61, "2.Petrus"),
    pair (62, "1.Johannes"),
    pair (63, "2.Johannes"),
    pair (64, "3.Johannes"),
    pair (65, "Judas"),
    pair (66, "Offenbarung")
  };
  return mapping [book];
}


const char * resource_external_net_bible_name ()
{
  return "NET Bible";
}


const char * resource_external_biblehub_interlinear_name ()
{
  return "Biblehub Interlinear";
}

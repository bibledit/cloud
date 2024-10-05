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


#include <resource/external.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include "assets/view.h"
#include "resource/logic.h"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <jsonxx/jsonxx.h>
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


// Local forward declarations:


unsigned int resource_external_count ();
int gbs_digitaal_json_callback (void *userdata, int type, const char *data, uint32_t length);
std::string bibleserver_processor (std::string directory, int book, int chapter, int verse);
std::string resource_external_get_statenbijbel_gbs (int book, int chapter, int verse);
std::string resource_external_get_statenbijbel_plus_gbs (int book, int chapter, int verse);
std::string resource_external_get_king_james_version_gbs (int book, int chapter, int verse);
std::string resource_external_get_king_james_version_plus_gbs (int book, int chapter, int verse);
std::string resource_external_get_biblehub_interlinear (int book, int chapter, int verse);
std::string resource_external_get_biblehub_scrivener (int book, int chapter, int verse);
std::string resource_external_get_biblehub_westminster (int book, int chapter, int verse);
std::string resource_external_get_net_bible (int book, int chapter, int verse);
std::string resource_external_get_blue_letter_bible (int book, int chapter, int verse);
std::string resource_external_get_elberfelder_bibel (int book, int chapter, int verse);
std::string resource_external_convert_book_biblehub (int book);
std::string resource_external_convert_book_netbible (int book);
std::string resource_external_convert_book_blueletterbible (int book);
std::string resource_external_convert_book_bibleserver (int book);


typedef struct
{
  std::string name;
  const char *versification;
  const char *mapping;
  const char *type;
  std::string (* func) (int, int, int);
} resource_record;


#define UNKNOWN "u"
#define ORIGINAL "o"
#define BIBLE "b"


// Implementation:


resource_record resource_table [] =
{
  { "Statenbijbel GBS", "Dutch Traditional", "Dutch Traditional", BIBLE, & resource_external_get_statenbijbel_gbs },
  { "Statenbijbel Plus GBS", "Dutch Traditional", "Dutch Traditional", BIBLE, & resource_external_get_statenbijbel_plus_gbs },
  { "King James Version GBS", filter::strings::english (), filter::strings::english (), BIBLE, & resource_external_get_king_james_version_gbs },
  { "King James Version Plus GBS", filter::strings::english (), filter::strings::english (), BIBLE, & resource_external_get_king_james_version_plus_gbs },
  { resource_external_biblehub_interlinear_name (), filter::strings::english (), filter::strings::english (), ORIGINAL, & resource_external_get_biblehub_interlinear },
  { "Scrivener Greek", filter::strings::english (), filter::strings::english (), ORIGINAL, & resource_external_get_biblehub_scrivener },
  { "Westminster Hebrew", filter::strings::english (), filter::strings::english (), ORIGINAL, & resource_external_get_biblehub_westminster },
  { resource_external_net_bible_name (), filter::strings::english (), filter::strings::english (), BIBLE, & resource_external_get_net_bible },
  { "Blue Letter Bible", filter::strings::english (), filter::strings::english (), ORIGINAL, & resource_external_get_blue_letter_bible },
  { "Elberfelder Bibel", filter::strings::english (), filter::strings::english (), BIBLE, & resource_external_get_elberfelder_bibel },
  { resource_logic_easy_english_bible_name (), filter::strings::english (), filter::strings::english (), BIBLE, & resource_logic_easy_english_bible_get },
};


struct gbs_basic_walker: pugi::xml_tree_walker
{
  std::vector <std::string> texts {};
  bool canonical_text {true};

  virtual bool for_each (pugi::xml_node& node) override
  {
    pugi::xml_node_type nodetype = node.type();
    if (nodetype == pugi::node_pcdata) {
      // Handle plain character data.
      std::string text = node.text().get();
      if (canonical_text) texts.push_back(text);
      // If the text above was a footnote caller,
      // in the GBS layout the following text could be canonical again.
      canonical_text = true;
    } else {
      // Handle the node itself.
      std::string nodename = node.name ();
      std::string classname = node.attribute ("class").value ();
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
std::string gbs_basic_processor (std::string url, int verse)
{
  std::string text {};
  
  // Get the html from the server.
  std::string html = resource_logic_web_or_cache_get (url, text);

  // The GBS data does not load at all in XML.
  // If it were tidied through gumbo it does not load well as XML, just a few bits load.
  // So another approach is taken.
  // * Split the html up into lines.
  // * Look for the line with a starting signature depending on the verse number.
  // * Starting from that line, add several more lines, enough to cover the whole verse.
  // * Load the resulting block of text into pugixml.

  std::vector <std::string> lines = filter::strings::explode(html, '\n');
  std::string html_fragment {};
  
  // Example verse container within the html:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  std::string search1 {};
  std::string search2 {};
  if (verse != 0) {
    search1 = R"(class="verse )";
    search2 = " verse-" + std::to_string (verse) + " ";
  }
  else {
    search1 = R"(class="summary")";
    search2 = search1;
  }

  int line_count {0};
  for (const auto & line : lines) {
    if (!line_count) {
      size_t pos = line.find (search1);
      if (pos == std::string::npos) continue;
      pos = line.find (search2);
      if (pos == std::string::npos) continue;
      line_count++;
    }
    if (line_count) {
      if (line_count < 100) {
        line_count++;
        html_fragment.append (line);
        html_fragment.append ("\n");
      } else {
        line_count = 0;
      }
    }
  }
  
  // Parse the html into a DOM.
  pugi::xml_document document;
  document.load_string (html_fragment.c_str());

  // Example verse container within the XML:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  std::string selector;
  if (verse != 0) selector = "//div[contains(@class,'verse-" + std::to_string (verse) + " ')]";
  else selector = "//p[@class='summary']";
  pugi::xpath_node xpathnode = document.select_node(selector.c_str());
  pugi::xml_node div_node = xpathnode.node();
  
  // Extract relevant information.
  gbs_basic_walker walker {};
  div_node.traverse (walker);
  for (size_t i {0}; i < walker.texts.size(); i++) {
    if (i) text.append (" ");
    text.append (filter::strings::trim(walker.texts[i]));
  }
  
  // Done.
  return text;
}


struct gbs_plus_walker: pugi::xml_tree_walker
{
  std::vector <std::string> texts {};
  bool canonical_text {true};
  bool verse_references {false};
  std::string reference_number {};

  virtual bool for_each (pugi::xml_node& node) override
  {
    pugi::xml_node_type nodetype = node.type();
    if (nodetype == pugi::node_pcdata) {
      // Handle plain character data.
      if (canonical_text && !verse_references) {
        std::string text = node.text().get();
        texts.push_back(text);
      }
      // If the text above was a footnote caller,
      // in the GBS layout the following text could be canonical again.
      canonical_text = true;
    } else {
      // Handle the node itself.
      std::string nodename = node.name ();
      std::string classname = node.attribute ("class").value ();
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
        std::string title = node.attribute("data-title").value();
        texts.push_back(title);
        std::string content = node.attribute("data-content").value();
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


struct gbs_annotation_walker: pugi::xml_tree_walker
{
  std::vector <std::string> texts {};
  bool within_annotations {false};

  virtual bool for_each (pugi::xml_node& node) override
  {
    pugi::xml_node_type nodetype = node.type();
    if (nodetype == pugi::node_pcdata) {
      if (within_annotations) {
        std::string text = node.text().get();
        texts.push_back(text);
      }
    } else {
      // Handle the node itself.
      std::string nodename = node.name ();
      std::string classname = node.attribute ("class").value ();
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
std::string gbs_plus_processor (std::string url, int book, [[maybe_unused]] int chapter, int verse)
{
  std::string text {};
  
  // Get the html from the server.
  std::string html {resource_logic_web_or_cache_get (url, text)};

  // The GBS data does not load at all in XML.
  // If it were tidied through gumbo it does not load well as XML, just a few bits load.
  // So another approach is taken.
  // * Split the html up into lines.
  // * Look for the line with a starting signature depending on the verse number.
  // * Starting from that line, add several more lines, enough to cover the whole verse.
  // * Load the resulting block of text into pugixml.
  
  std::vector <std::string> lines {filter::strings::explode(html, '\n')};
  std::string html_fragment {};
  
  // Example verse container within the html:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  std::string search1 {};
  std::string search2 {};
  if (verse != 0) {
    search1 = R"(class="verse )";
    search2 = " verse-" + std::to_string (verse) + " ";
  }
  else {
    search1 = R"(class="summary")";
    search2 = search1;
  }

  int line_count {0};
  for (const auto & line : lines) {
    if (!line_count) {
      size_t pos = line.find (search1);
      if (pos == std::string::npos) continue;
      pos = line.find (search2);
      if (pos == std::string::npos) continue;
      line_count++;
    }
    if (line_count) {
      if (line_count < 100) {
        line_count++;
        html_fragment.append (line);
        html_fragment.append ("\n");
      } else {
        line_count = 0;
      }
    }
  }
  
  // Parse the html fragment into a DOM.
  pugi::xml_document document {};
  document.load_string (html_fragment.c_str());

  // Example verse container within the XML:
  // Verse 0:
  // <p class="summary">...</>
  // Other verses:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change">...
  std::string selector {};
  if (verse != 0) selector = "//div[contains(@class,'verse-" + std::to_string (verse) + " ')]";
  else selector = "//p[@class='summary']";
  pugi::xpath_node xpathnode = document.select_node(selector.c_str());
  pugi::xml_node div_node = xpathnode.node();

  // Example text:
  // <div class="verse verse-1 active size-change bold-change cursive-change align-change" id="1" onclick="i_toggle_annotation('sv','30217','Hebr.','10','1', '1201')"><span class="verse-number">  1</span><div class="verse-text "><p class="text">      WANT<span class="verwijzing"> a</span><span class="kanttekening">1</span>de wet, hebbende <span class="kanttekening"> 2</span>een schaduw <span class="kanttekening"> 3</span>der toekomende goederen, niet <span class="kanttekening"> 4</span>het beeld zelf der zaken, kan met <span class="kanttekening"> 5</span>dezelfde offeranden die zij alle jaar <span class="kanttekening"> 6</span>geduriglijk opofferen, nimmermeer <span class="kanttekening"> 7</span>heiligen degenen die <span class="kanttekening"> 8</span>daar toegaan.    </p><span class="verse-references"><div class="verse-reference"><span class="reference-number">a </span><a href="/statenvertaling/kolossenzen/2/#17" target="_blank" class="reference" data-title="Kol. 2:17" data-content="Welke zijn een schaduw der toekomende dingen, maar het lichaam is van Christus.">Kol. 2:17</a>. <a href="/statenvertaling/hebreeen/8/#5" target="_blank" class="reference" data-title="Hebr. 8:5" data-content="Welke het voorbeeld en de schaduw der hemelse dingen dienen, gelijk Mozes door Goddelijke aanspraak vermaand was, als hij den tabernakel volmaken zou. Want zie, zegt Hij, dat gij het alles maakt naar de afbeelding die u op den berg getoond is.">Hebr. 8:5</a>.        </div></span></div></div>
  
  // Extract relevant information.
  gbs_plus_walker walker {};
  div_node.traverse (walker);
  for (size_t i {0}; i < walker.texts.size(); i++) {
    if (i) text.append (" ");
    text.append (filter::strings::trim(walker.texts[i]));
  }
  
  // Get the raw annotations html.
  std::string annotation_info {div_node.attribute("onclick").value()};
  std::vector <std::string> bits {filter::strings::explode(annotation_info, '\'')};
  if (bits.size() >= 13) {
    std::string annotation_url {"https://bijbel-statenvertaling.com/includes/ajax/kanttekening.php"};
    std::map <std::string, std::string> post {};
    post ["prefix"] = bits[1];
    post ["verse_id"] = bits[3];
    post ["short_bookname"] = bits[5];
    post ["chapter"] = bits[7];
    post ["verse"] = bits[9];
    post ["slug_id"] = bits[11];
    post ["book_id"] = std::to_string(book);
    std::string error {};
    std::string annotation_html {filter_url_http_post (annotation_url, std::string(), post, error, false, false, {})};
    if (error.empty()) {
      annotation_html = filter::strings::fix_invalid_html_gumbo (annotation_html);
      pugi::xml_document annotation_document {};
      annotation_document.load_string (annotation_html.c_str());
      std::string selector2 {"//body"};
      pugi::xpath_node xpathnode2 {annotation_document.select_node(selector2.c_str())};
      pugi::xml_node body_node {xpathnode2.node()};
      std::stringstream ss {};
      body_node.print (ss, "", pugi::format_raw);
      gbs_annotation_walker annotation_walker {};
      body_node.traverse (annotation_walker);
      for (auto fragment : annotation_walker.texts) {
        text.append(" ");
        text.append (filter::strings::trim(fragment));
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
std::string bibleserver_processor (std::string directory, int book, int chapter, int verse)
{
  std::string bookname = resource_external_convert_book_bibleserver (book);
  
  std::string url = "http://www.bibleserver.com/text/" + directory + "/" + bookname + std::to_string (chapter);
  
  std::string error;
  std::string text = resource_logic_web_or_cache_get (url, error);
  std::string tidy = filter::strings::html_tidy (text);
  std::vector <std::string> tidied = filter::strings::explode (tidy, '\n');

  text.clear ();
  bool relevant_line = false;
  for (auto & line : tidied) {
    size_t pos = line.find ("noscript");
    if (pos != std::string::npos) relevant_line = false;
    if (relevant_line) {
      if (!text.empty ()) text.append (" ");
      text.append (line);
    }
    pos = line.find ("no=\"" + std::to_string (verse) + "," + std::to_string (verse) + "\"");
    if (pos != std::string::npos) relevant_line = true;
  }
  filter::strings::replace_between (text, "<", ">", "");
  text = filter::strings::trim (text);
  
  text += "<p><a href=\"" + url + "\">" + url + "</a></p>\n";
  
  return text;
}


std::string resource_external_convert_book_gbs_statenbijbel (int book)
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
    default: return std::string();
  }
  return std::string();
}


std::string resource_external_convert_book_gbs_king_james_bible (int book)
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
    default: return std::string();
  }
  return std::string();
}


// This script fetches the Statenbijbel from the Dutch GBS.
std::string resource_external_get_statenbijbel_gbs (int book, int chapter, int verse)
{
  // Hebrews 11: https://bijbel-statenvertaling.com/statenvertaling/hebreeen/11/
  std::string url = "http://bijbel-statenvertaling.com/statenvertaling/" + resource_external_convert_book_gbs_statenbijbel (book) + "/" + std::to_string(chapter) + "/";
  return gbs_basic_processor (url, verse);
}


// This displays the Statenbijbel from the Dutch GBS.
// It also includes headers, introductions, and notes.
std::string resource_external_get_statenbijbel_plus_gbs (int book, int chapter, int verse)
{
  // Hebrews 11: https://bijbel-statenvertaling.com/statenvertaling/hebreeen/11/
  std::string url = "http://bijbel-statenvertaling.com/statenvertaling/" + resource_external_convert_book_gbs_statenbijbel (book) + "/" + std::to_string(chapter) + "/";
  return gbs_plus_processor (url, book, chapter, verse);
}


// This script displays the King James Bible published by the Dutch GBS.
std::string resource_external_get_king_james_version_gbs (int book, int chapter, int verse)
{
  std::string url = "http://bijbel-statenvertaling.com/authorised-version/" + resource_external_convert_book_gbs_king_james_bible (book) + "/" + std::to_string(chapter) + "/";
  return gbs_basic_processor (url, verse);
}


// This script displays the Statenbijbel from the Dutch GBS.
// It also includes headers, introductions, and notes.
std::string resource_external_get_king_james_version_plus_gbs (int book, int chapter, int verse)
{
  std::string url = "http://bijbel-statenvertaling.com/authorised-version/" + resource_external_convert_book_gbs_king_james_bible (book) + "/" + std::to_string(chapter) + "/";
  return gbs_plus_processor (url, book, chapter, verse);
}


// This displays the interlinear Bible from biblehub.com.
std::string resource_external_get_biblehub_interlinear (int book, int chapter, int verse)
{
  // Sample URL:
  // http://biblehub.com/interlinear/genesis/1-1.htm
 
  std::string bookname = resource_external_convert_book_biblehub (book);
  
  std::string url = "http://biblehub.com/interlinear/" + bookname + "/" + std::to_string (chapter) + "-" + std::to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  std::string error;
  std::string html = resource_logic_web_or_cache_get (url, error);
  std::string tidy = filter::strings::html_tidy (html);
  std::vector <std::string> tidied = filter::strings::explode (tidy, '\n');
  
  std::vector <std::string> filtered_lines;
  
  int relevant_line = 0;
  for (auto & line : tidied) {
    if (line.find ("<div") != std::string::npos) {
      relevant_line = 0;
    }
    if (relevant_line) {
      relevant_line++;
    }
    if (relevant_line > 3) {
      if (line.find ("iframe") == std::string::npos) {
        filtered_lines.push_back (line);
      }
    }
    if (line.find ("\"vheading\"") != std::string::npos) {
      relevant_line++;
    }
  }
  
  html = filter::strings::implode (filtered_lines, "\n");
  
  html = filter::strings::replace ("/abbrev.htm", "http://biblehub.com/abbrev.htm", html);
  html = filter::strings::replace ("/hebrew/", "http://biblehub.com/hebrew/", html);
  html = filter::strings::replace ("/hebrewparse.htm", "http://biblehub.com/hebrewparse.htm", html);
  html = filter::strings::replace ("/greek/", "http://biblehub.com/greek/", html);
  html = filter::strings::replace ("/grammar/", "http://biblehub.com/grammar/", html);
  //html = filter::strings::replace ("height=\"165\"", "", html);
  html = filter::strings::replace ("height=\"160\"", "", html);
  html = filter::strings::replace ("height=\"145\"", "", html);
  html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity () + filter::strings::unicode_non_breaking_space_entity (), filter::strings::unicode_non_breaking_space_entity (), html);
  
  // Stylesheet for using web fonts,
  // because installing fonts on some tablets is very hard.
  const std::string stylesheet =
  "<style>\n"
  "span[class*='ref'] {\n"
  "display: none;\n"
  "}\n"
  "</style>\n";
  std::string output = stylesheet;
  // The following line prevents the Hebrew and Greek from floating around
  // the name of the Resource, which would disturb the order of the words in Hebrew.
  output += "<p></p>\n";
  output += html;
  
  return output;
}


// Filters the Scrivener Greek text from biblehub.com.
std::string resource_external_get_biblehub_scrivener (int book, int chapter, int verse)
{
  std::string bookname = resource_external_convert_book_biblehub (book);
  
  std::string url = "http://biblehub.com/text/" + bookname + "/" + std::to_string (chapter) + "-" + std::to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  std::string error;
  std::string html = resource_logic_web_or_cache_get (url, error);
  std::string tidy = filter::strings::html_tidy (html);
  std::vector <std::string> tidied = filter::strings::explode (tidy, '\n');

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
    if (line.find ("Scrivener") != std::string::npos) hits = 1;
    if (hits == 1) if (line.find ("greek") != std::string::npos) hits = 2;
    if ((hits == 2) || (hits == 3)) {
      html.append (line);
      hits++;
    }
  }
  
  if (html.empty ()) return html;
  
  const std::string stylesheet =
  "<style>\n"
  "</style>\n";
  
  return stylesheet + "\n" + html;
}


// This displays the Westminster Leningrad Codex from biblehub.com.
std::string resource_external_get_biblehub_westminster (int book, int chapter, int verse)
{
  // No New Testament in the Westminster Leningrad Codex.
  if (book >= 40) {
    return std::string();
  }
  
  std::string bookname = resource_external_convert_book_biblehub (book);
  
  // Sample URL:
  // http://biblehub.com/text/genesis/1-1.htm
  std::string url = "http://biblehub.com/text/" + bookname + "/" + std::to_string (chapter) + "-" + std::to_string (verse) + ".htm";
  
  // Get the html from the server, and tidy it up.
  std::string error;
  std::string html = resource_logic_web_or_cache_get (url, error);
  std::string tidy = filter::strings::html_tidy (html);
  std::vector <std::string> tidied = filter::strings::explode (tidy, '\n');
  
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
    if (line.find ("Westminster") != std::string::npos) hits = 1;
    if (hits == 1) if (line.find ("class=\"heb\"") != std::string::npos) hits = 2;
    if (hits == 2) {
      html.append (line);
      if (line.find ("</span>") != std::string::npos) hits = 0;
    }
  }
  
  if (html.empty ()) return html;
  
  // Change class "heb" to "hebrew", because that is what Bibledit uses for all Hebrew text.
  html = filter::strings::replace ("heb", "hebrew", html);
  
  
  // Stylesheet for using web fonts,
  // because installing fonts on some tablets is very hard.
  const std::string stylesheet =
  "<style>\n"
  "</style>\n";

  std::string output = stylesheet;
  
  // The following line prevents the Hebrew from floating around the name of the Resource,
  // which would disturb the order of the words.
  output += "<p></p>\n";
  
  output += html;

  return output;

  // In Job 24:14, it lacks a space between two words, making it hard to read.
  // In Job 31:36, one letter is joined to the wrong word.
}


// This displays the text and the notes of the NET Bible.
std::string resource_external_get_net_bible (int book, int chapter, int verse)
{
  std::string bookname = resource_external_convert_book_netbible (book);
  
  std::string url = bookname + " " + std::to_string (chapter) + ":" + std::to_string (verse);
  url = filter_url_urlencode (url);
  url.insert (0, "https://netbible.org/resource/netTexts/");
  
  std::string error;
  std::string text = resource_logic_web_or_cache_get (url, error);
  
  // Due to an error, the result could include PHP.
  // See https://github.com/bibledit/cloud/issues/579.
  // So if the text contains ".php", then there's that error.
  if (text.find(".php") != std::string::npos) text.clear();
  
  std::string output = text;
  
  url = bookname + " " + std::to_string (chapter) + ":" + std::to_string (verse);
  url = filter_url_urlencode (url);
  url.insert (0, "https://netbible.org/resource/netNotes/");
  
  std::string notes = resource_logic_web_or_cache_get (url, error);
  // If notes fail with an error, don't include the note text.
  if (!error.empty ()) notes.clear ();

  // It the verse contains no notes, the website returns an unusual message.
  if (notes.find ("We are currently offline") != std::string::npos) notes.clear ();

  // Deal with the following message:
  // Warning Message
  // An error was detected which prevented the loading of this page. If this problem persists, please contact the website administrator.
  // libraries/DB_Bible.php [780]:
  // Invalid argument supplied for foreach()
  // Loaded in 0.0303 seconds, using 0.59MB of memory. Generated by Kohana v2.3.4.
  // This error contains so much additional code, that the entire Bibledit program gets confused.
  if (notes.find ("Warning Message") != std::string::npos) notes.clear ();

  // The "bibleref" class experiences interference from other resources,
  // so that the reference would become invisible.
  // Remove this class, and the references will remain visible.
  notes = filter::strings::replace ("class=\"bibleref\"", "", notes);
  
  output += notes;
  
  return output;
}


// Blue Letter Bible.
std::string resource_external_get_blue_letter_bible (int book, int chapter, int verse)
{
  if (verse) {};
  
  std::string bookname = resource_external_convert_book_blueletterbible (book);
  
  std::string output;
  
  std::string url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=$" + std::to_string (chapter) + "&t=KJV&ss=1";
  
  output += "<a href=\"" + url + "\">KJV</a>";
  
  output += " | ";
  
  url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=" + std::to_string (chapter) + "&t=WLC";
  
  output += "<a href=\"" + url + "\">WLC</a>";
  
  output += " | ";
  
  url = "http://www.blueletterbible.org/Bible.cfm?b=" + filter_url_urlencode (bookname) + "&c=" + std::to_string (chapter) + "&t=mGNT";
  
  output += "<a href=\"" + url + "\">mGNT</a>";

  return output;
}


// This displays the text of the Elberfelder Bibel.
std::string resource_external_get_elberfelder_bibel (int book, int chapter, int verse)
{
  // The directory on bibleserver.com
  std::string directory = "ELB";
  return bibleserver_processor (directory, book, chapter, verse);
}


// The number of available external resource scripts.
unsigned int resource_external_count ()
{
  return sizeof (resource_table) / sizeof (*resource_table);
}


// Gets the names of all the known Web resources.
std::vector <std::string> resource_external_names ()
{
  std::vector <std::string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    names.push_back (resource_table [i].name);
  }
  sort (names.begin (), names.end ());
  return names;
}


// Get the names of the Web resources which are original language resources.
std::vector <std::string> resource_external_get_original_language_resources ()
{
  std::vector <std::string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    if (strcmp (resource_table [i].type, ORIGINAL) == 0) {
      names.push_back (resource_table [i].name);
    }
  }
  sort (names.begin (), names.end ());
  return names;
}


// Get the names of the Web resources which are Bibles and notes.
std::vector <std::string> resource_external_get_bibles ()
{
  std::vector <std::string> names;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    if (strcmp (resource_table [i].type, BIBLE) == 0) {
      names.push_back (resource_table [i].name);
    }
  }
  sort (names.begin (), names.end ());
  return names;
}


// Returns the versification for the resource.
std::string resource_external_versification (std::string name)
{
  std::string versification = filter::strings::english ();
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    std::string resource = resource_table [i].name;
    if (name == resource) {
      versification = resource_table [i].versification;
    }
  }
  return versification;
}


// Returns the versification for the resource.
std::string resource_external_mapping (std::string name)
{
  std::string versification = filter::strings::english ();
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    std::string resource = resource_table [i].name;
    if (name == resource) {
      versification = resource_table [i].mapping;
    }
  }
  return versification;
}


// Returns what type the Web resource $name is.
std::string resource_external_type (std::string name)
{
  std::string type = UNKNOWN;
  for (unsigned int i = 0; i < resource_external_count (); i++) {
    std::string resource = resource_table [i].name;
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
std::string resource_external_cloud_fetch_cache_extract (const std::string& name, int book, int chapter, int verse)
{
  std::string (* function_name) (int, int, int) {nullptr};

  for (unsigned int i {0}; i < resource_external_count (); i++) {
    std::string resource {resource_table [i].name};
    if (name == resource) {
      function_name = resource_table [i].func;
    }
  }
  
  if (function_name == nullptr) return std::string();

  std::string result = function_name (book, chapter, verse);
  
  return result;
}


std::string resource_external_convert_book_biblehub (int book)
{
  // Map Bibledit books to biblehub.com books.
  std::map <int, std::string> mapping = {
    std::pair (1, "genesis"),
    std::pair (2, "exodus"),
    std::pair (3, "leviticus"),
    std::pair (4, "numbers"),
    std::pair (5, "deuteronomy"),
    std::pair (6, "joshua"),
    std::pair (7, "judges"),
    std::pair (8, "ruth"),
    std::pair (9, "1_samuel"),
    std::pair (10, "2_samuel"),
    std::pair (11, "1_kings"),
    std::pair (12, "2_kings"),
    std::pair (13, "1_chronicles"),
    std::pair (14, "2_chronicles"),
    std::pair (15, "ezra"),
    std::pair (16, "nehemiah"),
    std::pair (17, "esther"),
    std::pair (18, "job"),
    std::pair (19, "psalms"),
    std::pair (20, "proverbs"),
    std::pair (21, "ecclesiastes"),
    std::pair (22, "songs"),
    std::pair (23, "isaiah"),
    std::pair (24, "jeremiah"),
    std::pair (25, "lamentations"),
    std::pair (26, "ezekiel"),
    std::pair (27, "daniel"),
    std::pair (28, "hosea"),
    std::pair (29, "joel"),
    std::pair (30, "amos"),
    std::pair (31, "obadiah"),
    std::pair (32, "jonah"),
    std::pair (33, "micah"),
    std::pair (34, "nahum"),
    std::pair (35, "habakkuk"),
    std::pair (36, "zephaniah"),
    std::pair (37, "haggai"),
    std::pair (38, "zechariah"),
    std::pair (39, "malachi"),
    std::pair (40, "matthew"),
    std::pair (41, "mark"),
    std::pair (42, "luke"),
    std::pair (43, "john"),
    std::pair (44, "acts"),
    std::pair (45, "romans"),
    std::pair (46, "1_corinthians"),
    std::pair (47, "2_corinthians"),
    std::pair (48, "galatians"),
    std::pair (49, "ephesians"),
    std::pair (50, "philippians"),
    std::pair (51, "colossians"),
    std::pair (52, "1_thessalonians"),
    std::pair (53, "2_thessalonians"),
    std::pair (54, "1_timothy"),
    std::pair (55, "2_timothy"),
    std::pair (56, "titus"),
    std::pair (57, "philemon"),
    std::pair (58, "hebrews"),
    std::pair (59, "james"),
    std::pair (60, "1_peter"),
    std::pair (61, "2_peter"),
    std::pair (62, "1_john"),
    std::pair (63, "2_john"),
    std::pair (64, "3_john"),
    std::pair (65, "jude"),
    std::pair (66, "revelation")
  };
  return mapping [book];
}


std::string resource_external_convert_book_netbible (int book)
{
  // Map Bibledit books to net.bible.com books as used at the web service.
  std::map <int, std::string> mapping = {
    std::pair (1, "Genesis"),
    std::pair (2, "Exodus"),
    std::pair (3, "Leviticus"),
    std::pair (4, "Numbers"),
    std::pair (5, "Deuteronomy"),
    std::pair (6, "Joshua"),
    std::pair (7, "Judges"),
    std::pair (8, "Ruth"),
    std::pair (9, "1 Samuel"),
    std::pair (10, "2 Samuel"),
    std::pair (11, "1 Kings"),
    std::pair (12, "2 Kings"),
    std::pair (13, "1 Chronicles"),
    std::pair (14, "2 Chronicles"),
    std::pair (15, "Ezra"),
    std::pair (16, "Nehemiah"),
    std::pair (17, "Esther"),
    std::pair (18, "Job"),
    std::pair (19, "Psalms"),
    std::pair (20, "Proverbs"),
    std::pair (21, "Ecclesiastes"),
    std::pair (22, "Song of Solomon"),
    std::pair (23, "Isaiah"),
    std::pair (24, "Jeremiah"),
    std::pair (25, "Lamentations"),
    std::pair (26, "Ezekiel"),
    std::pair (27, "Daniel"),
    std::pair (28, "Hosea"),
    std::pair (29, "Joel"),
    std::pair (30, "Amos"),
    std::pair (31, "Obadiah"),
    std::pair (32, "Jonah"),
    std::pair (33, "Micah"),
    std::pair (34, "Nahum"),
    std::pair (35, "Habakkuk"),
    std::pair (36, "Zephaniah"),
    std::pair (37, "Haggai"),
    std::pair (38, "Zechariah"),
    std::pair (39, "Malachi"),
    std::pair (40, "Matthew"),
    std::pair (41, "Mark"),
    std::pair (42, "Luke"),
    std::pair (43, "John"),
    std::pair (44, "Acts"),
    std::pair (45, "Romans"),
    std::pair (46, "1 Corinthians"),
    std::pair (47, "2 Corinthians"),
    std::pair (48, "Galatians"),
    std::pair (49, "Ephesians"),
    std::pair (50, "Philippians"),
    std::pair (51, "Colossians"),
    std::pair (52, "1 Thessalonians"),
    std::pair (53, "2 Thessalonians"),
    std::pair (54, "1 Timothy"),
    std::pair (55, "2 Timothy"),
    std::pair (56, "Titus"),
    std::pair (57, "Philemon"),
    std::pair (58, "Hebrews"),
    std::pair (59, "James"),
    std::pair (60, "1 Peter"),
    std::pair (61, "2 Peter"),
    std::pair (62, "1 John"),
    std::pair (63, "2 John"),
    std::pair (64, "3 John"),
    std::pair (65, "Jude"),
    std::pair (66, "Revelation")
  };
  return mapping [book];
}


std::string resource_external_convert_book_blueletterbible (int book)
{
  // This array maps the the book identifiers from Bibledit
  // to the book names as used by the blueletterbible.org web service.
  std::map <int, std::string> mapping = {
    std::pair (1, "Gen"),
    std::pair (2, "Exd"),
    std::pair (3, "Lev"),
    std::pair (4, "Num"),
    std::pair (5, "Deu"),
    std::pair (6, "Jos"),
    std::pair (7, "Jdg"),
    std::pair (8, "Rth"),
    std::pair (9, "1Sa"),
    std::pair (10, "2Sa"),
    std::pair (11, "1Ki"),
    std::pair (12, "2Ki"),
    std::pair (13, "1Ch"),
    std::pair (14, "2Ch"),
    std::pair (15, "Ezr"),
    std::pair (16, "Neh"),
    std::pair (17, "Est"),
    std::pair (18, "Job"),
    std::pair (19, "Psa"),
    std::pair (20, "Pro"),
    std::pair (21, "Ecc"),
    std::pair (22, "Sgs"),
    std::pair (23, "Isa"),
    std::pair (24, "Jer"),
    std::pair (25, "Lam"),
    std::pair (26, "Eze"),
    std::pair (27, "Dan"),
    std::pair (28, "Hsa"),
    std::pair (29, "Joe"),
    std::pair (30, "Amo"),
    std::pair (31, "Oba"),
    std::pair (32, "Jon"),
    std::pair (33, "Mic"),
    std::pair (34, "Nah"),
    std::pair (35, "Hab"),
    std::pair (36, "Zep"),
    std::pair (37, "Hag"),
    std::pair (38, "Zec"),
    std::pair (39, "Mal"),
    std::pair (40, "Mat"),
    std::pair (41, "Mar"),
    std::pair (42, "Luk"),
    std::pair (43, "Jhn"),
    std::pair (44, "Act"),
    std::pair (45, "Rom"),
    std::pair (46, "1Cr"),
    std::pair (47, "2Cr"),
    std::pair (48, "Gal"),
    std::pair (49, "Eph"),
    std::pair (50, "Phl"),
    std::pair (51, "Col"),
    std::pair (52, "1Th"),
    std::pair (53, "2Th"),
    std::pair (54, "1Ti"),
    std::pair (55, "2Ti"),
    std::pair (56, "Tts"),
    std::pair (57, "Phm"),
    std::pair (58, "Hbr"),
    std::pair (59, "Jam"),
    std::pair (60, "1Pe"),
    std::pair (61, "2Pe"),
    std::pair (62, "1Jo"),
    std::pair (63, "2Jo"),
    std::pair (64, "3Jo"),
    std::pair (65, "Jud"),
    std::pair (66, "Rev")
  };
  return mapping [book];
}


std::string resource_external_convert_book_bibleserver (int book)
{
  // On bibleserver.com, Genesis is "1.Mose", Exodus is "2.Mose", and so on.
  std::map <int, std::string> mapping = {
    std::pair (1, "1.Mose"),
    std::pair (2, "2.Mose"),
    std::pair (3, "3.Mose"),
    std::pair (4, "4.Mose"),
    std::pair (5, "5.Mose"),
    std::pair (6, "Josua"),
    std::pair (7, "Richter"),
    std::pair (8, "Rut"),
    std::pair (9, "1.Samuel"),
    std::pair (10, "2.Samuel"),
    std::pair (11, "1.Könige"),
    std::pair (12, "2.Könige"),
    std::pair (13, "1.Chronik"),
    std::pair (14, "2.Chronik"),
    std::pair (15, "Esra"),
    std::pair (16, "Nehemia"),
    std::pair (17, "Esther"),
    std::pair (18, "Hiob"),
    std::pair (19, "Psalm"),
    std::pair (20, "Sprüche"),
    std::pair (21, "Prediger"),
    std::pair (22, "Hoheslied"),
    std::pair (23, "Jesaja"),
    std::pair (24, "Jeremia"),
    std::pair (25, "Klagelieder"),
    std::pair (26, "Hesekiel"),
    std::pair (27, "Daniel"),
    std::pair (28, "Hosea"),
    std::pair (29, "Joel"),
    std::pair (30, "Amos"),
    std::pair (31, "Obadja"),
    std::pair (32, "Jona"),
    std::pair (33, "Micha"),
    std::pair (34, "Nahum"),
    std::pair (35, "Habakuk"),
    std::pair (36, "Zefanja"),
    std::pair (37, "Haggai"),
    std::pair (38, "Sacharja"),
    std::pair (39, "Maleachi"),
    std::pair (40, "Matthäus"),
    std::pair (41, "Markus"),
    std::pair (42, "Lukas"),
    std::pair (43, "Johannes"),
    std::pair (44, "Apostelgeschichte"),
    std::pair (45, "Römer"),
    std::pair (46, "1.Korinther"),
    std::pair (47, "2.Korinther"),
    std::pair (48, "Galater"),
    std::pair (49, "Epheser"),
    std::pair (50, "Philipper"),
    std::pair (51, "Kolosser"),
    std::pair (52, "1.Thessalonicher"),
    std::pair (53, "2.Thessalonicher"),
    std::pair (54, "1.Timotheus"),
    std::pair (55, "2.Timotheus"),
    std::pair (56, "Titus"),
    std::pair (57, "Philemon"),
    std::pair (58, "Hebräer"),
    std::pair (59, "Jakobus"),
    std::pair (60, "1.Petrus"),
    std::pair (61, "2.Petrus"),
    std::pair (62, "1.Johannes"),
    std::pair (63, "2.Johannes"),
    std::pair (64, "3.Johannes"),
    std::pair (65, "Judas"),
    std::pair (66, "Offenbarung")
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

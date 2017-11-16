/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <unittests/text.h>
#include <unittests/utilities.h>
#include <database/config/bible.h>
#include <filter/text.h>
#include <filter/url.h>
#include <filter/string.h>


void test_text ()
{
  trace_unit_tests (__func__);
  
  string TextTestOdt  = "/tmp/TextTest.odt";
  string TextTestHtml = "/tmp/TextTest.html";
  string TextTestTxt  = "/tmp/TextTest.txt";
  
  string bible = "phpunit";
  
  // The unittests depend on known settings.
  Database_Config_Bible::setExportChapterDropCapsFrames (bible, true);
  
  // Test extraction of all sorts of information from USFM code.
  // Test basic formatting into OpenDocument.
  {
    string usfm = ""
    "\\id GEN\n"
    "\\h Header\n"
    "\\h1 Header1\n"
    "\\h2 Header2\n"
    "\\h3 Header3\n"
    "\\toc1 The Book of Genesis\n"
    "\\toc2 Genesis\n"
    "\\toc3 Gen\n"
    "\\cl Chapter\n"
    "\\c 1\n"
    "\\cp Ⅰ\n"
    "\\p\n"
    "\\v 1 Text chapter 1\n"
    "\\c 2\n"
    "\\cp ②\n"
    "\\h Header4\n"
    "\\p\n"
    "\\v 2 Text chapter 2\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    // Check that it finds the running headers.
    int desiredRunningHeaders = 5;
    int actualRunningHeaders = (int)filter_text.runningHeaders.size();
    evaluate (__LINE__, __func__, desiredRunningHeaders, actualRunningHeaders);
    if (actualRunningHeaders == desiredRunningHeaders) {
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[0].verse);
      evaluate (__LINE__, __func__, "h", filter_text.runningHeaders[0].marker);
      evaluate (__LINE__, __func__, "Header", filter_text.runningHeaders[0].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[1].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[1].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[1].verse);
      evaluate (__LINE__, __func__, "h1", filter_text.runningHeaders[1].marker);
      evaluate (__LINE__, __func__, "Header1", filter_text.runningHeaders[1].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[2].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[2].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[2].verse);
      evaluate (__LINE__, __func__, "h2", filter_text.runningHeaders[2].marker);
      evaluate (__LINE__, __func__, "Header2", filter_text.runningHeaders[2].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[3].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[3].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[3].verse);
      evaluate (__LINE__, __func__, "h3", filter_text.runningHeaders[3].marker);
      evaluate (__LINE__, __func__, "Header3", filter_text.runningHeaders[3].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[4].book);
      evaluate (__LINE__, __func__, 2, filter_text.runningHeaders[4].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[4].verse);
      evaluate (__LINE__, __func__, "h", filter_text.runningHeaders[4].marker);
      evaluate (__LINE__, __func__, "Header4", filter_text.runningHeaders[4].value);
    }

    // Check table of contents items.
    int desiredlongTOCs = 1;
    int actuallongTOCs = filter_text.longTOCs.size();
    evaluate (__LINE__, __func__, desiredlongTOCs, actuallongTOCs);
    if (desiredlongTOCs == actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.longTOCs[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.longTOCs[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.longTOCs[0].verse);
      evaluate (__LINE__, __func__, "toc1", filter_text.longTOCs[0].marker);
      evaluate (__LINE__, __func__, "The Book of Genesis", filter_text.longTOCs[0].value);
    }
    int desiredshortTOCs = 1;
    int actualshortTOCs = filter_text.shortTOCs.size();
    evaluate (__LINE__, __func__, desiredshortTOCs, actualshortTOCs);
    if (desiredlongTOCs == actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.shortTOCs[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.shortTOCs[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.shortTOCs[0].verse);
      evaluate (__LINE__, __func__, "toc2", filter_text.shortTOCs[0].marker);
      evaluate (__LINE__, __func__, "Genesis", filter_text.shortTOCs[0].value);
    }

    // Check book abbreviation.
    int desiredbookAbbreviations = 1;
    int actualbookAbbreviations = filter_text.bookAbbreviations.size();
    evaluate (__LINE__, __func__, desiredbookAbbreviations, actualbookAbbreviations);
    if (desiredlongTOCs == actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.bookAbbreviations[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.bookAbbreviations[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.bookAbbreviations[0].verse);
      evaluate (__LINE__, __func__, "toc3", filter_text.bookAbbreviations[0].marker);
      evaluate (__LINE__, __func__, "Gen", filter_text.bookAbbreviations[0].value);
    }
    
    // Check chapter specials.
    int desiredchapterLabels = 1;
    int actualchapterLabels = filter_text.chapterLabels.size();
    evaluate (__LINE__, __func__, desiredchapterLabels, actualchapterLabels);
    if (desiredlongTOCs == actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.chapterLabels[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.chapterLabels[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.chapterLabels[0].verse);
      evaluate (__LINE__, __func__, "cl", filter_text.chapterLabels[0].marker);
      evaluate (__LINE__, __func__, "Chapter", filter_text.chapterLabels[0].value);
    }
    int desiredpublishedChapterMarkers = 2;
    int actualpublishedChapterMarkers = filter_text.publishedChapterMarkers.size();
    evaluate (__LINE__, __func__, desiredpublishedChapterMarkers, actualpublishedChapterMarkers);
    if (desiredlongTOCs == actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[0].book);
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.publishedChapterMarkers[0].verse);
      evaluate (__LINE__, __func__, "cp", filter_text.publishedChapterMarkers[0].marker);
      evaluate (__LINE__, __func__, "Ⅰ", filter_text.publishedChapterMarkers[0].value);
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[1].book);
      evaluate (__LINE__, __func__, 2, filter_text.publishedChapterMarkers[1].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.publishedChapterMarkers[1].verse);
      evaluate (__LINE__, __func__, "cp", filter_text.publishedChapterMarkers[1].marker);
      evaluate (__LINE__, __func__, "②", filter_text.publishedChapterMarkers[1].value);
    }
    
    // OpenDocument output.
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Header4\n"
    "=======\n"
    "\n"
    "Header4 Ⅰ\n"
    "=========\n"
    "\n"
    "[-- Image: frame1 --]\n"
    "\n"
    "Ⅰ\n"
    "\n"
    "Text chapter 1\n"
    "\n"
    "Header4 ②\n"
    "=========\n"
    "\n"
    "[-- Image: frame2 --]\n"
    "\n"
    "②\n"
    "\n"
    "Text chapter 2\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (TextTestOdt);
  filter_url_unlink (TextTestHtml);

  // There are two books here.
  // This normally gives one new page between these two books.
  // Test that basic USFM code gets transformed correctly.
  {
    string usfm =
    "\\id GEN\n"
    "\\ide XYZ\n"
    "\\c 1\n"
    "\\p Text Genesis 1\n"
    "\\c 2\n"
    "\\p Text Genesis 2\n"
    "\\id MAT\n"
    "\\c 1\n"
    "\\p Text Matthew 1\n"
    "\\c 2\n"
    "\\p Text Matthew 2\n"
    "\\rem Comment\n"
    "\\xxx Unknown markup\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "Genesis 1\n"
    "=========\n"
    "\n"
    "Text Genesis 1\n"
    "\n"
    "Genesis 2\n"
    "=========\n"
    "\n"
    "Text Genesis 2\n"
    "\n"
    "Matthew\n"
    "=======\n"
    "\n"
    "Matthew 1\n"
    "=========\n"
    "\n"
    "Text Matthew 1\n"
    "\n"
    "Matthew 2\n"
    "=========\n"
    "\n"
    "Text Matthew 2\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
    evaluate (__LINE__, __func__, {"Matthew 2:0 Unknown marker \\xxx, formatting error: Unknown markup"}, filter_text.fallout);
    evaluate (__LINE__, __func__, {"Genesis 0:0 Text encoding: \\ide XYZ", "Matthew 2:0 Comment: \\rem Comment"}, filter_text.info);
  }

  
  // Test multiple books in one OpenDocument file.
  // The headers of each new book should be correct.
  {
    string directory = filter_url_create_root_path ("unittests", "tests");
    string usfm_ruth = filter_url_file_get_contents (filter_url_create_path (directory, "08-Ruth.usfm"));
    string usfm_1_peter = filter_url_file_get_contents (filter_url_create_path (directory, "60-1Peter.usfm"));
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm_ruth);
    filter_text.addUsfmCode (usfm_1_peter);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Ruth\n"
    "====\n"
    "\n"
    "The Book of\n"
    "\n"
    "Ruth\n"
    "\n"
    "Ruth 1\n"
    "======\n"
    "\n"
    "1 In the days when the judges judged, there was a famine in the\n"
    "land. A certain man of Bethlehem Judah went to live in the\n"
    "country of Moab with his wife and his two sons.\n"
    "\n"
    "1 Peter\n"
    "=======\n"
    "\n"
    "Peter’s First Letter\n"
    "\n"
    "1 Peter 1\n"
    "=========\n"
    "\n"
    "1 Peter, an apostle of Jesus Christ, to the chosen ones who are\n"
    "living as foreigners in the Dispersion in Pontus, Galatia,\n"
    "Cappadocia, Asia, and Bithynia,\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test transformation of verse numbers and text following.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Verse One.\n"
    "\\p Paragraph One.\n"
    "\\v 2 Verse Two.\n"
    "\\p\n"
    "\\v 3 Verse Three.\n"
    "\\v 4 Verse Four.\n"
    "\\v 5 Verse Five.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "1" + en_space_u2002 () + "Verse One.\n"
    "\n"
    "Paragraph One. 2" + en_space_u2002 () + "Verse Two.\n"
    "\n"
    "3" + en_space_u2002 () + "Verse Three. 4" + en_space_u2002 () + "Verse Four. 5" + en_space_u2002 () + "Verse Five.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test footnotes and cross references.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Text 1\\x + \\xt Isa. 1.1.\\x*\\x - \\xt Isa. 2.2.\\x*\\x + \\xt Isa. 3.3.\\x*, text 2\\f + \\fk Word1: \\fl Heb. \\fq Explanation1.\\f*\\f + \\fk Word2: \\fl Heb. \\fq Explanation2.\\f*, text3.\\f + \\fk Test: \\fl Heb. \\fq Note at the very end.\\f*\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "1" + en_space_u2002 () + "Text 1a\n"
    "\n"
    "Isa. 1.1.\n"
    "\n"
    "Isa. 2.2.\n"
    "\n"
    "b\n"
    "\n"
    "Isa. 3.3.\n"
    "\n"
    ", text 21\n"
    "\n"
    "Word1: Heb. Explanation1.\n"
    "\n"
    "2\n"
    "\n"
    "Word2: Heb. Explanation2.\n"
    "\n"
    ", text3.3\n"
    "\n"
    "Test: Heb. Note at the very end.\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test clear text export.
  {
    string usfm =
    "\\id GEN\n"
    "\\h Genesis\n"
    "\\toc2 Genesis\n"
    "\\mt2 The book of\n"
    "\\mt Genesis\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 In the be\\x + \\xt Isa. 1.1.\\x*\\x - \\xt Isa. 2.2.\\x*ginning, God created\\f + \\fk Word1: \\fl Heb. \\fq Explanation1.\\f*\\f + \\fk Word2: \\fl Heb. \\fq Explanation2.\\f* the heavens and the earth.\\f + \\fk Test: \\fl Heb. \\fq Note at the very end.\\f*\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.text_text->get ();
    string notes = filter_text.text_text->getnote ();
    string standard =
    "The book of\n"
    "Genesis\n"
    "1\n"
    "1 In the beginning, God created the heavens and the earth.";
    evaluate (__LINE__, __func__, standard, output);
    string standardnotes =
    "Isa. 1.1.\n"
    "Isa. 2.2.\n"
    "Word1: Heb. Explanation1.\n"
    "Word2: Heb. Explanation2.\n"
    "Test: Heb. Note at the very end.";
    evaluate (__LINE__, __func__, standardnotes, notes);
  }
  
  // Test clear text export.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Chapter 1, verse one.\n"
    "\\v 2 Verse two.\n"
    "\\c 2\n"
    "\\p\n"
    "\\v 1 Chapter 2, verse one.\n"
    "\\v 2 Verse two.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.text_text->get ();
    string notes = filter_text.text_text->getnote ();
    string standard =
    "1\n"
    "1 Chapter 1, verse one. 2 Verse two.\n"
    "2\n"
    "1 Chapter 2, verse one. 2 Verse two.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (output));
    evaluate (__LINE__, __func__, "", notes);
  }
  
  // Test verses headings.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse one.\n"
    "\\v 2 Verse two.\n"
    "\\s Heading one\n"
    "\\p\n"
    "\\v 3 Verse three\n"
    "\\p\n"
    "\\s Heading two\n"
    "\\p\n"
    "\\v 4 Verse four.\n"
    "\\v 5 Verse five.\n"
    "\\c 2\n"
    "\\s Heading three\n"
    "\\p\n"
    "\\v 1 Verse one.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.verses_headings;
    map <int, string> standard = { {0, "Heading three"}, {2, "Heading one"}, {3, "Heading two"} };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses headings.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse one.\n"
    "\\s \\s Usuku lweN\\nd kosi\\nd* luyeza masinyane\n"
    "\\p\n"
    "\\v 2 Verse two\n"
    "\\p\n"
    "\\s Heading \\add two\\add*\n"
    "\\p\n"
    "\\v 3 Verse three\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.verses_headings;
    map <int, string> standard = { {1, "Usuku lweNkosi luyeza masinyane"}, {2, "Heading two"} };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses text.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Verse\\x + \\xt Isa. 1.1.\\x* two.\n"
    "\\v 3 Verse three\\x + \\xt Isa. 1.1.\\x*.\n"
    "\\s Heading one\n"
    "\\p\n"
    "\\v 4 Verse four.\n"
    "\\p\n"
    "\\s Heading two\n"
    "\\p\n"
    "\\v 5 Verse five.\n"
    "\\v 6 Verse six.\n"
    "\\c 2\n"
    "\\s Heading three\n"
    "\\p\n"
    "\\v 1 Verse one\\x + \\xt Isa. 1.1.\\x*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "Verse one."},
      {2, "Verse two."},
      {3, "Verse three."},
      {4, "Verse four."},
      {5, "Verse five."},
      {6, "Verse six."},
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses text.
  {
    string usfm =
    "\\c 15\n"
    "\\s Heading\n"
    "\\p\n"
    "\\v 1 He said:\n"
    "\\p I will sing to the Lord.\n"
    "\\v 2 The Lord is my strength.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "He said: I will sing to the Lord."},
      {2, "The Lord is my strength." }
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test paragraph starting markers.
  {
    string usfm =
    "\\c 1\n"
    "\\s Heading\n"
    "\\p\n"
    "\\v 1 He said:\n"
    "\\q1 I will sing to the Lord.\n"
    "\\v 2 The Lord is my strength.\n"
    "\\q2 I trust in Him.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    evaluate (__LINE__, __func__, {"p", "q1", "q2"}, filter_text.paragraph_starting_markers);
  }
  
  // Test improved paragraph detection.
  {
    string path = filter_url_create_root_path ("unittests", "tests", "ShonaNumbers23.usfm");
    string usfm = filter_url_file_get_contents (path);
    Filter_Text filter_text = Filter_Text ("");
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    for (size_t i = 0; i < filter_text.verses_paragraphs.size (); i++) {
      map <int, string> verses_paragraph = filter_text.verses_paragraphs [i];
      map <int, string> standard;
      if (i == 0) {
        standard = {
          make_pair (1, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, ugondigadzirire pano nzombe nomwe nemakondohwe manomwe."),
          make_pair (2, "Bharaki ndokuita Bharamu sezvakataura; Bharaki naBharamu ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
          make_pair (3, "Zvino Bharamu wakati kuna Bharaki: Mira pachipo chako chinopiswa, asi ini ndichaenda; zvimwe Jehovha achauya kuzosangana neni; zvino chinhu chaanondiratidza ndichakuudza. Zvino wakaenda pakakwirira."),
          make_pair (4, "Zvino Mwari wakasangana naBharamu, iye ndokuti kwaari: Ndagadzira aritari nomwe uye ndapa nzombe diki negondohwe paaritari imwe neimwe. "),
          make_pair (5, "Jehovha ndokuisa shoko mumuromo waBharamu, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
          make_pair (6, "Zvino wakadzokera kwaari, uye tarira, amire pachipo chake chinopiswa, iye nemachinda ese aMoabhu."),
          make_pair (7, "Zvino wakasimudza mufananidzo wake ndokuti: Bharaki mambo waMoabhu wakandivinga kubva Siriya, pamakomo ekumabvazuva achiti: Uya nditukire Jakove, uye uya unyombe Israeri."),
          make_pair (8, "Ini ndingatuka sei Mwari waasina kutuka; uye ndinganyomba sei Jehovha waasina kunyomba?"),
          make_pair (9, "Nokuti kubva panhongonya yemabwe ndinomuona, uye kubva pazvikomo ndinomutarisa; tarira, vanhu vachagara vega, uye havangaverengwi pakati pendudzi."),
          make_pair (10, "Ndiani angaverenga guruva raJakove, nekuverenga chechina chaIsraeri? Mweya wangu ngaufe rufu rwevakarurama, uye magumo angu ngaave seake!"),
          make_pair (11, "Zvino Bharaki wakati kuna Bharamu: Wandiitei? Ndakutora kuti utuke vavengi vangu; zvino tarira, wavaropafadza nekuvaropafadza."),
          make_pair (12, "Zvino wakapindura ndokuti: Handifaniri kuchenjerera kutaura izvo Jehovha zvaanoisa mumuromo mangu here?"),
          make_pair (13, "Zvino Bharaki wakati kwaari: Uya hako neni kune imwe nzvimbo paungavaona kubvapo; uchaona kuguma kwavo chete, uye haungavaoni vese; ugonditukira ivo kubva ipapo.")
        };
      }
      if (i == 1) {
        standard = {
          make_pair (14, "Zvino wakamutora akamuisa mumunda weZofimu, panhongonya yePisiga, ndokuvaka aritari nomwe, ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
          make_pair (15, "Zvino wakati kuna Bharaki: Mira pano pachipo chako chinopiswa, zvino ini ndichasangana naJehovha uko."),
          make_pair (16, "Zvino Jehovha wakasangana naBharamu, ndokuisa shoko mumuromo make, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
          make_pair (17, "Zvino asvika kwaari, tarira, amire pachipo chake chinopiswa, nemachinda aMoabhu anaye. Bharaki ndokuti kwaari: Jehovha wataurei?"),
          make_pair (18, "Zvino wakasimudza mufananidzo wake ndokuti: Simuka Bharaki, ugonzwa; urereke nzeve kwandiri, iwe mwanakomana waZipori:"),
          make_pair (19, "Mwari haasi munhu kuti areve nhema, kana mwanakomana wemunhu kuti ashanduke. Iye wakareva, haangaiti kanhi? Kana wakareva, haangasimbisi kanhi?"),
          make_pair (20, "Tarira, ndagamuchira kuti ndiropafadze; zvino waropafadza, uye handigoni kuzvidzosa."),
          make_pair (21, "Haana kutarisira chakaipa kuna Jakove; uye haana kuona kutsauka kuna Israeri; Jehovha Mwari wake anaye, nekudanidzira kwamambo kuri pakati pavo."),
          make_pair (22, "Mwari wakavabudisa Egipita, ane sesimba renyati."),
          make_pair (23, "Zvirokwazvo hakuna un'anga hunopikisa Jakove, kana kuvuka kunopikisa Israeri. Ikozvino zvicharehwa pamusoro paJakove nepamusoro paIsraeri kuti Mwari wakazviita."),
          make_pair (24, "Tarirai, vanhu vachasimuka seshumbakadzi, vazvisimudze seshumba. Havangarari pasi kusvika vadya chakajimbiwa, uye vanwa ropa revakaurawa.")
        };
      }
      if (i == 2) {
        standard = {
          make_pair (25, "Zvino Bharaki wakati kuna Bharamu: Zvino usatongovatuka kana kutongovaropafadza."),
          make_pair (26, "Asi Bharamu wakapindura, ndokuti kuna Bharaki: Handina kukuudza here ndichiti: Zvese Jehovha zvaanotaura, ndizvo zvandichaita?")
        };
      }
      if (i == 3) {
        standard = {
          make_pair (27, "Zvino Bharaki wakati kuna Bharamu: Uya hako, ndichakuendesa kune imwe nzvimbo; zvimwe zvingarurama pameso aMwari kuti unditukire ivo kubva ipapo."),
          make_pair (28, "Zvino Bharaki wakatora ndokuisa Bharamu panhongonya yePeori, pakatarisa pasi pachiso cherenje."),
          make_pair (29, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, undigadzirire pano nzombe diki nomwe nemakondohwe manomwe."),
          make_pair (30, "Bharaki ndokuita Bharamu sezvaakataura, ndokupa nzombe diki negondohwe paaritari imwe neimwe.")
        };
      }
      evaluate (__LINE__, __func__, standard, verses_paragraph);
    }
    evaluate (__LINE__, __func__, {"p", "p", "p", "p"}, filter_text.paragraph_starting_markers);
  }
  
  // Test embedded text.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 He said: I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n"
    "\\v 2 The \\nd Lord\\nd* is my strength.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "He said: I will sing to the Lord."},
      {2, "The Lord is my strength."}
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test embedded html.
  {
    // Open character style, and embedded character style, and close both normally.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded html.
  {
    // Open character style, open embedded character style, close embedded one, then close the outer one.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span class=\"add\"> God</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded html.
  {
    // Open character style, open embedded character style, then closing the outer one closes the embedded one also.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded OpenDocument.
  {
    // Open character style, and embedded character style, and close both normally.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "Genesis 1\n"
    "=========\n"
    "\n"
    "1 I will sing to the Lord.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Exercise bits in document to generate text and note citations.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Text 1\\x + \\xt Isa. 1.1.\\x* text\\f + \\fk Word: \\fl Heb. \\fq Explanation1.\\f* text\\fe + \\fk Word: \\fl Heb. \\fq Explanation1.\\fe*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_text_and_note_citations = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_text_and_note_citations->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "1 Text 1a text1 text1.";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test embedded OpenDocument.
  {
    // Open character style, open embedded character style, close embedded one, then close the outer one.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "Genesis 1\n"
    "=========\n"
    "\n"
    "1 I will sing to the Lord God.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test embedded OpenDocument.
  {
    // Open character style, open embedded character style, then closing the outer one closes the embedded one also.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    string command = "odt2txt " + TextTestOdt + " > " + TextTestTxt;
    int ret = system (command.c_str());
    string odt;
    if (ret == 0) odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "=======\n"
    "\n"
    "Genesis 1\n"
    "=========\n"
    "\n"
    "1 I will sing to the Lord.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  filter_url_unlink (TextTestOdt);
  filter_url_unlink (TextTestHtml);
  filter_url_unlink (TextTestTxt);
  refresh_sandbox (true);
}

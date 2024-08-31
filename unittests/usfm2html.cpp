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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/state.h>
#include <editor/usfm2html.h>
#include <styles/logic.h>
#include <filter/string.h>
#include <webserver/request.h>


TEST (editor, usfm2html)
{
  Database_State::create ();

  // Test text length for one verse.
  {
    std::string usfm =
    "\\c 2\n"
    "\\p\n"
    "\\v 1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho\\x + Dute. 4.19. Hlab. 33.6.\\x*.\n";
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    EXPECT_EQ (61, static_cast<int>(editor_usfm2html.m_text_tength));
    EXPECT_EQ ((std::map <int, int>{ std::pair (0, 0), std::pair (1, 2) }), editor_usfm2html.m_verse_start_offsets);
    EXPECT_EQ ("1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho.", editor_usfm2html.m_current_paragraph_content);
  }

  // Test text length for several verses.
  {
    std::string usfm =
    "\\c 2\n"
    "\\p\n"
    "\\v 1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho\\x + Dute. 4.19. Hlab. 33.6.\\x*.\n"
    "\\v 2 UNkulunkulu wasewuqeda ngosuku lwesikhombisa umsebenzi wakhe abewenza. Waphumula ngosuku lwesikhombisa\\x + Eks. 20.11; 31.17. Dute. 5.14. Heb. 4.4.\\x* emsebenzini wakhe wonke abewenza.\n"
    "\\v 3 UNkulunkulu wasebusisa usuku lwesikhombisa, walungcwelisa; ngoba ngalo waphumula emsebenzini wakhe wonke, uNkulunkulu awudalayo wawenza\\f + \\fk wawenza: \\fl Heb. \\fq ukuwenza.\\f*.\n"
    "\\s Isivande seEdeni\n"
    "\\p\n"
    "\\v 4 Lezi yizizukulwana zamazulu lezomhlaba ekudalweni kwakho\\x + 1.1.\\x*, mhla iN\\nd kosi\\nd* uNkulunkulu isenza umhlaba lamazulu,\n"
    "\\v 5 laso sonke isihlahlakazana sensimu, singakabi khona emhlabeni, layo yonke imibhida yeganga\\x + 1.12.\\x*, ingakamili; ngoba iN\\nd kosi\\nd* uNkulunkulu yayinganisanga izulu emhlabeni, njalo kwakungelamuntu wokulima umhlabathi.\n"
    "\\v 6 Kodwa kwenyuka inkungu ivela emhlabathini, yasithelela ubuso bonke bomhlabathi.\n"
    "\\v 7 IN\\nd kosi\\nd* uNkulunkulu yasibumba umuntu ngothuli oluvela emhlabathini\\x + 3.19,23. Hlab. 103.14. Tshu. 12.7. 1 Kor. 15.47.\\x*, yaphefumulela emakhaleni akhe umoya wempilo; umuntu wasesiba ngumphefumulo ophilayo\\x + 7.22. Jobe 33.4. Isa. 2.22. 1 Kor. 15.45.\\x*.\n";
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    EXPECT_EQ (913, static_cast<int>(editor_usfm2html.m_text_tength));
    EXPECT_EQ ((std::map <int, int>{ std::pair (0, 0),
      std::pair (1, 2),
      std::pair (2, 62),
      std::pair (3, 202),
      std::pair (4, 359),
      std::pair (5, 469),
      std::pair (6, 676),
      std::pair (7, 758) }),
               editor_usfm2html.m_verse_start_offsets);
    EXPECT_EQ (550, static_cast<int>(editor_usfm2html.m_current_paragraph_content.size ()));
  }

  // Space after starting marker
  {
    std::string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Text \\add of the \\add*1st\\add  second verse\\add*.\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    std::string html = editor_usfm2html.get ();
    std::string standard =
    R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st</span><span class="i-add"> second verse</span><span>.</span></p>)";
    EXPECT_EQ (standard, html);
  }

  // Apostrophy etc.
  {
    std::string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Judha muranda waJesu Kristu, uye munin'ina waJakobho ...\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    std::string html = editor_usfm2html.get ();
    std::string standard =
    R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Judha muranda waJesu Kristu, uye munin'ina waJakobho ...</span></p>)";
    EXPECT_EQ (standard, html);
  }

  // Most recent paragraph style.
  {
    std::string usfm =
    "\\c 2\n"
    "\\p\n"
    "\\v 1 One\n"
    "\\q2\n"
    "\\v 2 Two\n"
    "\\v 3 Three\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    EXPECT_EQ ("q2", editor_usfm2html.m_current_paragraph_style);
    EXPECT_EQ ("2 Two 3 Three", editor_usfm2html.m_current_paragraph_content);
  }

  // Most recent paragraph style and length 0.
  {
    std::string usfm =
    "\\c 2\n"
    "\\p\n"
    "\\v 1 One\n"
    "\\q2\n"
    "\\v 2 Two\n"
    "\\v 3 Three\n"
    "\\q3\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    EXPECT_EQ ("q3", editor_usfm2html.m_current_paragraph_style);
    EXPECT_EQ ("", editor_usfm2html.m_current_paragraph_content);
  }

  // Convert styles for Quill-based editor.
  {
    std::string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Text \\add of the \\add*1st \\add second verse\\add*.\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    std::string html = editor_usfm2html.get ();
    std::string standard =
    R"(<p class="b-c"><span>1</span></p>)"
    R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st </span><span class="i-add">second verse</span><span>.</span></p>)";
    EXPECT_EQ (standard, html);
  }

}

#endif


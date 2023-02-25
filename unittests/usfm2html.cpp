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


#include <unittests/usfm2html.h>
#include <unittests/utilities.h>
#include <database/state.h>
#include <editor/usfm2html.h>
#include <styles/logic.h>
#include <filter/string.h>
#include <webserver/request.h>
using namespace std;


void test_usfm2html ()
{
  trace_unit_tests (__func__);
  Database_State::create ();

  // Test text length for one verse.
  {
    string usfm =
    "\\c 2\n"
    "\\p\n"
    "\\v 1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho\\x + Dute. 4.19. Hlab. 33.6.\\x*.\n";
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    evaluate (__LINE__, __func__, 61, static_cast<int>(editor_usfm2html.textLength));
    evaluate (__LINE__, __func__,  { pair (0, 0), pair (1, 2) }, editor_usfm2html.verseStartOffsets);
    evaluate (__LINE__, __func__, "1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho.", editor_usfm2html.currentParagraphContent);
  }

  // Test text length for several verses.
  {
    string usfm =
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
    evaluate (__LINE__, __func__, 913, static_cast<int>(editor_usfm2html.textLength));
    evaluate (__LINE__, __func__, { pair (0, 0),
      pair (1, 2),
      pair (2, 62),
      pair (3, 202),
      pair (4, 359),
      pair (5, 469),
      pair (6, 676),
      pair (7, 758) },
              editor_usfm2html.verseStartOffsets);
    evaluate (__LINE__, __func__, 550, static_cast<int>(editor_usfm2html.currentParagraphContent.size ()));
  }

  // Space after starting marker
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Text \\add of the \\add*1st\\add  second verse\\add*.\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    string html = editor_usfm2html.get ();
    string standard =
    R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st</span><span class="i-add"> second verse</span><span>.</span></p>)";
    evaluate (__LINE__, __func__, standard, html);
  }

  // Apostrophy etc.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Judha muranda waJesu Kristu, uye munin'ina waJakobho ...\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    string html = editor_usfm2html.get ();
    string standard =
    R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Judha muranda waJesu Kristu, uye munin'ina waJakobho ...</span></p>)";
    evaluate (__LINE__, __func__, standard, html);
  }

  // Most recent paragraph style.
  {
    string usfm =
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
    evaluate (__LINE__, __func__, "q2", editor_usfm2html.currentParagraphStyle);
    evaluate (__LINE__, __func__, "2 Two 3 Three", editor_usfm2html.currentParagraphContent);
  }

  // Most recent paragraph style and length 0.
  {
    string usfm =
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
    evaluate (__LINE__, __func__, "q3", editor_usfm2html.currentParagraphStyle);
    evaluate (__LINE__, __func__, "", editor_usfm2html.currentParagraphContent);
  }

  // Convert styles for Quill-based editor.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Text \\add of the \\add*1st \\add second verse\\add*.\n";
    Webserver_Request request;
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
    editor_usfm2html.run ();
    string html = editor_usfm2html.get ();
    string standard =
    R"(<p class="b-c"><span>1</span></p>)"
    R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st </span><span class="i-add">second verse</span><span>.</span></p>)";
    evaluate (__LINE__, __func__, standard, html);
  }

}

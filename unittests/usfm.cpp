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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <styles/logic.h>
#include <checks/usfm.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/books.h>
#include "usfm.h"


constexpr const char* bible = "bible";


// Test the check on malformed verse.
TEST (usfm, check_malformed_verse)
{
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 2,He said.\n";
  Checks_Usfm check = Checks_Usfm (bible);
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  const auto results = check.get_results ();
  const decltype(results) standard = { {2, "Malformed verse number: \\v 2,He said."} };
  EXPECT_EQ (standard, results);
}


TEST (usfm, check_new_line)
{
  // Test check on new line in USFM as properly formatted.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p He said.\n"
    "\\v 1 He said.\n"
    "\\p He said.\n"
    "\\v 2 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard{};
    EXPECT_EQ (standard, results);
  }
  // Test check on new line in USFM.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p He said.\n"
    "\\v 1 He said.\n"
    "\\\n"
    "\\p He said.\n"
    "\\v 2 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(New line within USFM:  \ \p He s)"} };
    EXPECT_EQ (standard, results);
  }
  // Test check on new line in USFM.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p He said.\n"
    "\\v 1 He said.\n"
    "\\\n"
    "\\p He said.\n"
    "\\v 3 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(New line within USFM:  \ \p He s)"} };
    EXPECT_EQ (standard, results);
  }
}


// Test check on unknown USFM.
TEST (usfm, check_unknown_markup)
{
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p He said to the \\+nd LORD\\+nd*.\n"
    "\\v 1 He said \\add something\\add*.\n"
    "\\p,p He said.\n"
    "\\v 3 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {1, R"(Marker not in stylesheet: \p,p )"} };
    EXPECT_EQ (standard, results);
  }
  {
    const std::string usfm =
    "\\c 1\n"
    "\\pHe said.\n"
    "\\v 1 He said \\add something\\add*.\n"
    "\\p He said.\n"
    "\\v 3 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(Marker not in stylesheet: \pHe )"} };
    EXPECT_EQ (standard, results);
  }
}


// Test check on \id.
TEST (usfm, check_id)
{
  {
    const std::string usfm =
    "\\id GENN\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(Unknown ID: \id GENN)"} };
    EXPECT_EQ (standard, results);
  }
  {
    const std::string usfm =
    "\\id\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(Unknown ID: \id\p )"} };
    EXPECT_EQ (standard, results);
  }
  {
    const std::string usfm =
    "\\id Gen\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, R"(Unknown ID: \id Gen)"} };
    EXPECT_EQ (standard, results);
  }
}


// Test check on forward slash.
TEST (usfm, check_forward_slash)
{
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\p He said.\n"
    "\\v 1 He said. He said something/add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, "Forward slash instead of backslash: /add"} };
    EXPECT_EQ (standard, results);
  }
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\p He said.\n"
    "\\v 1 /v He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = { {0, "Forward slash instead of backslash: /v"} };
    EXPECT_EQ (standard, results);
  }
}


// Test that checking clean USFM has no output.
TEST (usfm, clean_usfm)
{
  const std::string usfm =
  "\\id GEN\n"
  "\\c 35\n"
  "\\s UNkulunkulu ubusisa uJakobe eBhetheli\n"
  "\\p\n"
  "\\v 1 UNkulunkulu wasesithi kuJakobe: Sukuma, yenyukela eBhetheli\\x + 28.13-19.\\x*, uhlale khona; umenzele lapho ilathi uNkulunkulu owabonakala kuwe mhla ubaleka ebusweni bukaEsawu umnewenu\\x + 27.43.\\x*.\n"
  "\\v 2 UJakobe wasesithi kundlu yakhe\\x + 18.19. Josh. 24.15.\\x* lakubo bonke ababelaye: Susani onkulunkulu bezizwe abaphakathi kwenu\\x + 31.19. Josh. 24.2,23. 1 Sam. 7.3.\\x*, lizihlambulule, lintshintshe izembatho zenu\\x + Eks. 19.10.\\x*.\n"
  "\\v 3 Asisukume senyukele eBhetheli, ngimenzele khona uNkulunkulu ilathi owangiphendula ngosuku lokuhlupheka kwami\\x + 32.7,24.\\x*, njalo waba lami endleleni engahamba kuyo\\x + 28.20; 31.3.\\x*.\n"
  "\\v 4 Basebenika uJakobe bonke onkulunkulu bezizwe ababesesandleni sabo, lamacici\\x + Hos. 2.13. Gen. 24.22.\\x* ayesezindlebeni zabo; uJakobe wasekufihla ngaphansi \\add kwesihlahla\\add* \\w se-okhi\\w**\\x + Josh. 24.26. 2 Sam. 18.9. 1 Kho. 13.14. 1 Lan. 10.12. Gen. 35.8.\\x* eliseShekema.\n"
  "\\v 5 Basebehamba; lokwesabeka kukaNkulunkulu\\x + 9.2. Eks. 15.16; 23.27. Dute. 11.25. Josh. 2.9. 2 Lan. 14.14.\\x* kwakuphezu kwemizi eyayibazingelezele, njalo kabawaxotshanga amadodana kaJakobe.\n"
  "\\p\n"
  "\\v 6 UJakobe wasefika eLuzi\\x + 28.19.\\x*, eselizweni leKhanani, eyiBhetheli, yena labantu bonke ababelaye.\n"
  "\\v 7 Wasesakha khona ilathi, wayibiza indawo \\add ngokuthi\\add* iEli-Bhetheli\\f + \\fk iEli-Bhetheli: \\fl okuyikuthi, \\fq uNkulunkulu weBhetheli.\\f*, ngoba lapho uNkulunkulu wabonakala kuye ekubalekeleni ubuso bomnewabo.\n"
  "\\v 8 Njalo uDebora umlizane kaRebeka wafa\\x + 24.59.\\x*, wangcwatshwa ezansi kweBhetheli ngaphansi kwesihlahla se-okhi\\x + 13.18; 35.4.\\x*, ngakho wasitha ibizo laso iAloni-Bakuthi\\f + \\fk iAloni-Bakuthi: \\fl okuyikuthi, \\fq ie-oki yokulila.\\f*.\n"
  "\\p\n"
  "\\v 9 UNkulunkulu wasebuya wabonakala kuJakobe evela ePadani-Arama, wambusisa.\n"
  "\\v 10 UNkulunkulu wasesithi kuye: Ibizo lakho nguJakobe; ibizo lakho kalisayikuthiwa nguJakobe\\x + 17.5,15.\\x*, kodwa uIsrayeli kuzakuba libizo lakho. Wabiza ibizo lakhe \\add ngokuthi\\add* uIsrayeli\\x + 32.28.\\x*.\n"
  "\\v 11 UNkulunkulu wasesithi kuye: NginguNkulunkulu uSomandla\\x + 17.1.\\x*; zala wande\\x + 28.3; 48.4.\\x*; isizwe, yebo ibandla lezizwe kuzavela kuwe\\x + 17.5,6,16; 26.4.\\x*; lamakhosi azaphuma ekhalweni lwakho.\n"
  "\\v 12 Lelizwe engalinika uAbrahama loIsaka ngizalinika wena, lenzalweni yakho emva kwakho ngizayinika ilizwe\\x + 12.7; 13.15; 17.8; 26.3; 28.13.\\x*.\n"
  "\\v 13 UNkulunkulu wasesenyuka esuka kuye endaweni lapho ayekhuluma laye khona\\x + 17.22.\\x*.\n"
  "\\v 14 UJakobe wasemisa insika endaweni lapho ayekhuluma laye khona, insika yelitshe\\x + 28.18; 31.45.\\x*. Wathululela phezu kwayo umnikelo wokunathwayo\\x + Nani 28.7.\\x*, wathela phezu kwayo amagcobo.\n"
  "\\v 15 UJakobe waseyitha ibizo lendawo, lapho uNkulunkulu ayekhulume laye khona, iBhetheli\\x + 28.19.\\x*.\n"
  "\\s Ukufa kukaRasheli\n"
  "\\p\n"
  "\\v 16 Basebehamba besuka eBhetheli; kwathi kusesengummango\\f + \\fk ummango: \\fl Heb. \\fq isiqetshana somhlabathi.\\f* ukuya eEfrathi, uRasheli wabeletha, wayelobunzima ekubeletheni.\n"
  "\\v 17 Kwasekusithi ekubeletheni kwakhe nzima, umbelethisi wathi kuye: Ungesabi, ngoba lalo uzakuba yindodana yakho\\x + 30.24. 1 Sam. 4.20.\\x*.\n"
  "\\v 18 Kwasekusithi umphefumulo wakhe usuphuma, ngoba esesifa, wayitha ibizo layo uBenoni\\f + \\fk uBenoni: \\fl okuyikuthi, \\fq indodana yosizi lwami.\\f*; kodwa uyise wayibiza \\add ngokuthi\\add* nguBhenjamini\\f + \\fk uBhenjamini: \\fl okuyikuthi, \\fq indodana yesandla sokunene.\\f*.\n"
  "\\v 19 URasheli wasesifa\\x + 48.7.\\x*, wangcwatshwa endleleni eya eEfrathi, eyiBhethelehema\\x + Ruthe 1.2; 4.11. Mika 5.2. Mat. 2.6,16-18.\\x*.\n"
  "\\v 20 UJakobe wasemisa insika phezu kwengcwaba lakhe; le yinsika yengcwaba likaRasheli kuze kube lamuhla\\x + 1 Sam. 10.2.\\x*.\n"
  "\\p\n"
  "\\v 21 UIsrayeli wasehamba, wamisa ithente lakhe ngaphambili komphotshongo weEderi\\x + Mika 4.8.\\x*.\n"
  "\\v 22 Kwasekusithi uIsrayeli ehlala kulelolizwe, uRubeni waya walala loBiliha\\x + 49.4. 1 Lan. 5.1. Gen. 30.4; 37.2. 2 Sam. 16.22; 20.3. 1 Kor. 5.1.\\x* umfazi omncinyane kayise; uIsrayeli wasekuzwa.\n"
  "\\s Amadodana kaJakobe lokufa kukaIsaka\n"
  "\\p Amadodana kaJakobe-ke ayelitshumi lambili\\x + 46.8-27. Eks. 1.2-4.\\x*;\n"
  "\\v 23 amadodana kaLeya: Izibulo likaJakobe, uRubeni, loSimeyoni loLevi loJuda loIsakari loZebuluni;\n"
  "\\v 24 amadodana kaRasheli: OJosefa loBhenjamini;\n"
  "\\v 25 lamadodana kaBiliha, incekukazi kaRasheli: ODani loNafithali;\n"
  "\\v 26 lamadodana kaZilipa, incekukazi kaLeya: OGadi loAsheri; la ngamadodana kaJakobe azalelwa yena ePadani-Arama.\n"
  "\\p\n"
  "\\v 27 UJakobe wasefika kuIsaka uyise eMamre\\x + 13.18; 23.2,19.\\x*, eKiriyathi-Arba\\x + Josh. 14.15; 15.13.\\x*, eyiHebroni, lapho uAbrahama ahlala khona njengowezizwe, loIsaka\\x + 28.10.\\x*.\n"
  "\\v 28 Lensuku zikaIsaka zaziyiminyaka elikhulu lamatshumi ayisificaminwembili.\n"
  "\\v 29 UIsaka wasehodoza, wafa, wabuthelwa ezizweni zakibo\\x + 15.15; 25.8.\\x*, emdala, enele ngensuku. Amadodana akhe uEsawu loJakobe asemngcwaba\\x + 25.9; 49.31.\\x*.\n";
  Checks_Usfm check = Checks_Usfm (bible);
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  const auto results = check.get_results ();
  const decltype(results) standard = {};
  EXPECT_EQ (standard, results);
}


// Test check on a widow backslash.
TEST (usfm, check_widow_backslash)
{
  const std::string usfm =
  "\\id GEN\n"
  "\\p\n"
  "\\v 1 \\ He said.\n";
  Checks_Usfm check = Checks_Usfm (bible);
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  const auto results = check.get_results ();
  const decltype(results) standard = { {1, R"(Widow backslash: \ )"} };
  EXPECT_EQ (standard, results);
}


TEST (usfm, check_matching_markers)
{
  // Test check on matching markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\v 1  He said \\add addition\\add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {};
    EXPECT_EQ (standard, results);
  }
  
  // Test check on matching markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\v 1  He said addition\\add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {
      std::pair (1, R"(Closing marker does not match opening marker : \add*)")
    };
    EXPECT_EQ (standard, results);
  }
  // Test check on matching markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\v 1  He said \\add addition\\add .\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {
      {1, R"(Repeating opening marker: \add )"},
      {1, "Unclosed markers: add"}
    };
    EXPECT_EQ (standard, results);
  }
  // Test check on matching markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\v 8 Kodwa uNowa wazuza umusa emehlweni eN\\nd kosi\\x + 19.19.\\nd*\\x*.\n"
    "\\v 9 Kodwa uNowa wazuza umusa emehlweni eN\\nd kosi\\x + 19.19.\\x*\\nd*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {};
    EXPECT_EQ (standard, results);
  }
  // Test check on matching markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\v 8 Kodwa uNowa wazuza umusa \\add emehlweni eN\\nd kosi\\x + 19.19.\\nd*\\x*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {
      {8, R"(Embedded marker requires a plus sign: \nd kosi)"},
      {8, R"(Embedded marker requires a plus sign: \nd*\x*)"},
      {8, "Unclosed markers: add"},
    };
    EXPECT_EQ (standard, results);
  }
}


TEST (usfm, check_toc_markers)
{
  // Check on correct \toc[1-3] markers.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\toc1 The book of Genesis\n"
    "\\toc2 Genesis\n"
    "\\toc3 Gen\n";
    Checks_Usfm check ("");
    check.initialize (1, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {};
    EXPECT_EQ (standard, results);
  }
  
  // The \toc[1-3] markers are the wrong chapter.
  {
    const std::string usfm =
    "\\id GEN\n"
    "\\toc1 The book of Genesis\n"
    "\\toc2 Genesis\n"
    "\\toc3 Gen\n";
    Checks_Usfm check ("");
    check.initialize (1, 2);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {
      {0, R"(The following marker belongs in chapter 0: \toc1 )"},
      {0, R"(The following marker belongs in chapter 0: \toc2 )"},
      {0, R"(The following marker belongs in chapter 0: \toc3 )"}
    };
    EXPECT_EQ (standard, results);
  }
  
  // Lacks \toc# markers.
  {
    const std::string usfm = "\\id GEN\n";
    Checks_Usfm check ("");
    check.initialize (3, 0);
    check.check (usfm);
    check.finalize ();
    const auto results = check.get_results ();
    const decltype(results) standard = {
      {0, R"(The book lacks the marker for the verbose book name: \toc1 )"},
      {0, R"(The book lacks the marker for the short book name: \toc2 )"}
    };
    EXPECT_EQ (standard, results);
  }
}


TEST (usfm, convert_line_number_to_verse_number)
{
  // Test converting line number to verse number.
  {
    const std::string usfm = "\\id MIC";
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::linenumber_to_versenumber (usfm, 0));
  }
  
  // Test converting line number to verse number.
  {
    const std::string usfm =
    "\\id MIC\n"
    "\\v 1 Verse";
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::linenumber_to_versenumber (usfm, 1));
  }
  
  // Test converting line number to verse number.
  {
    const std::string usfm =
    "\\v 1 Verse";
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::linenumber_to_versenumber (usfm, 0));
  }
  
  // Test converting line number to verse number.
  {
    const std::string usfm =
    "\\p\n"
    "\\v 3 Verse 3 (out of order).\n"
    "\\v 1 Verse 1. \n"
    "\\v 2 Verse 1.";
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::linenumber_to_versenumber (usfm, 0));
    EXPECT_EQ (std::vector <int>{3}, filter::usfm::linenumber_to_versenumber (usfm, 1));
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::linenumber_to_versenumber (usfm, 2));
    EXPECT_EQ (std::vector <int>{2}, filter::usfm::linenumber_to_versenumber (usfm, 3));
  }
  
  // Test converting line number to verse number.
  {
    const std::string usfm =
    "\\id MIC\n"
    "\\v 1-2 Verse";
    EXPECT_EQ ((std::vector <int>{1, 2}), filter::usfm::linenumber_to_versenumber (usfm, 1));
  }
}


TEST (usfm, convert_offset_to_verse_number)
{
  // Test converting offset to verse number.
  {
    const std::string usfm = "\\id MIC";
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 0));
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 7));
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 17));
  }
  
  // Test converting offset to verse number.
  {
    const std::string usfm =
    "\\id MIC\n"
    "\\v 1 Verse";
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 7));
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 8));
  }
  
  // Test converting offset to verse number.
  {
    const std::string usfm =
    "\\id MIC\n"
    "\\v 1-3 Verse";
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 7));
    EXPECT_EQ ((std::vector <int>{1, 2, 3}), filter::usfm::offset_to_versenumber (usfm, 8));
  }
  
  // Test converting offset to verse number.
  {
    const std::string usfm =
    "\\v 1 Verse";
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 0));
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 2));
  }
  
  // Test converting offset to verse number.
  {
    const std::string usfm =
    "\\p\n"
    "\\v 3 Verse 3 (out of order).\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.";
    
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 0));
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 1));
    
    EXPECT_EQ (std::vector <int>{0}, filter::usfm::offset_to_versenumber (usfm, 2));
    EXPECT_EQ (std::vector <int>{3}, filter::usfm::offset_to_versenumber (usfm, 3));
    EXPECT_EQ (std::vector <int>{3}, filter::usfm::offset_to_versenumber (usfm, 4));
    
    EXPECT_EQ (std::vector <int>{3}, filter::usfm::offset_to_versenumber (usfm, 31));
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 32));
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 33));
    
    EXPECT_EQ (std::vector <int>{1}, filter::usfm::offset_to_versenumber (usfm, 45));
    EXPECT_EQ (std::vector <int>{2}, filter::usfm::offset_to_versenumber (usfm, 46));
    EXPECT_EQ (std::vector <int>{2}, filter::usfm::offset_to_versenumber (usfm, 47));
  }
}


// Test converting verse number to offset.
TEST (usfm, convert_verse_number_to_offset)
{
  const std::string usfm =
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2 Verse 2.\n"
  "\\v 3 Verse 3.\n"
  "\\v 4-5 Verse 4 and 5.";
  EXPECT_EQ (3, filter::usfm::versenumber_to_offset (usfm, 1));
  EXPECT_EQ (17, filter::usfm::versenumber_to_offset (usfm, 2));
  EXPECT_EQ (31, filter::usfm::versenumber_to_offset (usfm, 3));
  EXPECT_EQ (45, filter::usfm::versenumber_to_offset (usfm, 4));
  EXPECT_EQ (45, filter::usfm::versenumber_to_offset (usfm, 5));
  EXPECT_EQ (66, filter::usfm::versenumber_to_offset (usfm, 6));
  EXPECT_EQ (66, filter::usfm::versenumber_to_offset (usfm, 6));
}


TEST (usfm, get_verse_usfm)
{
  // Testing getting USFM for verse, basic and for Quill-based verse editor.
  {
    const std::string usfm =
    "\\p\n"
    "\\v 1 One";
    EXPECT_EQ (R"(\p)", filter::usfm::get_verse_text (usfm, 0));
    EXPECT_EQ ("", filter::usfm::get_verse_text_quill (usfm, 0));
    EXPECT_EQ (R"(\v 1 One)", filter::usfm::get_verse_text (usfm, 1));
    EXPECT_EQ (usfm, filter::usfm::get_verse_text_quill (usfm, 1));
    EXPECT_EQ ("", filter::usfm::get_verse_text (usfm, 2));
    EXPECT_EQ ("", filter::usfm::get_verse_text_quill (usfm, 2));
  }
  
  // Testing getting USFM for verse, basic and Quill.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\s Isibingelelo\n"
    "\\p\n"
    "\\v 1 Umdala\n"
    "\\p\n"
    "\\v 2 Sithandwa\n"
    "\\v 3 Ngoba\n"
    "\\v 4 Kangilantokozo\n"
    "\\s Inkathazo\n"
    "\\p\n"
    "\\v 5 Sithandwa\n"
    "\\v 6 abafakazele\n"
    "\\v 7 Ngoba\n"
    "\\v 8 Ngakho\n"
    "\\p\n"
    "\\v 9 Ngabhalela\n"
    "\\v 10 Ngakho\n"
    "\\p\n"
    "\\v 11 Sithandwa\n"
    "\\v 12 NgoDemetriyu\n"
    "\\s Isicino\n"
    "\\p\n"
    "\\v 13 Bengilezinto\n"
    "\\v 14 kodwa\n"
    "\\p Ukuthula";
    std::string result;

    result =
    "\\c 1\n"
    "\\s Isibingelelo\n"
    "\\p";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 0));
    result =
    "\\c 1\n"
    "\\s Isibingelelo";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1 Umdala\n"
    "\\p";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 1));
    result =
    "\\p\n"
    "\\v 1 Umdala";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2 Sithandwa";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 2));
    result =
    "\\p\n"
    "\\v 2 Sithandwa";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 2));
    
    result = "\\v 3 Ngoba";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 3));
    result = "\\v 3 Ngoba";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4 Kangilantokozo\n"
    "\\s Inkathazo\n"
    "\\p";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 4));
    result =
    "\\v 4 Kangilantokozo\n"
    "\\s Inkathazo";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 4));
    
    result =
    "\\v 5 Sithandwa";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 5));
    result =
    "\\p\n"
    "\\v 5 Sithandwa";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 5));
    
    result =
    "\\v 12 NgoDemetriyu\n"
    "\\s Isicino\n"
    "\\p";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 12));
    result =
    "\\v 12 NgoDemetriyu\n"
    "\\s Isicino";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 12));
    
    result =
    "\\v 14 kodwa\n"
    "\\p Ukuthula";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 14));
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 14));
    
    result = "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 15));
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 15));
  }

  // Test getting the verse text from USFM.
  {
    const std::string usfm =
    "\\v 1 Verse 1.\n"
    "\\v 2-4 Verse 2, 3, and 4.\n"
    "\\v 5 Verse 5.\n"
    "\\v 6 Verse 6.";
    std::string result;
    
    result = filter::usfm::get_verse_text (usfm, 2);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
    result = filter::usfm::get_verse_text_quill (usfm, 2);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
    
    result = filter::usfm::get_verse_text (usfm, 3);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
    result = filter::usfm::get_verse_text_quill (usfm, 3);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
    
    result = filter::usfm::get_verse_text (usfm, 4);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
    result = filter::usfm::get_verse_text_quill (usfm, 4);
    EXPECT_EQ ("\\v 2-4 Verse 2, 3, and 4.", result);
  }

  // Testing USFM extraction for Quill-based visual verse editor with more than one empty paragraph in sequence.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\b\n"
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2 Two\n"
    "\\b\n"
    "\\p\n"
    "\\v 3 Three\n"
    "\\v 4 Four\n"
    "";
    std::string result;
    
    result =
    "\\c 1\n"
    "\\b\n"
    "\\p"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 0));
    result =
    "\\c 1"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1 One"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 1));
    result =
    "\\b\n"
    "\\p\n"
    "\\v 1 One"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2 Two\n"
    "\\b\n"
    "\\p"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 2));
    result =
    "\\v 2 Two"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 2));
    
    result =
    "\\v 3 Three"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 3));
    result =
    "\\b\n"
    "\\p\n"
    "\\v 3 Three"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4 Four"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 4));
    result =
    "\\v 4 Four"
    "";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 4));
    
    result = "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 5));
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 5));
  }
  
  // Testing USFM extraction for Quill-based visual verse editor with empty verses.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1\n"
    "\\v 2\n"
    "\\p\n"
    "\\v 3\n"
    "\\v 4\n"
    ;
    std::string result;
    
    result =
    "\\c 1\n"
    "\\p"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 0));
    result = "\\c 1";
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 1));
    result =
    "\\p\n"
    "\\v 1"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2\n"
    "\\p"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 2));
    result =
    "\\v 2"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 2));
    
    result =
    "\\v 3"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 3));
    result =
    "\\p\n"
    "\\v 3"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 4));
    result =
    "\\v 4"
    ;
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 4));
    
    result = "";
    EXPECT_EQ (result, filter::usfm::get_verse_text (usfm, 5));
    EXPECT_EQ (result, filter::usfm::get_verse_text_quill (usfm, 5));
  }
  
  // Test getting the USFM for a verse where there's alternate verse number or published verse numbers.
  // It used to get mixed up on those cases.
  // This regression test notices whether it keeps behaving as it should.
  {
    const std::string path = filter_url_create_root_path ({"unittests", "tests", "usfm01.usfm"});
    const std::string chapter_usfm = filter_url_file_get_contents (path);
    std::string usfm;
    
    usfm = filter::usfm::get_verse_text (chapter_usfm, 1);
    EXPECT_EQ ("\\v 1 Verse 1.", usfm);
    
    usfm = filter::usfm::get_verse_text (chapter_usfm, 12);
    EXPECT_EQ ("\\v 12 Verse 12 one.\n"
               "\\p \\va 1b \\va* Alternate verse 1b, \\va 2 \\va* alternate verse 2.\n"
               "\\s Header 13\n"
               "\\r refs 13\n"
               "\\p", usfm);
  }
  
  // Test getting text from USFM with a range of verses.
  {
    const std::string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three\n"
    "\\v 4 Four\n"
    "\\v 5 Five";
    std::string result;
    
    result =
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 1, 2, "", false));
    result =
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 1, 2, "", true));
    
    result =
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 1, 3, "", false));
    result =
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 1, 3, "", true));
    
    result =
    "\\v 4 Four";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 3, 4, "\\v 2-3 Two three", false));
    result =
    "\\v 4 Four";
    EXPECT_EQ (result, filter::usfm::get_verse_range_text (usfm, 3, 4, "\\v 2-3 Two three", true));
  }
}


TEST (usfm, is_marker)
{
  // Test on detecting a general USFM marker.
  {
    EXPECT_EQ (true, filter::usfm::is_usfm_marker (R"(\id)"));
    EXPECT_EQ (true, filter::usfm::is_usfm_marker (R"(\c )"));
    EXPECT_EQ (false, filter::usfm::is_usfm_marker ("c"));
    EXPECT_EQ (true, filter::usfm::is_usfm_marker (R"(\add )"));
    EXPECT_EQ (true, filter::usfm::is_usfm_marker (R"(\add*)"));
    EXPECT_EQ (true, filter::usfm::is_usfm_marker (R"(\+add*)"));
  }
  
  // Test on detecting an opening marker.
  {
    EXPECT_EQ (true, filter::usfm::is_opening_marker (R"(\id)"));
    EXPECT_EQ (true, filter::usfm::is_opening_marker (R"(\c )"));
    EXPECT_EQ (false, filter::usfm::is_opening_marker (R"(\c*)"));
    EXPECT_EQ (true, filter::usfm::is_opening_marker (R"(\+add )"));
    EXPECT_EQ (false, filter::usfm::is_opening_marker (R"(\+add*)"));
  }
  
  // Test on detecting embedded marker.
  {
    EXPECT_EQ (false, filter::usfm::is_embedded_marker (R"(\add)"));
    EXPECT_EQ (false, filter::usfm::is_embedded_marker (R"(\add*)"));
    EXPECT_EQ (true, filter::usfm::is_embedded_marker (R"(\+add)"));
    EXPECT_EQ (true, filter::usfm::is_embedded_marker (R"(\+add )"));
    EXPECT_EQ (true, filter::usfm::is_embedded_marker (R"(\+add*)"));
  }
}


// Test on extracting a book identifier.
TEST (usfm, get_book_id)
{
  EXPECT_EQ ("GEN", filter::usfm::get_book_identifier ({ R"(\id)", "GEN"    }, 0));
  EXPECT_EQ ("XXX", filter::usfm::get_book_identifier ({ R"(\id)", "GEN"    }, 1));
  EXPECT_EQ ("GE" , filter::usfm::get_book_identifier ({ R"(\id)", "GE"     }, 0));
  EXPECT_EQ ("GEN", filter::usfm::get_book_identifier ({ R"(\id)", "GENxxx" }, 0));
  EXPECT_EQ ("GEN", filter::usfm::get_book_identifier ({ "",       "GENxxx" }, 0));
}


// Test on extracting a verse number.
TEST (usfm, peek_verse_number)
{
  EXPECT_EQ ("1", filter::usfm::peek_verse_number ("1"));
  EXPECT_EQ ("1", filter::usfm::peek_verse_number ("1 "));
  EXPECT_EQ ("1a", filter::usfm::peek_verse_number ("1a"));
  EXPECT_EQ ("2-3", filter::usfm::peek_verse_number ("2-3"));
  EXPECT_EQ ("2b,3", filter::usfm::peek_verse_number ("2b,3"));
  EXPECT_EQ ("2b,3,", filter::usfm::peek_verse_number ("2b,3, 4"));
  EXPECT_EQ ("2a-3b", filter::usfm::peek_verse_number ("2a-3b And he said"));
}


// Test on Genesis USFM.
TEST (usfm, genesis)
{
  const std::string directory = filter_url_create_root_path ({"unittests", "tests"});
  const std::string bookusfm = filter_url_file_get_contents (filter_url_create_path ({directory, "01GEN.SFM"}));
  
  // Test getting all chapter number from USFM.
  std::vector <int> chapters = filter::usfm::get_chapter_numbers (bookusfm);
  std::vector <int> all_chapters (51);
  std::iota (all_chapters.begin(), all_chapters.end(), 0);
  EXPECT_EQ (all_chapters, chapters);
  
  // Test getting contents for chapter 0.
  std::string usfm = filter::usfm::get_chapter_text (bookusfm, 0);
  std::string standard = filter_url_file_get_contents (filter_url_create_path ({directory, "01GEN-0.SFM"}));
  EXPECT_EQ (standard, usfm);
  
  chapters = filter::usfm::get_chapter_numbers (usfm);
  EXPECT_EQ (std::vector <int>{ 0 }, chapters);
  
  // Test getting contents for last chapter in USFM.
  usfm = filter::usfm::get_chapter_text (bookusfm, 50);
  standard = filter_url_file_get_contents (filter_url_create_path ({directory, "01GEN-50.SFM"}));
  EXPECT_EQ (standard, usfm);
  
  chapters = filter::usfm::get_chapter_numbers (usfm);
  EXPECT_EQ ((std::vector <int>{0,50}), chapters);
  
  // Test getting the text of a chapter somewhere within a block of USFM.
  usfm = filter::usfm::get_chapter_text (bookusfm, 25);
  standard = filter_url_file_get_contents (filter_url_create_path ({directory, "01GEN-25.SFM"}));
  EXPECT_EQ (standard, usfm);
  
  chapters = filter::usfm::get_chapter_numbers (usfm);
  EXPECT_EQ ((std::vector <int>{ 0, 25 }), chapters);
  
  // Test getting non-existing chapter text.
  usfm = filter::usfm::get_chapter_text (bookusfm, 51);
  EXPECT_EQ ("", usfm);
  
  chapters = filter::usfm::get_chapter_numbers (usfm);
  EXPECT_EQ (std::vector <int>{ 0 }, chapters);
  
  // Test getting text for chapter that has a space after chapter number.
  std::string modified_book_usfm = filter::strings::replace ("\\c 10", "\\c 10 ", bookusfm);
  usfm = filter::usfm::get_chapter_text (modified_book_usfm, 10);
  standard = filter_url_file_get_contents (filter_url_create_path ({directory, "01GEN-10.SFM"}));
  EXPECT_EQ (standard, usfm);
  
  chapters = filter::usfm::get_chapter_numbers (modified_book_usfm);
  EXPECT_EQ (all_chapters, chapters);
}


// Regression test on instance of Nehemia 12 in combination with the Paratext bridge.
TEST (usfm, nehemiah_12)
{
  const std::string directory = filter_url_create_root_path ({"unittests", "tests"});
  const std::string book_usfm = filter_url_file_get_contents (filter_url_create_path ({directory, "16NEHTSIC.SFM"}));
  const std::string chapter_usfm = filter::usfm::get_chapter_text (book_usfm, 12);
  EXPECT_EQ (7355, chapter_usfm.size());
}


// Test getting the markers and the text from USFM.
TEST (usfm, get_markers_and_text)
{
  std::vector <std::string> standard;
  standard = { R"(\id )", "GEN", R"(\c )", "10" };
  EXPECT_EQ (standard, filter::usfm::get_markers_and_text (R"(\id GEN\c 10)"));
  standard = { "noise", R"(\id )", "GEN", R"(\c )", "10" };
  EXPECT_EQ (standard, filter::usfm::get_markers_and_text (R"(noise\id GEN\c 10)"));
  standard = { R"(\p)", R"(\v )", "1 In ", R"(\add )", "the", R"(\add*)" };
  EXPECT_EQ (standard, filter::usfm::get_markers_and_text (R"(\p\v 1 In \add the\add*)"));
  standard = { R"(\v )", "2 Text ", R"(\add )", "of the ", R"(\add*)", "1st", R"(\add )", "second verse", R"(\add*)", "." };
  EXPECT_EQ (standard, filter::usfm::get_markers_and_text (R"(\v 2 Text \add of the \add*1st\add second verse\add*.)"));
  standard = { R"(\p)", R"(\v )", "1 In ", R"(\+add )", "the", R"(\+add*)" };
  EXPECT_EQ (standard, filter::usfm::get_markers_and_text (R"(\p\v 1 In \+add the\+add*)"));
}


// Test getting the markers from a fragment of USFM.
TEST (usfm, get_marker)
{
  EXPECT_EQ ("", filter::usfm::get_marker (""));
  EXPECT_EQ ("id", filter::usfm::get_marker (R"(\id GEN)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\add insertion)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\add)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\add*)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\add*\add)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\+add)"));
  EXPECT_EQ ("add", filter::usfm::get_marker (R"(\+add*)"));
}


constexpr const auto usfm1 =
R"(\c 1)" "\n"
R"(\s The)" "\n"
R"(Creation)" "\n"
R"(\p)" "\n"
R"(\v 1 In)" "\n"
R"(the)" "\n"
R"(beginning,)" "\n"
R"(\bd God\bd*)" "\n"
R"(created)" "\n"
R"(...)" "\n"
R"(\p)" "\n"
R"(\v 2)" "\n"
;



TEST (usfm, importing)
{
  // Test importing common USFM.
  {
    EXPECT_EQ (0, static_cast<int>(filter::usfm::usfm_import ("", stylesv2::standard_sheet ()).size()));
    
    std::vector <filter::usfm::BookChapterData> imported = filter::usfm::usfm_import ("\\id MIC\n\\c 1\n\\s Heading\n\\p\n\\v 1 Verse one.", stylesv2::standard_sheet ());
    EXPECT_EQ (2, imported.size ());
    if (imported.size () == 2) {
      EXPECT_EQ (33, imported [0].m_book);
      EXPECT_EQ (0, imported [0].m_chapter);
      EXPECT_EQ ("\\id MIC", imported [0].m_data);
      EXPECT_EQ (33, imported [1].m_book);
      EXPECT_EQ (1, imported [1].m_chapter);
      EXPECT_EQ ("\\c 1\n\\s Heading\n\\p\n\\v 1 Verse one.", imported [1].m_data);
    }
    
    EXPECT_EQ ((std::vector<int>{0, 1, 2}), filter::usfm::get_verse_numbers ("\\v 1 test\\v 2 test"));
  }

  // Test importing USFM with \vp markup.
  {
    std::string usfm = R"(
\id MIC
\c 1
\s Heading
\p
\v 1 \vp A\vp* Verse one.
\v 2 \vp B\vp* Verse two.
)";
    std::string standard_chapter = R"(
\c 1
\s Heading
\p
\v 1 \vp A\vp* Verse one.
\v 2 \vp B\vp* Verse two.
)";
    standard_chapter = filter::strings::trim (standard_chapter);
    std::vector <filter::usfm::BookChapterData> imported = filter::usfm::usfm_import (usfm, stylesv2::standard_sheet ());
    EXPECT_EQ (2, imported.size ());
    if (imported.size () == 2) {
      EXPECT_EQ (33, imported [0].m_book);
      EXPECT_EQ (0, imported [0].m_chapter);
      EXPECT_EQ ("\\id MIC", imported [0].m_data);
      EXPECT_EQ (33, imported [1].m_book);
      EXPECT_EQ (1, imported [1].m_chapter);
      EXPECT_EQ (standard_chapter, filter::strings::trim (imported [1].m_data));
    }
  }

  // Test importing USFM demo chapter.
  {
    const std::string usfm = filter_url_file_get_contents (filter_url_create_root_path ({"demo", "92-1JNeng-web.usfm"}));
    const std::vector <filter::usfm::BookChapterData> imported = filter::usfm::usfm_import (usfm, stylesv2::standard_sheet ());
    // It imports book 0 due to the copyright notices at the top of the USFM file.
    EXPECT_EQ (7, imported.size ());
  }
  
  // Test weirdly formed USFM that it properly inserts spaces on import.
  // https://github.com/bibledit/cloud/issues/993
  {
    const std::vector <filter::usfm::BookChapterData> imported = filter::usfm::usfm_import (usfm1, stylesv2::standard_sheet ());
    constexpr const auto chapters {1};
    EXPECT_EQ (chapters, imported.size ());
    if (imported.size () == chapters) {
      EXPECT_EQ (0, imported.at(0).m_book);
      EXPECT_EQ (1, imported.at(0).m_chapter);
      constexpr const auto standard =
      R"(\c 1)" "\n"
      R"(\s The Creation)" "\n"
      R"(\p)" "\n"
      R"(\v 1 In the beginning, \bd God\bd* created ...)" "\n"
      R"(\p)" "\n"
      R"(\v 2)"
      ;
      EXPECT_EQ (standard, imported.at(0).m_data);
    }
  }
}


TEST (usfm, get_opening_closing_usfm)
{
  // Test building opening USFM marker.
  {
    EXPECT_EQ (R"(\id )", filter::usfm::get_opening_usfm ("id"));
    EXPECT_EQ (R"(\add )", filter::usfm::get_opening_usfm ("add"));
    EXPECT_EQ (R"(\add )", filter::usfm::get_opening_usfm ("add", false));
    EXPECT_EQ (R"(\+add )", filter::usfm::get_opening_usfm ("add", true));
  }
  // Test building closing USFM marker.
  {
    EXPECT_EQ (R"(\wj*)", filter::usfm::get_closing_usfm ("wj"));
    EXPECT_EQ (R"(\add*)", filter::usfm::get_closing_usfm ("add"));
    EXPECT_EQ (R"(\add*)", filter::usfm::get_closing_usfm ("add", false));
    EXPECT_EQ (R"(\+add*)", filter::usfm::get_closing_usfm ("add", true));
  }
}


TEST (usfm, get_verse_numbers)
{
  // Test getting verse numbers from USFM.
  {
    std::string usfm = "\\c 1\n\\s Isibingelelo\n\\p\n\\v 1 Umdala\n\\p\n\\v 2 Sithandwa\n\\v 3 Ngoba\n\\v 4 Kangilantokozo\n\\s Inkathazo\n\\p\n\\v 5 Sithandwa\n\\v 6 abafakazele\n\\v 7 Ngoba\n\\v 8 Ngakho\n\\p\n\\v 9 Ngabhalela\n\\v 10 Ngakho\n\\p\n\\v 11 Sithandwa\n\\v 12 NgoDemetriyu\n\\s Isicino\n\\p\n\\v 13 Bengilezinto\n\\v 14 kodwa\n\\p Ukuthula";
    EXPECT_EQ ((std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }), filter::usfm::get_verse_numbers (usfm));
    usfm = ""
    "\\c 80\n"
    "\\s Umkhuleko wokusizwa kukaIsrayeli\n"
    "\\d Kumqondisi wokuhlabelela. NgeShoshanimi. Ubufakazi. Isihlabelelo sikaAsafi\n"
    "\\p\n"
    "\\v 1 Melusi kaIsrayeli, beka indlebe, okhokhela uJosefa\\x + Hlab. 81.5.\\x* njengomhlambi\\x + Gen. 48.15. 49.24. Hlab. 77.20. Hlab. 95.7.\\x*, ohlezi \\add phakathi\\add* \\w kwamakherubhi\\w**\\x + Hlab. 99.1. Eks. 25.22.\\x*, khanyisa\\x + Hlab. 50.2.\\x*.\n"
    "\\v 2 Phambi kukaEfrayimi loBhenjamini loManase\\x + Nani 2.18-23.\\x* vusa amandla akho, uze ube lusindiso lwethu\\x + Hlab. 35.23.\\x*.\n"
    "\\p\n"
    "\\v 3 Siphendule, Nkulunkulu\\x + 80.7,14,19. Hlab. 60.1. 85.4. Lilo 5.21.\\x*, wenze ubuso bakho bukhanye, ngakho sizasindiswa\\x + Nani 6.25. Hlab. 4.6.\\x*.\n"
    "\\p\n"
    "\\v 4 \nnd kosi\nd* Nkulunkulu wamabandla\\x + Ps 59.5. 84.8.\\x*, koze kube nini uthukuthelela umkhuleko wabantu bakho\\x + Hlab. 74.10.\\x*?\n"
    "\\v 5 Ubenze badla isinkwa sezinyembezi\\x + Hlab. 42.3. Hlab. 102.9.\\x*, wabanathisa izinyembezi ngesilinganiso\\x + Isa. 40.12.\\x*.\n"
    "\\v 6 Usenza sibe yingxabano kubomakhelwane bethu, lezitha zethu ziyahlekisana \\add ngathi\\x + Hlab. 44.13. 79.4.\\x*\\add*.\n"
    "\\p\n"
    "\\v 7 Siphendule, Nkulunkulu wamabandla, wenze ubuso bakho bukhanye, ngakho sizasindiswa\\x + 80.7,14,19. Hlab. 60.1. 85.4. Lilo 5.21.\\x*.\n"
    "\\p\n"
    "\\v 8 Waliletha ivini livela eGibhithe\\x + Isa. 5.1-7. 27.2. Jer. 2.21. 12.10. Hez. 15.6. 17.6. 19.10. Mat. 21.33. Mark. 12.1. Luka 20.9. Joha. 15.1-6.\\x*, wazixotsha izizwe\\x + Hlab. 78.55.\\x*, walihlanyela lona\\x + Hlab. 44.2.\\x*.\n"
    "\\v 9 Walungisa \\add indawo\\add* phambi kwalo\\x + Gen. 24.31. Josh. 24.12.\\x*, wagxilisa impande zalo, laze lagcwala umhlaba.\n"
    "\\v 10 Izintaba zembeswa ngomthunzi walo, lezingatsha zalo zi\\add njenge\\add*misedari kaNkulunkulu.\n"
    "\\v 11 Lanabisela ingatsha zalo elwandle\\x + Jobe 14.9.\\x*, lamahlumela alo\\x + Jobe 8.16.\\x* emfuleni\\x + Hlab. 72.8.\\x*.\n"
    "\\v 12 Uyibhobozeleni imiduli yalo\\x + Hlab. 89.40,41. Isa. 5.5.\\x*, ukuze balikhe bonke abadlula ngendlela\\x + Hlab. 89.40,41. Isa. 5.5.\\x*?\n"
    "\\v 13 Ingulube yasehlathini iyalihlikiza\\x + Jer. 5.6.\\x*, lenyamazana yeganga iyalidla\\x + Hlab. 50.11.\\x*.\n"
    "\\p\n"
    "\\v 14 Nkulunkulu wamabandla, akubuyele\\x + 80.3.\\x*, ukhangele phansi usemazulwini\\x + Isa. 63.15.\\x*, ubone, wethekelele lelivini\\x + Hlab. 84.9.\\x*,\n"
    "\\v 15 ngitsho isivini isandla sakho sokunene esasihlanyelayo, lendodana\\x + Gen. 49.22.\\x* \\add o\\add*waziqinisela \\add yona\\x + 80.17. Isa.44.14.\\x*\\add*.\n"
    "\\v 16 Sitshisiwe ngomlilo\\x + Isa. 33.12.\\x*, saqunyelwa phansi\\x + Isa. 33.12.\\x*; bayabhubha ngokukhuza kobuso bakho\\x + Hlab. 76.6. Hlab. 39.11.\\x*.\n"
    "\\v 17 Isandla sakho kasibe phezu komuntu\\x + Hlab. 89.21. Luka 1.66.\\x* wesandla sakho sokunene\\x + 80.15.\\x*, phezu kwendodana yomuntu\\x + Hlab. 8.4,5.\\x*, \\add o\\add*ziqinisele \\add yona\\x + 80.17. Isa. 44.14.\\x*\\x + Hlab. 89.21. Luka 1.66.\\x*\\add*.\n"
    "\\v 18 Ngakho kasiyikubuyela emuva sisuke kuwe; sivuselele, khona sizabiza ibizo lakho\\x + Hlab. 71.20.\\x*.\n"
    "\\p\n"
    "\\v 19 \nnd kosi\nd*, Nkulunkulu wamabandla, siphendule, wenze ubuso bakho bukhanye, ngakho sizasindiswa\\x + 80.3,7.\\x*.\n";
    EXPECT_EQ ((std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 }), filter::usfm::get_verse_numbers (usfm));
  }
  
  // Test getting verse numbers from USFM.
  {
    std::string usfm;
    std::vector <int> verses;
    
    usfm = "\\v 1-2 Umdala\n\\p\n\\v 3 Ngoba\n";
    verses = filter::usfm::get_verse_numbers (usfm);
    EXPECT_EQ ((std::vector<int>{ 0, 1, 2, 3 }), verses);
    
    usfm = "\\v 10-12b Fragment\n\\p\n\\v 13 Fragment\n";
    verses = filter::usfm::get_verse_numbers (usfm);
    EXPECT_EQ ((std::vector<int>{ 0, 10, 11, 12, 13 }), verses);
    
    usfm = "\\v 10,11a Fragment\n\\p\n\\v 13 Fragment\n";
    verses = filter::usfm::get_verse_numbers (usfm);
    EXPECT_EQ ((std::vector<int>{ 0, 10, 11, 13 }), verses);
    
    usfm = "\\v 10,12 Fragment\n\\p\n\\v 13 Fragment\n";
    verses = filter::usfm::get_verse_numbers (usfm);
    EXPECT_EQ ((std::vector<int>{ 0, 10, 12, 13 }), verses);
  }
}


// Testing on USFM without verse text.
TEST (usfm, contains_empty_verses)
{
  std::string usfm;
  
  usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.";
  EXPECT_EQ (false, filter::usfm::contains_empty_verses (usfm));
  
  usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.\n"
  "\\v 2 Two";
  EXPECT_EQ (false, filter::usfm::contains_empty_verses (usfm));
  
  usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.\n"
  "\\v 2";
  EXPECT_EQ (true, filter::usfm::contains_empty_verses (usfm));
  
  usfm = "\\v 1 \n"
  "\\v 2 Two";
  EXPECT_EQ (true, filter::usfm::contains_empty_verses (usfm));
  
  usfm = "\\v 1 \n"
  "\\v 2 ";
  EXPECT_EQ (true, filter::usfm::contains_empty_verses (usfm));
}


// Testing removing fig word-level attributes.
TEST (usfm, extract_fig)
{
  std::string usfm;
  std::string result;
  std::string dummy;
  std::string standard;
  
  usfm = R"(\v 18 At once they left their nets and went with him. \fig At once they left their nets.|src="avnt016.jpg" size="span" ref="1.18"\fig*)";
  result = filter::usfm::extract_fig (usfm, dummy, dummy, dummy, dummy, dummy, dummy, dummy);
  standard = R"(\v 18 At once they left their nets and went with him. )";
  EXPECT_EQ (standard, result);
  
  usfm = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.|src="avnt017.tif" size="col" ref="1.31"\fig*)";
  result = filter::usfm::extract_fig (usfm, dummy, dummy, dummy, dummy, dummy, dummy, dummy);
  standard = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. )";
  EXPECT_EQ (standard, result);
}


// Test check on embedded markers.
TEST (usfm, check_embedded_markers)
{
  std::string usfm = R"(
\id GEN
\v 1 This is a \w \+add sen\+add*rd3\w* tence.
\v 2 This is a \w \add sen\add*rd3\w* tence.
\v 3 This \add is\add* \w a\w* longer \w \+add sen\+add*tence\w* for sure.
\v 4 This \add is\add* \w a\w* longer \w \add sen\add*tence\w* for sure.
\v 5 Praise is \w \+add du\+add*e\w* to God.
\v 6 Praise is \w \add du\add*e\w* to God.
)";
  Checks_Usfm check = Checks_Usfm ("bible");
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  std::vector <std::pair<int, std::string>> results = check.get_results ();
  std::vector <std::pair<int, std::string>> standard = {
    std::pair (2, "Embedded marker requires a plus sign: \\add sen"),
    std::pair (2, "Embedded marker requires a plus sign: \\add*rd3"),
    std::pair (4, "Embedded marker requires a plus sign: \\add sen"),
    std::pair (4, "Embedded marker requires a plus sign: \\add*tence"),
    std::pair (6, "Embedded marker requires a plus sign: \\add du"),
    std::pair (6, "Embedded marker requires a plus sign: \\add*e"),
  };
  EXPECT_EQ (standard, results);
}


// Test on correct or incorrect \vp ...\vp* markup.
TEST (usfm, check_vp_vp_markup)
{
  std::string usfm = R"(
\id GEN
\c 1
\v 1 \vp A\vp* Praise God.
\v 2 \vp B Praise Jesus.
\v 3 Praise them, all of you.
)";
  Checks_Usfm check = Checks_Usfm ("");
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  std::vector <std::pair<int, std::string>> results = check.get_results ();
  std::vector <std::pair<int, std::string>> standard = {
    std::pair (3, "Unclosed markers: vp"),
  };
  EXPECT_EQ (standard, results);
}


// Test extracting the figure attributes.
// https://ubsicap.github.io/usfm/characters/index.html#fig-fig
TEST (usfm, figure_extraction)
{
  std::string usfm_in;
  std::string caption;
  std::string alt;
  std::string src;
  std::string size;
  std::string loc;
  std::string copy;
  std::string ref;
  std::string usfm_out;
  
  // USFM without any figure information.
  usfm_in = R"(Text \fig Empty figure.\fig* text.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ (std::string(), caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ (std::string(), src);
  EXPECT_EQ (std::string(), size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ (std::string(), ref);
  EXPECT_EQ (R"(Text  text.)", usfm_out);
  
  // USFM 1/2.x with invalid figure information.
  usfm_in = R"(Text \fig DESC|SIZE|LOC|COPY|CAP|REF\fig* text.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ (std::string(), caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ (std::string(), src);
  EXPECT_EQ (std::string(), size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ (std::string(), ref);
  EXPECT_EQ ("Text  text.", usfm_out);
  
  // USFM 2.4 example taken from https://paratext.org/files/documentation/usfmReference2_4.pdf
  usfm_in = R"(Text \fig |avnt016.tif|span|||At once they left their nets.|1.18\fig* text.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ ("At once they left their nets.", caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ ("avnt016.tif", src);
  EXPECT_EQ ("span", size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ ("1.18", ref);
  EXPECT_EQ ("Text  text.", usfm_out);
  
  // USFM 2.4 example taken from https://paratext.org/files/documentation/usfmReference2_4.pdf
  usfm_in = R"(Text \fig |avnt017.tif|col|||Took her by the hand, and...the fever left her.|1.31\fig* text.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ ("Took her by the hand, and...the fever left her.", caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ ("avnt017.tif", src);
  EXPECT_EQ ("col", size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ ("1.31", ref);
  EXPECT_EQ ("Text  text.", usfm_out);
  
  // Default USFM 1/2.x \fig definition.
  usfm_in = R"(Text \fig DESC|FILE|SIZE|LOC|COPY|CAP|REF\fig* text.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ ("CAP", caption);
  EXPECT_EQ ("DESC", alt);
  EXPECT_EQ ("FILE", src);
  EXPECT_EQ ("SIZE", size);
  EXPECT_EQ ("LOC", loc);
  EXPECT_EQ ("COPY", copy);
  EXPECT_EQ ("REF", ref);
  EXPECT_EQ ("Text  text.", usfm_out);
  
  // USFM 3.0 example.
  usfm_in = R"(\v 18 At once they left their nets and went with him. \fig At once they left their nets.|src="avnt016.jpg" size="span" ref="1.18"\fig*.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ ("At once they left their nets.", caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ ("avnt016.jpg", src);
  EXPECT_EQ ("span", size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ ("1.18", ref);
  EXPECT_EQ (R"(\v 18 At once they left their nets and went with him. .)", usfm_out);
  
  // USFM 3.0 example.
  usfm_in = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.|src="avnt017.tif" size="col" ref="1.31"\fig*.)";
  usfm_out = filter::usfm::extract_fig (usfm_in, caption, alt, src, size, loc, copy, ref);
  EXPECT_EQ ("Took her by the hand, and...the fever left her.", caption);
  EXPECT_EQ (std::string(), alt);
  EXPECT_EQ ("avnt017.tif", src);
  EXPECT_EQ ("col", size);
  EXPECT_EQ (std::string(), loc);
  EXPECT_EQ (std::string(), copy);
  EXPECT_EQ ("1.31", ref);
  EXPECT_EQ (R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. .)", usfm_out);
}


// Test determining standard q poetry styles.
TEST (usfm, is_standard_q_poetry)
{
  EXPECT_EQ (true,  filter::usfm::is_standard_q_poetry ("q"));
  EXPECT_EQ (true,  filter::usfm::is_standard_q_poetry ("q1"));
  EXPECT_EQ (true,  filter::usfm::is_standard_q_poetry ("q2"));
  EXPECT_EQ (true,  filter::usfm::is_standard_q_poetry ("q3"));
  EXPECT_EQ (false, filter::usfm::is_standard_q_poetry ("q4"));
  EXPECT_EQ (false, filter::usfm::is_standard_q_poetry ("q5"));
}


// Test checking valid USFM of the fig markup.
TEST (usfm, check_valid_fig)
{
  {
    const std::string usfm = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.|src="avnt017.tif" size="col" ref="1.31"\fig*.)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const std::vector <std::pair<int, std::string>> results = check.get_results ();
    const std::vector <std::pair<int, std::string>> standard = {
      {31,  "Could not find Bible image: avnt017.tif"},
    };
    EXPECT_EQ (standard, results);
  }
  {
    const std::string usfm = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.|src=“avnt017.tif“ size="col" ref="1.31"\fig*.)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    const std::vector <std::pair<int, std::string>> results = check.get_results ();
    const std::vector <std::pair<int, std::string>> standard = {
      {31,  R"(Empty figure source: Took her by the hand, and...the fever left her.|src=“avnt017.tif“ size="col" ref="1.31")"},
      {31,  R"(Unusual quotation mark found: Took her by the hand, and...the fever left her.|src=“avnt017.tif“ size="col" ref="1.31")"},
    };
    EXPECT_EQ (standard, results);
  }
}


// Text detecting markup sequence without intervening text.
TEST (usfm, check_markers_without_embedded_text)
{
  std::string usfm = R"(\p \v 1 One \add \add* \v 2 Two)";
  Checks_Usfm check = Checks_Usfm (std::string());
  check.initialize (0, 0);
  check.check (usfm);
  check.finalize ();
  std::vector <std::pair<int, std::string>> results = check.get_results ();
  std::vector <std::pair<int, std::string>> standard = {
    std::pair (1,  R"(Opening markup is followed by closing markup without intervening text: \add \add*)")
  };
  EXPECT_EQ (standard, results);
}


TEST (usfm, check_notes)
{
  // Test that a properly formatted note gives no checking results.
  {
    std::string usfm = R"(\v 1 \f + \ft text\f*)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { };
    EXPECT_EQ (standard, results);
  }

  // Test a note consisting of opening markup without text.
  // It should give a checking result.
  {
    std::string usfm = R"(\v 2 \fe + \ft \fe*)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {
      std::pair (2,  R"(Opening markup is followed by closing markup without intervening text: \ft \fe*)")
    };
    EXPECT_EQ (standard, results);
  }

  // Test that a correctly formatted note,
  // followed by incorrect markup, is still not flagged.
  {
    std::string usfm = R"(\v 3 \x + \xt xref \x* \q \q )";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { };
    EXPECT_EQ (standard, results);
  }

  // Test that a a note e.g. \ft followed by e.g. \add, is not flagged.
  {
    std::string usfm = R"(\v 4 \f + \ft \add add\add* \f*)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { };
    EXPECT_EQ (standard, results);
  }

  // Test a corect cross reference is not flagged.
  {
    std::string usfm = R"(\v 5 \x + \xt xref\x*)";
    Checks_Usfm check = Checks_Usfm (std::string());
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { };
    EXPECT_EQ (standard, results);
  }
}


TEST (usfm, check_all_markers)
{
  // Check that the USFM fragment with all markers has all markers defined according to the stylesheet v2.
  const std::string usfm {filter::strings::replace ("\n", " ", usfm_with_all_markers)};
  for (const std::string& marker : database::styles::get_markers (stylesv2::standard_sheet())) {
    const std::string opener = filter::usfm::get_opening_usfm (marker);
    const size_t pos = usfm.find(opener);
    if (pos == std::string::npos) {
      ADD_FAILURE() << "The standard stylesheet v2 contains " << std::quoted(marker) << " but the fragment of USFM with all markers does not contain " << opener;
    }
  }
  // Checkk that the default style definitions have no duplicates.
  {
    std::set<std::string> markers{};
    for (const auto& style : stylesv2::styles) {
      if (markers.count(style.marker)) {
        ADD_FAILURE() << "Duplicate style definition for marker " << style.marker;
      }
      markers.insert(style.marker);
    }
  }
}


TEST (usfm, usfm_import_all_markers)
{
  using namespace filter::usfm;
  const std::string stylesheet = stylesv2::standard_sheet ();

  const std::vector<BookChapterData> book_chapter_data {usfm_import (usfm_with_all_markers, stylesheet)};
  for (const auto& data : book_chapter_data) {
    EXPECT_EQ (data.m_book, static_cast<int>(book_id::_genesis));
    const int chapter = data.m_chapter;
    constexpr std::array<int,10> chapters {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    if (std::find(chapters.cbegin(), chapters.cend(), chapter) == chapters.cend()) {
      ADD_FAILURE() << "Unexpected chapter number " << chapter;
    }
    const std::string usfm = data.m_data;
    const size_t pos = std::string(usfm_with_all_markers).find(usfm);
    if (pos == std::string::npos)
      ADD_FAILURE() << "The import routine created this different USFM fragment:\n" << usfm;
  }
}


// Test making one string out of the USFM.
TEST (usfm, one_string)
{
  EXPECT_EQ ("", filter::usfm::one_string (""));
  
  EXPECT_EQ (R"(\id GEN)", filter::usfm::one_string (R"(\id GEN)" "\n"));
  
  EXPECT_EQ (R"(\v 10 text)", filter::usfm::one_string (R"(\v 10)" "\n" "text"));
  
  EXPECT_EQ (R"(\v 10\v 11)", filter::usfm::one_string (R"(\v 10)" "\n" R"(\v 11)"));
  
  EXPECT_EQ (R"(\v 10 text\p\v 11)", filter::usfm::one_string (R"(\v 10 text)" "\n" R"(\p\v 11)"));
  
  {
    const std::string inputusfm =
    R"(\v 9  If we confess our sins, he is faithful and just to forgive)" "\n"
    R"(us \add our\add* sins, and to cleanse us from all unrighteousness.)";
    const std::string outputusfm = filter::usfm::one_string (inputusfm);
    const std::string standard = R"(\v 9  If we confess our sins, he is faithful and just to forgive us \add our\add* sins, and to cleanse us from all unrighteousness.)";
    EXPECT_EQ (standard, outputusfm);
  }

  {
    const std::string one_string = filter::usfm::one_string (usfm1);
    EXPECT_EQ (R"(\c 1\s The Creation\p\v 1 In the beginning, \bd God\bd* created ...\p\v 2)", one_string);
  }
}


#endif

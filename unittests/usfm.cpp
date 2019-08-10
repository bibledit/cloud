/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <unittests/usfm.h>
#include <unittests/utilities.h>
#include <styles/logic.h>
#include <checks/usfm.h>
#include <filter/usfm.h>
#include <filter/url.h>
#include <filter/string.h>


void test_usfm ()
{
  trace_unit_tests (__func__);
  
  string bible = "bible";
  
  // Test check on malformed verse.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 2,He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (2, "Malformed verse number: \\v 2,He said.")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on new line in USFM as properly formatted.
  {
    string usfm =
    "\\c 1\n"
    "\\p He said.\n"
    "\\v 1 He said.\n"
    "\\p He said.\n"
    "\\v 2 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on new line in USFM.
  {
    string usfm =
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
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "New line within USFM:  \\ \\p He s")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on new line in USFM.
  {
    string usfm =
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
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "New line within USFM:  \\ \\p He s")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  
  // Test check on unknown USFM.
  {
    string usfm =
    "\\c 1\n"
    "\\p He said to the \\+nd LORD\\+nd*.\n"
    "\\v 1 He said \\add something\\add*.\n"
    "\\p,p He said.\n"
    "\\v 3 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Marker not in stylesheet: \\p,p ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on unknown USFM.
  {
    string usfm =
    "\\c 1\n"
    "\\pHe said.\n"
    "\\v 1 He said \\add something\\add*.\n"
    "\\p He said.\n"
    "\\v 3 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Marker not in stylesheet: \\pHe ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on \id.
  {
    string usfm =
    "\\id GENN\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Unknown ID: \\id GENN")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on \id.
  {
    string usfm =
    "\\id\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Unknown ID: \\id\\p ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on \id.
  {
    string usfm =
    "\\id Gen\n"
    "\\p He said.\n"
    "\\v 1 He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Unknown ID: \\id Gen")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on forward slash.
  {
    string usfm =
    "\\id GEN\n"
    "\\p He said.\n"
    "\\v 1 He said. He said something/add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Forward slash instead of backslash: /add")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on forward slash.
  {
    string usfm =
    "\\id GEN\n"
    "\\p He said.\n"
    "\\v 1 /v He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (0, "Forward slash instead of backslash: /v")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test that checking clean USFM has no output.
  {
    string usfm =
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
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on a widow backslash.
  {
    string usfm =
    "\\id GEN\n"
    "\\p\n"
    "\\v 1 \\ He said.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Widow backslash: \\ ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on matching markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1  He said \\add addition\\add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on matching markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1  He said addition\\add*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Closing marker does not match opening marker : \\add*")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on matching markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1  He said \\add addition\\add .\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Repeating opening marker: \\add "),
      make_pair (1, "Unclosed markers: add")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on matching markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 8 Kodwa uNowa wazuza umusa emehlweni eN\\nd kosi\\x + 19.19.\\nd*\\x*.\n"
    "\\v 9 Kodwa uNowa wazuza umusa emehlweni eN\\nd kosi\\x + 19.19.\\x*\\nd*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test check on matching markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 8 Kodwa uNowa wazuza umusa \\add emehlweni eN\\nd kosi\\x + 19.19.\\nd*\\x*.\n";
    Checks_Usfm check = Checks_Usfm (bible);
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (8, "Embedded marker requires a plus sign: \\nd kosi"),
      make_pair (8, "Embedded marker requires a plus sign: \\nd*\\x*"),
      make_pair (8, "Unclosed markers: add"),
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Check on correct \toc[1-3] markers.
  {
    string usfm =
    "\\id GEN\n"
    "\\toc1 The book of Genesis\n"
    "\\toc2 Genesis\n"
    "\\toc3 Gen\n";
    Checks_Usfm check ("");
    check.initialize (1, 0);
    check.check (usfm);
    check.finalize ();
    evaluate (__LINE__, __func__, 0, check.getResults ().size ());
    vector <pair<int, string>> results = check.getResults ();
  }

  // The \toc[1-3] markers are the wrong chapter.
  {
    string usfm =
    "\\id GEN\n"
    "\\toc1 The book of Genesis\n"
    "\\toc2 Genesis\n"
    "\\toc3 Gen\n";
    Checks_Usfm check ("");
    check.initialize (1, 2);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    evaluate (__LINE__, __func__, 3, results.size ());
    vector <pair<int, string>> standard = {
      make_pair (0, "The following marker belongs in chapter 0: \\toc1 "),
      make_pair (0, "The following marker belongs in chapter 0: \\toc2 "),
      make_pair (0, "The following marker belongs in chapter 0: \\toc3 ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Lacks \toc# markers.
  {
    string usfm =
    "\\id GEN\n";
    Checks_Usfm check ("");
    check.initialize (3, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    evaluate (__LINE__, __func__, 2, results.size ());
    vector <pair<int, string>> standard = {
      make_pair (0, "The book lacks the marker for the verbose book name: \\toc1 "),
      make_pair (0, "The book lacks the marker for the short book name: \\toc2 ")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  
  // Test converting line number to verse number.
  {
    string usfm =
    "\\id MIC";
    evaluate (__LINE__, __func__, {0}, usfm_linenumber_to_versenumber (usfm, 0));
  }
  
  // Test converting line number to verse number.
  {
    string usfm =
    "\\id MIC\n"
    "\\v 1 Verse";
    evaluate (__LINE__, __func__, {1}, usfm_linenumber_to_versenumber (usfm, 1));
  }
  
  // Test converting line number to verse number.
  {
    string usfm =
    "\\v 1 Verse";
    evaluate (__LINE__, __func__, {1}, usfm_linenumber_to_versenumber (usfm, 0));
  }
  
  // Test converting line number to verse number.
  {
    string usfm =
    "\\p\n"
    "\\v 3 Verse 3 (out of order).\n"
    "\\v 1 Verse 1. \n"
    "\\v 2 Verse 1.";
    evaluate (__LINE__, __func__, {0}, usfm_linenumber_to_versenumber (usfm, 0));
    evaluate (__LINE__, __func__, {3}, usfm_linenumber_to_versenumber (usfm, 1));
    evaluate (__LINE__, __func__, {1}, usfm_linenumber_to_versenumber (usfm, 2));
    evaluate (__LINE__, __func__, {2}, usfm_linenumber_to_versenumber (usfm, 3));
  }
  
  // Test converting line number to verse number.
  {
    string usfm =
    "\\id MIC\n"
    "\\v 1-2 Verse";
    evaluate (__LINE__, __func__, {1, 2}, usfm_linenumber_to_versenumber (usfm, 1));
  }
  
  // Test converting offset to verse number.
  {
    string usfm = "\\id MIC";
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 0));
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 7));
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 17));
  }
  
  // Test converting offset to verse number.
  {
    string usfm =
    "\\id MIC\n"
    "\\v 1 Verse";
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 7));
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 8));
  }
  
  // Test converting offset to verse number.
  {
    string usfm =
    "\\id MIC\n"
    "\\v 1-3 Verse";
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 7));
    evaluate (__LINE__, __func__, {1, 2, 3}, usfm_offset_to_versenumber (usfm, 8));
  }
  
  // Test converting offset to verse number.
  {
    string usfm =
    "\\v 1 Verse";
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 0));
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 2));
  }
  
  // Test converting offset to verse number.
  {
    string usfm =
    "\\p\n"
    "\\v 3 Verse 3 (out of order).\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.";
    
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 0));
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 1));
    
    evaluate (__LINE__, __func__, {0}, usfm_offset_to_versenumber (usfm, 2));
    evaluate (__LINE__, __func__, {3}, usfm_offset_to_versenumber (usfm, 3));
    evaluate (__LINE__, __func__, {3}, usfm_offset_to_versenumber (usfm, 4));
    
    evaluate (__LINE__, __func__, {3}, usfm_offset_to_versenumber (usfm, 31));
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 32));
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 33));
    
    evaluate (__LINE__, __func__, {1}, usfm_offset_to_versenumber (usfm, 45));
    evaluate (__LINE__, __func__, {2}, usfm_offset_to_versenumber (usfm, 46));
    evaluate (__LINE__, __func__, {2}, usfm_offset_to_versenumber (usfm, 47));
  }
  
  // Test converting verse number to offset.
  {
    string usfm =
    "\\p\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.\n"
    "\\v 3 Verse 3.\n"
    "\\v 4-5 Verse 4 and 5.";
    evaluate (__LINE__, __func__, 3, usfm_versenumber_to_offset (usfm, 1));
    evaluate (__LINE__, __func__, 17, usfm_versenumber_to_offset (usfm, 2));
    evaluate (__LINE__, __func__, 31, usfm_versenumber_to_offset (usfm, 3));
    evaluate (__LINE__, __func__, 45, usfm_versenumber_to_offset (usfm, 4));
    evaluate (__LINE__, __func__, 45, usfm_versenumber_to_offset (usfm, 5));
    evaluate (__LINE__, __func__, 66, usfm_versenumber_to_offset (usfm, 6));
    evaluate (__LINE__, __func__, 66, usfm_versenumber_to_offset (usfm, 6));
  }

  // Testing getting USFM for verse, basic and for Quill-based verse editor.
  {
    string usfm =
    "\\p\n"
    "\\v 1 One";
    evaluate (__LINE__, __func__, "\\p", usfm_get_verse_text (usfm, 0));
    evaluate (__LINE__, __func__, "", usfm_get_verse_text_quill (usfm, 0));
    evaluate (__LINE__, __func__, "\\v 1 One", usfm_get_verse_text (usfm, 1));
    evaluate (__LINE__, __func__, usfm, usfm_get_verse_text_quill (usfm, 1));
    evaluate (__LINE__, __func__, "", usfm_get_verse_text (usfm, 2));
    evaluate (__LINE__, __func__, "", usfm_get_verse_text_quill (usfm, 2));
  }
  
  // Testing getting USFM for verse, basic and Quill.
  {
    string usfm =
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
    
    string result =
    "\\c 1\n"
    "\\s Isibingelelo\n"
    "\\p";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 0));
    result =
    "\\c 1\n"
    "\\s Isibingelelo";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1 Umdala\n"
    "\\p";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 1));
    result =
    "\\p\n"
    "\\v 1 Umdala";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2 Sithandwa";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 2));
    result =
    "\\p\n"
    "\\v 2 Sithandwa";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 2));
    
    result =
    "\\v 3 Ngoba";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 3));
    result =
    "\\v 3 Ngoba";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4 Kangilantokozo\n"
    "\\s Inkathazo\n"
    "\\p";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 4));
    result =
    "\\v 4 Kangilantokozo\n"
    "\\s Inkathazo";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 4));
    
    result =
    "\\v 5 Sithandwa";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 5));
    result =
    "\\p\n"
    "\\v 5 Sithandwa";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 5));
    
    result =
    "\\v 12 NgoDemetriyu\n"
    "\\s Isicino\n"
    "\\p";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 12));
    result =
    "\\v 12 NgoDemetriyu\n"
    "\\s Isicino";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 12));
    
    result =
    "\\v 14 kodwa\n"
    "\\p Ukuthula";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 14));
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 14));
    
    result.clear ();
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 15));
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 15));
  }
 
  // Test getting the verse text from USFM.
  {
    string usfm =
    "\\v 1 Verse 1.\n"
    "\\v 2-4 Verse 2, 3, and 4.\n"
    "\\v 5 Verse 5.\n"
    "\\v 6 Verse 6.";
    string result;
    
    result = usfm_get_verse_text (usfm, 2);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
    result = usfm_get_verse_text_quill (usfm, 2);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
    
    result = usfm_get_verse_text (usfm, 3);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
    result = usfm_get_verse_text_quill (usfm, 3);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
    
    result = usfm_get_verse_text (usfm, 4);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
    result = usfm_get_verse_text_quill (usfm, 4);
    evaluate (__LINE__, __func__, "\\v 2-4 Verse 2, 3, and 4.", result);
  }
  
  // Testing USFM extraction for Quill-based visual verse editor with more than one empty paragraph in sequence.
  {
    string usfm =
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
    string result;
    
    result =
    "\\c 1\n"
    "\\b\n"
    "\\p"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 0));
    result =
    "\\c 1"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1 One"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 1));
    result =
    "\\b\n"
    "\\p\n"
    "\\v 1 One"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2 Two\n"
    "\\b\n"
    "\\p"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 2));
    result =
    "\\v 2 Two"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 2));
    
    result =
    "\\v 3 Three"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 3));
    result =
    "\\b\n"
    "\\p\n"
    "\\v 3 Three"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4 Four"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 4));
    result =
    "\\v 4 Four"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 4));
    
    result.clear ();
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 5));
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 5));
  }
  
  // Testing USFM extraction for Quill-based visual verse editor with empty verses.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1\n"
    "\\v 2\n"
    "\\p\n"
    "\\v 3\n"
    "\\v 4\n"
    "";
    string result;
    
    result =
    "\\c 1\n"
    "\\p"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 0));
    result =
    "\\c 1"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 0));
    
    result =
    "\\v 1"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 1));
    result =
    "\\p\n"
    "\\v 1"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 1));
    
    result =
    "\\v 2\n"
    "\\p"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 2));
    result =
    "\\v 2"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 2));
    
    result =
    "\\v 3"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 3));
    result =
    "\\p\n"
    "\\v 3"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 3));
    
    result =
    "\\v 4"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 4));
    result =
    "\\v 4"
    "";
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 4));
    
    result.clear ();
    evaluate (__LINE__, __func__, result, usfm_get_verse_text (usfm, 5));
    evaluate (__LINE__, __func__, result, usfm_get_verse_text_quill (usfm, 5));
  }
  
  // Test getting the USFM for a verse where there's alternate verse number or published verse numbers.
  // It used to get mixed up on those cases.
  // This regression test notices whether it keeps behaving as it should.
  {
    string path = filter_url_create_root_path ("unittests", "tests", "usfm01.usfm");
    string chapter_usfm = filter_url_file_get_contents (path);
    string usfm;
    
    usfm = usfm_get_verse_text (chapter_usfm, 1);
    evaluate (__LINE__, __func__, "\\v 1 Verse 1.", usfm);
    
    usfm = usfm_get_verse_text (chapter_usfm, 12);
    evaluate (__LINE__, __func__, "\\v 12 Verse 12 one.\n"
              "\\p \\va 1b \\va* Alternate verse 1b, \\va 2 \\va* alternate verse 2.\n"
              "\\s Header 13\n"
              "\\r refs 13\n"
              "\\p", usfm);
  }
 
  // Test getting text from USFM with a range of verses.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three\n"
    "\\v 4 Four\n"
    "\\v 5 Five";
    string result;
    
    result =
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 1, 2, "", false));
    result =
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 1, 2, "", true));
    
    result =
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 1, 3, "", false));
    result =
    "\\p\n"
    "\\v 1 One\n"
    "\\v 2-3 Two three";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 1, 3, "", true));
    
    result =
    "\\v 4 Four";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 3, 4, "\\v 2-3 Two three", false));
    result =
    "\\v 4 Four";
    evaluate (__LINE__, __func__, result, usfm_get_verse_range_text (usfm, 3, 4, "\\v 2-3 Two three", true));
  }

  // Test on detecting a general USFM marker.
  {
    evaluate (__LINE__, __func__, true, usfm_is_usfm_marker ("\\id"));
    evaluate (__LINE__, __func__, true, usfm_is_usfm_marker ("\\c "));
    evaluate (__LINE__, __func__, false, usfm_is_usfm_marker ("c"));
    evaluate (__LINE__, __func__, true, usfm_is_usfm_marker ("\\add "));
    evaluate (__LINE__, __func__, true, usfm_is_usfm_marker ("\\add*"));
    evaluate (__LINE__, __func__, true, usfm_is_usfm_marker ("\\+add*"));
  }
  
  // Test on detecting an opening marker.
  {
    evaluate (__LINE__, __func__, true, usfm_is_opening_marker ("\\id"));
    evaluate (__LINE__, __func__, true, usfm_is_opening_marker ("\\c "));
    evaluate (__LINE__, __func__, false, usfm_is_opening_marker ("\\c*"));
    evaluate (__LINE__, __func__, true, usfm_is_opening_marker ("\\+add "));
    evaluate (__LINE__, __func__, false, usfm_is_opening_marker ("\\+add*"));
  }
  
  // Test on detecting embedded marker.
  {
    evaluate (__LINE__, __func__, false, usfm_is_embedded_marker ("\\add"));
    evaluate (__LINE__, __func__, false, usfm_is_embedded_marker ("\\add*"));
    evaluate (__LINE__, __func__, true, usfm_is_embedded_marker ("\\+add"));
    evaluate (__LINE__, __func__, true, usfm_is_embedded_marker ("\\+add "));
    evaluate (__LINE__, __func__, true, usfm_is_embedded_marker ("\\+add*"));
  }

  // Test on extracting a book identifier.
  {
    evaluate (__LINE__, __func__, "GEN", usfm_get_book_identifier ({ "\\id", "GEN" }, 0));
    evaluate (__LINE__, __func__, "XXX", usfm_get_book_identifier ({ "\\id", "GEN" }, 1));
    evaluate (__LINE__, __func__, "GE", usfm_get_book_identifier ({ "\\id", "GE" }, 0));
    evaluate (__LINE__, __func__, "GEN", usfm_get_book_identifier ({ "\\id", "GENxxx" }, 0));
    evaluate (__LINE__, __func__, "GEN", usfm_get_book_identifier ({ "", "GENxxx" }, 0));
  }
  
  // Test on extracting a verse number.
  {
    evaluate (__LINE__, __func__, "1", usfm_peek_verse_number ("1"));
    evaluate (__LINE__, __func__, "1", usfm_peek_verse_number ("1 "));
    evaluate (__LINE__, __func__, "1a", usfm_peek_verse_number ("1a"));
    evaluate (__LINE__, __func__, "2-3", usfm_peek_verse_number ("2-3"));
    evaluate (__LINE__, __func__, "2b,3", usfm_peek_verse_number ("2b,3"));
    evaluate (__LINE__, __func__, "2b,3,", usfm_peek_verse_number ("2b,3, 4"));
    evaluate (__LINE__, __func__, "2a-3b", usfm_peek_verse_number ("2a-3b And he said"));
  }

  // Test on Genesis USFM.
  {
    string directory = filter_url_create_root_path ("unittests", "tests");
    string bookusfm = filter_url_file_get_contents (filter_url_create_path (directory, "01GEN.SFM"));
    
    // Test getting all chapter number from USFM.
    vector <int> chapters = usfm_get_chapter_numbers (bookusfm);
    vector <int> all_chapters;
    for (int i = 0; i <= 50; i++) all_chapters.push_back (i);
    evaluate (__LINE__, __func__, all_chapters, chapters);
    
    // Test getting contents for chapter 0.
    string usfm = usfm_get_chapter_text (bookusfm, 0);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "01GEN-0.SFM"));
    evaluate (__LINE__, __func__, standard, usfm);
    
    chapters = usfm_get_chapter_numbers (usfm);
    evaluate (__LINE__, __func__, { 0 }, chapters);
    
    // Test getting contents for last chapter in USFM.
    usfm = usfm_get_chapter_text (bookusfm, 50);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "01GEN-50.SFM"));
    evaluate (__LINE__, __func__, standard, usfm);
    
    chapters = usfm_get_chapter_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 50 }, chapters);
    
    // Test getting the text of a chapter somewhere within a block of USFM.
    usfm = usfm_get_chapter_text (bookusfm, 25);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "01GEN-25.SFM"));
    evaluate (__LINE__, __func__, standard, usfm);
    
    chapters = usfm_get_chapter_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 25 }, chapters);
    
    // Test getting non-existing chapter text.
    usfm = usfm_get_chapter_text (bookusfm, 51);
    evaluate (__LINE__, __func__, "", usfm);
    
    chapters = usfm_get_chapter_numbers (usfm);
    evaluate (__LINE__, __func__, { 0 }, chapters);
    
    // Test getting text for chapter that has a space after chapter number.
    string modified_book_usfm = filter_string_str_replace ("\\c 10", "\\c 10 ", bookusfm);
    usfm = usfm_get_chapter_text (modified_book_usfm, 10);
    standard = filter_url_file_get_contents (filter_url_create_path (directory, "01GEN-10.SFM"));
    evaluate (__LINE__, __func__, standard, usfm);
    
    chapters = usfm_get_chapter_numbers (modified_book_usfm);
    evaluate (__LINE__, __func__, all_chapters, chapters);
  }
  
  // Test making one string out of the USFM.
  {
    evaluate (__LINE__, __func__, "", usfm_one_string (""));
    evaluate (__LINE__, __func__, "\\id GEN", usfm_one_string ("\\id GEN\n"));
    evaluate (__LINE__, __func__, "\\v 10 text", usfm_one_string ("\\v 10\ntext"));
    evaluate (__LINE__, __func__, "\\v 10\\v 11", usfm_one_string ("\\v 10\n\\v 11"));
    evaluate (__LINE__, __func__, "\\v 10 text\\p\\v 11", usfm_one_string ("\\v 10 text\n\\p\\v 11"));
    string inputusfm =
    "\\v 9  If we confess our sins, he is faithful and just to forgive\n"
    "us \\add our\\add* sins, and to cleanse us from all unrighteousness.";
    string outputusfm = usfm_one_string (inputusfm);
    string standard = filter_string_str_replace ("\n", " ", inputusfm);
    evaluate (__LINE__, __func__, standard, outputusfm);
  }
  
  // Test getting the markers and the text from USFM.
  {
    evaluate (__LINE__, __func__, { "\\id ", "GEN", "\\c ", "10" }, usfm_get_markers_and_text ("\\id GEN\\c 10"));
    evaluate (__LINE__, __func__, { "noise", "\\id ", "GEN", "\\c ", "10" }, usfm_get_markers_and_text ("noise\\id GEN\\c 10"));
    evaluate (__LINE__, __func__, { "\\p", "\\v ", "1 In ", "\\add ", "the", "\\add*" }, usfm_get_markers_and_text ("\\p\\v 1 In \\add the\\add*"));
    evaluate (__LINE__, __func__, { "\\v ", "2 Text ", "\\add ", "of the ", "\\add*", "1st", "\\add ", "second verse", "\\add*", "." }, usfm_get_markers_and_text ("\\v 2 Text \\add of the \\add*1st\\add second verse\\add*."));
    evaluate (__LINE__, __func__, { "\\p", "\\v ", "1 In ", "\\+add ", "the", "\\+add*" }, usfm_get_markers_and_text ("\\p\\v 1 In \\+add the\\+add*"));
  }

  // Test getting the markers from a fragment of USFM.
  {
    evaluate (__LINE__, __func__, "", usfm_get_marker (""));
    evaluate (__LINE__, __func__, "id", usfm_get_marker ("\\id GEN"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\add insertion"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\add"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\add*"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\add*\\add"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\+add"));
    evaluate (__LINE__, __func__, "add", usfm_get_marker ("\\+add*"));
  }

  // Test importing USFM.
  {
    evaluate (__LINE__, __func__, 0, (int)usfm_import ("", styles_logic_standard_sheet ()).size());
    vector <BookChapterData> import2 = usfm_import ("\\id MIC\n\\c 1\n\\s Heading\n\\p\n\\v 1 Verse one.", styles_logic_standard_sheet ());
    evaluate (__LINE__, __func__, 2, (int)import2.size());
    if (import2.size () == 2) {
      evaluate (__LINE__, __func__, 33, import2 [0].book);
      evaluate (__LINE__, __func__, 0, import2 [0].chapter);
      evaluate (__LINE__, __func__, "\\id MIC", import2 [0].data);
      evaluate (__LINE__, __func__, 33, import2 [1].book);
      evaluate (__LINE__, __func__, 1, import2 [1].chapter);
      evaluate (__LINE__, __func__, "\\c 1\n\\s Heading\n\\p\n\\v 1 Verse one.", import2 [1].data);
    } else evaluate (__LINE__, __func__, "executing tests", "skipping tests");
    
    evaluate (__LINE__, __func__, {0, 1, 2}, usfm_get_verse_numbers ("\\v 1 test\\v 2 test"));
  }
  
  // Test importing USFM.
  {
    string usfm = filter_url_file_get_contents (filter_url_create_root_path ("demo", "92-1JNeng-web.usfm"));
    vector <BookChapterData> import = usfm_import (usfm, styles_logic_standard_sheet ());
    // It imports book 0 due to the copyright notices at the top of the USFM file.
    evaluate (__LINE__, __func__, 7, (int)import.size());
  }

  // Test building opening USFM marker.
  {
    evaluate (__LINE__, __func__, "\\id ", usfm_get_opening_usfm ("id"));
    evaluate (__LINE__, __func__, "\\add ", usfm_get_opening_usfm ("add"));
    evaluate (__LINE__, __func__, "\\add ", usfm_get_opening_usfm ("add", false));
    evaluate (__LINE__, __func__, "\\+add ", usfm_get_opening_usfm ("add", true));
  }
  
  // Test building closing USFM marker.
  {
    evaluate (__LINE__, __func__, "\\wj*", usfm_get_closing_usfm ("wj"));
    evaluate (__LINE__, __func__, "\\add*", usfm_get_closing_usfm ("add"));
    evaluate (__LINE__, __func__, "\\add*", usfm_get_closing_usfm ("add", false));
    evaluate (__LINE__, __func__, "\\+add*", usfm_get_closing_usfm ("add", true));
  }

  // Test getting verse numbers from USFM.
  {
    string usfm = "\\c 1\n\\s Isibingelelo\n\\p\n\\v 1 Umdala\n\\p\n\\v 2 Sithandwa\n\\v 3 Ngoba\n\\v 4 Kangilantokozo\n\\s Inkathazo\n\\p\n\\v 5 Sithandwa\n\\v 6 abafakazele\n\\v 7 Ngoba\n\\v 8 Ngakho\n\\p\n\\v 9 Ngabhalela\n\\v 10 Ngakho\n\\p\n\\v 11 Sithandwa\n\\v 12 NgoDemetriyu\n\\s Isicino\n\\p\n\\v 13 Bengilezinto\n\\v 14 kodwa\n\\p Ukuthula";
    evaluate (__LINE__, __func__, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, usfm_get_verse_numbers (usfm));
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
    evaluate (__LINE__, __func__, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 }, usfm_get_verse_numbers (usfm));
  }
  
  // Test getting verse numbers from USFM.
  {
    string usfm = "\\v 1-2 Umdala\n\\p\n\\v 3 Ngoba\n";
    vector <int> verses = usfm_get_verse_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 1, 2, 3 }, verses);
    
    usfm = "\\v 10-12b Fragment\n\\p\n\\v 13 Fragment\n";
    verses = usfm_get_verse_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 10, 11, 12, 13 }, verses);
    
    usfm = "\\v 10,11a Fragment\n\\p\n\\v 13 Fragment\n";
    verses = usfm_get_verse_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 10, 11, 13 }, verses);
    
    usfm = "\\v 10,12 Fragment\n\\p\n\\v 13 Fragment\n";
    verses = usfm_get_verse_numbers (usfm);
    evaluate (__LINE__, __func__, { 0, 10, 12, 13 }, verses);
  }
  
  // Testing on USFM without verse text.
  {
    string usfm;
    
    usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.";
    evaluate (__LINE__, __func__, false, usfm_contains_empty_verses (usfm));
    
    usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.\n"
    "\\v 2 Two";
    evaluate (__LINE__, __func__, false, usfm_contains_empty_verses (usfm));
    
    usfm = "\\v 1 Zvino namazuva\\x + Gen.1.1.\\x* okutonga kwavatongi nzara yakange iripo panyika.\n"
    "\\v 2";
    evaluate (__LINE__, __func__, true, usfm_contains_empty_verses (usfm));
    
    usfm = "\\v 1 \n"
    "\\v 2 Two";
    evaluate (__LINE__, __func__, true, usfm_contains_empty_verses (usfm));
    
    usfm = "\\v 1 \n"
    "\\v 2 ";
    evaluate (__LINE__, __func__, true, usfm_contains_empty_verses (usfm));
  }
  
  // Testing removing word level attributes.
  {
    string usfm;
    string result;
    string standard;

    usfm = R"(\v 1 This is an example \w gracious|lemma="grace"\w* this is an example \w gracious|grace\w* this is an example \w gracious|strong="H01234,G05485"\w* this is an example \w gracious\w* this is an example \w gracious|x-myattr="metadata"\w* this is an example \w gracious|lemma="grace" x-myattr="metadata"\w*.)";
    result = usfm_remove_word_level_attributes (usfm);
    standard = R"(\v 1 This is an example \w gracious\w* this is an example \w gracious\w* this is an example \w gracious\w* this is an example \w gracious\w* this is an example \w gracious\w* this is an example \w gracious\w*.)";
    evaluate (__LINE__, __func__, standard, result);

    usfm = R"(\v 18 At once they left their nets and went with him. \fig At once they left their nets.|src="avnt016.jpg" size="span" ref="1.18"\fig*)";
    result = usfm_remove_word_level_attributes (usfm);
    standard = R"(\v 18 At once they left their nets and went with him. \fig At once they left their nets.\fig*)";
    evaluate (__LINE__, __func__, standard, result);

    usfm = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.|src="avnt017.tif" size="col" ref="1.31"\fig*)";
    result = usfm_remove_word_level_attributes (usfm);
    standard = R"(\v 31 He went to her, took her by the hand, and helped her up. The fever left her, and she began to wait on them. \fig Took her by the hand, and...the fever left her.\fig*)";
    evaluate (__LINE__, __func__, standard, result);
  }
  
  // Test check on embedded markers.
  {
    string usfm = R"(
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
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (2, "Embedded marker requires a plus sign: \\add sen"),
      make_pair (2, "Embedded marker requires a plus sign: \\add*rd3"),
      make_pair (4, "Embedded marker requires a plus sign: \\add sen"),
      make_pair (4, "Embedded marker requires a plus sign: \\add*tence"),
      make_pair (6, "Embedded marker requires a plus sign: \\add du"),
      make_pair (6, "Embedded marker requires a plus sign: \\add*e"),
    };
    evaluate (__LINE__, __func__, standard, results);
  }

  // Test on correct or incorrect \vp ...\vp* markup.
  {
    string usfm = R"(
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
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (3, "Unclosed markers: vp"),
    };
    evaluate (__LINE__, __func__, standard, results);
  }

}

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
#include <filter/diff.h>
#include <filter/merge.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/modifications.h>
#include <database/state.h>
#include <client/logic.h>
#include <bb/logic.h>


TEST (filter, diff)
{
  // Difference.
  {
    std::vector <std::string> removals, additions;
    std::string output = filter_diff_diff ("Old text", "New text", &removals, &additions);
    std::string standard = R"(<span style="text-decoration: line-through;"> Old </span> <span style="font-weight: bold;"> New </span> text)";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (std::vector <std::string>{"Old"}, removals);
    EXPECT_EQ (std::vector <std::string>{"New"}, additions);
  }

  // Difference.
  {
    std::vector <std::string> removals, additions;
    std::string output = filter_diff_diff ("this is really old text", "and this is new text",
                                      &removals, &additions);
    std::string standard = R"(<span style="font-weight: bold;"> and </span> this is <span style="text-decoration: line-through;"> really </span> <span style="text-decoration: line-through;"> old </span> <span style="font-weight: bold;"> new </span> text)";
    EXPECT_EQ (standard, output);
    EXPECT_EQ ((std::vector <std::string>{"really", "old"}), removals);
    EXPECT_EQ ((std::vector <std::string>{"and", "new"}), additions);
  }
  
  // Diff with new lines in the text.
  {
    std::vector <std::string> removals, additions;
    std::string oldtext =
    "Genesis 1.1 1 In the beginning God created the heavens and the earth.\n"
    "Genesis 1.2 2 And the earth was without form, and void; and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters.\n"
    "Genesis 1.3 3 And God said, Let there be light: and there was light.\n";
    
    std::string newtext =
R"(Genesis 1.1 1 In the beginning God created the heaven and the earth.
Genesis 1.2 2 And the earth was without form and void and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters.
Genesis 1.3 3 And God said: "Let there be light". And there was light.
)";
   
    std::string output = filter_diff_diff (oldtext, newtext, &removals, &additions);
    
    std::string standard = filter_url_file_get_contents (filter_url_create_root_path ({"unittests", "tests", "diff.txt"}));
    EXPECT_EQ (standard, output);
    
    EXPECT_EQ ((std::vector <std::string>{"heavens", "form,", "void;", "said,", "Let", "light:", "and"}),
               removals);
    EXPECT_EQ ((std::vector <std::string>{"heaven", "form", "void", "said:", R"("Let)", R"(light".)", "And"}),
               additions);
  }

  
  // Update differing content for use in Quilljs editor.
  {
    std::vector <std::string> oldinput = {"a", "badd", "c",      "e", "f", "g", "\np"     };
    std::vector <std::string> newinput = {"a", "badd", "c", "d", "e", "f",      "\np", "h"};
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <bool> additions;
    std::vector <std::string> content;
    int new_line_diff_count;
    filter_diff_diff_utf16 (oldinput, newinput, positions, sizes, additions, content, new_line_diff_count);
    EXPECT_EQ ((std::vector <int>{3, 6, 7}), positions);
    EXPECT_EQ ((std::vector <int>{1, 1, 1}), sizes);
    EXPECT_EQ ((std::vector <bool>{true, false, true}), additions);
    EXPECT_EQ ((std::vector <std::string>{"d", "g", "h"}), content);
    EXPECT_EQ (0, new_line_diff_count);
  }
  {
    std::vector <std::string> oldinput = {"a",         "c",      "e", "f", "g", "\np",     "i"};
    std::vector <std::string> newinput = {"a", "badd", "c", "d", "e", "f",      "\np", "h"    };
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <bool> additions;
    std::vector <std::string> content;
    int new_line_diff_count;
    filter_diff_diff_utf16 (oldinput, newinput, positions, sizes, additions, content, new_line_diff_count);
    EXPECT_EQ ((std::vector <int>{1, 3, 6, 7, 8}), positions);
    EXPECT_EQ ((std::vector <int>{1, 1, 1, 1, 1}), sizes);
    EXPECT_EQ ((std::vector <bool>{true, true, false, true, false}), additions);
    EXPECT_EQ ((std::vector <std::string>{"badd", "d", "g", "h", "i"}), content);
    EXPECT_EQ (0, new_line_diff_count);
  }
  {
    std::vector <std::string> oldinput = {"\n", "\n", "a", "b", "\n", "\n", "c"      };
    std::vector <std::string> newinput = {"\n",       "a",      "\n", "b",  "c", "\n"};
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <bool> additions;
    std::vector <std::string> content;
    int new_line_diff_count;
    filter_diff_diff_utf16 (oldinput, newinput, positions, sizes, additions, content, new_line_diff_count);
    EXPECT_EQ ((std::vector <int>{1, 2, 3, 3, 5}), positions);
    EXPECT_EQ ((std::vector <int>{1, 1, 1, 1, 1}), sizes);
    EXPECT_EQ ((std::vector <bool>{false, false, false, true, true}), additions);
    EXPECT_EQ ((std::vector <std::string>{"\n", "b", "\n", "b", "\n"}), content);
    EXPECT_EQ (3, new_line_diff_count);
  }
  {
    // Positions                0     1     2    3     4    5     6     7    8
    std::vector <std::string> oldinput = {"\n", "\n", "a", "😀", "b", "\n", "\n", "c"      };
    std::vector <std::string> newinput = {"\n",       "a",            "\n", "😃", "c", "\n"};
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <bool> additions;
    std::vector <std::string> content;
    int new_line_diff_count;
    filter_diff_diff_utf16 (oldinput, newinput, positions, sizes, additions, content, new_line_diff_count);
    EXPECT_EQ ((std::vector <int>{1, 2, 2, 3, 3, 6}), positions);
    EXPECT_EQ ((std::vector <int>{1, 2, 1, 1, 2, 1}), sizes);
    EXPECT_EQ ((std::vector <bool>{false, false, false, false, true, true}), additions);
    EXPECT_EQ ((std::vector <std::string>{"\n", "😀", "b", "\n", "😃", "\n"}), content);
    EXPECT_EQ (3, new_line_diff_count);
  }

  
  // Character similarity.
  {
    int similarity = filter_diff_character_similarity ("Old text", "New text");
    EXPECT_EQ (45, similarity);
    
    similarity = filter_diff_character_similarity ("New text", "New text");
    EXPECT_EQ (100, similarity);
    
    similarity = filter_diff_character_similarity ("ABCDEFGH", "IJKLMNOPQRST");
    EXPECT_EQ (0, similarity);
  }
  
  // Similarity with text that used to crash the routine but was fixed.
  {
    // Invalid UTF8 results in 0% similarity.
    // An update was made on the routine that calculates the similarity between two strings.
    // It used to split the strings up into whole unicode points.
    // After the update it no longer does that.
    // It now splits the strings up into just bytes.
    // So the similarity is now more realistic.
    std::string path = filter_url_create_root_path ({"unittests", "tests"});
    std::string oldtext = filter_url_file_get_contents (filter_url_create_path ({path, "invalid-utf8-old.txt"}));
    std::string newtext = filter_url_file_get_contents (filter_url_create_path ({path, "invalid-utf8-new.txt"}));
    int similarity = filter_diff_character_similarity (oldtext, newtext);
    EXPECT_EQ (99, similarity);
  }
  
  // Similarity.
  {
    std::string first =
    "\\c 29\n"
    "\\s Kugadzwa kwevapristi\n"
    "\\p\n"
    "\\v 1 Zvino ichi ndicho chinhu chauchavaitira kuvaita vatsvene, kuti vandishumire pabasa reupristi: Tora nzombe imwe diki; nemakondohwe maviri asina chaanopomerwa,\n"
    "\\v 2 nechingwa chisina kuviriswa, nemakeke asina kuviriswa akakanyiwa nemafuta, nezvingwa zvitete zvisina kuviriswa zvine mafuta; uchazviita neupfu hwakatsetseka hwegorosi.\n"
    "\\v 3 Zvino uchazviisa mudengu rimwe, ugopinda nazvo zviri mudengu, pamwe nenzombe diki nemakondohwe maviri.\n"
    "\\v 4 NaAroni nevanakomana vake uchauya navo kumukova wetende rekusangana, ugovashambidza nemvura;\n"
    "\\v 5 uchatorawo hanzu ugopfekedza Aroni nguvo refu, nejazi reefodhi, neefodhi, nechidzitiro chechipfuva; uye umusunge chiuno nebhanhire reefodhi rakarukwa neumhizha;\n"
    "\\v 6 uisewo ngundu pamusoro wake, nekuisa korona tsvene pangundu.\n"
    "\\v 7 Zvino uchatora mafuta ekuzodza, ugodira pamusoro wake, ugomuzodza.\n"
    "\\v 8 Nevanakomana vake uchavaswededza pedo ugovapfekedza majazi.\n"
    "\\v 9 Uchavasungawo zviuno nebhanhire, Aroni nevanakomana vake, uvasungirire nguwani; uye basa reupristi richava ravo chive chimiso nekusingaperi; uye uchaeresa Aroni nevanakomana vake.\n"
    "\\v 10 Zvino uchauisa nzombe pamberi petende rekusangana, uye Aroni nevanakomana vake vachaisa maoko avo pamusoro pemusoro wenzombe.\n"
    "\\v 11 Zvino uchabaya nzombe pamberi paJehovha pamukova wetende rekusangana.\n"
    "\\v 12 Zvino uchatora ropa renzombe, ugoisa panyanga dzearitari nemunwe wako, ndokudururira ropa rese mujinga memuzasi mearitari:\n"
    "\\v 13 Zvino uchatora mafuta ese anofukidza zvemukati, nebutiro riri pamusoro pechiropa, neitsvo mbiri, nemafuta ari padziri uzvipise pamusoro pearitari.\n"
    "\\v 14 Asi nyama yenzombe, nedehwe rayo, nechizvizvi chayo, uchapisa nemoto kunze kwemisasa; chipiriso chezvivi.\n"
    "\\p\n"
    "\\v 15 Uchatorawo gondohwe rimwe, uye Aroni nevanakomana vake vachaisa maoko avo pamusoro pemusoro wegondohwe,\n"
    "\\v 16 zvino uchauraya gondohwe ndokutora ropa raro ndokusasa kupoteredza aritari.\n"
    "\\v 17 Zvino uchaguranya gondohwe ndokusuka zvemukati zvaro nemakumbo aro ugoisa panhindi dzaro nepamusoro waro.\n"
    "\\v 18 Zvino uchapisa gondohwe rese paaritari; chipiriso chinopiswa kuna Jehovha, hwema hunonhuhwira, chipiriso chinoitirwa Jehovha, nemoto.\n"
    "\\v 19 Zvino uchatora gondohwe rechipiri, uye Aroni nevanakomana vake vagoisa maoko avo pamusoro pemusoro wegondohwe.\n"
    "\\v 20 Ipapo uchauraya gondohwe ndokutora paropa raro, ndokuisa pamucheto wezasi wenzeve yerudyi yaAroni, nepamicheto yezasi yenzeve dzerudyi dzevanakomana vake, nepagunwe reruoko rwerudyi, nepagumwe guru retsoka yavo yerudyi, nekusasawo ropa paaritari kupoteredza.\n"
    "\\v 21 Zvino uchatora paropa riri paaritari nepamafuta ekuzodza ugosasa pamusoro paAroni napanguvo dzake nepamusoro pevanakomana vake, nepanguvo dzevanakomana vake pamwe naye, agoitwa mutsvene iye nenguvo dzake nevanakomana vake nenguvo dzevanakomana vake pamwe naye.\n"
    "\\v 22 Zvino utorewo mamwe mafuta egondohwe, nebemhe raro, namafuta anofukidza ura, namafuta ari pamusoro pechiropa, netsvo mbiri, namafuta ari padziri, nebandauko rorudyi (nokuti igondohwe rokugadza naro),\n"
    "\\v 23 nebundu rimwe rechingwa, nechingwa chimwe chidiki chinamafuta, nechingwa chimwe chitete chinobva mudengu rezvingwa zvisina kuviriswa, zviri pamberi paJehovha.\n"
    "\\v 24 Zvino uzviise zvese pamaoko aAroni napamaoko avanakomana vake, vagozvizunguzira, chive chipo chinozunguzirwa pamberi paJehovha.\n"
    "\\v 25 Zvino uzvibvise pamaoko avo, uzvipise paaritari pamusoro pechipiriso chinopiswa, kuti chive chipo chinonhuhwira pamberi paJehovha; chipiriso chakaitirwa Jehovha pamoto.\n"
    "\\v 26 Utorewo chityu chegondohwe rokugadza naro raAroni, urizunguzire pamberi paJehovha, kuti chive chipo chinozunguzirwa; ndiwo mugove wako.\n"
    "\\v 27 Zvino chityu chinozunguzirwa uchiite chitsvene, nebandauko rechipiriso chinosimudzwa, chinozunguzirwa nekusimudzwa, zvegondohwe rokugadza naro, kuti zvive zvaAroni nezvavanakomana vake.\n"
    "\\v 28 Izvozvo zvinofanira kupiwa Aroni navanakomana vake navana vaIsraeri nokusingaperi nokuti chipiriso chinosimudzwa, chinobva kuvana vaIsraeri pazvibairo zvezvipiriso zvavo zvokuyananisa, chive chipiriso chavo chinosimudzirwa Jehovha.\n"
    "\\p\n"
    "\\v 29 Nguvo dzaAroni dzichava dzavanakomana vake vanomutevera, kuti vazodzwe vakadzipfeka, vagadzwe vakadzipfeka.\n"
    "\\v 30 Mwanakomana wake anopinda paupristi panzvimbo yake, anofanira kudzipfeka mazuva manomwe, kana achipinda mutende rekusangana kushumira panzvimbo tsvene.\n"
    "\\p\n"
    "\\v 31 Zvino unofanira kutora gondohwe rokugadza naro, ubike nyama yaro panzvimbo tsvene.\n"
    "\\v 32 Aroni navanakomana vake ngavadye nyama yegondohwe, nechingwa chiri mudengu pamukova wetende rokusangana.\n"
    "\\v 33 Vanofanira kudya izvozvo zvaiyananiswa nazvo, pakugadzwa nokuitwa vatsvene kwavo; asi mweni ngaarege kuzvidya, nokuti zvitsvene.\n"
    "\\v 34 Kana imwe nyama yokugadza nayo ikasara, kana chimwe chingwa, kusvikira mangwana, unofanira kupisa zvasara nomoto; hazvifaniri kudyiwa, nokuti zvitsvene.\n"
    "\\p\n"
    "\\v 35 Unofanira kuitira Aroni navanakomana vake saizvozvo, zvese sezvandakuraira; uite mazuva manomwe uchivagadza.\n"
    "\\v 36 Zuva rimwe nerimwe unofanira kubaira nzombe yechipiriso chezvivi, chokuyananisa nacho; unofanira kunatsa aritari, kana uchiiyananisira; uizodze kuti uiite tsvene.\n"
    "\\v 37 Unofanira kuita mazuva manomwe pakuyananisira aritari, nokuiita tsvene; kuti aritari ive tsvene kwazvo, uye zvese zvinobata paaritari zvinofanira kuva zvitsvene.\n"
    "\\s Zvibairo zvezuva rimwe nerimwe\n"
    "\\p\n"
    "\\v 38 Zvino ndizvo zvaunofanira kubaira paaritari: Zuva rimwe nerimwe nguva dzese makwayana maviri.\n"
    "\\v 39 Rimwe gwayana unofanira kuribaira mangwanani, rimwe gwayana unofanira kuribaira madekwani.\n"
    "\\v 40 Pamwe chete negwayana rimwe unofanira kuisa chipiriso choupfu hwakatsetseka hwakasvika pachegumi cheefa hwakakanyiwa nechechina chehini yamafuta akasviniwa nechechina chehini yewaini, chive chipiriso chinodururwa.\n"
    "\\v 41 Rimwe gwayana uribaire madekwani, uriitire sezvawakaitira chipiriso choupfu chamangwanani, nezvawakaitira chipiriso charo chinomwiwa, kuti chive chipiriso chinonhuhwira, chipiriso chakaitirwa Jehovha pamoto.\n"
    "\\v 42 Chinofanira kuva chipiriso chinopiswa nguva dzese pamarudzi enyu ese, pamukova wetende rekusangana pamberi paJehovha, pandichasangana nemi, nditaurepo newe.\n"
    "\\v 43 Ndipo pandichasangana nevana vaIsraeri, uye tende richaitwa dzvene nekubwinya kwangu.\n"
    "\\v 44 Tende rokusangana nearitari ndichazviita zvitsvene; uye Aroni navanakomana vake ndichavaita vatsvene, kuti vandishumire pabasa roupristi.\n"
    "\\v 45 Ndichagara pakati pavana vaIsraeri, kuti ndive Mwari wavo.\n"
    "\\v 46 Vachaziva kuti ndini Jehovha wavo, wakavabudisa panyika kuti ndigare pakati pavo; ndini Jehovha Mwari wavo.\n";
    
    std::string second =
    "\\c 29\n"
    "\\s Kugadzwa kwevapristi\n"
    "\\p\n"
    "\\v 1 Zvino ichi ndicho chinhu chauchavaitira kuvaita vatsvene, kuti vandishumire pabasa reupristi: Tora nzombe imwe diki; nemakondohwe maviri asina chaanopomerwa,\n"
    "\\v 2 nechingwa chisina kuviriswa, nemakeke asina kuviriswa akakanyiwa nemafuta, nezvingwa zvitete zvisina kuviriswa zvine mafuta; uchazviita neupfu hwakatsetseka hwegorosi.\n"
    "\\v 3 Zvino uchazviisa mudengu rimwe, ugopinda nazvo zviri mudengu, pamwe nenzombe diki nemakondohwe maviri.\n"
    "\\v 4 NaAroni nevanakomana vake uchauya navo kumukova wetende rekusangana, ugovashambidza nemvura;\n"
    "\\v 5 uchatorawo hanzu ugopfekedza Aroni nguvo refu, nejazi reefodhi, neefodhi, nechidzitiro chechipfuva; uye umusunge chiuno nebhanhire reefodhi rakarukwa neumhizha;\n"
    "\\v 6 uisewo ngundu pamusoro wake, nekuisa korona tsvene pangundu.\n"
    "\\v 7 Zvino uchatora mafuta ekuzodza, ugodira pamusoro wake, ugomuzodza.\n"
    "\\v 8 Nevanakomana vake uchavaswededza pedo ugovapfekedza majazi.\n"
    "\\v 9 Uchavasungawo zviuno nebhanhire, Aroni nevanakomana vake, uvasungirire nguwani; uye basa reupristi richava ravo chive chimiso nekusingaperi; uye uchaeresa Aroni nevanakomana vake.\n"
    "\\v 10 Zvino uchauisa nzombe pamberi petende rekusangana, uye Aroni nevanakomana vake vachaisa maoko avo pamusoro pemusoro wenzombe.\n"
    "\\v 11 Zvino uchabaya nzombe pamberi paJehovha pamukova wetende rekusangana.\n"
    "\\v 12 Zvino uchatora ropa renzombe, ugoisa panyanga dzearitari nemunwe wako, ndokudururira ropa rese mujinga memuzasi mearitari:\n"
    "\\v 13 Zvino uchatora mafuta ese anofukidza zvemukati, nebutiro riri pamusoro pechiropa, neitsvo mbiri, nemafuta ari padziri uzvipise pamusoro pearitari.\n"
    "\\v 14 Asi nyama yenzombe, nedehwe rayo, nechizvizvi chayo, uchapisa nemoto kunze kwemisasa; chipiriso chezvivi.\n"
    "\\p\n"
    "\\v 15 Uchatorawo gondohwe rimwe, uye Aroni nevanakomana vake vachaisa maoko avo pamusoro pemusoro wegondohwe,\n"
    "\\v 16 zvino uchauraya gondohwe ndokutora ropa raro ndokusasa kupoteredza aritari.\n"
    "\\v 17 Zvino uchaguranya gondohwe ndokusuka zvemukati zvaro nemakumbo aro ugoisa panhindi dzaro nepamusoro waro.\n"
    "\\v 18 Zvino uchapisa gondohwe rese paaritari; chipiriso chinopiswa kuna Jehovha, hwema hunonhuhwira, chipiriso chinoitirwa Jehovha, nemoto.\n"
    "\\v 19 Zvino uchatora gondohwe rechipiri, uye Aroni nevanakomana vake vagoisa maoko avo pamusoro pemusoro wegondohwe.\n"
    "\\v 20 Ipapo uchauraya gondohwe ndokutora paropa raro, ndokuisa pamucheto wezasi wenzeve yerudyi yaAroni, nepamicheto yezasi yenzeve dzerudyi dzevanakomana vake, nepagunwe reruoko rwerudyi, nepagumwe guru retsoka yavo yerudyi, nekusasawo ropa paaritari kupoteredza.\n"
    "\\v 21 Zvino uchatora paropa riri paaritari nepamafuta ekuzodza ugosasa pamusoro paAroni napanguvo dzake nepamusoro pevanakomana vake, nepanguvo dzevanakomana vake pamwe naye, agoitwa mutsvene iye nenguvo dzake nevanakomana vake nenguvo dzevanakomana vake pamwe naye.\n"
    "\\v 22 Zvino utorewo mamwe mafuta egondohwe, nebemhe raro, namafuta anofukidza ura, namafuta ari pamusoro pechiropa, netsvo mbiri, namafuta ari padziri, nebandauko rorudyi (nokuti igondohwe rokugadza naro),\n"
    "\\v 23 nebundu rimwe rechingwa, nechingwa chimwe chidiki chinamafuta, nechingwa chimwe chitete chinobva mudengu rezvingwa zvisina kuviriswa, zviri pamberi paJehovha.\n"
    "\\v 24 Zvino uzviise zvese pamaoko aAroni napamaoko avanakomana vake, vagozvizunguzira, chive chipo chinozunguzirwa pamberi paJehovha.\n"
    "\\v 25 Zvino uzvibvise pamaoko avo, uzvipise paaritari pamusoro pechipiriso chinopiswa, kuti chive chipo chinonhuhwira pamberi paJehovha; chipiriso chakaitirwa Jehovha pamoto.\n"
    "\\v 26 Utorewo chityu chegondohwe rokugadza naro raAroni, urizunguzire pamberi paJehovha, kuti chive chipo chinozunguzirwa; ndiwo mugove wako.\n"
    "\\v 27 Zvino chityu chinozunguzirwa uchiite chitsvene, nebandauko rechipiriso chinosimudzwa, chinozunguzirwa nekusimudzwa, zvegondohwe rokugadza naro, kuti zvive zvaAroni nezvavanakomana vake.\n"
    "\\v 28 Izvozvo zvinofanira kupiwa Aroni navanakomana vake navana vaIsraeri nokusingaperi nokuti chipiriso chinosimudzwa, chinobva kuvana vaIsraeri pazvibairo zvezvipiriso zvavo zvokuyananisa, chive chipiriso chavo chinosimudzirwa Jehovha.\n"
    "\\p\n"
    "\\v 29 Nguvo dzaAroni dzichava dzavanakomana vake vanomutevera, kuti vazodzwe vakadzipfeka, vagadzwe vakadzipfeka.\n"
    "\\v 30 Mwanakomana wake anopinda paupristi panzvimbo yake, anofanira kudzipfeka mazuva manomwe, kana achipinda mutende rekusangana kushumira panzvimbo tsvene.\n"
    "\\p\n"
    "\\v 31 Zvino unofanira kutora gondohwe rokugadza naro, ubike nyama yaro panzvimbo tsvene.\n"
    "\\v 32 Aroni navanakomana vake ngavadye nyama yegondohwe, nechingwa chiri mudengu pamukova wetende rokusangana.\n"
    "\\v 33 Vanofanira kudya izvozvo zvaiyananiswa nazvo, pakugadzwa nokuitwa vatsvene kwavo; asi mweni ngaarege kuzvidya, nokuti zvitsvene.\n"
    "\\v 34 Kana imwe nyama yokugadza nayo ikasara, kana chimwe chingwa, kusvikira mangwana, unofanira kupisa zvasara nomoto; hazvifaniri kudyiwa, nokuti zvitsvene.\n"
    "\\p\n"
    "\\v 35 Unofanira kuitira Aroni navanakomana vake saizvozvo, zvese sezvandakuraira; uite mazuva manomwe uchivagadza.\n"
    "\\v 36 Zuva rimwe nerimwe unofanira kubaira nzombe yechipiriso chezvivi, chokuyananisa nacho; unofanira kunatsa aritari, kana uchiiyananisira; uizodze kuti uiite tsvene.\n"
    "\\v 37 Unofanira kuita mazuva manomwe pakuyananisira aritari, nokuiita tsvene; kuti aritari ive tsvene kwazvo, uye zvese zvinobata paaritari zvinofanira kuva zvitsvene.\n"
    "\\s Zvibairo zvezuva rimwe nerimwe\n"
    "\\p\n"
    "\\v 41 Rimwe gwayana uribaire madekwani, uriitire sezvawakaitira chipiriso choupfu chamangwanani, nezvawakaitira chipiriso charo chinomwiwa, kuti chive chipiriso chinonhuhwira, chipiriso chakaitirwa Jehovha pamoto.\n"
    "\\v 42 Chinofanira kuva chipiriso chinopiswa nguva dzese pamarudzi enyu ese, pamukova wetende rekusangana pamberi paJehovha, pandichasangana nemi, nditaurepo newe.\n"
    "\\v 43 Ndipo pandichasangana nevana vaIsraeri, uye tende richaitwa dzvene nekubwinya kwangu.\n"
    "\\v 44 Tende rokusangana nearitari ndichazviita zvitsvene; uye Aroni navanakomana vake ndichavaita vatsvene, kuti vandishumire pabasa roupristi.\n"
    "\\v 45 Ndichagara pakati pavana vaIsraeri, kuti ndive Mwari wavo.\n"
    "\\v 46 Vachaziva kuti ndini Jehovha wavo, wakavabudisa panyika kuti ndigare pakati pavo; ndini Jehovha Mwari wavo.\n";
    int similarity = filter_diff_character_similarity (first, second);
    EXPECT_EQ (94, similarity);
    
    similarity = filter_diff_word_similarity (first, second);
    EXPECT_EQ (94, similarity);
    
    similarity = filter_diff_word_similarity ("one two three", "three two one");
    EXPECT_EQ (20, similarity);
    
    similarity = filter_diff_word_similarity ("one two three", "one two three four");
    EXPECT_EQ (75, similarity);
  }
  
  {
    refresh_sandbox (true);
    Webserver_Request request;
    Database_State::create ();
    
    client_logic_enable_client (false);
    database::modifications::truncateTeams ();
    
    std::string temporary_folder = filter_url_tempfile ();
    filter_url_mkdir (temporary_folder);
    
    database::bibles::create_bible ("phpunit");
    bible_logic::store_chapter ("phpunit", 1, 2, "old chapter text");
    database::modifications::truncateTeams ();
    bible_logic::store_chapter ("phpunit", 1, 2, "new chapter text");
    
    filter_diff_produce_verse_level ("phpunit", temporary_folder);
    
    std::string path, standard, output;
    
    path = filter_url_create_path ({"unittests", "tests", "verses_old.usfm"});
    standard = filter_url_file_get_contents (path);
    path = filter_url_create_path ({temporary_folder, "verses_old.usfm"});
    output = filter_url_file_get_contents (path);
    EXPECT_EQ (standard, output);
    
    path = filter_url_create_path ({"unittests", "tests", "verses_new.usfm"});
    standard = filter_url_file_get_contents (path);
    path = filter_url_create_path ({temporary_folder, "verses_new.usfm"});
    output = filter_url_file_get_contents (path);
    EXPECT_EQ (standard, output);
    
    path = filter_url_create_path ({"unittests", "tests", "verses_old.txt"});
    standard = filter_url_file_get_contents (path);
    path = filter_url_create_path ({temporary_folder, "verses_old.txt"});
    output = filter_url_file_get_contents (path);
    EXPECT_EQ (standard, output);
    
    path = filter_url_create_path ({"unittests", "tests", "verses_new.txt"});
    standard = filter_url_file_get_contents (path);
    path = filter_url_create_path ({temporary_folder, "verses_new.txt"});
    output = filter_url_file_get_contents (path);
    EXPECT_EQ (standard, output);
    
    std::string oldfile = filter_url_create_path ({temporary_folder, "verses_old.usfm"});
    std::string newfile = filter_url_create_path ({temporary_folder, "verses_new.usfm"});
    std::string outputfile = filter_url_create_path ({temporary_folder, "changed_verses.html"});
    filter_diff_run_file (oldfile, newfile, outputfile);
    
    path = filter_url_create_path ({"unittests", "tests", "changed_verses.html"});
    standard = filter_url_file_get_contents (path);
    path = filter_url_create_path ({temporary_folder, "changed_verses.html"});
    output = filter_url_file_get_contents (path);
    EXPECT_EQ (standard, output);
  }

  refresh_sandbox (true);
}

#endif


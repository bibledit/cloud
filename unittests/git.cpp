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
#include <webserver/request.h>
#include <database/state.h>
#include <database/git.h>
#include <database/login.h>
#include <filter/url.h>
#include <filter/git.h>
#include <filter/shell.h>
#include <filter/string.h>
#include <filter/date.h>


void test_filter_git_setup ([[maybe_unused]] Webserver_Request& webserver_request,
                            [[maybe_unused]] std::string bible,
                            [[maybe_unused]] std::string newbible,
                            [[maybe_unused]] std::string psalms_0_data,
                            [[maybe_unused]] std::string psalms_11_data,
                            [[maybe_unused]] std::string song_of_solomon_2_data)
{
#ifdef HAVE_CLOUD

  refresh_sandbox (true);
  
  Database_State::create ();
  database::git::create ();
  Database_Login::create ();
  
  std::string repository = filter_git_directory (bible);
  std::string newrepository = filter_git_directory (newbible);
  
  filter_url_mkdir (repository);
  filter_url_mkdir (newrepository);
  
  webserver_request.remote_address = "127.0.0.1";
  webserver_request.user_agent = "unittest";
  webserver_request.database_users ()->create ();
  webserver_request.session_logic ()->set_username ("unittest");
  database::bibles::create_bible (bible);
  
  bool result;
  result = filter_git_init (repository);
  EXPECT_EQ (true, result);
  result = filter_git_init (newrepository);
  EXPECT_EQ (true, result);
  
  filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "0"}));
  filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "11"}));
  filter_url_mkdir (filter_url_create_path ({repository, "Song of Solomon", "2"}));
  
  filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}), psalms_0_data);
  filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "11", "data"}), psalms_11_data);
  filter_url_file_put_contents (filter_url_create_path ({repository, "Song of Solomon", "2", "data"}), song_of_solomon_2_data);
  
  filter_url_mkdir (filter_url_create_path ({newrepository, "Psalms", "0"}));
  filter_url_mkdir (filter_url_create_path ({newrepository, "Psalms", "11"}));
  filter_url_mkdir (filter_url_create_path ({newrepository, "Song of Solomon", "2"}));
  
  filter_url_file_put_contents (filter_url_create_path ({newrepository, "Psalms", "0", "data"}), psalms_0_data);
  filter_url_file_put_contents (filter_url_create_path ({newrepository, "Psalms", "11", "data"}), psalms_11_data);
  filter_url_file_put_contents (filter_url_create_path ({newrepository, "Song of Solomon", "2", "data"}), song_of_solomon_2_data);
#endif
}


TEST (git, basic)
{
#ifdef HAVE_CLOUD
  
  std::string bible = "localrepo";
  std::string newbible = "newlocalrepo";
  std::string repository = filter_git_directory (bible);
  std::string newrepository = filter_git_directory (newbible);
  std::string remoterepository = filter_git_directory ("remoterepo");
  std::string clonedrepository = filter_git_directory ("clonedrepo");
  Webserver_Request webserver_request;
  
  std::string psalms_0_data =
  "\\id PSA\n"
  "\\h Izihlabelelo\n"
  "\\toc2 Izihlabelelo\n"
  "\\mt2 UGWALO\n"
  "\\mt LWEZIHLABELELO";
  
  std::string psalms_11_data =
  "\\c 11\n"
  "\\s IN\\sc KOSI\\sc* iyisiphephelo sabaqotho\n"
  "\\d Kumqondisi wokuhlabelela. EsikaDavida\n"
  "\\p\n"
  "\\v 1 Ngithembela eN\\sc KOSI\\sc*ni\\x + Hlab. 25.2.\\x*. Lingatsho njani emphefumulweni wami: Balekela entabeni yenu \\add njeng\\add*enyoni\\x + 1 Sam. 23.14,19. 26.19,20.\\x*.\n"
  "\\v 2 Ngoba, khangela, ababi bayagobisa idandili\\x + Hlab. 7.12. Hlab. 64.4.\\x*, balungisa umtshoko wabo entanjeni\\x + Hlab. 7.12. 21.12.\\x*, ukuze batshoke emnyameni abaqotho ngenhliziyo\\x + Hlab. 7.10.\\x*.\n"
  "\\v 3 Nxa izisekelo zidilizwa\\x + Isa. 19.10. Hlab. 82.5. Hlab. 75.3.\\x*, angenzani olungileyo\\x + Jobe 22.13.\\x*?\n"
  "\\p\n"
  "\\v 4 IN\\sc KOSI\\x + Hab. 2.20.\\x*\\sc* isethempelini layo elingcwele\\x + Hlab. 5.7. Hlab. 150.1.\\x*; iN\\sc KOSI\\sc*, isihlalo sayo sobukhosi sisemazulwini\\x + Hlab. 2.4. 103.19. 115.3. 123.1. Isa. 66.1. Mat. 5.34. 23.22. Seb. 7.49. Isam. 4.2.\\x*; amehlo ayo ayakhangela\\x + Jobe 24.23. Hlab. 33.13. 34.15. 66.7. Hlab. 14.2. 102.19. 113.5,6.\\x*, inkophe zayo ziyahlola, abantwana babantu.\n"
  "\\v 5 IN\\sc KOSI\\sc* iyamhlola olungileyo, kodwa omubi lothanda ubudlwangudlwangu, umphefumulo wayo uyamzonda\\x + Gen. 22.1.\\x*.\n"
  "\\v 6 Uzanisa phezu kwababi imijibila, umlilo, lesolufa*\\x + Jobe 18.15.\\x*, lomoya otshisayo\\x + Hlab. 119.53. Lilo 5.10.\\x*, kuzakuba yisabelo senkezo yabo\\x + Hlab. 75.8. Jobe 21.20. Hlab. 16.5.\\x*.\n"
  "\\v 7 Ngoba ilungile iN\\sc KOSI\\sc*, iyathanda ukulunga\\x + Hlab. 33.5. 45.7. Hlab. 37.28. 146.8.\\x*; ubuso bayo buyabona oqotho\\x + Hlab. 33.18. Hlab. 17.2.\\x*.";
  
  std::string song_of_solomon_2_data =
  "\\c 2\n"
  "\\p\n"
  "\\v 1 Ngilirozi\\x + Isa. 35.1.\\x* leSharoni\\x + Josh. 12.18.\\x*, umduze wezigodi\\x + 2.16. 4.5. 5.13. 6.2,3. 7.2. 2 Lan. 4.5. Hos. 14.5. Hlab. 45.\\x*.\n"
  "\\p\n"
  "\\v 2 Njengomduze phakathi kwameva\\x + 2.16. 4.5. 5.13. 6.2,3. 7.2. 2 Lan. 4.5. Hos. 14.5. Hlab. 45.\\x*, unjalo umngane wami phakathi kwamadodakazi\\x + 1.15.\\x*.\n"
  "\\p\n"
  "\\v 3 Njengesihlahla sama-aphula phakathi kwezihlahla zeganga, sinjalo isithandwa sami phakathi kwamadodana\\x + Zaga 25.11.\\x*. Ngahlala emthunzini waso ngathokoza kakhulu\\x + Isa. 25.4. 32.2.\\x*, lesithelo saso simnandi ekunambitheni kwami\\x + Isam. 22.2.\\x*.\n"
  "\\v 4 Sangisa endlini yewayini, lesiboniso saso phezu kwami siluthando\\x + 1.4.\\x*.\n"
  "\\v 5 Ngisekelani\\x + Gen. 27.37.\\x* ngeziphiso zewayini\\x + 2 Sam. 6.19. 1 Lan. 16.3. Hos. 3.1.\\x*, lingiqinise ngama-aphula\\x + Zaga 25.11.\\x*, ngoba ngigul\\add isw\\add*a \\add lu\\add*thando\\x + 5.8.\\x*.\n"
  "\\v 6 Isandla saso sokhohlo singaphansi kwekhanda lami\\x + 8.3. Dute. 33.27.\\x*, lesokunene saso siyangigona\\x + 8.3. Dute. 33.27.\\x*.\n"
  "\\v 7 Ngiyalifungisa\\x + 3.5. 8.4. 5.8,9.\\x*, madodakazi eJerusalema\\x + 1.5.\\x*, ngemiziki\\x + 2.9,17.\\x*\\x + Zaga 6.5.\\x* kumbe ngezimpala zeganga\\x + 2.9,17.\\x*\\x + Zaga 5.19.\\x*, ukuze lingaphazamisi lingavusi uthando luze luthande.\n"
  "\\p\n"
  "\\v 8 Ilizwi lesithandwa sami! Khangela sona siyeza, siseqa phezu kwezintaba, siqolotsha phezu kwamaqaqa\\x + Isa. 52.7.\\x*.\n"
  "\\v 9 Isithandwa sami sinjengomziki\\x + 2.7,17. 8.14. 2 Sam. 22.34.\\x* kumbe njengethole lendluzele\\x + 2.7,17. 8.14. 2 Sam. 22.34.\\x*\\x + 4.5. 7.3.\\x*. Khangela simi ngemva komduli wethu, silunguza emawindini, sizibonakalisa\\f + \\fk zibonakalisa: \\fl Heb. \\fq hluma.\\f* eminxibeni yewindi\\x + Isa. 14.16.\\x*.\n"
  "\\v 10 Isithandwa sami saphendula sathi kimi\\x + Hlu. 18.14.\\x*: Vuka wena\\x + 2.13.\\x*, mngane wami, omuhle wami, a\\add si\\add*hambe wena\\x + 1.15.\\x*.\n"
  "\\v 11 Ngoba khangela, ubusika sebudlulile, izulu seliphelile, lihambile.\n"
  "\\v 12 Amaluba ayabonakala emhlabeni, isikhathi sokuhlabelela \\add se\\add*sifikile, lelizwi lejuba liyezwakala elizweni lakithi\\x + Hlab. 74.19. Jer. 8.7.\\x*.\n"
  "\\v 13 Isihlahla somkhiwa sivuthisa imikhiwa yaso eluhlaza, lamavini \\add ale\\add*mpoko\\x + 2.15. 7.12.\\x* aletha iphunga elimnandi\\x + Tshu. 7.1.\\x*. Vuka wena, mngane wami, omuhle wami wena, \\add si\\add*hambe\\x + 2.10.\\x*.\n"
  "\\p\n"
  "\\v 14 Juba lami\\x + 5.2. 6.9. 1.15. Mat. 10.16.\\x*, \\add elis\\add*engoxweni yedwala\\x + Jer. 48.28.\\x*\\x + Jer. 49.16. Obad. 3.\\x*, ekusithekeni kweliwa\\x + Hez. 38.20.\\x*, ngitshengisa ubuso bakho, ngizwise ilizwi lakho\\x + 8.13.\\x*, ngoba ilizwi lakho limnandi, lobuso bakho buyabukeka\\x + 1.5.\\x*.\n"
  "\\v 15 Sibambeleni amakhanka, amakhanka amancinyane, ona izivini\\x + Hez. 13.4. Luka 13.32.\\x*, ngoba izivini zethu \\add zile\\add*zimpoko\\x + 2.15. 7.12.\\x*.\n"
  "\\v 16 Isithandwa sami ngesami, lami ngingowaso\\x + 6.3. 7.10.\\x*, eselusa phakathi kwemiduze\\x + 2.1. 4.5. 6.3.\\x*.\n"
  "\\v 17 Kuze kube semadabukakusa, lamathunzi abaleke\\x + 4.6.\\x*, phenduka, sithandwa sami, ube njengomziki kumbe njengethole lendluzele\\x + 8.14. 2.9.\\x* phezu kwezintaba zeBhetheri\\x + 2 Sam. 2.29.\\x*.";
  
  // Test sync Bible to git.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "0", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "11", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Song of Solomon", "2", "data"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Exodus", "1", "data"})));
    
    database::bibles::store_chapter (bible, 2, 1, song_of_solomon_2_data);
    filter_git_sync_bible_to_git (bible, repository);
    
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "0", "data"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "11", "data"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Song of Solomon", "2", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Exodus", "1", "data"})));
    
    // Remove generated journal entries.
    refresh_sandbox (false);
  }
  
  // Sync a Bible to git.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "0", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "11", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Song of Solomon", "2", "data"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Exodus", "1", "data"})));
    
    database::bibles::store_chapter (bible, 19, 1, song_of_solomon_2_data);
    filter_git_sync_bible_to_git (bible, repository);
    
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "0", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "1", "data"})));
    
    std::string data = filter_url_file_get_contents (filter_url_create_path ({repository, "Psalms", "1", "data"}));
    EXPECT_EQ (song_of_solomon_2_data, data);
    
    // Remove generated journal entries.
    refresh_sandbox (false);
  }
  
  // Sync a Bible to git.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "0", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "11", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Song of Solomon", "2", "data"})));
    EXPECT_EQ (false, file_or_dir_exists (filter_url_create_path ({repository, "Exodus", "1", "data"})));
    
    database::bibles::store_chapter (bible, 19, 1, song_of_solomon_2_data);
    database::bibles::store_chapter (bible, 22, 2, psalms_11_data);
    database::bibles::store_chapter (bible, 19, 11, song_of_solomon_2_data);
    filter_git_sync_bible_to_git (bible, repository);
    
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, ".git"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "1", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Song of Solomon", "2", "data"})));
    EXPECT_EQ (true, file_or_dir_exists (filter_url_create_path ({repository, "Psalms", "11", "data"})));
    
    std::string data = filter_url_file_get_contents (filter_url_create_path ({repository, "Song of Solomon", "2", "data"}));
    EXPECT_EQ (psalms_11_data, data);
    
    data = filter_url_file_get_contents (filter_url_create_path ({repository, "Psalms", "11", "data"}));
    EXPECT_EQ (song_of_solomon_2_data, data);
    
    data = filter_url_file_get_contents (filter_url_create_path ({repository, "Psalms", "1", "data"}));
    EXPECT_EQ (song_of_solomon_2_data, data);
    
    // Remove generated journal entries.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git to Bible and adding chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // The Bible has been created, but has no data yet.
    // Run the filter, and check that all three chapters are now in the database.
    filter_git_sync_git_to_bible (repository, bible);
    std::vector <int> books = database::bibles::get_books (bible);
    EXPECT_EQ ((std::vector <int>{19, 22}), books);
    // Check that the data matches.
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ (psalms_0_data, usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ (psalms_11_data, usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ (song_of_solomon_2_data, usfm);
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git to Bible ad deleting chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // Put that into the database.
    filter_git_sync_git_to_bible (repository, bible);
    // Remove one book and one chapter from the git repository,
    // and check that after running the filter, the database is updated accordingly.
    filter_url_rmdir (repository + "/Song of Solomon");
    filter_url_rmdir (repository + "/Psalms/0");
    filter_git_sync_git_to_bible (repository, bible);
    std::vector <int> books = database::bibles::get_books (bible);
    EXPECT_EQ (std::vector <int>{19}, books);
    // Check that the data matches.
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ ("", usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ (psalms_11_data, usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ ("", usfm);
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git to Bible and updating chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // Put that into the database.
    filter_git_sync_git_to_bible (repository, bible);
    // Update some chapters in the git repository,
    // and check that after running the filter, the database is updated accordingly.
    filter_url_file_put_contents (repository + "/Psalms/11/data", "\\c 11");
    filter_url_file_put_contents (repository + "/Song of Solomon/2/data", "\\c 2");
    filter_git_sync_git_to_bible (repository, bible);
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ (psalms_0_data, usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ ("\\c 11", usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ ("\\c 2", usfm);
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git chapter to Bible and adding chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // The Bible has been created, but has no data yet.
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ ("", usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ ("", usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ ("", usfm);
    
    // Run the filter for each chapter, and check that all three chapters make it into the database.
    filter_git_sync_git_chapter_to_bible (repository, bible, 19, 0);
    usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ (psalms_0_data, usfm);
    
    filter_git_sync_git_chapter_to_bible (repository, bible, 19, 11);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ (psalms_11_data, usfm);
    
    filter_git_sync_git_chapter_to_bible (repository, bible, 22, 2);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ (song_of_solomon_2_data, usfm);
    
    // Check the two books are there.
    std::vector <int> books = database::bibles::get_books (bible);
    EXPECT_EQ ((std::vector <int>{19, 22}), books);
    
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git chapter to Bible and deleting chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // Put that into the database.
    filter_git_sync_git_to_bible (repository, bible);
    
    // Remove one book and one chapter from the git repository,
    filter_url_rmdir (repository + "/Song of Solomon");
    filter_url_rmdir (repository + "/Psalms/0");
    
    // Run updates on the three chapters.
    filter_git_sync_git_chapter_to_bible (repository, bible, 19, 0);
    filter_git_sync_git_chapter_to_bible (repository, bible, 19, 11);
    filter_git_sync_git_chapter_to_bible (repository, bible, 22, 2);
    
    // There should still be two books, although one book would have no chapters.
    std::vector <int> books = database::bibles::get_books (bible);
    EXPECT_EQ ((std::vector <int>{19, 22}), books);
    
    // Check that the chapter data matches.
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ ("", usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ (psalms_11_data, usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ ("", usfm);
    
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Test synchronizing git chapter to Bible and updating chapters.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    
    // The git repository has Psalm 0, Psalm 11, and Song of Solomon 2.
    // Put that into the Bible database.
    filter_git_sync_git_to_bible (repository, bible);
    
    // Update some chapters in the git repository.
    filter_url_file_put_contents (repository + "/Psalms/11/data", "\\c 11");
    filter_url_file_put_contents (repository + "/Song of Solomon/2/data", "\\c 2");
    
    // Run updates on the two chapters.
    filter_git_sync_git_chapter_to_bible (repository, bible, 19, 11);
    filter_git_sync_git_chapter_to_bible (repository, bible, 22, 2);
    
    // Check that the database is updated accordingly.
    std::string usfm = database::bibles::get_chapter (bible, 19, 0);
    EXPECT_EQ (psalms_0_data, usfm);
    usfm = database::bibles::get_chapter (bible, 19, 11);
    EXPECT_EQ ("\\c 11", usfm);
    usfm = database::bibles::get_chapter (bible, 22, 2);
    EXPECT_EQ ("\\c 2", usfm);
    
    // Remove the journal entries the test created.
    refresh_sandbox (false);
  }
  
  // Setting values in the configuration.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    filter_git_config_set_bool (repository, "foo.bar", false);
    filter_git_config_set_int (repository, "bar.baz", 11);
    std::string path = filter_url_create_path ({repository, ".git", "config"});
    std::string contents = filter_url_file_get_contents (path);
    EXPECT_EQ (true, contents.find ("[foo]") != std::string::npos);
    EXPECT_EQ (true, contents.find ("[bar]") != std::string::npos);
    EXPECT_EQ (true, contents.find ("bar = false") != std::string::npos);
    EXPECT_EQ (true, contents.find ("baz = 11") != std::string::npos);
    refresh_sandbox (false);
  }
  
  // Test of basic git operations in combination with a remote repository.
  {
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);
    std::string error;
    bool success;
    std::string remoteurl = "file://" + remoterepository;
    std::vector <std::string> messages;
    
    // Create bare remote reository.
    filter_url_mkdir (remoterepository);
    filter_git_init (remoterepository, true);
    
    // Test read access to the remote repository.
    success = filter_git_remote_read (remoterepository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Test cloning the repository.
    success = filter_git_remote_clone (remoteurl, clonedrepository, 0, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Store some Bible data in the cloned repository.
    filter_url_mkdir (filter_url_create_path ({clonedrepository, "Psalms", "0"}));
    filter_url_file_put_contents (filter_url_create_path ({clonedrepository, "Psalms", "0", "data"}), psalms_0_data);
    filter_url_mkdir (filter_url_create_path ({clonedrepository, "Psalms", "11"}));
    filter_url_file_put_contents (filter_url_create_path ({clonedrepository, "Psalms", "11", "data"}), psalms_11_data);
    filter_url_mkdir (filter_url_create_path ({clonedrepository, "Song of Solomon", "2"}));
    filter_url_file_put_contents (filter_url_create_path ({clonedrepository, "Song of Solomon", "2", "data"}), song_of_solomon_2_data);
    
    // Add the Bible data to the git index.
    success = filter_git_add_remove_all (clonedrepository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Commit the index to the repository.
    success = filter_git_commit (clonedrepository, "username", "unittest", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Remove some Bible data from the cloned repository.
    filter_url_rmdir (filter_url_create_path ({clonedrepository, "Psalms"}));
    success = filter_git_add_remove_all (clonedrepository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Commit the index to the repository.
    success = filter_git_commit (clonedrepository, "username", "unittest", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Push to the remote repository.
    success = filter_git_push (clonedrepository, messages);
    EXPECT_EQ (true, success);
    EXPECT_EQ (2, static_cast<int>(messages.size()));
    
    // Pull from remote repository.
    success = filter_git_pull (clonedrepository, messages);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::vector<std::string>{"Already up to date."}, messages);
    
    // Remove journal entries.
    refresh_sandbox (false);
  }
  
  // Get Git Passage
  {
    Passage passage = filter_git_get_passage ("From https://github.com/joe/test");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage ("   443579b..90dcb57  master     -> origin/master");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage ("Updating 443579b..90dcb57");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage ("Fast-forward");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage (" Genesis/3/data | 2 +-");
    Passage standard = Passage ("", 1, 3, "");
    EXPECT_EQ (true, standard.equal (passage));
    
    passage = filter_git_get_passage (" 1 file changed, 1 insertion(+), 1 deletion(-)");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage (" delete mode 100644 Leviticus/1/data");
    EXPECT_EQ (0, passage.m_book);
    
    passage = filter_git_get_passage (" Revelation/3/data | 2 +-");
    standard = Passage ("", 66, 3, "");
    EXPECT_EQ (true, standard.equal (passage));
    
    passage = filter_git_get_passage ("	modified:   Exodus/3/data");
    standard = Passage ("", 2, 3, "");
    EXPECT_EQ (true, standard.equal (passage));
  }
  
  // Exercise the "git status" filter.
  {
    // Refresh the repository, and store three chapters in it.
    test_filter_git_setup (webserver_request, bible, newbible, psalms_0_data, psalms_11_data, song_of_solomon_2_data);

    std::vector <std::string> paths;
    int size = 0;

    // There should be three modified paths.
    paths = filter_git_status (repository);
    size = 10;
    EXPECT_EQ (size, paths.size());
    if (static_cast<int> (paths.size()) == size) {
      EXPECT_EQ (6, (paths[6].find ("Psalms/")));
      EXPECT_EQ (6, (paths[7].find ("Song of Solomon/")));
    }

    // Add the files to the index.
    std::string error;
    std::vector <std::string> messages;
    filter_git_add_remove_all (repository, error);
    EXPECT_EQ ("", error);
    
    // There should still be three paths.
    paths = filter_git_status (repository);
    size = 10;
    EXPECT_EQ (size, paths.size());
    if (static_cast<int> (paths.size()) == size) {
      EXPECT_EQ (6, (paths[6].find ("new file:   Psalms/0/data")));
      EXPECT_EQ (6, (paths[7].find ("new file:   Psalms/11/data")));
      EXPECT_EQ (6, (paths[8].find ("new file:   Song of Solomon/2/data")));
    }
    
    // Commit the index.
    filter_git_commit (repository, "user", "unittest", messages, error);
    EXPECT_EQ (std::string(), error);
    
    // There should be no modified paths now.
    paths = filter_git_status (repository);
    EXPECT_EQ ((std::vector<std::string>{"On branch master", "nothing to commit, working tree clean"}), paths);

    // Remove both Psalms chapters.
    filter_url_rmdir (filter_url_create_path ({repository, "Psalms"}));
    
    // There should be two modified paths now.
    paths = filter_git_status (repository);
    size = 8;
    EXPECT_EQ (size, paths.size());
    if (static_cast<int> (paths.size()) == size) {
      EXPECT_EQ (6, (paths[4].find ("deleted:    Psalms/0/data")));
      EXPECT_EQ (6, (paths[5].find ("deleted:    Psalms/11/data")));
    }
    
    // Add / remove the files to the index.
    filter_git_add_remove_all (repository, error);
    EXPECT_EQ (std::string(), error);
    
    // There should still be two paths now.
    paths = filter_git_status (repository);
    size = 6;
    EXPECT_EQ (size, paths.size());
    if (static_cast<int> (paths.size()) == size) {
      EXPECT_EQ (6, (paths[3].find ("deleted:    Psalms/0/data")));
      EXPECT_EQ (6, (paths[4].find ("deleted:    Psalms/11/data")));
    }
    
    // Commit the index.
    filter_git_commit (repository, "user", "unittest", messages, error);
    EXPECT_EQ ("", error);
    
    // There should be no modified paths now.
    paths = filter_git_status (repository);
    EXPECT_EQ ((std::vector<std::string>{"On branch master", "nothing to commit, working tree clean"}), paths);

    // Remove journal entries.
    refresh_sandbox (false);
  }

  // Test git's internal conflict resolution.
  {
    refresh_sandbox (true);
    std::string error;
    bool success;
    std::vector <std::string> messages;
    
    // Create remote repository.
    filter_url_mkdir (remoterepository);
    filter_git_init (remoterepository, true);
    std::string remoteurl = "file://" + remoterepository;
    
    // Clone the remote repository.
    success = filter_git_remote_clone (remoteurl, repository, 0, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Configure the local repository for being able to fast forward.
    filter_git_config (repository);
    
    // Store three chapters in the local repository and push it to the remote repository.
    filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "0"}));
    filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "11"}));
    filter_url_mkdir (filter_url_create_path ({repository, "Song of Solomon", "2"}));
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}), psalms_0_data);
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "11", "data"}), psalms_11_data);
    filter_url_file_put_contents (filter_url_create_path ({repository, "Song of Solomon", "2", "data"}), song_of_solomon_2_data);
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);
    success = filter_git_commit (repository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);
    success = filter_git_push (repository, messages, true);
    EXPECT_EQ (true, success);
    
    // Clone the remote repository to a new local repository.
    success = filter_git_remote_clone (remoteurl, newrepository, 0, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);

    // Set the stage for a conflict that git itself cannot merge:
    // Change something in the new repository, push it to the remote.
    std::string newcontents =
    "\\id PSA\n"
    "\\h Izihlabelelo\n"
    "\\toc2 Izihlabelelo\n"
    "\\mt2 THE BOOK\n"
    "\\mt OF PSALMS\n";
    filter_url_file_put_contents (filter_url_create_path ({newrepository, "Psalms", "0", "data"}), newcontents);
    success = filter_git_add_remove_all (newrepository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);
    success = filter_git_commit (newrepository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_push (newrepository, messages, true);
    // Change something in the repository, and pull from remote:
    // Git fails to merge by itself.
    std::string contents =
    "\\id PSALM\n"
    "\\h Izihlabelelo\n"
    "\\toc2 Izihlabelelo\n"
    "\\mt2 UGWALO\n"
    "\\mt LWEZIHLABELELO\n";
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}), contents);
    EXPECT_EQ (true, success);
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);
    success = filter_git_commit (repository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (std::string(), error);
    success = filter_git_pull (repository, messages);
    EXPECT_EQ (false, success);
    success = find (messages.begin(), messages.end(), "Auto-merging Psalms/0/data") != messages.end();
    EXPECT_EQ (true, success);
    success = find (messages.begin(), messages.end(), "CONFLICT (content): Merge conflict in Psalms/0/data") != messages.end();
    EXPECT_EQ (true, success);
    success = filter_git_push (repository, messages);
    EXPECT_EQ (false, success);
    std::vector <std::string> paths = { "Psalms/0/data" };
    success = filter_git_resolve_conflicts (repository, paths, error);
    EXPECT_EQ (std::string(), error);
    EXPECT_EQ (true, success);
    // Check the merge result.
    std::string standard =
    "\\id PSALM\n"
    "\\h Izihlabelelo\n"
    "\\toc2 Izihlabelelo\n"
    "\\mt2 THE BOOK\n"
    "\\mt OF PSALMS";
    contents = filter_url_file_get_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}));
    EXPECT_EQ (standard, contents);
    
    // Remove journal entries.
    refresh_sandbox (false);
  }

  {
    refresh_sandbox (true);
    std::string error;
    bool success;
    std::vector <std::string> messages;
    
    // Create remote repository.
    filter_url_mkdir (remoterepository);
    filter_git_init (remoterepository, true);
    std::string remoteurl = "file://" + remoterepository;
    
    // Clone the remote repository.
    success = filter_git_remote_clone (remoteurl, repository, 0, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);

    // Configure the local repository for being able to fast forward.
    filter_git_config (repository);

    // Store three chapters in the local repository and push it to the remote repository.
    psalms_0_data =
    "Line one one one\n"
    "Line two two two\n"
    "Line three three three\n";
    filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "0"}));
    filter_url_mkdir (filter_url_create_path ({repository, "Psalms", "11"}));
    filter_url_mkdir (filter_url_create_path ({repository, "Song of Solomon", "2"}));
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}), psalms_0_data);
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "11", "data"}), psalms_11_data);
    filter_url_file_put_contents (filter_url_create_path ({repository, "Song of Solomon", "2", "data"}), song_of_solomon_2_data);
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_commit (repository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_push (repository, messages, true);
    EXPECT_EQ (true, success);
    
    // Clone the remote repository to a new local repository.
    success = filter_git_remote_clone (remoteurl, newrepository, 0, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    
    // Set the stage for a conflict that git itself can merge:
    // Change something in the new repository, push it to the remote.
    std::string newcontents =
    "Line 1 one one\n"
    "Line two two two\n"
    "Line three three three\n";
    filter_url_file_put_contents (filter_url_create_path ({newrepository, "Psalms", "0", "data"}), newcontents);
    success = filter_git_add_remove_all (newrepository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_commit (newrepository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_push (newrepository, messages, true);
    // Change something in the repository, and pull from remote:
    // Git fails to merge by itself.
    std::string contents =
    "Line one one 1 one\n"
    "Line two 2 two 2 two\n"
    "Line three 3 three 3 three\n";
    filter_url_file_put_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}), contents);
    EXPECT_EQ (true, success);
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    success = filter_git_commit (repository, "test", "test", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    // Pulling changes should result in a merge conflict.
    success = filter_git_pull (repository, messages);
    EXPECT_EQ (false, success);
    
    // Resolve the conflict.
    success = filter_git_resolve_conflicts (repository, messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ ("", error);
    EXPECT_EQ (std::vector<std::string>{"Psalms/0/data"}, messages);

    // Verify the resolved contents on correctness.
    contents = filter_url_file_get_contents (filter_url_create_path ({repository, "Psalms", "0", "data"}));
    std::string standard =
    "Line 1 one 1 one\n"
    "Line two 2 two 2 two\n"
    "Line three 3 three 3 three";
    EXPECT_EQ (standard, contents);
    
    // The status still displays the file as in conflict.
    // messages = filter_git_status (repository);
    // EXPECT_EQ ({"Psalms/0/data"}, messages);
    
    // Commit and push the result.
    success = filter_git_commit (repository, "", "message", messages, error);
    EXPECT_EQ (true, success);
    EXPECT_EQ (5, static_cast<int>(messages.size()));
    success = filter_git_push (repository, messages);
    EXPECT_EQ (true, success);
    EXPECT_EQ (2, static_cast<int>(messages.size()));
    
    // Status up-to-date.
    messages = filter_git_status (repository);
    EXPECT_EQ (4, static_cast<int>(messages.size ()));
    
    // Remove journal entries.
    refresh_sandbox (false);
  }
  
  // Test one user saving Bible data in an uninterrupted sequence.
  // Check that it leads to correct records in git.
  {
    refresh_sandbox (true);
    
    std::string error;
    bool success;
    std::vector <std::string> messages;
    
    test_filter_git_setup (webserver_request, bible, newbible,
                           "Psalm 0\n", "Psalm 11\n", "Song of Solomon 2\n");
    
    // Commit the data to the repository.
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    success = filter_git_commit (repository, "", "initial commit", messages, error);
    EXPECT_EQ (true, success);
    
    int psalms = 19;
    std::string user1 = "user1";
    std::string user2 = "user2";
    std::string oldusfm1, newusfm1;
    std::string out_err;
    
    // Create records of user saving data.
    oldusfm1 = "Psalm 11\n";
    newusfm1 = oldusfm1 + "Praise";
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    oldusfm1 = newusfm1;
    newusfm1.append (" Jesus");
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    oldusfm1 = newusfm1;
    newusfm1.append (" forever.\n");
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    filter_git_sync_modifications_to_git (bible, repository);
    
    // Check the elaborate log.
    // Disable color codes in the output for easier parsing.
    filter::shell::run ("cd " + repository + " && git log -p --color=never", out_err);
    EXPECT_EQ (true, out_err.find ("+Praise Jesus forever.") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("Author: user1 <bibledit@bibledit.org>") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("User modification") != std::string::npos);
    
    // Remove journal entries.
    refresh_sandbox (false);
  }
  
  // Test one user saving Bible data,
  // but this time the sequence is interrupted by undefined other users.
  {
    refresh_sandbox (true);
    
    std::string error;
    bool success;
    std::vector <std::string> messages;
    
    test_filter_git_setup (webserver_request, bible, newbible, "Psalm 0\n", "Psalm 11\n", "Song of Solomon 2\n");
    
    // Commit the data to the repository.
    success = filter_git_add_remove_all (repository, error);
    EXPECT_EQ (true, success);
    success = filter_git_commit (repository, "", "initial commit", messages, error);
    EXPECT_EQ (true, success);
    
    int psalms = 19;
    std::string user1 = "user1";
    std::string user2 = "user2";
    std::string oldusfm1, newusfm1;
    std::string out_err;
    
    // Create records of two users saving data.
    oldusfm1 = "Psalm 11\n";
    newusfm1 = oldusfm1 + "Praise";
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    oldusfm1 = newusfm1;
    newusfm1.append (" Jesus");
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    database::git::store_chapter (user2, bible, psalms, 11, oldusfm1 + " xx\n", newusfm1 + " xxx\n");
    oldusfm1 = newusfm1;
    newusfm1.append (" forever.\n");
    database::git::store_chapter (user1, bible, psalms, 11, oldusfm1, newusfm1);
    filter_git_sync_modifications_to_git (bible, repository);
    filter::shell::run ("cd " + repository + " && git log -p --color=never", out_err);
    EXPECT_EQ (true, out_err.find ("+Praise Jesus forever.") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("Author: user1 <bibledit@bibledit.org>") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("Author: user2 <bibledit@bibledit.org>") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("User modification") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("System-generated to clearly display user modification in next commit") != std::string::npos);
    EXPECT_EQ (true, out_err.find ("+Praise Jesus xxx") != std::string::npos);
    
    // Remove journal entries.
    refresh_sandbox (false);
  }
#endif
}


TEST (database, git)
{
#ifdef HAVE_CLOUD

  refresh_sandbox (false);
  
  // Create the database.
  database::git::create ();
  
  std::string user = "user";
  std::string bible = "bible";
  
  // Store one chapter, and check there's one rowid as a result.
  database::git::store_chapter (user, bible, 1, 2, "old", "new");
  std::vector <int> rowids = database::git::get_rowids (user, "");
  EXPECT_EQ (std::vector <int>{}, rowids);
  rowids = database::git::get_rowids ("", bible);
  EXPECT_EQ (std::vector <int>{}, rowids);
  rowids = database::git::get_rowids (user, bible);
  EXPECT_EQ (std::vector <int>{1}, rowids);
  
  // Store some more chapters to get more rowids in the database.
  database::git::store_chapter (user, bible, 2, 5, "old2", "new5");
  database::git::store_chapter (user, bible, 3, 6, "old3", "new6");
  database::git::store_chapter (user, bible, 4, 7, "old4", "new7");
  
  // Retrieve and check a certain rowid whether it has the correct values.
  std::string user2, bible2;
  int book, chapter;
  std::string oldusfm, newusfm;
  bool get = database::git::get_chapter (1, user2, bible2, book, chapter, oldusfm, newusfm);
  EXPECT_EQ (true, get);
  EXPECT_EQ (user, user2);
  EXPECT_EQ (bible, bible2);
  EXPECT_EQ (1, book);
  EXPECT_EQ (2, chapter);
  EXPECT_EQ ("old", oldusfm);
  EXPECT_EQ ("new", newusfm);
  
  // Erase a rowid, and check that the remaining ones in the database are correct.
  database::git::erase_rowid (2);
  rowids = database::git::get_rowids (user, bible);
  EXPECT_EQ ((std::vector<int>{1, 3, 4}), rowids);
  
  // Getting a non-existent rowid should fail.
  get = database::git::get_chapter (2, user, bible, book, chapter, oldusfm, newusfm);
  EXPECT_EQ (false, get);
  
  // Update the timestamps and check that expired entries get removed and recent ones remain.
  rowids = database::git::get_rowids ("user", "bible");
  EXPECT_EQ (3, rowids.size ());
  database::git::optimize ();
  rowids = database::git::get_rowids (user, bible);
  EXPECT_EQ (3, rowids.size ());
  database::git::touch_timestamps (filter::date::seconds_since_epoch () - 432000 - 1);
  database::git::optimize ();
  rowids = database::git::get_rowids (user, bible);
  EXPECT_EQ (0, rowids.size ());
  
  // Test that it reads distinct users.
  database::git::store_chapter (user, bible, 2, 5, "old", "new");
  database::git::store_chapter (user, bible, 2, 5, "old", "new");
  database::git::store_chapter ("user2", bible, 2, 5, "old", "new");
  std::vector <std::string> users = database::git::get_users (bible);
  EXPECT_EQ ((std::vector<std::string>{user, "user2"}), users);
  
#endif
}

#endif


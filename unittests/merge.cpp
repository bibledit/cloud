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
#include <filter/merge.h>
#include <filter/url.h>


TEST (filter, merge)
{
  {
    std::string ancestor_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    std::string server_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    std::string user_usfm = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti ‘muka Allah’ dalam bahasa Ibrani.\f* karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    std::string standard = R"(\v 30 Yakub menamai tempat tersebut Peniel,\f \fr 32:30 \fk Peniel \ft Nama ini berarti ‘muka Allah’ dalam bahasa Ibrani.\f* karena dia sudah bertatapan muka dengan Allah secara langsung dan dia masih hidup.)";
    std::vector <Merge_Conflict> conflicts;
    std::string result = filter_merge_run (ancestor_usfm, server_usfm, user_usfm, true, conflicts);
    EXPECT_EQ (standard, result);
  }

  // Test line merge for simple modifications.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\s Ukuvuka kukaJesu\n";
    std::string serverModificationData =
    "\\c 29\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\s Ukuvuka kukaJesu";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (true, conflicts.empty ());
  }
  
  // Test line merge for equal modifications
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\s Ukuvuka kukaJesu\n"
    "\\s Ukuvuka kukaJesu\n";
    std::string serverModificationData =
    "\\c 28\n"
    "\\s Ukuvuka kukaJesu\n"
    "\\s Ukuvuka kukaJesu\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 28\n"
    "\\s Ukuvuka kukaJesu\n"
    "\\s Ukuvuka kukaJesu";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (true, conflicts.empty ());
  }
  
  // Test line merge for multiple modifications
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, lesembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\pp\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Marko 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya osukwini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, lesembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, lesematho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65-66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\pp\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Marko 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya osukwini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, lesematho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65-66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (true, conflicts.empty ());
  }
  
  // Test word merge for simple modifications
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 29\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, basebesiba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, basebesiba njengabafileyo\\x + 27.65,66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (true, conflicts.empty ());
  }
  
  // Test word merge for conflicting modifications.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 29\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, basebesiba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, basebesiba njengabafileyo\\x + 27.65,66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (1, conflicts.size ());
  }
  
  // Test word merge for multiple modifications
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, lesembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 29\n"
    "\\s Ukuvuka lokuzibonakalisa kukaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Hlu. 13.6.\\x*, lesembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 28\n"
    "\\s Ukuvuka lokuzibonakaliswa kwaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Dan. 10.6. Hlu. 13.6.\\x*, njalo isembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\s Ukuvuka lokuzibonakaliswa kwaJesu\n"
    "\\p\n"
    "\\v 1 Kwathi ekupheleni kwesabatha\\x + Mark. 16.1-8. Luka 24.1-10.\\x*, emadabukakusa kusiya o\\add sukw\\add*ini lokuqala lweviki\\x + Joha. 20.1.\\x*, kwafika uMariya Magadalena\\x + Joha. 20.1.\\x*, lomunye uMariya, ukuzabona ingcwaba\\x + 27.56,61. Mark. 16.1. Luka 24.10.\\x*.\n"
    "\\v 2 Futhi khangela, kwaba khona ukuzamazama komhlaba okukhulu\\x + 27.51,54.\\x*; ngoba ingilosi yeNkosi yehla ivela ezulwini\\x + Mark. 16.5. Luka 24.4. Joha. 20.12.\\x*, yasondela yagiqa ilitshe yalisusa emnyango, yahlala phezu kwalo\\x + 27.60,66.\\x*.\n"
    "\\v 3 Lokubonakala kwayo kwakunjengombane\\x + Hlu. 13.6.\\x*, njalo isembatho sayo sasimhlophe njengeliqhwa elikhithikileyo\\x + Dan. 7.9. Mark. 9.3.\\x*.\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Test grapheme merge for simple modifications
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\v 4 Abalindi bathuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 29\n"
    "\\v 4 Abalindi basebethuthumela besabe baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\v 4 Abalindi bathuthumela besabe baba njengabafileyo\\x + 27.65,66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Test that in case of a conflict, it takes the server's version.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 28\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba, baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string userModificationData =
    "\\c 28\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string serverModificationData =
    "\\c 29\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba; baba njengabafileyo\\x + 27.65,66.\\x*.\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 29\n"
    "\\v 4 Abalindi basebethuthumela ngokuyesaba; baba njengabafileyo\\x + 27.65,66.\\x*.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (1, conflicts.size ());
  }
  
  // Realistic merge example.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string userModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string serverModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth verse.\n"
    "\\v 5\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first verse.\n"
    "\\v 2 And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth verse.\n"
    "\\v 5";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Merge situation taken from real life.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string path;
    path = filter_url_create_root_path ({"unittests", "tests", "merge_1_base.usfm"});
    std::string mergeBaseData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_1_modification.usfm"});
    std::string userModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_1_server.usfm"});
    std::string serverModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_1_result.usfm"});
    std::string standard = filter_url_file_get_contents (path);
    
    std::string output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, true, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Testing the clever merge routine on chapter 0.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\id GEN\n"
    "\\p Some text one.\n";
    std::string userModificationData =
    "\\id GEN\n"
    "\\p Some text two.\n";
    std::string serverModificationData =
    "\\id GEN\n"
    "\\p Some text one.\n";
    std::string output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    std::string standard =
    "\\id GEN\n"
    "\\p Some text two.";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, true, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Testing switching from separate verses into a combined verse.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string userModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1-2 This is really the text of the first (1st) verse. And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string serverModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 1\n"
    "\\p\n"
    "\\v 1-2 This is really the text of the first (1st) verse. And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Testing switching from a combined verse to separate verses.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string mergeBaseData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1-2 This is really the text of the first (1st) verse. And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string userModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string serverModificationData =
    "\\c 1\n"
    "\\p\n"
    "\\v 1-2 This is really the text of the first (1st) verse. And this is what the second verse contains.\n"
    "\\v 3 The third verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5\n";
    std::string output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, false, conflicts);
    std::string standard =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 This is really the text of the first (1st) verse.\n"
    "\\v 2 And this is what the second (2nd) verse contains.\n"
    "\\v 3 The third (3rd) verse.\n"
    "\\v 4 The fourth (4th) verse.\n"
    "\\v 5";
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }
  
  // Merge situation taken from real life.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string path;
    path = filter_url_create_root_path ({"unittests", "tests", "merge_2_base.usfm"});
    std::string mergeBaseData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_2_modification.usfm"});
    std::string userModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_2_server.usfm"});
    std::string serverModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_2_result.usfm"});
    std::string standard = filter_url_file_get_contents (path);
    
    std::string output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (3, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, true, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (3, conflicts.size ());
  }

  // Merge situation taken from real life in July 2020.
  {
    std::vector <Merge_Conflict> conflicts;
    std::string path;
    path = filter_url_create_root_path ({"unittests", "tests", "merge_3_base.usfm"});
    std::string mergeBaseData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_3_modification.usfm"});
    std::string userModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_3_server.usfm"});
    std::string serverModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_3_result.usfm"});
    std::string standard = filter_url_file_get_contents (path);
    
    std::string output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, true, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }

  // Merge situation taken from issue https://github.com/bibledit/cloud/issues/418
  {
    std::vector <Merge_Conflict> conflicts;
    std::string path;
    path = filter_url_create_root_path ({"unittests", "tests", "merge_4_base.usfm"});
    std::string mergeBaseData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_4_modification.usfm"});
    std::string userModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_4_server.usfm"});
    std::string serverModificationData = filter_url_file_get_contents (path);
    path = filter_url_create_root_path ({"unittests", "tests", "merge_4_result.usfm"});
    std::string standard = filter_url_file_get_contents (path);
    
    std::string output = filter_merge_run_clever (mergeBaseData, userModificationData, serverModificationData, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
    
    conflicts.clear ();
    output = filter_merge_run (mergeBaseData, userModificationData, serverModificationData, true, conflicts);
    EXPECT_EQ (standard, output);
    EXPECT_EQ (0, conflicts.size ());
  }

  refresh_sandbox (true);
}

#endif


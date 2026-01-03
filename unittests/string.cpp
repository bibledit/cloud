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
#include <filter/string.h>
#include <filter/url.h>


TEST (filter, string)
{
  // Test string replacement filter.
  {
    // Shows that std::string handles UTF-8 well for simple operations.
    EXPECT_EQ ("⇊⇦", filter::string::replace ("⇖", "", "⇊⇖⇦"));
    // Exercise the replacement counter.
    int counter = 0;
    EXPECT_EQ ("a", filter::string::replace ("bc", "", "abc", &counter));
    EXPECT_EQ (1, counter);
    // Handle empty input.
    EXPECT_EQ ("test", filter::string::replace ("", "", "test"));
    EXPECT_EQ ("", filter::string::replace ("", "", ""));
    // Same test for the real Unicode replacer.
    EXPECT_EQ ("⇊⇦", filter::string::unicode_string_str_replace ("⇖", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖⇦", filter::string::unicode_string_str_replace ("", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇖⇦", filter::string::unicode_string_str_replace ("⇊", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖", filter::string::unicode_string_str_replace ("⇦", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖⇦", filter::string::unicode_string_str_replace ("a", "", "⇊⇖⇦"));
    EXPECT_EQ ("a", filter::string::unicode_string_str_replace ("bc", "", "abc"));
  }

  // Test filter::string::array_unique, a C++ equivalent for PHP's filter::string::array_unique function.
  {
    std::vector <std::string> reference;
    reference.push_back ("aaa");
    reference.push_back ("b");
    reference.push_back ("zzz");
    reference.push_back ("x");
    reference.push_back ("yyy");
    reference.push_back ("k");
    std::vector <std::string> input;
    input.push_back ("aaa");
    input.push_back ("b");
    input.push_back ("aaa");
    input.push_back ("b");
    input.push_back ("aaa");
    input.push_back ("zzz");
    input.push_back ("x");
    input.push_back ("x");
    input.push_back ("yyy");
    input.push_back ("k");
    input.push_back ("k");
    std::vector <std::string> output = filter::string::array_unique (input);
    EXPECT_EQ (reference, output);
  }

  // Test filter::string::array_unique, a C++ equivalent for PHP's filter::string::array_unique function.
  {
    std::vector <int> reference;
    reference.push_back (111);
    reference.push_back (2);
    reference.push_back (999);
    reference.push_back (7);
    reference.push_back (888);
    reference.push_back (5);
    std::vector <int> input;
    input.push_back (111);
    input.push_back (2);
    input.push_back (111);
    input.push_back (2);
    input.push_back (111);
    input.push_back (999);
    input.push_back (7);
    input.push_back (7);
    input.push_back (888);
    input.push_back (5);
    input.push_back (5);
    std::vector <int> output = filter::string::array_unique (input);
    EXPECT_EQ (reference, output);
  }

  // Test filter::string::array_diff.
  {
    const std::vector <std::string> from       { "aaa", "bbb", "ccc", "zzz", "b", "x" };
    const std::vector <std::string> against    {        "bbb", "ccc",             "x" };
    const std::vector <std::string> difference { "aaa",               "zzz", "b"      };
    const std::vector <std::string> output = filter::string::array_diff (from, against);
    EXPECT_EQ (output, difference);
  }

  // Test filter::string::array_diff.
  {
    const std::vector <int> from       { 111, 222, 333, 999, 2, 8 };
    const std::vector <int> against    {      222, 333,         8 };
    const std::vector <int> difference { 111,           999, 2    };
    const std::vector <int> output = filter::string::array_diff (from, against);
    EXPECT_EQ (output, difference);
  }

  // Test for filter::string::array_intersect, a C++ equivalent for PHP's filter::string::array_intersect function.
  {
    std::vector <int> one;
    std::vector <int> two;
    one = {1};
    two = {2};
    EXPECT_EQ (std::vector <int>{}, filter::string::array_intersect (one, two));
    one = {1, 2, 3};
    two = {2, 3, 4};
    EXPECT_EQ ((std::vector <int>{2, 3}), filter::string::array_intersect (one, two));
    one = {1, 2, 3, 4, 5};
    two = {2, 3, 4};
    EXPECT_EQ ((std::vector <int>{2, 3, 4}), filter::string::array_intersect (one, two));
  }
  
  // Test hex2bin and filter::string::bin2hex as equivalents to PHP's functions.
  {
    std::string bin = "This is a 123 test.";
    std::string hex = "5468697320697320612031323320746573742e";
    EXPECT_EQ (hex, filter::string::bin2hex (bin));
    EXPECT_EQ (bin, filter::string::hex2bin (hex));
    bin = "סֶ	א	ב	ױ";
    hex = "d7a1d6b609d79009d79109d7b1";
    EXPECT_EQ (hex, filter::string::bin2hex (bin));
    EXPECT_EQ (bin, filter::string::hex2bin (hex));
    bin.clear ();
    hex.clear ();
    EXPECT_EQ (hex, filter::string::bin2hex (bin));
    EXPECT_EQ (bin, filter::string::hex2bin (hex));
    hex = "a";
    EXPECT_EQ ("", filter::string::bin2hex (bin));
    EXPECT_EQ (bin, filter::string::hex2bin (hex));
  }

  // Test string modifiers.
  {
    EXPECT_EQ (std::string(), filter::string::trim ("  "));
    EXPECT_EQ (std::string(), filter::string::trim (std::string()));
    EXPECT_EQ ("xx", filter::string::trim ("\t\nxx\n\r"));
    EXPECT_EQ (std::string(), filter::string::ltrim ("  "));
    EXPECT_EQ (std::string(), filter::string::ltrim (std::string()));
    EXPECT_EQ ("xx\n\r", filter::string::ltrim ("xx\n\r"));
    EXPECT_EQ ("xx  ", filter::string::ltrim ("  xx  "));
    EXPECT_EQ (std::string(), filter::string::rtrim ("  "));
    EXPECT_EQ (std::string(), filter::string::rtrim (std::string()));
    EXPECT_EQ ("xx", filter::string::rtrim ("xx\n\r"));
    EXPECT_EQ ("\n\rxx", filter::string::rtrim ("\n\rxx"));
    EXPECT_EQ ("  xx", filter::string::rtrim ("  xx  "));
    EXPECT_EQ ("0000012345", filter::string::fill ("12345", 10, '0'));
  }
  
  // Numeric tests.
  {
    EXPECT_EQ (true, filter::string::is_numeric ("1"));
    EXPECT_EQ (true, filter::string::is_numeric ("1234"));
    EXPECT_EQ (false, filter::string::is_numeric ("X"));
    EXPECT_EQ (false, filter::string::is_numeric ("120X"));
  }
  
  // String conversion to int.
  {
    EXPECT_EQ (0, filter::string::convert_to_int (""));
    EXPECT_EQ (123, filter::string::convert_to_int ("123"));
    EXPECT_EQ (123, filter::string::convert_to_int ("123xx"));
    EXPECT_EQ (0, filter::string::convert_to_int ("xxx123xx"));
  }
  
  // Unicode validity test.
  {
    EXPECT_EQ (true, filter::string::unicode_string_is_valid ("valid"));
    EXPECT_EQ (true, filter::string::unicode_string_is_valid ("בְּרֵאשִׁית, בָּרָא אֱלֹהִים, אֵת הַשָּׁמַיִם, וְאֵת הָאָרֶץ"));
  }
  
  // Searching in array.
  {
    std::vector <std::string> haystack = {"needle"};
    std::string needle = "needle";
    EXPECT_EQ (true, filter::string::in_array (needle, haystack));
    needle.clear ();
    EXPECT_EQ (false, filter::string::in_array (needle, haystack));
    haystack.clear ();
    EXPECT_EQ (false, filter::string::in_array (needle, haystack));
    needle = "needle";
    EXPECT_EQ (false, filter::string::in_array (needle, haystack));
    EXPECT_EQ (true, filter::string::in_array (1, {1, 2, 3}));
    EXPECT_EQ (false, filter::string::in_array (1, {2, 3}));
  }

  // Test random number generator.
  {
    int floor = 100'000;
    int ceiling = 999'999;
    int r1 = filter::string::rand (floor, ceiling);
    if ((r1 < floor) || (r1 > ceiling)) EXPECT_EQ ("Random generator out of bounds", std::to_string (r1));
    int r2 = filter::string::rand (floor, ceiling);
    if ((r2 < floor) || (r2 > ceiling)) EXPECT_EQ ("Random generator out of bounds", std::to_string (r2));
    if (r1 == r2) EXPECT_EQ ("Random generator should generate different values", std::to_string (r1) + " " + std::to_string (r2));
  }

  // Convert html to plain text.
  {
    std::string html =
    "<p>author</p>\n"
    "<p>Text 1<div>Text 2</div><div>Text 3</div></p>";
    std::string plain =
    "author\n"
    "Text 1\n"
    "Text 2\n"
    "Text 3";
    EXPECT_EQ (plain, filter::string::html2text (html));
  }
  {
    std::string html =
    "<p>writer (15 Nov):</p>\n"
    "<p>Second note.<div>Second matter.</div><div>A second round is needed.</div></p>\n"
    R"(<p>Here is <b>bold</b>, and here is <i>italics</i>.<div>Here is <sup>superscript</sup>&nbsp;and here is <sub>subscript</sub>.</div><div style="text-align: center;">Centered</div><div style="text-align: left;">Left justified</div><div style="text-align: left;"><ol><li>Numbered list.</li></ol>No numbered text.</div><div style="text-align: left;">Link to <a href="http://google.nl">http://google.nl</a>.</div><div style="text-align: left;">Here follows an image:&nbsp;<img src="http://localhost/image">.<br></div><h1>Header 1</h1><div>Normal text again below the header.</div></p>)" "\n";
    std::string plain =
    "writer (15 Nov):\n"
    "Second note.\n"
    "Second matter.\n"
    "A second round is needed.\n"
    "Here is bold, and here is italics.\n"
    "Here is superscript and here is subscript.\n"
    "Centered\n"
    "Left justified\n"
    "Numbered list.\n"
    "No numbered text.\n"
    "Link to http://google.nl.\n"
    "Here follows an image: .\n"
    "Header 1\n"
    "Normal text again below the header.\n";
    html = filter::string::any_space_to_standard_space (html);
    html = filter::string::html2text (html);
    EXPECT_EQ (filter::string::trim (plain), filter::string::trim (html));
  }
  {
    std::string html =
    "test notes four\n"
    "\n"
    "Logbook:\n"
    "\n";
    std::string plain =
    "test notes fourLogbook:";
    EXPECT_EQ (filter::string::trim (plain), filter::string::trim (filter::string::html2text (html)));
  }
  {
    std::string html =
    "Line one.<BR>\n"
    "\n"
    "Line two.<BR>\n"
    "\n"
    "Line three.<BR>\n";
    std::string plain =
    "Line one.\n"
    "Line two.\n"
    "Line three.\n";
    EXPECT_EQ (filter::string::trim (plain), filter::string::trim (filter::string::html2text (html)));
  }

  // Email address extraction.
  {
    EXPECT_EQ ("foo@bar.eu", filter::string::extract_email ("Foo Bar <foo@bar.eu>"));
    EXPECT_EQ ("foo@bar.eu", filter::string::extract_email ("<foo@bar.eu>"));
    EXPECT_EQ ("foo@bar.eu", filter::string::extract_email ("foo@bar.eu"));
    
    std::string body = "body";
    EXPECT_EQ (body, filter::string::extract_body (body));
    
    body =
    "\n"
    "test\n"
    "\n"
    "On Wed, 2011-03-02 at 08:26 +0100, Bibledit wrote:\n"
    "\n"
    "> test notes three\n"
    "\n"
    "\n"
    "> test\n"
    "\n"
    "On Wed, 2011-03-02 at 08:26 +0100, Bibledit wrote:\n"
    "\n"
    ">    test notes three \n";
    EXPECT_EQ ("test", filter::string::extract_body (body, "2011", "Bibledit"));
  }
  
  // Word markup.
  {
    std::string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    std::vector <std::string> words = { "makumbo", "akasia", "matanda" };
    std::string result = filter::string::markup_words (words, text);
    std::string standard =
    "Zvino uchagadzira <mark>makumbo</mark> uye <mark>Makumbo</mark> uye <mark>maKumbo</mark> uye <mark>MAKUMBO</mark> emu<mark>akasia</mark>*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa <mark>makumbo</mark> muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dze<mark>makumbo</mark> kutakura tafura.\n"
    "Zvino uchaita <mark>makumbo</mark> ne<mark>matanda</mark> ne<mark>Matanda</mark> ne<mark>maTANDA</mark> emu<mark>Akasia</mark>, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    EXPECT_EQ (standard, result);
  }

  // Word markup.
  {
    std::string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    std::vector <std::string> words;
    std::string result = filter::string::markup_words (words, text);
    EXPECT_EQ (text, result);
  }

  // Test tidying html.
  {
    std::string folder = filter_url_create_root_path ({"unittests", "tests"});
    std::string html = filter_url_file_get_contents (filter_url_create_path ({folder, "biblehub-john-1-1.html"}));
    std::vector <std::string> tidy = filter::string::explode (filter::string::html_tidy (html), '\n');
    EXPECT_EQ (747, static_cast<int>(tidy.size()));
  }
  
  {
    std::string input = "<span>Praise the LORD&#xB6;, all &amp; you nations</span>";
    std::string output = filter::string::convert_xml_character_entities_to_characters (input);
    std::string standard = filter::string::replace ("&#xB6;", "¶", input);
    EXPECT_EQ (standard, output);
    
    input = "<span>Praise the LORD &#x5D0; all you nations</span>";
    output = filter::string::convert_xml_character_entities_to_characters (input);
    standard = filter::string::replace ("&#x5D0;", "א", input);
    EXPECT_EQ (standard, output);
    
    input = "Username";
    output = filter::string::encrypt_decrypt ("key", input);
    output = filter::string::encrypt_decrypt ("key", output);
    EXPECT_EQ (input, output);
    
    input = "בְּרֵאשִׁ֖ית בָּרָ֣א אֱלֹהִ֑ים אֵ֥ת הַשָּׁמַ֖יִם וְאֵ֥ת הָאָֽרֶץ";
    output = filter::string::encrypt_decrypt ("בְּרֵאשִׁ֖ית", input);
    output = filter::string::encrypt_decrypt ("בְּרֵאשִׁ֖ית", output);
    EXPECT_EQ (input, output);
  }
  
  {
    std::string one = filter::string::get_new_random_string ();
    std::this_thread::sleep_for (std::chrono::milliseconds (10));
    std::string two = filter::string::get_new_random_string ();
    EXPECT_EQ (32, one.length ());
    EXPECT_EQ (true, one != two);
  }
  
  {
    EXPECT_EQ (4, static_cast<int>( filter::string::unicode_string_length ("test")));
    EXPECT_EQ (4, static_cast<int>( filter::string::unicode_string_length ("ᨁᨃᨅᨕ")));
  }
  
  {
    std::string hebrew = "אָבּגּדּהּ";
    EXPECT_EQ ("st1234", filter::string::unicode_string_substr ("test1234", 2));
    EXPECT_EQ ("גּדּהּ", filter::string::unicode_string_substr (hebrew, 2));
    EXPECT_EQ ("", filter::string::unicode_string_substr (hebrew, 5));
    EXPECT_EQ ("", filter::string::unicode_string_substr (hebrew, 6));
    EXPECT_EQ ("test", filter::string::unicode_string_substr ("test123456", 0, 4));
    EXPECT_EQ ("12", filter::string::unicode_string_substr ("test123456", 4, 2));
    EXPECT_EQ ("גּדּ", filter::string::unicode_string_substr (hebrew, 2, 2));
    EXPECT_EQ ("גּדּהּ", filter::string::unicode_string_substr (hebrew, 2, 10));
  }
  
  {
    std::string hebrew {"אָבּגּדּהּ"};
    std::string needle {"דּ"};
    EXPECT_EQ (3, static_cast<int>(filter::string::unicode_string_strpos ("012345", "3")));
    EXPECT_EQ (5, static_cast<int>(filter::string::unicode_string_strpos ("012345", "5")));
    EXPECT_EQ (0, static_cast<int>(filter::string::unicode_string_strpos ("012345", "0")));
    EXPECT_EQ (-1, static_cast<int>(filter::string::unicode_string_strpos ("012345", "6")));
    EXPECT_EQ (3, static_cast<int>(filter::string::unicode_string_strpos (hebrew, needle)));
    EXPECT_EQ (3, static_cast<int>(filter::string::unicode_string_strpos (hebrew, needle, 3)));
    EXPECT_EQ (-1, static_cast<int>(filter::string::unicode_string_strpos (hebrew, needle, 4)));
    EXPECT_EQ (-1, static_cast<int>(filter::string::unicode_string_strpos ("", "3")));
  }
  
  {
    EXPECT_EQ (2, static_cast<int>(filter::string::unicode_string_strpos_case_insensitive ("AbCdEf", "c")));
    EXPECT_EQ (2, static_cast<int>(filter::string::unicode_string_strpos_case_insensitive ("AbCdEf", "cD")));
    EXPECT_EQ (-1, static_cast<int>(filter::string::unicode_string_strpos_case_insensitive ("AbCdEf", "ce")));
  }
  
  {
    EXPECT_EQ ("test1234", filter::string::unicode_string_casefold ("test1234"));
    EXPECT_EQ ("test1234", filter::string::unicode_string_casefold ("TEST1234"));
    EXPECT_EQ ("θεος", filter::string::unicode_string_casefold ("Θεος"));
    EXPECT_EQ ("α α β β", filter::string::unicode_string_casefold ("Α α Β β"));
    EXPECT_EQ ("אָבּגּדּהּ", filter::string::unicode_string_casefold ("אָבּגּדּהּ"));
  }
  
  {
    EXPECT_EQ ("TEST1234", filter::string::unicode_string_uppercase ("test1234"));
    EXPECT_EQ ("TEST1234", filter::string::unicode_string_uppercase ("TEST1234"));
    EXPECT_EQ ("ΘΕΟΣ", filter::string::unicode_string_uppercase ("Θεος"));
    EXPECT_EQ ("Α Α Β Β", filter::string::unicode_string_uppercase ("Α α Β β"));
    EXPECT_EQ ("אָבּגּדּהּ", filter::string::unicode_string_uppercase ("אָבּגּדּהּ"));
  }
  
  {
    EXPECT_EQ ("ABCDEFG", filter::string::unicode_string_transliterate ("ABCDEFG"));
    EXPECT_EQ ("Ιησου Χριστου", filter::string::unicode_string_transliterate ("Ἰησοῦ Χριστοῦ"));
    EXPECT_EQ ("אבגדה", filter::string::unicode_string_transliterate ("אָבּגּדּהּ"));
  }
  
  {
    std::vector <std::string> needles;
    needles = filter::string::search_needles ("ABC", "one abc two ABc three aBc four");
    EXPECT_EQ ((std::vector <std::string>{ "abc", "ABc", "aBc" }), needles);
    needles = filter::string::search_needles ("abc", "one abc two ABc three aBc four");
    EXPECT_EQ ((std::vector <std::string>{ "abc", "ABc", "aBc" }), needles);
  }
  
  {
    EXPECT_EQ (false, filter::string::unicode_string_is_punctuation ("A"));
    EXPECT_EQ (false, filter::string::unicode_string_is_punctuation ("z"));
    EXPECT_EQ (true, filter::string::unicode_string_is_punctuation ("."));
    EXPECT_EQ (true, filter::string::unicode_string_is_punctuation (","));
  }
  
  {
    EXPECT_EQ (false, filter::string::convert_to_bool ("0"));
    EXPECT_EQ (false, filter::string::convert_to_bool ("false"));
    EXPECT_EQ (false, filter::string::convert_to_bool ("FALSE"));
    EXPECT_EQ (true, filter::string::convert_to_bool ("1"));
    EXPECT_EQ (true, filter::string::convert_to_bool ("true"));
    EXPECT_EQ (true, filter::string::convert_to_bool ("TRUE"));
  }
  
  {
    EXPECT_EQ (21109, filter::string::unicode_string_convert_to_codepoint ("創"));
    EXPECT_EQ (97, filter::string::unicode_string_convert_to_codepoint ("a"));
  }
  
  {
    // Check that the function to desanitize html no longer corrupts UTF-8.
    std::string html = "<p>“Behold”, from “הִנֵּה”, means look at</p>";
    std::string desanitized = filter::string::any_space_to_standard_space (filter::string::unescape_special_xml_characters (html));
    EXPECT_EQ (html, desanitized);
    // Regression test for fix for corrupting Greek.
    html = "Ada juga seorang pengemis yang ita bernama Lazarus.<span class=i-notecall1>1</span> Setiap hari dia terbaring di pintu gerbang rumah orang kaya itu. Badan Lazarus penuh dengan luka bernanah dan busuk.<span class=i-notecall2>2</span></p><p class=b-notes><br></p><p class=b-f><span class=i-notebody1>1</span> <span class=i-fr>16:20 </span><span class=i-fk>Lazarus </span><span class=i-ft>Orang miskin Lazarus dalam perumpamaan ini berbeda dengan Lazarus— sahabat Isa yang dihidupkan oleh Isa dari kematian (Yoh. 11).</span></p><p class=b-x><span class=i-notebody2>2</span> <span class=i-xo>16:20 </span><span class=i-xt>Πτωχὸς δέ τις ἦν ὀνόματι Λάζαρος, ὃς ἐβέβλητο πρὸς τὸν πυλῶνα αὐτοῦ ἡλκωμένος</span></p>";
    desanitized = filter::string::any_space_to_standard_space (html);
    EXPECT_EQ (html, desanitized);
  }
  
  // Test whitespace characters, breaking and non-breaking.
  {
    // The "­" below is not an empty string, but the soft hyphen U+00AD.
    std::string standard_soft_hyphen = "­";
    EXPECT_EQ (standard_soft_hyphen, filter::string::soft_hyphen_u00AD ());
    EXPECT_EQ ("\u00AD", filter::string::soft_hyphen_u00AD ());

    EXPECT_EQ ("\u00A0", filter::string::non_breaking_space_u00A0 ());

    // The space below is "en space", U+2002.
    std::string standard_en_space_u2002 = " ";
    EXPECT_EQ (standard_en_space_u2002, filter::string::en_space_u2002 ());
    EXPECT_EQ ("\u2002", filter::string::en_space_u2002 ());
  }
  
  // Test conversion of boolean to true / false string.
  {
    EXPECT_EQ ("true", filter::string::convert_to_true_false (true));
    EXPECT_EQ ("false", filter::string::convert_to_true_false (false));
  }
  
  // Test two explode functions.
  {
    std::vector <std::string> result;

    // Explode on single delimiter.
    result = filter::string::explode ("a b c", ' ');
    EXPECT_EQ ((std::vector <std::string>{"a", "b", "c"}), result);

    // Explode on a single space.
    result = filter::string::explode ("a b c", " ");
    EXPECT_EQ ((std::vector <std::string>{"a", "b", "c"}), result);

    // Explode on a set consisting of two spaces.
    result = filter::string::explode ("a b c", "  ");
    EXPECT_EQ ((std::vector <std::string>{"a", "b", "c"}), result);

    // Explode on a semicolon, on a comma, and on a full stop.
    result = filter::string::explode ("aa.bb,cc;", ";,.");
    EXPECT_EQ ((std::vector <std::string>{"aa", "bb", "cc"}), result);

    // Explode on two punctuation marks, leaving one in the output.
    result = filter::string::explode ("aa.bb,cc;", ";,");
    EXPECT_EQ ((std::vector <std::string>{"aa.bb", "cc"}), result);
  }
  
  // Test limited implode function.
  {
    {
      std::vector<std::string> container {"Dot", "Name", "1", "9"};
      filter::string::implode_from_beginning_remain_with_max_n_bits (container, 3, ".");
      EXPECT_EQ ((std::vector <std::string>{"Dot.Name", "1", "9"}), container);
    }
    {
      std::vector<std::string> container {"NoDotName", "1", "9"};
      filter::string::implode_from_beginning_remain_with_max_n_bits (container, 3, ".");
      EXPECT_EQ ((std::vector <std::string>{"NoDotName", "1", "9"}), container);
    }
    {
      std::vector<std::string> container {"", "Dot", "Name", "1", "9"};
      filter::string::implode_from_beginning_remain_with_max_n_bits (container, 3, ".");
      EXPECT_EQ ((std::vector <std::string>{".Dot.Name", "1", "9"}), container);
    }
    {
      std::vector<std::string> container {"Dot", "Name", "", "1", "9"};
      filter::string::implode_from_beginning_remain_with_max_n_bits (container, 3, ".");
      EXPECT_EQ ((std::vector <std::string>{"Dot.Name.", "1", "9"}), container);
    }
  }
  
  // Test the array mover function.
  {
    std::vector <std::string> container;
    
    container = {};
    filter::string::array_move_up_down (container, 0, false);
    EXPECT_EQ (std::vector <std::string>{}, container);
  }
  
  // Test UTF-16 number of bytes whether 1 or 2.
  {
    std::u16string u16;
    u16 = filter::string::convert_to_u16string ("a");
    EXPECT_EQ (1, static_cast<int> (u16.length()));
    u16 = filter::string::convert_to_u16string ("ℵ");
    EXPECT_EQ (1, static_cast<int> (u16.length()));
    u16 = filter::string::convert_to_u16string ("😀");
    EXPECT_EQ (2, static_cast<int> (u16.length()));
  }
  
  // Test tidying invalid html.
  {
    std::string path_invalid = filter_url_create_root_path ({"unittests", "tests", "html-invalid-1.html"});
    std::string html_invalid = filter_url_file_get_contents(path_invalid);

    std::string path_valid {};
    std::string html_valid {};

    std::string html_tidied = filter::string::fix_invalid_html_gumbo (html_invalid);
    path_valid = filter_url_create_root_path ({"unittests", "tests", "html-fixed-1-gumbo.html"});
    html_valid = filter_url_file_get_contents(path_valid);
    EXPECT_EQ (html_valid, html_tidied);

    html_invalid = R"(
<!DOCTYPE html>
<html>
<body>
<h1>My First Heading</h2>
<p>My first paragraph.</p>
<script>
document.getElementById("demo").innerHTML = "Hello JavaScript!";
</script>
</body>
</html>
)";
    html_tidied = filter::string::fix_invalid_html_gumbo (html_invalid);
    html_valid =
R"(<!DOCTYPE html>
<html>
 <head>
 </head>
 <body>
  <h1>
   My First Heading
  </h1>
  <p>   My first paragraph.</p>
 </body>
</html>
)";
    EXPECT_EQ (html_valid, html_tidied);
  }
  
  // Test "tidying" empty html.
  {
    std::string result = filter::string::fix_invalid_html_gumbo (std::string());
    std::string standard {
R"(<html>
 <head>
 </head>
 <body>
 </body>
</html>
)"
    };
    EXPECT_EQ (standard, result);
  }
  
  // Test tidying html with special XML characters.
  {
    std::string html = R"(<p>Paragraph & < > paragraph</p>)";
    std::string tidied = filter::string::fix_invalid_html_gumbo (html);
    std::string valid =
R"(<html>
 <head>
 </head>
 <body>
  <p>   Paragraph &amp; &lt; &gt; paragraph</p>
 </body>
</html>
)";
    EXPECT_EQ (valid, tidied);

  }
  
  // Test collapsing double spaces.
  {
    std::string standard;
    std::string result;

    standard = "ABC abc";
    result = filter::string::collapse_whitespace ("ABC abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::string::collapse_whitespace ("ABC  abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::string::collapse_whitespace ("ABC   abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::string::collapse_whitespace ("ABC      abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::string::collapse_whitespace ("ABC               abc");
    EXPECT_EQ (standard, result);
  }

  // Test escaping and unescaping special XML characters.
  {
    std::string standard, result;

    standard = "&quot; &quot;";
    result = filter::string::escape_special_xml_characters (R"(" ")");
    EXPECT_EQ (standard, result);

    standard = R"(" ")";
    result = filter::string::unescape_special_xml_characters ("&quot; &quot;");
    EXPECT_EQ (standard, result);
  }
  
  // Test conversion from Windows-1252 encoding to UTF-8 encoding.
  {
    std::string input = "Windows 1252";
    std::string output = filter::string::convert_windows1252_to_utf8 (input);
    EXPECT_EQ (input, output);
  }
}

#endif


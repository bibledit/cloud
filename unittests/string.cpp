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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
using namespace std;


TEST (filter, string)
{
  // Test string replacement filter.
  {
    // Shows that std::string handles UTF-8 well for simple operations.
    EXPECT_EQ ("⇊⇦", filter::strings::replace ("⇖", "", "⇊⇖⇦"));
    // Exercise the replacement counter.
    int counter = 0;
    EXPECT_EQ ("a", filter::strings::replace ("bc", "", "abc", &counter));
    EXPECT_EQ (1, counter);
    // Same test for the real Unicode replacer.
    EXPECT_EQ ("⇊⇦", filter::strings::unicode_string_str_replace ("⇖", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖⇦", filter::strings::unicode_string_str_replace ("", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇖⇦", filter::strings::unicode_string_str_replace ("⇊", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖", filter::strings::unicode_string_str_replace ("⇦", "", "⇊⇖⇦"));
    EXPECT_EQ ("⇊⇖⇦", filter::strings::unicode_string_str_replace ("a", "", "⇊⇖⇦"));
    EXPECT_EQ ("a", filter::strings::unicode_string_str_replace ("bc", "", "abc"));
  }

  // Test filter::strings::array_unique, a C++ equivalent for PHP's filter::strings::array_unique function.
  {
    vector <string> reference;
    reference.push_back ("aaa");
    reference.push_back ("b");
    reference.push_back ("zzz");
    reference.push_back ("x");
    reference.push_back ("yyy");
    reference.push_back ("k");
    vector <string> input;
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
    vector <string> output = filter::strings::array_unique (input);
    EXPECT_EQ (reference, output);
  }

  // Test filter::strings::array_unique, a C++ equivalent for PHP's filter::strings::array_unique function.
  {
    vector <int> reference;
    reference.push_back (111);
    reference.push_back (2);
    reference.push_back (999);
    reference.push_back (7);
    reference.push_back (888);
    reference.push_back (5);
    vector <int> input;
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
    vector <int> output = filter::strings::array_unique (input);
    EXPECT_EQ (reference, output);
  }

  // Test filter::strings::array_diff, a C++ equivalent for PHP's filter::strings::array_diff function.
  {
    vector <string> reference;
    reference.push_back ("aaa");
    reference.push_back ("zzz");
    reference.push_back ("b");
    vector <string> from;
    from.push_back ("aaa");
    from.push_back ("bbb");
    from.push_back ("ccc");
    from.push_back ("zzz");
    from.push_back ("b");
    from.push_back ("x");
    vector <string> against;
    against.push_back ("bbb");
    against.push_back ("ccc");
    against.push_back ("x");
    vector <string> output = filter::strings::array_diff (from, against);
    EXPECT_EQ (reference, output);
  }

  // Test filter::strings::array_diff, a C++ equivalent for PHP's filter::strings::array_diff function.
  {
    vector <int> reference;
    reference.push_back (111);
    reference.push_back (999);
    reference.push_back (2);
    vector <int> from;
    from.push_back (111);
    from.push_back (222);
    from.push_back (333);
    from.push_back (999);
    from.push_back (2);
    from.push_back (8);
    vector <int> against;
    against.push_back (222);
    against.push_back (333);
    against.push_back (8);
    vector <int> output = filter::strings::array_diff (from, against);
    EXPECT_EQ (reference, output);
  }

  // Test for array_intersect, a C++ equivalent for PHP's array_intersect function.
  {
    vector <int> one;
    vector <int> two;
    one = {1};
    two = {2};
    EXPECT_EQ (vector <int>{}, array_intersect (one, two));
    one = {1, 2, 3};
    two = {2, 3, 4};
    EXPECT_EQ ((vector <int>{2, 3}), array_intersect (one, two));
    one = {1, 2, 3, 4, 5};
    two = {2, 3, 4};
    EXPECT_EQ ((vector <int>{2, 3, 4}), array_intersect (one, two));
  }
  
  // Test hex2bin and filter::strings::bin2hex as equivalents to PHP's functions.
  {
    string bin = "This is a 123 test.";
    string hex = "5468697320697320612031323320746573742e";
    EXPECT_EQ (hex, filter::strings::bin2hex (bin));
    EXPECT_EQ (bin, filter::strings::hex2bin (hex));
    bin = "סֶ	א	ב	ױ";
    hex = "d7a1d6b609d79009d79109d7b1";
    EXPECT_EQ (hex, filter::strings::bin2hex (bin));
    EXPECT_EQ (bin, filter::strings::hex2bin (hex));
    bin.clear ();
    hex.clear ();
    EXPECT_EQ (hex, filter::strings::bin2hex (bin));
    EXPECT_EQ (bin, filter::strings::hex2bin (hex));
    hex = "a";
    EXPECT_EQ ("", filter::strings::bin2hex (bin));
    EXPECT_EQ (bin, filter::strings::hex2bin (hex));
  }

  // Test string modifiers.
  {
    EXPECT_EQ (string(), filter::strings::trim ("  "));
    EXPECT_EQ (string(), filter::strings::trim (string()));
    EXPECT_EQ ("xx", filter::strings::trim ("\t\nxx\n\r"));
    EXPECT_EQ (string(), filter::strings::ltrim ("  "));
    EXPECT_EQ (string(), filter::strings::ltrim (string()));
    EXPECT_EQ ("xx\n\r", filter::strings::ltrim ("xx\n\r"));
    EXPECT_EQ ("xx  ", filter::strings::ltrim ("  xx  "));
    EXPECT_EQ (string(), filter::strings::rtrim ("  "));
    EXPECT_EQ (string(), filter::strings::rtrim (string()));
    EXPECT_EQ ("xx", filter::strings::rtrim ("xx\n\r"));
    EXPECT_EQ ("\n\rxx", filter::strings::rtrim ("\n\rxx"));
    EXPECT_EQ ("  xx", filter::strings::rtrim ("  xx  "));
    EXPECT_EQ ("0000012345", filter::strings::fill ("12345", 10, '0'));
  }
  
  // Numeric tests.
  {
    EXPECT_EQ (true, filter::strings::is_numeric ("1"));
    EXPECT_EQ (true, filter::strings::is_numeric ("1234"));
    EXPECT_EQ (false, filter::strings::is_numeric ("X"));
    EXPECT_EQ (false, filter::strings::is_numeric ("120X"));
  }
  
  // String conversion to int.
  {
    EXPECT_EQ (0, filter::strings::convert_to_int (""));
    EXPECT_EQ (123, filter::strings::convert_to_int ("123"));
    EXPECT_EQ (123, filter::strings::convert_to_int ("123xx"));
    EXPECT_EQ (0, filter::strings::convert_to_int ("xxx123xx"));
  }
  
  // Unicode validity test.
  {
    EXPECT_EQ (true, filter::strings::unicode_string_is_valid ("valid"));
    EXPECT_EQ (true, filter::strings::unicode_string_is_valid ("בְּרֵאשִׁית, בָּרָא אֱלֹהִים, אֵת הַשָּׁמַיִם, וְאֵת הָאָרֶץ"));
  }
  
  // Searching in array.
  {
    vector <string> haystack = {"needle"};
    string needle = "needle";
    EXPECT_EQ (true, in_array (needle, haystack));
    needle.clear ();
    EXPECT_EQ (false, in_array (needle, haystack));
    haystack.clear ();
    EXPECT_EQ (false, in_array (needle, haystack));
    needle = "needle";
    EXPECT_EQ (false, in_array (needle, haystack));
    EXPECT_EQ (true, in_array (1, {1, 2, 3}));
    EXPECT_EQ (false, in_array (1, {2, 3}));
  }

  // Test random number generator.
  {
    int floor = 100'000;
    int ceiling = 999'999;
    int r1 = filter::strings::rand (floor, ceiling);
    if ((r1 < floor) || (r1 > ceiling)) EXPECT_EQ ("Random generator out of bounds", filter::strings::convert_to_string (r1));
    int r2 = filter::strings::rand (floor, ceiling);
    if ((r2 < floor) || (r2 > ceiling)) EXPECT_EQ ("Random generator out of bounds", filter::strings::convert_to_string (r2));
    if (r1 == r2) EXPECT_EQ ("Random generator should generate different values", filter::strings::convert_to_string (r1) + " " + filter::strings::convert_to_string (r2));
  }

  // Convert html to plain text.
  {
    string html =
    "<p>author</p>\n"
    "<p>Text 1<div>Text 2</div><div>Text 3</div></p>";
    string plain =
    "author\n"
    "Text 1\n"
    "Text 2\n"
    "Text 3";
    EXPECT_EQ (plain, filter::strings::html2text (html));
  }
  {
    string html =
    "<p>writer (15 Nov):</p>\n"
    "<p>Second note.<div>Second matter.</div><div>A second round is needed.</div></p>\n"
    R"(<p>Here is <b>bold</b>, and here is <i>italics</i>.<div>Here is <sup>superscript</sup>&nbsp;and here is <sub>subscript</sub>.</div><div style="text-align: center;">Centered</div><div style="text-align: left;">Left justified</div><div style="text-align: left;"><ol><li>Numbered list.</li></ol>No numbered text.</div><div style="text-align: left;">Link to <a href="http://google.nl">http://google.nl</a>.</div><div style="text-align: left;">Here follows an image:&nbsp;<img src="http://localhost/image">.<br></div><h1>Header 1</h1><div>Normal text again below the header.</div></p>)" "\n";
    string plain =
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
    html = filter::strings::any_space_to_standard_space (html);
    html = filter::strings::html2text (html);
    EXPECT_EQ (filter::strings::trim (plain), filter::strings::trim (html));
  }
  {
    string html =
    "test notes four\n"
    "\n"
    "Logbook:\n"
    "\n";
    string plain =
    "test notes fourLogbook:";
    EXPECT_EQ (filter::strings::trim (plain), filter::strings::trim (filter::strings::html2text (html)));
  }
  {
    string html =
    "Line one.<BR>\n"
    "\n"
    "Line two.<BR>\n"
    "\n"
    "Line three.<BR>\n";
    string plain =
    "Line one.\n"
    "Line two.\n"
    "Line three.\n";
    EXPECT_EQ (filter::strings::trim (plain), filter::strings::trim (filter::strings::html2text (html)));
  }

  // Email address extraction.
  {
    EXPECT_EQ ("foo@bar.eu", filter::strings::extract_email ("Foo Bar <foo@bar.eu>"));
    EXPECT_EQ ("foo@bar.eu", filter::strings::extract_email ("<foo@bar.eu>"));
    EXPECT_EQ ("foo@bar.eu", filter::strings::extract_email ("foo@bar.eu"));
    
    string body = "body";
    EXPECT_EQ (body, filter::strings::extract_body (body));
    
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
    EXPECT_EQ ("test", filter::strings::extract_body (body, "2011", "Bibledit"));
  }
  
  // Word markup.
  {
    string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    vector <string> words = { "makumbo", "akasia", "matanda" };
    string result = filter::strings::markup_words (words, text);
    string standard =
    "Zvino uchagadzira <mark>makumbo</mark> uye <mark>Makumbo</mark> uye <mark>maKumbo</mark> uye <mark>MAKUMBO</mark> emu<mark>akasia</mark>*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa <mark>makumbo</mark> muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dze<mark>makumbo</mark> kutakura tafura.\n"
    "Zvino uchaita <mark>makumbo</mark> ne<mark>matanda</mark> ne<mark>Matanda</mark> ne<mark>maTANDA</mark> emu<mark>Akasia</mark>, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    EXPECT_EQ (standard, result);
  }

  // Word markup.
  {
    string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    vector <string> words;
    string result = filter::strings::markup_words (words, text);
    EXPECT_EQ (text, result);
  }

  // Test tidying html.
  {
    string folder = filter_url_create_root_path ({"unittests", "tests"});
    string html = filter_url_file_get_contents (filter_url_create_path ({folder, "biblehub-john-1-1.html"}));
    vector <string> tidy = filter::strings::explode (filter::strings::html_tidy (html), '\n');
    EXPECT_EQ (747, static_cast<int>(tidy.size()));
  }
  
  {
    string input = "<span>Praise the LORD&#xB6;, all &amp; you nations</span>";
    string output = filter::strings::convert_xml_character_entities_to_characters (input);
    string standard = filter::strings::replace ("&#xB6;", "¶", input);
    EXPECT_EQ (standard, output);
    
    input = "<span>Praise the LORD &#x5D0; all you nations</span>";
    output = filter::strings::convert_xml_character_entities_to_characters (input);
    standard = filter::strings::replace ("&#x5D0;", "א", input);
    EXPECT_EQ (standard, output);
    
    input = "Username";
    output = filter::strings::encrypt_decrypt ("key", input);
    output = filter::strings::encrypt_decrypt ("key", output);
    EXPECT_EQ (input, output);
    
    input = "בְּרֵאשִׁ֖ית בָּרָ֣א אֱלֹהִ֑ים אֵ֥ת הַשָּׁמַ֖יִם וְאֵ֥ת הָאָֽרֶץ";
    output = filter::strings::encrypt_decrypt ("בְּרֵאשִׁ֖ית", input);
    output = filter::strings::encrypt_decrypt ("בְּרֵאשִׁ֖ית", output);
    EXPECT_EQ (input, output);
  }
  
  {
    string one = filter::strings::get_new_random_string ();
    this_thread::sleep_for (chrono::milliseconds (10));
    string two = filter::strings::get_new_random_string ();
    EXPECT_EQ (32, one.length ());
    EXPECT_EQ (true, one != two);
  }
  
  {
    EXPECT_EQ (4, static_cast<int>( filter::strings::unicode_string_length ("test")));
    EXPECT_EQ (4, static_cast<int>( filter::strings::unicode_string_length ("ᨁᨃᨅᨕ")));
  }
  
  {
    string hebrew = "אָבּגּדּהּ";
    EXPECT_EQ ("st1234", filter::strings::unicode_string_substr ("test1234", 2));
    EXPECT_EQ ("גּדּהּ", filter::strings::unicode_string_substr (hebrew, 2));
    EXPECT_EQ ("", filter::strings::unicode_string_substr (hebrew, 5));
    EXPECT_EQ ("", filter::strings::unicode_string_substr (hebrew, 6));
    EXPECT_EQ ("test", filter::strings::unicode_string_substr ("test123456", 0, 4));
    EXPECT_EQ ("12", filter::strings::unicode_string_substr ("test123456", 4, 2));
    EXPECT_EQ ("גּדּ", filter::strings::unicode_string_substr (hebrew, 2, 2));
    EXPECT_EQ ("גּדּהּ", filter::strings::unicode_string_substr (hebrew, 2, 10));
  }
  
  {
    string hebrew {"אָבּגּדּהּ"};
    string needle {"דּ"};
    EXPECT_EQ (3, static_cast<int>(filter::strings::unicode_string_strpos ("012345", "3")));
    EXPECT_EQ (5, static_cast<int>(filter::strings::unicode_string_strpos ("012345", "5")));
    EXPECT_EQ (0, static_cast<int>(filter::strings::unicode_string_strpos ("012345", "0")));
    EXPECT_EQ (-1, static_cast<int>(filter::strings::unicode_string_strpos ("012345", "6")));
    EXPECT_EQ (3, static_cast<int>(filter::strings::unicode_string_strpos (hebrew, needle)));
    EXPECT_EQ (3, static_cast<int>(filter::strings::unicode_string_strpos (hebrew, needle, 3)));
    EXPECT_EQ (-1, static_cast<int>(filter::strings::unicode_string_strpos (hebrew, needle, 4)));
    EXPECT_EQ (-1, static_cast<int>(filter::strings::unicode_string_strpos ("", "3")));
  }
  
  {
    EXPECT_EQ (2, static_cast<int>(filter::strings::unicode_string_strpos_case_insensitive ("AbCdEf", "c")));
    EXPECT_EQ (2, static_cast<int>(filter::strings::unicode_string_strpos_case_insensitive ("AbCdEf", "cD")));
    EXPECT_EQ (-1, static_cast<int>(filter::strings::unicode_string_strpos_case_insensitive ("AbCdEf", "ce")));
  }
  
  {
    EXPECT_EQ ("test1234", filter::strings::unicode_string_casefold ("test1234"));
    EXPECT_EQ ("test1234", filter::strings::unicode_string_casefold ("TEST1234"));
    EXPECT_EQ ("θεος", filter::strings::unicode_string_casefold ("Θεος"));
    EXPECT_EQ ("α α β β", filter::strings::unicode_string_casefold ("Α α Β β"));
    EXPECT_EQ ("אָבּגּדּהּ", filter::strings::unicode_string_casefold ("אָבּגּדּהּ"));
  }
  
  {
    EXPECT_EQ ("TEST1234", filter::strings::unicode_string_uppercase ("test1234"));
    EXPECT_EQ ("TEST1234", filter::strings::unicode_string_uppercase ("TEST1234"));
    EXPECT_EQ ("ΘΕΟΣ", filter::strings::unicode_string_uppercase ("Θεος"));
    EXPECT_EQ ("Α Α Β Β", filter::strings::unicode_string_uppercase ("Α α Β β"));
    EXPECT_EQ ("אָבּגּדּהּ", filter::strings::unicode_string_uppercase ("אָבּגּדּהּ"));
  }
  
  {
    EXPECT_EQ ("ABCDEFG", filter::strings::unicode_string_transliterate ("ABCDEFG"));
    EXPECT_EQ ("Ιησου Χριστου", filter::strings::unicode_string_transliterate ("Ἰησοῦ Χριστοῦ"));
    EXPECT_EQ ("אבגדה", filter::strings::unicode_string_transliterate ("אָבּגּדּהּ"));
  }
  
  {
    vector <string> needles;
    needles = filter::strings::search_needles ("ABC", "one abc two ABc three aBc four");
    EXPECT_EQ ((vector <string>{ "abc", "ABc", "aBc" }), needles);
    needles = filter::strings::search_needles ("abc", "one abc two ABc three aBc four");
    EXPECT_EQ ((vector <string>{ "abc", "ABc", "aBc" }), needles);
  }
  
  {
    EXPECT_EQ (false, filter::strings::unicode_string_is_punctuation ("A"));
    EXPECT_EQ (false, filter::strings::unicode_string_is_punctuation ("z"));
    EXPECT_EQ (true, filter::strings::unicode_string_is_punctuation ("."));
    EXPECT_EQ (true, filter::strings::unicode_string_is_punctuation (","));
  }
  
  {
    EXPECT_EQ (false, filter::strings::convert_to_bool ("0"));
    EXPECT_EQ (false, filter::strings::convert_to_bool ("false"));
    EXPECT_EQ (false, filter::strings::convert_to_bool ("FALSE"));
    EXPECT_EQ (true, filter::strings::convert_to_bool ("1"));
    EXPECT_EQ (true, filter::strings::convert_to_bool ("true"));
    EXPECT_EQ (true, filter::strings::convert_to_bool ("TRUE"));
  }
  
  {
    EXPECT_EQ (21109, filter::strings::unicode_string_convert_to_codepoint ("創"));
    EXPECT_EQ (97, filter::strings::unicode_string_convert_to_codepoint ("a"));
  }
  
  {
    // Check that the function to desanitize html no longer corrupts UTF-8.
    string html = "<p>“Behold”, from “הִנֵּה”, means look at</p>";
    string desanitized = filter::strings::any_space_to_standard_space (filter::strings::unescape_special_xml_characters (html));
    EXPECT_EQ (html, desanitized);
    // Regression test for fix for corrupting Greek.
    html = "Ada juga seorang pengemis yang ita bernama Lazarus.<span class=i-notecall1>1</span> Setiap hari dia terbaring di pintu gerbang rumah orang kaya itu. Badan Lazarus penuh dengan luka bernanah dan busuk.<span class=i-notecall2>2</span></p><p class=b-notes><br></p><p class=b-f><span class=i-notebody1>1</span> <span class=i-fr>16:20 </span><span class=i-fk>Lazarus </span><span class=i-ft>Orang miskin Lazarus dalam perumpamaan ini berbeda dengan Lazarus— sahabat Isa yang dihidupkan oleh Isa dari kematian (Yoh. 11).</span></p><p class=b-x><span class=i-notebody2>2</span> <span class=i-xo>16:20 </span><span class=i-xt>Πτωχὸς δέ τις ἦν ὀνόματι Λάζαρος, ὃς ἐβέβλητο πρὸς τὸν πυλῶνα αὐτοῦ ἡλκωμένος</span></p>";
    desanitized = filter::strings::any_space_to_standard_space (html);
    EXPECT_EQ (html, desanitized);
  }
  
  // Test whitespace characters, breaking and non-breaking.
  {
    // The "­" below is not an empty string, but the soft hyphen U+00AD.
    string standard_soft_hyphen = "­";
    EXPECT_EQ (standard_soft_hyphen, filter::strings::soft_hyphen_u00AD ());
    EXPECT_EQ ("\u00AD", filter::strings::soft_hyphen_u00AD ());

    EXPECT_EQ ("\u00A0", filter::strings::non_breaking_space_u00A0 ());

    // The space below is "en space", U+2002.
    string standard_en_space_u2002 = " ";
    EXPECT_EQ (standard_en_space_u2002, filter::strings::en_space_u2002 ());
    EXPECT_EQ ("\u2002", filter::strings::en_space_u2002 ());
  }
  
  // Test conversion of boolean to true / false string.
  {
    EXPECT_EQ ("true", filter::strings::convert_to_true_false (true));
    EXPECT_EQ ("false", filter::strings::convert_to_true_false (false));
  }
  
  // Test two explode functions.
  {
    vector <string> result;

    // Explode on single delimiter.
    result = filter::strings::explode ("a b c", ' ');
    EXPECT_EQ ((vector <string>{"a", "b", "c"}), result);

    // Explode on a single space.
    result = filter::strings::explode ("a b c", " ");
    EXPECT_EQ ((vector <string>{"a", "b", "c"}), result);

    // Explode on a set consisting of two spaces.
    result = filter::strings::explode ("a b c", "  ");
    EXPECT_EQ ((vector <string>{"a", "b", "c"}), result);

    // Explode on a semicolon, on a comma, and on a full stop.
    result = filter::strings::explode ("aa.bb,cc;", ";,.");
    EXPECT_EQ ((vector <string>{"aa", "bb", "cc"}), result);

    // Explode on two punctuation marks, leaving one in the output.
    result = filter::strings::explode ("aa.bb,cc;", ";,");
    EXPECT_EQ ((vector <string>{"aa.bb", "cc"}), result);
  }
  
  // Test the array mover function.
  {
    vector <string> container;
    
    container = {};
    filter::strings::array_move_up_down (container, 0, false);
    EXPECT_EQ (vector <string>{}, container);
  }
  
  // Test UTF-16 number of bytes whether 1 or 2.
  {
    u16string u16;
    u16 = filter::strings::convert_to_u16string ("a");
    EXPECT_EQ (1, static_cast<int> (u16.length()));
    u16 = filter::strings::convert_to_u16string ("ℵ");
    EXPECT_EQ (1, static_cast<int> (u16.length()));
    u16 = filter::strings::convert_to_u16string ("😀");
    EXPECT_EQ (2, static_cast<int> (u16.length()));
  }
  
  // Test tidying invalid html.
  {
    string path_invalid = filter_url_create_root_path ({"unittests", "tests", "html-invalid-1.html"});
    string html_invalid = filter_url_file_get_contents(path_invalid);

    string path_valid {};
    string html_valid {};

    string html_tidied = filter::strings::fix_invalid_html_gumbo (html_invalid);
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
    html_tidied = filter::strings::fix_invalid_html_gumbo (html_invalid);
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
    string result = filter::strings::fix_invalid_html_gumbo (string());
    string standard {
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
    string html = R"(<p>Paragraph & < > paragraph</p>)";
    string tidied = filter::strings::fix_invalid_html_gumbo (html);
    string valid =
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
    string standard;
    string result;

    standard = "ABC abc";
    result = filter::strings::collapse_whitespace ("ABC abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::strings::collapse_whitespace ("ABC  abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::strings::collapse_whitespace ("ABC   abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::strings::collapse_whitespace ("ABC      abc");
    EXPECT_EQ (standard, result);

    standard = "ABC abc";
    result = filter::strings::collapse_whitespace ("ABC               abc");
    EXPECT_EQ (standard, result);
  }

  // Test escaping and unescaping special XML characters.
  {
    string standard, result;

    standard = "&quot; &quot;";
    result = filter::strings::escape_special_xml_characters (R"(" ")");
    EXPECT_EQ (standard, result);

    standard = R"(" ")";
    result = filter::strings::unescape_special_xml_characters ("&quot; &quot;");
    EXPECT_EQ (standard, result);
  }
  
  // Test conversion from Windows-1252 encoding to UTF-8 encoding.
  {
    std::string input = "Windows 1252";
    std::string output = filter::strings::convert_windows1252_to_utf8 (input);
    EXPECT_EQ (input, output);
  }
}

#endif


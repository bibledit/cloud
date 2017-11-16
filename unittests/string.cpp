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


#include <unittests/string.h>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>


void test_string ()
{
  trace_unit_tests (__func__);
  
  // Test filter_string_str_replace.
  {
    // Shows that std::string handles UTF-8 well for simple operations.
    evaluate (__LINE__, __func__, "⇊⇦", filter_string_str_replace ("⇖", "", "⇊⇖⇦"));
    // Exercise the replacement counter.
    int counter = 0;
    evaluate (__LINE__, __func__, "a", filter_string_str_replace ("bc", "", "abc", &counter));
    evaluate (__LINE__, __func__, 1, counter);
  }

  // Test array_unique, a C++ equivalent for PHP's array_unique function.
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
    vector <string> output = array_unique (input);
    evaluate (__LINE__, __func__, reference, output);
  }

  // Test array_unique, a C++ equivalent for PHP's array_unique function.
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
    vector <int> output = array_unique (input);
    evaluate (__LINE__, __func__, reference, output);
  }

  // Test filter_string_array_diff, a C++ equivalent for PHP's filter_string_array_diff function.
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
    vector <string> output = filter_string_array_diff (from, against);
    evaluate (__LINE__, __func__, reference, output);
  }

  // Test filter_string_array_diff, a C++ equivalent for PHP's filter_string_array_diff function.
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
    vector <int> output = filter_string_array_diff (from, against);
    evaluate (__LINE__, __func__, reference, output);
  }

  // Test for array_intersect, a C++ equivalent for PHP's array_intersect function.
  {
    vector <int> one;
    vector <int> two;
    one = {1};
    two = {2};
    evaluate (__LINE__, __func__, {}, array_intersect (one, two));
    one = {1, 2, 3};
    two = {2, 3, 4};
    evaluate (__LINE__, __func__, {2, 3}, array_intersect (one, two));
    one = {1, 2, 3, 4, 5};
    two = {2, 3, 4};
    evaluate (__LINE__, __func__, {2, 3, 4}, array_intersect (one, two));
  }
  
  // Test hex2bin and bin2hex as equivalents to PHP's functions.
  {
    string bin = "This is a 123 test.";
    string hex = "5468697320697320612031323320746573742e";
    evaluate (__LINE__, __func__, hex, bin2hex (bin));
    evaluate (__LINE__, __func__, bin, hex2bin (hex));
    bin = "סֶ	א	ב	ױ";
    hex = "d7a1d6b609d79009d79109d7b1";
    evaluate (__LINE__, __func__, hex, bin2hex (bin));
    evaluate (__LINE__, __func__, bin, hex2bin (hex));
    bin.clear ();
    hex.clear ();
    evaluate (__LINE__, __func__, hex, bin2hex (bin));
    evaluate (__LINE__, __func__, bin, hex2bin (hex));
    hex = "a";
    evaluate (__LINE__, __func__, "", bin2hex (bin));
    evaluate (__LINE__, __func__, bin, hex2bin (hex));
  }

  // Test string modifiers.
  {
    evaluate (__LINE__, __func__, "", filter_string_trim ("  "));
    evaluate (__LINE__, __func__, "", filter_string_trim (""));
    evaluate (__LINE__, __func__, "xx", filter_string_trim ("\t\nxx\n\r"));
    evaluate (__LINE__, __func__, "", filter_string_ltrim ("  "));
    evaluate (__LINE__, __func__, "", filter_string_ltrim (""));
    evaluate (__LINE__, __func__, "xx\n\r", filter_string_ltrim ("xx\n\r"));
    evaluate (__LINE__, __func__, "xx  ", filter_string_ltrim ("  xx  "));
    evaluate (__LINE__, __func__, "0000012345", filter_string_fill ("12345", 10, '0'));
  }
  
  // Numeric tests.
  {
    evaluate (__LINE__, __func__, true, filter_string_is_numeric ("1"));
    evaluate (__LINE__, __func__, true, filter_string_is_numeric ("1234"));
    evaluate (__LINE__, __func__, false, filter_string_is_numeric ("X"));
    evaluate (__LINE__, __func__, false, filter_string_is_numeric ("120X"));
  }
  
  // String conversion to int.
  {
    evaluate (__LINE__, __func__, 0, convert_to_int (""));
    evaluate (__LINE__, __func__, 123, convert_to_int ("123"));
    evaluate (__LINE__, __func__, 123, convert_to_int ("123xx"));
    evaluate (__LINE__, __func__, 0, convert_to_int ("xxx123xx"));
  }
  
  // Unicode validity test.
  {
    evaluate (__LINE__, __func__, true, unicode_string_is_valid ("valid"));
    evaluate (__LINE__, __func__, true, unicode_string_is_valid ("בְּרֵאשִׁית, בָּרָא אֱלֹהִים, אֵת הַשָּׁמַיִם, וְאֵת הָאָרֶץ"));
  }
  
  // Searching in array.
  {
    vector <string> haystack = {"needle"};
    string needle = "needle";
    evaluate (__LINE__, __func__, true, in_array (needle, haystack));
    needle.clear ();
    evaluate (__LINE__, __func__, false, in_array (needle, haystack));
    haystack.clear ();
    evaluate (__LINE__, __func__, false, in_array (needle, haystack));
    needle = "needle";
    evaluate (__LINE__, __func__, false, in_array (needle, haystack));
    evaluate (__LINE__, __func__, true, in_array (1, {1, 2, 3}));
    evaluate (__LINE__, __func__, false, in_array (1, {2, 3}));
  }

  // Test random number generator.
  {
    int floor = 100000;
    int ceiling = 999999;
    int r1 = filter_string_rand (floor, ceiling);
    if ((r1 < floor) || (r1 > ceiling)) evaluate (__LINE__, __func__, "Random generator out of bounds", convert_to_string (r1));
    int r2 = filter_string_rand (floor, ceiling);
    if ((r2 < floor) || (r2 > ceiling)) evaluate (__LINE__, __func__, "Random generator out of bounds", convert_to_string (r2));
    if (r1 == r2) evaluate (__LINE__, __func__, "Random generator should generate different values", convert_to_string (r1) + " " + convert_to_string (r2));
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
    evaluate (__LINE__, __func__, plain, filter_string_html2text (html));
  }
  {
    string html =
    "<p>writer (15 Nov):</p>\n"
    "<p>Second note.<div>Second matter.</div><div>A second round is needed.</div></p>\n"
    "<p>Here is <b>bold</b>, and here is <i>italics</i>.<div>Here is <sup>superscript</sup>&nbsp;and here is <sub>subscript</sub>.</div><div style=\"text-align: center;\">Centered</div><div style=\"text-align: left;\">Left justified</div><div style=\"text-align: left;\"><ol><li>Numbered list.</li></ol>No numbered text.</div><div style=\"text-align: left;\">Link to <a href=\"http://google.nl\">http://google.nl</a>.</div><div style=\"text-align: left;\">Here follows an image:&nbsp;<img src=\"http://localhost/image\">.<br></div><h1>Header 1</h1><div>Normal text again below the header.</div></p>\n";
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
    evaluate (__LINE__, __func__, filter_string_trim (plain), filter_string_trim (filter_string_html2text (html)));
  }
  {
    string html =
    "test notes four\n"
    "\n"
    "Logbook:\n"
    "\n";
    string plain =
    "test notes fourLogbook:";
    evaluate (__LINE__, __func__, filter_string_trim (plain), filter_string_trim (filter_string_html2text (html)));
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
    evaluate (__LINE__, __func__, filter_string_trim (plain), filter_string_trim (filter_string_html2text (html)));
  }

  // Email address extraction.
  {
    evaluate (__LINE__, __func__, "foo@bar.eu", filter_string_extract_email ("Foo Bar <foo@bar.eu>"));
    evaluate (__LINE__, __func__, "foo@bar.eu", filter_string_extract_email ("<foo@bar.eu>"));
    evaluate (__LINE__, __func__, "foo@bar.eu", filter_string_extract_email ("foo@bar.eu"));
    
    string body = "body";
    evaluate (__LINE__, __func__, body, filter_string_extract_body (body));
    
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
    evaluate (__LINE__, __func__, "test", filter_string_extract_body (body, "2011", "Bibledit"));
  }
  
  // Substring replacement.
  {
    evaluate (__LINE__, __func__, "ABXEFG", substr_replace ("ABCDEFG", "X", 2, 2));
    evaluate (__LINE__, __func__, "ABX", substr_replace ("ABCDEFG", "X", 2, 5));
    evaluate (__LINE__, __func__, "ABXG", substr_replace ("ABCDEFG", "X", 2, 4));
  }

  // Word markup.
  {
    string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    vector <string> words = { "makumbo", "akasia", "matanda" };
    string result = filter_string_markup_words (words, text);
    string standard =
    "Zvino uchagadzira <mark>makumbo</mark> uye <mark>Makumbo</mark> uye <mark>maKumbo</mark> uye <mark>MAKUMBO</mark> emu<mark>akasia</mark>*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa <mark>makumbo</mark> muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dze<mark>makumbo</mark> kutakura tafura.\n"
    "Zvino uchaita <mark>makumbo</mark> ne<mark>matanda</mark> ne<mark>Matanda</mark> ne<mark>maTANDA</mark> emu<mark>Akasia</mark>, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    evaluate (__LINE__, __func__, standard, result);
  }

  // Word markup.
  {
    string text =
    "Zvino uchagadzira makumbo uye Makumbo uye maKumbo uye MAKUMBO emuakasia*, ndokuaputira negoridhe.\n"
    "Zvino uchaisa makumbo muzvindori panhivi dzeareka, kuti areka itakurwe nawo.\n"
    "Zvindori zvichava pamupendero kuti zvive nzvimbo dzemakumbo kutakura tafura.\n"
    "Zvino uchaita makumbo nematanda neMatanda nemaTANDA emuAkasia, ugoiputira negoridhe, kuti tafura itakurwe nawo.\n";
    vector <string> words;
    string result = filter_string_markup_words (words, text);
    evaluate (__LINE__, __func__, text, result);
  }

  // Test tidying html.
  {
    string folder = filter_url_create_root_path ("unittests", "tests");
    string html = filter_url_file_get_contents (filter_url_create_path (folder, "/biblehub-john-1-1.html"));
    vector <string> tidy = filter_string_explode (html_tidy (html), '\n');
    evaluate (__LINE__, __func__, 747, (int)tidy.size());
  }
  
  {
    string input = "<span>Praise the LORD&#xB6;, all &amp; you nations</span>";
    string output = convert_xml_character_entities_to_characters (input);
    string standard = filter_string_str_replace ("&#xB6;", "¶", input);
    evaluate (__LINE__, __func__, standard, output);
    
    input = "<span>Praise the LORD &#x5D0; all you nations</span>";
    output = convert_xml_character_entities_to_characters (input);
    standard = filter_string_str_replace ("&#x5D0;", "א", input);
    evaluate (__LINE__, __func__, standard, output);
    
    input = "Username";
    output = encrypt_decrypt ("key", input);
    output = encrypt_decrypt ("key", output);
    evaluate (__LINE__, __func__, input, output);
    
    input = "בְּרֵאשִׁ֖ית בָּרָ֣א אֱלֹהִ֑ים אֵ֥ת הַשָּׁמַ֖יִם וְאֵ֥ת הָאָֽרֶץ";
    output = encrypt_decrypt ("בְּרֵאשִׁ֖ית", input);
    output = encrypt_decrypt ("בְּרֵאשִׁ֖ית", output);
    evaluate (__LINE__, __func__, input, output);
  }
  
  {
    string one = get_new_random_string ();
    this_thread::sleep_for (chrono::milliseconds (10));
    string two = get_new_random_string ();
    evaluate (__LINE__, __func__, 32, one.length ());
    evaluate (__LINE__, __func__, true, one != two);
  }
  
  {
    evaluate (__LINE__, __func__, 4, (int)unicode_string_length ("test"));
    evaluate (__LINE__, __func__, 4, (int)unicode_string_length ("ᨁᨃᨅᨕ"));
  }
  
  {
    string hebrew = "אָבּגּדּהּ";
    evaluate (__LINE__, __func__, "st1234", unicode_string_substr ("test1234", 2));
    evaluate (__LINE__, __func__, "גּדּהּ", unicode_string_substr (hebrew, 2));
    evaluate (__LINE__, __func__, "", unicode_string_substr (hebrew, 5));
    evaluate (__LINE__, __func__, "", unicode_string_substr (hebrew, 6));
    evaluate (__LINE__, __func__, "test", unicode_string_substr ("test123456", 0, 4));
    evaluate (__LINE__, __func__, "12", unicode_string_substr ("test123456", 4, 2));
    evaluate (__LINE__, __func__, "גּדּ", unicode_string_substr (hebrew, 2, 2));
    evaluate (__LINE__, __func__, "גּדּהּ", unicode_string_substr (hebrew, 2, 10));
  }
  
  {
    string hebrew = "אָבּגּדּהּ";
    string needle = "דּ";
    evaluate (__LINE__, __func__, 3, (int)unicode_string_strpos ("012345", "3"));
    evaluate (__LINE__, __func__, 5, (int)unicode_string_strpos ("012345", "5"));
    evaluate (__LINE__, __func__, 0, (int)unicode_string_strpos ("012345", "0"));
    evaluate (__LINE__, __func__, -1, (int)unicode_string_strpos ("012345", "6"));
    evaluate (__LINE__, __func__, 3, (int)unicode_string_strpos (hebrew, needle));
    evaluate (__LINE__, __func__, 3, (int)unicode_string_strpos (hebrew, needle, 3));
    evaluate (__LINE__, __func__, -1, (int)unicode_string_strpos (hebrew, needle, 4));
    evaluate (__LINE__, __func__, -1, (int)unicode_string_strpos ("", "3"));
  }
  
  {
    evaluate (__LINE__, __func__, 2, (int)unicode_string_strpos_case_insensitive ("AbCdEf", "c"));
    evaluate (__LINE__, __func__, 2, (int)unicode_string_strpos_case_insensitive ("AbCdEf", "cD"));
    evaluate (__LINE__, __func__, -1, (int)unicode_string_strpos_case_insensitive ("AbCdEf", "ce"));
  }
  
  {
    evaluate (__LINE__, __func__, "test1234", unicode_string_casefold ("test1234"));
    evaluate (__LINE__, __func__, "test1234", unicode_string_casefold ("TEST1234"));
    evaluate (__LINE__, __func__, "θεος", unicode_string_casefold ("Θεος"));
    evaluate (__LINE__, __func__, "α α β β", unicode_string_casefold ("Α α Β β"));
    evaluate (__LINE__, __func__, "אָבּגּדּהּ", unicode_string_casefold ("אָבּגּדּהּ"));
  }
  
  {
    evaluate (__LINE__, __func__, "TEST1234", unicode_string_uppercase ("test1234"));
    evaluate (__LINE__, __func__, "TEST1234", unicode_string_uppercase ("TEST1234"));
    evaluate (__LINE__, __func__, "ΘΕΟΣ", unicode_string_uppercase ("Θεος"));
    evaluate (__LINE__, __func__, "Α Α Β Β", unicode_string_uppercase ("Α α Β β"));
    evaluate (__LINE__, __func__, "אָבּגּדּהּ", unicode_string_uppercase ("אָבּגּדּהּ"));
  }
  
  {
    evaluate (__LINE__, __func__, "ABCDEFG", unicode_string_transliterate ("ABCDEFG"));
    evaluate (__LINE__, __func__, "Ιησου Χριστου", unicode_string_transliterate ("Ἰησοῦ Χριστοῦ"));
    evaluate (__LINE__, __func__, "אבגדה", unicode_string_transliterate ("אָבּגּדּהּ"));
  }
  
  {
    vector <string> needles;
    needles = filter_string_search_needles ("ABC", "one abc two ABc three aBc four");
    evaluate (__LINE__, __func__, { "abc", "ABc", "aBc" }, needles);
    needles = filter_string_search_needles ("abc", "one abc two ABc three aBc four");
    evaluate (__LINE__, __func__, { "abc", "ABc", "aBc" }, needles);
  }
  
  {
    evaluate (__LINE__, __func__, false, unicode_string_is_punctuation ("A"));
    evaluate (__LINE__, __func__, false, unicode_string_is_punctuation ("z"));
    evaluate (__LINE__, __func__, true, unicode_string_is_punctuation ("."));
    evaluate (__LINE__, __func__, true, unicode_string_is_punctuation (","));
  }
  
  {
    evaluate (__LINE__, __func__, false, convert_to_bool ("0"));
    evaluate (__LINE__, __func__, false, convert_to_bool ("false"));
    evaluate (__LINE__, __func__, false, convert_to_bool ("FALSE"));
    evaluate (__LINE__, __func__, true, convert_to_bool ("1"));
    evaluate (__LINE__, __func__, true, convert_to_bool ("true"));
    evaluate (__LINE__, __func__, true, convert_to_bool ("TRUE"));
  }
  
  {
    evaluate (__LINE__, __func__, 21109, unicode_string_convert_to_codepoint ("創"));
    evaluate (__LINE__, __func__, 97, unicode_string_convert_to_codepoint ("a"));
  }
  
  {
    // Check that the function to desanitize html no longer corrupts UTF-8.
    string html = "<p>“Behold”, from “הִנֵּה”, means look at</p>";
    string desanitized = filter_string_desanitize_html (html);
    evaluate (__LINE__, __func__, html, desanitized);
  }
  
  // Test whitespace characters, breaking and non-breaking.
  {
    // The "­" below is not an empty string, but the soft hyphen U+00AD.
    string standard_soft_hyphen = "­";
    evaluate (__LINE__, __func__, standard_soft_hyphen, soft_hyphen_u00AD ());
    evaluate (__LINE__, __func__, "\u00AD", soft_hyphen_u00AD ());

    evaluate (__LINE__, __func__, "\u00A0", non_breaking_space_u00A0 ());

    // The space below is "en space", U+2002.
    string standard_en_space_u2002 = " ";
    evaluate (__LINE__, __func__, standard_en_space_u2002, en_space_u2002 ());
    evaluate (__LINE__, __func__, "\u2002", en_space_u2002 ());
  }

}

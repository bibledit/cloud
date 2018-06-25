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


#include <filter/string.h>
#include <utf8/utf8.h>
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <utf8proc/utf8proc.h>
#include <config/globals.h>
#ifdef HAVE_WINDOWS
#include <codecvt>
#endif


// A C++ equivalent for PHP's explode function.
// Split a string on a delimiter.
// Return a vector of strings.
vector <string> filter_string_explode (string value, char delimiter)
{
  vector <string> result;
  istringstream iss (value);
  for (string token; getline (iss, token, delimiter); )
  {
    result.push_back (move (token));
  }
  return result;
}


// A C++ equivalent for PHP's implode function.
// Join a vector of string, with delimiters, into a string.
// Return this string.
string filter_string_implode (vector <string>& values, string delimiter)
{
  string full;
  for (vector<string>::iterator it = values.begin (); it != values.end (); ++it)
  {
    full += (*it);
    if (it != values.end ()-1) full += delimiter;
  }
  return full;  
}


// A C++ rough equivalent for PHP's filter_string_str_replace function.
string filter_string_str_replace (string search, string replace, string subject, int * count)
{
  size_t offposition = subject.find (search);
  while (offposition != string::npos) {
    subject.replace (offposition, search.length (), replace);
    if (count) (*count)++;
    offposition = subject.find (search, offposition + replace.length ());
  }
  return subject;
}


// Replaces text that starts with "start" and ends with "end" with "replacement".
// Returns true if replacement was done.
bool filter_string_replace_between (string& line, const string& start, const string& end, const string& replacement)
{
  bool replacements_done = false;
  size_t beginpos = line.find (start);
  size_t endpos = line.find (end);
  while ((beginpos != string::npos) && (endpos != string::npos) && (endpos > beginpos)) {
    line.replace (beginpos, endpos - beginpos + end.length (), replacement);
    beginpos = line.find (start, beginpos + replacement.length ());
    endpos = line.find (end, beginpos + replacement.length ());
    replacements_done = true;
  }
  return replacements_done;
}


// It replaces a copy of string delimited by the start and length parameters with the string given in replacement.
// It is similar to PHP's function with the same name.
string substr_replace (string original, string replacement, size_t start, size_t length)
{
  if (length) original.erase (start, length);
  original.insert (start, replacement);
  return original;
}



// On some platforms the sizeof (unsigned int) is equal to the sizeof (size_t).
// Then compilation would fail if there were two functions "convert_to_string",
// one taking the unsigned int, and the other taking the size_t.
// Therefore there is now one function doing both.
// This may lead to embiguity errors for the C++ compiler.
// In such case the ambiguity can be removed by changing the type to be passed
// to this function to "size_t".
string convert_to_string (size_t i)
{
  ostringstream r;
  r << i;
  return r.str();
}


string convert_to_string (int i)
{
  ostringstream r;
  r << i;
  return r.str();
}


string convert_to_string (char * c)
{
  string s = c;
  return s;
}


string convert_to_string (const char * c)
{
  string s = c;
  return s;
}


string convert_to_string (bool b)
{
  if (b) return "1";
  return "0";
}


string convert_to_string (string s)
{
  return s;
}


string convert_to_string (float f)
{
  ostringstream r;
  r << f;
  return r.str();
}


int convert_to_int (string s)
{
  int i = atoi (s.c_str());
  return i;
}


int convert_to_int (float f)
{
  int i = (int)round(f);
  return i;
}


long long convert_to_long_long (string s)
{
  long long i = 0;
  istringstream r (s);
  r >> i;
  return i;
}


float convert_to_float (string s)
{
  float f = 0;
  istringstream r (s);
  r >> f;
  return f;
}


bool convert_to_bool (string s)
{
  if (s == "true") return true;
  if (s == "TRUE") return true;
  bool b;
  istringstream (s) >> b;
  return b;
}


// A C++ equivalent for PHP's array_unique function.
vector <string> array_unique (vector <string> values)
{
  vector <string> result;
  set <string> unique;
  for (unsigned int i = 0; i < values.size (); i++) {
    if (unique.find (values[i]) == unique.end ()) {
      unique.insert (values[i]);
      result.push_back ((values[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_unique function.
vector <int> array_unique (vector <int> values)
{
  vector <int> result;
  set <int> unique;
  for (unsigned int i = 0; i < values.size (); i++) {
    if (unique.find (values[i]) == unique.end ()) {
      unique.insert (values[i]);
      result.push_back ((values[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_diff function.
// Returns items in "from" which are not present in "against".
vector <string> filter_string_array_diff (vector <string> from, vector <string> against)
{
  vector <string> result;
  set <string> against2 (against.begin (), against.end ());
  for (unsigned int i = 0; i < from.size (); i++) {
    if (against2.find (from[i]) == against2.end ()) {
      result.push_back ((from[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's array_diff function.
// Returns items in "from" which are not present in "against".
vector <int> filter_string_array_diff (vector <int> from, vector <int> against)
{
  vector <int> result;
  set <int> against2 (against.begin (), against.end ());
  for (unsigned int i = 0; i < from.size (); i++) {
    if (against2.find (from[i]) == against2.end ()) {
      result.push_back ((from[i]));
    }
  }
  return result;
}


// A C++ equivalent for PHP's filter_string_trim function.
string filter_string_trim (string s)
{
  if (s.length () == 0)
    return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t beg = s.find_first_not_of(" \t\n\r");
  size_t end = s.find_last_not_of(" \t\n\r");
  // No non-spaces  
  if (beg == string::npos)
    return "";
  return string (s, beg, end - beg + 1);
}


// A C++ equivalent for PHP's filter_string_ltrim function.
string filter_string_ltrim (string s)
{
  if (s.length () == 0) return s;
  // Strip spaces, tabs, new lines, carriage returns.
  size_t pos = s.find_first_not_of(" \t\n\r");
  // No non-spaces  
  if (pos == string::npos) return "";
  return s.substr (pos);
}


// Fills a string up to "width", with the character "fill" at the left.
string filter_string_fill (string s, int width, char fill)
{
  ostringstream str;
  str << setfill (fill) << setw (width) << s;
  return str.str();
}


// Returns true/false whether s is numeric.
bool filter_string_is_numeric (string s)
{
  for (char c : s) if (!isdigit (c)) return false;
  return true;
}


// This takes the five special XML characters and escapes them.
// < : &lt;
// & : &amp;
// > : &gt;
// " : &quot;
// ' : &apos;
string escape_special_xml_characters (string s)
{
  s = filter_string_str_replace ("&", "&amp;", s);
  s = filter_string_str_replace ("\"", "&quot;", s);
  s = filter_string_str_replace ("'", "&apos;", s);
  s = filter_string_str_replace ("<", "&lt;", s);
  s = filter_string_str_replace (">", "&gt;", s);
  return s;
}


// This unescapes the five special XML characters.
string unescape_special_xml_characters (string s)
{
  s = filter_string_str_replace ("&quot;", """", s);
  s = filter_string_str_replace ("&amp;", "&", s);
  s = filter_string_str_replace ("&apos;", "'", s);
  s = filter_string_str_replace ("&lt;", "<", s);
  s = filter_string_str_replace ("&gt;", ">", s);
  return s;
}


// Converts other types of spaces to standard spaces.
string any_space_to_standard_space (string s)
{
  s = filter_string_str_replace (unicode_non_breaking_space_entity (), " ", s);
  s = filter_string_str_replace (non_breaking_space_u00A0 (), " ", s);
  return s;
}


// Returns a no-break space (NBSP) (x00A0).
string non_breaking_space_u00A0 ()
{
  // Use C-style code.
  return "\u00A0";
}


// Returns a soft hyphen.
string soft_hyphen_u00AD ()
{
  // Soft hyphen U+00AD.
  return "\u00AD";
}


// Returns an "en space", this is a nut, half an em space.
string en_space_u2002 ()
{
  // U+2002.
  return "\u2002";
}


// A "figure space".
// A space to use in numbers.
// It has the same width as the digits.
// It does not break, it keeps the numbers together.
string figure_space_u2007 ()
{
  return "\u2007";
}


// Returns a "narrow no-break space (x202F)
string narrow_non_breaking_space_u202F ()
{
  return "\u202F";
}

// Returns the length of string s in unicode points, not in bytes.
size_t unicode_string_length (string s)
{
  int length = utf8::distance (s.begin(), s.end());
  return length;
}


// Get the substring with unicode point pos(ition) and len(gth).
// If len = 0, the string from start till end is returned.
string unicode_string_substr (string s, size_t pos, size_t len)
{
  char * input = (char *) s.c_str();
  char * startiter = (char *) input;
  size_t length = strlen (input);
  char * veryend = input + length + 1;
  // Iterate forward pos times.
  while (pos > 0) {
    if (strlen (startiter)) {
      utf8::next (startiter, veryend);
    } else {
      // End reached: Return empty result.
      return "";
    }
    pos--;
  }
  // Zero len: Return result till the end of the string.
  if (len == 0) {
    s.assign (startiter);
    return s;
  }

  // Iterate forward len times.
  char * enditer = startiter;
  while (len > 0) {
    if (strlen (enditer)) {
      utf8::next (enditer, veryend);
    } else {
      // End reached: Return result.
      s.assign (startiter);
      return s;
    }
    len--;
  }
  // Return substring.
  size_t startpos = startiter - input;
  size_t lenpos = enditer - startiter;
  s = s.substr (startpos, lenpos);
  return s;
}


// Equivalent to PHP's mb_strpos function.
size_t unicode_string_strpos (string haystack, string needle, size_t offset)
{
  int haystack_length = unicode_string_length (haystack);
  int needle_length = unicode_string_length (needle);
  for (int pos = offset; pos <= haystack_length - needle_length; pos++) {
    string substring = unicode_string_substr (haystack, pos, needle_length);
    if (substring == needle) return pos;
  }
  return string::npos;
}


// Case-insensitive version of "unicode_string_strpos".
size_t unicode_string_strpos_case_insensitive (string haystack, string needle, size_t offset)
{
  haystack = unicode_string_casefold (haystack);
  needle = unicode_string_casefold (needle);
  
  int haystack_length = unicode_string_length (haystack);
  int needle_length = unicode_string_length (needle);
  for (int pos = offset; pos <= haystack_length - needle_length; pos++) {
    string substring = unicode_string_substr (haystack, pos, needle_length);
    if (substring == needle) return pos;
  }
  return string::npos;
}


// Converts string $s to lowercase.
string unicode_string_casefold (string s)
{
  string casefolded;
  try {
    // The UTF8 processor works with one Unicode point at a time.
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      // Get one UTF-8 character.
      string character = unicode_string_substr (s, pos, 1);
      // Convert it to a Unicode point.
      const utf8proc_uint8_t *str = (const unsigned char *) (character.c_str ());
      utf8proc_ssize_t len = character.length ();
      utf8proc_int32_t dst;
      utf8proc_ssize_t output = utf8proc_iterate (str, len, &dst);
      // Convert the Unicode point to lower case.
      utf8proc_int32_t luc = utf8proc_tolower (dst);
      // Convert the Unicode point back to a UTF-8 string.
      utf8proc_uint8_t buffer [10];
      output = utf8proc_encode_char (luc, buffer);
      buffer [output] = 0;
      stringstream ss;
      ss << buffer;
      // Add the casefolded UTF-8 character to the result.
      casefolded.append (ss.str ());
    }
  } catch (...) {
  }
  // Done.
  return casefolded;
/*
 The code below shows how to do it through the ICU library.
 But the ICU library could not be compiled properly for Android.
 Therefore it is not used throughout all platforms.

  // UTF-8 string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  // Case folding.
  source.foldCase ();
  // UTF-16 UnicodeString -> UTF-8 std::string
  string result;
  source.toUTF8String (result);
  // Ready.
  return result;
*/
}


string unicode_string_uppercase (string s)
{
  string uppercase;
  try {
    // The UTF8 processor works with one Unicode point at a time.
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      // Get one UTF-8 character.
      string character = unicode_string_substr (s, pos, 1);
      // Convert it to a Unicode point.
      const utf8proc_uint8_t *str = (const unsigned char *) (character.c_str ());
      utf8proc_ssize_t len = character.length ();
      utf8proc_int32_t dst;
      utf8proc_ssize_t output = utf8proc_iterate (str, len, &dst);
      // Convert the Unicode point to lower case.
      utf8proc_int32_t luc = utf8proc_toupper (dst);
      // Convert the Unicode point back to a UTF-8 string.
      utf8proc_uint8_t buffer [10];
      output = utf8proc_encode_char (luc, buffer);
      buffer [output] = 0;
      stringstream ss;
      ss << buffer;
      // Add the casefolded UTF-8 character to the result.
      uppercase.append (ss.str ());
    }
  } catch (...) {
  }
  // Done.
  return uppercase;
/*
 How to do the above through the ICU library.
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  source.toUpper ();
  string result;
  source.toUTF8String (result);
  return result;
*/
}


string unicode_string_transliterate (string s)
{
  string transliteration;
  try {
    size_t string_length = unicode_string_length (s);
    for (unsigned int pos = 0; pos < string_length; pos++) {
      string character = unicode_string_substr (s, pos, 1);
      const utf8proc_uint8_t *str = (const unsigned char *) (character.c_str ());
      utf8proc_ssize_t len = character.length ();
      uint8_t *dest;
      utf8proc_option_t options = (utf8proc_option_t) (UTF8PROC_DECOMPOSE | UTF8PROC_STRIPMARK);
      utf8proc_ssize_t output = utf8proc_map (str, len, &dest, options);
      (void) output;
      stringstream ss;
      ss << dest;
      transliteration.append (ss.str ());
      free (dest);
    }
  } catch (...) {
  }
  return transliteration;
/*
 Code showing how to do the transliteration through the ICU library.
  // UTF-8 string -> UTF-16 UnicodeString
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  
  // Transliterate UTF-16 UnicodeString following this rule:
  // decompose, remove diacritics, recompose
  UErrorCode status = U_ZERO_ERROR;
  Transliterator *transliterator = Transliterator::createInstance("NFD; [:M:] Remove; NFC",
                                                                  UTRANS_FORWARD,
                                                                  status);
  transliterator->transliterate(source);
  
  // UTF-16 UnicodeString -> UTF-8 std::string
  string result;
  source.toUTF8String (result);
  
  // Done.
  return result;
 */
}


// Returns true if string "s" is valid UTF8 encoded.
bool unicode_string_is_valid (string s)
{
  return utf8::is_valid (s.begin(), s.end());
}


// Returns whether $s is Unicode punctuation.
bool unicode_string_is_punctuation (string s)
{
  try {
    if (s.empty ()) return false;
    // Be sure to take only one character.
    s = unicode_string_substr (s, 0, 1);
    // Convert the string to a Unicode point.
    const utf8proc_uint8_t *str = (const unsigned char *) (s.c_str ());
    utf8proc_ssize_t len = s.length ();
    utf8proc_int32_t codepoint;
    utf8proc_ssize_t output = utf8proc_iterate (str, len, &codepoint);
    (void) output;
    // Get category.
    utf8proc_category_t category = utf8proc_category	(codepoint);
    if ((category >= UTF8PROC_CATEGORY_PC) && (category <= UTF8PROC_CATEGORY_PO)) return true;
  } catch (...) {
  }
  return false;
  /* 
  The following code shows how to do the above code through the ICU library.
  UnicodeString source = UnicodeString::fromUTF8 (StringPiece (s));
  StringCharacterIterator iter (source);
  UChar32 character = iter.first32 ();
  bool punctuation = u_ispunct (character);
  return punctuation;
  */
}


// Converts the string $s to a Unicode codepoint.
int unicode_string_convert_to_codepoint (string s)
{
  int point = 0;
  if (!s.empty ()) {
    try {
      // Be sure to take only one character.
      s = unicode_string_substr (s, 0, 1);
      // Convert the string to a Unicode point.
      const utf8proc_uint8_t *str = (const unsigned char *) (s.c_str ());
      utf8proc_ssize_t len = s.length ();
      utf8proc_int32_t codepoint;
      utf8proc_ssize_t output = utf8proc_iterate (str, len, &codepoint);
      (void) output;
      point = codepoint;
    } catch (...) {
    }
    
  }
  return point;
}


// C++ equivalent for PHP's rand function
int filter_string_rand (int floor, int ceiling)
{
  int range = ceiling - floor;
  int r = rand () % range + floor;
  return r;
}


string filter_string_html2text (string html)
{
  // Clean the html up.
  html = filter_string_str_replace ("\n", "", html);

  // The output text.
  string text;

  // Keep going while the html contains the < character.
  size_t pos = html.find ("<");
  while (pos != string::npos) {
    // Add the text before the <.
    text.append (html.substr (0, pos));
    html = html.substr (pos + 1);
    // Certain tags start new lines.
    string tag1 = unicode_string_casefold (html.substr (0, 1));
    string tag2 = unicode_string_casefold (html.substr (0, 2));
    string tag3 = unicode_string_casefold (html.substr (0, 3));
    if  ((tag1 == "p")
      || (tag3 == "div")
      || (tag2 == "li")
      || (tag3 == "/ol")
      || (tag3 == "/ul")
      || (tag2 == "h1")
      || (tag2 == "h2")
      || (tag2 == "h3")
      || (tag2 == "h4")
      || (tag2 == "h5")
      || (tag2 == "br")
       ) {
      text.append ("\n");
    }
    // Clear text out till the > character.
    pos = html.find (">");
    if (pos != string::npos) {
      html = html.substr (pos + 1);
    }
    // Next iteration.
    pos = html.find ("<");
  }
  // Add any remaining bit of text.
  text.append (html);

  // Replace xml entities with their text.
  text = unescape_special_xml_characters (text);

  while (text.find ("\n\n") != string::npos) {
    text = filter_string_str_replace ("\n\n", "\n", text);
  }
  text = filter_string_trim (text);
  return text;
}


// Extracts the pure email address from a string.
// input: Foo Bar <foo@bar.nl>
// input: foo@bar.nl
// Returns: foo@bar.nl
// If there is no valid email, it returns false.
string filter_string_extract_email (string input)
{
  size_t pos = input.find ("<");
  if (pos != string::npos) {
    input = input.substr (pos + 1);
  }
  pos = input.find (">");
  if (pos != string::npos) {
    input = input.substr (0, pos);
  }
  string email = input;
  if (!filter_url_email_is_valid (email)) email.clear();
  return email;
}


// Extracts a clean string from the email body given in input.
// It leaves out the bit that was quoted.
// If year and sender are given, it also removes lines that contain both strings.
// This is used to remove lines like:
// On Wed, 2011-03-02 at 08:26 +0100, Bibledit wrote:
string filter_string_extract_body (string input, string year, string sender)
{
  vector <string> inputlines = filter_string_explode (input, '\n');
  if (inputlines.empty ()) return "";
  vector <string> body;
  for (string & line : inputlines) {
    string trimmed = filter_string_trim (line);
    if (trimmed == "") continue;
    if (trimmed.find (">") == 0) continue;
    if ((year != "") && (sender != "")) {
      if (trimmed.find (year) != string::npos) {
        if (trimmed.find (sender) != string::npos) {
          continue;
        }
      }
    }
    body.push_back (line);
  }
  string bodystring = filter_string_implode (body, "\n");
  bodystring = filter_string_trim (bodystring);
  return bodystring;
}


// Returns an appropriate value.
string get_checkbox_status (bool enabled)
{
  if (enabled) return "checked";
  return "";
}


string get_disabled (bool disabled)
{
  if (disabled) return "disabled";
  return "";
}


void quick_swap(string & a, string & b)
{
  string t = a;
  a = b;
  b = t;
}


void quick_swap(unsigned int &a, unsigned int &b)
{
  unsigned int t = a;
  a = b;
  b = t;
}


void quick_swap(long unsigned int &a, long unsigned int &b)
{
  long unsigned int t = a;
  a = b;
  b = t;
}


void quick_swap(int &a, int &b)
{
  int t = a;
  a = b;
  b = t;
}


void quick_swap(bool & a, bool & b)
{
  bool t = a;
  a = b;
  b = t;
}


// This function is unusual in the sense that it does not sort one container,
// as the majority of sort functions do, but it accepts two containers.
// It sorts on the first, and reorders the second container at the same time,
// following the reordering done in the first container.
void quick_sort (vector <unsigned int>& one, vector <string> &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < unsigned int >&one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort (vector<unsigned int>& one, vector<bool>& two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        bool two_l = two[l];
        bool two_r = two[r];
        quick_swap(two_l, two_r);
        two[l] = two_l;
        two[r] = two_r;
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    bool two_l = two[l];
    bool two_beg = two[beg];
    quick_swap(two_l, two_beg);
    two[l] = two_l;
    two[beg] = two_beg;
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < int >&one, vector < unsigned int >&two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}

void quick_sort(vector < string > &one, vector < string > &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, vector < bool > &two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        bool two_l = two[l];
        bool two_r = two[r];
        quick_swap(two_l, two_r);
        two[l] = two_l;
        two[r] = two_r;
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    bool two_l = two[l];
    bool two_beg = two[beg];
    quick_swap(two_l, two_beg);
    two[l] = two_l;
    two[beg] = two_beg;
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort(vector < string > &one, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    string piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_sort(one, beg, l);
    quick_sort(one, r, end);
  }
}


void quick_sort(vector <long unsigned int>& one, vector <long unsigned int>& two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    long unsigned int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


void quick_sort (vector <int> & one, vector <int> & two, unsigned int beg, unsigned int end)
{
  if (end > beg + 1) {
    int piv = one[beg];
    unsigned int l = beg + 1;
    unsigned int r = end;
    while (l < r) {
      if (one[l] <= piv) {
        l++;
      } else {
        --r;
        quick_swap(one[l], one[r]);
        quick_swap(two[l], two[r]);
      }
    }
    --l;
    quick_swap(one[l], one[beg]);
    quick_swap(two[l], two[beg]);
    quick_sort(one, two, beg, l);
    quick_sort(one, two, r, end);
  }
}


#define MY_NUMBERS "0123456789"
string number_in_string (const string & str)
{
  // Looks for and returns a positive number in a string.
  string output = str;
  output.erase (0, output.find_first_of (MY_NUMBERS));
  size_t end_position = output.find_first_not_of (MY_NUMBERS);
  if (end_position != string::npos) {
    output.erase (end_position, output.length());
  }
  return output;
}
#undef MY_NUMBERS



// This function marks the array of $words in the string $text.
// It uses the <mark> markup for display as html.
string filter_string_markup_words (const vector <string>& words, string text)
{
  // Array of needles to look for.
  // The needles contain the search $words as they occur in the $text
  // in upper case or lower case, or any mixed case.
  vector <string> needles;
  for (auto & word : words) {
    if (word == "") continue;
    vector <string> new_needles = filter_string_search_needles (word, text);
    needles.insert (needles.end(), new_needles.begin(), new_needles.end());
  }
  needles = array_unique (needles);
  
  // All the $needles are converted to $markup,
  // which will replace the $needles.
  for (auto & needle : needles) {
    string markup = "<mark>" + needle + "</mark>";
    text = filter_string_str_replace (needle, markup, text);
  }
  
  // Result.
  return text;
}


// This function returns an array of needles to look for.
// The needles contain the $search word as it occurs in the $string
// in upper case or lower case or any mixed case.
vector <string> filter_string_search_needles (string search, string text)
{
  vector <string> needles;
  size_t position = unicode_string_strpos_case_insensitive (text, search, 0);
  while (position != string::npos) {
    string needle = unicode_string_substr (text, position, unicode_string_length (search));
    needles.push_back (needle);
    position = unicode_string_strpos_case_insensitive (text, search, position + 1);
  }
  needles = array_unique (needles);
  return needles;
}


// Returns an integer identifier based on the name of the current user.
int filter_string_user_identifier (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string username = request->session_logic()->currentUser ();
  string hash = md5 (username).substr (0, 5);
  int identifier = my_stoi (hash, NULL, 36);
  return identifier;
}


// C++ equivalent for PHP's bin2hex function.
string bin2hex (string bin)
{
  string res;
  const char hex[] = "0123456789abcdef";
  for (auto sc : bin)
  {
    unsigned char c = static_cast<unsigned char>(sc);
    res += hex[c >> 4];
    res += hex[c & 0xf];
  }
  return res;
}


// C++ equivalent for PHP's hex2bin function.
string hex2bin (string hex)
{
  string out;
  if (hex.length() % 2 == 0) {
    out.reserve (hex.length()/2);
    string extract;
    for (string::const_iterator pos = hex.begin(); pos < hex.end(); pos += 2)
    {
      extract.assign (pos, pos+2);
      out.push_back (my_stoi (extract, nullptr, 16));
    }
  }
  return out;
}


// Tidies up html.
string html_tidy (string html)
{
  html = filter_string_str_replace ("<", "\n<", html);
  return html;
}


// Converts elements from the HTML specification to the XML spec.
string html2xml (string html)
{
  // HTML specification: <hr>, XML specification: <hr/>.
  html = filter_string_str_replace ("<hr>", "<hr/>", html);

  // HTML specification: <br>, XML specification: <br/>.
  html = filter_string_str_replace ("<br>", "<br/>", html);

  return html;
}


// Converts XML character entities, like e.g. "&#xB6;" to normal UTF-8 character, like e.g. "¶".
string convert_xml_character_entities_to_characters (string data)
{
  bool keep_going = true;
  int iterations = 0;
  size_t pos1 = -1;
  do {
    iterations++;
    pos1 = data.find ("&#x", pos1 + 1);
    if (pos1 == string::npos) {
      keep_going = false;
      continue;
    }
    size_t pos2 = data.find (";", pos1);
    if (pos2 == string::npos) {
      keep_going = false;
      continue;
    }
    string entity = data.substr (pos1 + 3, pos2 - pos1 - 3);
    data.erase (pos1, pos2 - pos1 + 1);
    int codepoint;
    stringstream ss;
    ss << hex << entity;
    ss >> codepoint;
    
    // The following is not available in GNU libstdc++.
    // wstring_convert <codecvt_utf8 <char32_t>, char32_t> conv1;
    // string u8str = conv1.to_bytes (codepoint);
    
    int cp = codepoint;
    // Adapted from: http://www.zedwood.com/article/cpp-utf8-char-to-codepoint.
    char c[5]={ 0x00,0x00,0x00,0x00,0x00 };
    if     (cp<=0x7F) { c[0] = cp;  }
    else if(cp<=0x7FF) { c[0] = (cp>>6)+192; c[1] = (cp&63)+128; }
    else if(0xd800<=cp && cp<=0xdfff) {} //invalid block of utf8
    else if(cp<=0xFFFF) { c[0] = (cp>>12)+224; c[1]= ((cp>>6)&63)+128; c[2]=(cp&63)+128; }
    else if(cp<=0x10FFFF) { c[0] = (cp>>18)+240; c[1] = ((cp>>12)&63)+128; c[2] = ((cp>>6)&63)+128; c[3]=(cp&63)+128; }
    string u8str = string (c);
    
    data.insert (pos1, u8str);
  } while (keep_going & (iterations < 100000));
  return data;
}


// Encrypts the $data if the data is unencrypted.
// Decrypts the $data if the data is encrypted.
string encrypt_decrypt (string key, string data)
{
  // Encrypt the key.
  key = md5 (key);
  // Encrypt the data through the encrypted key.
  for (size_t i = 0; i < data.size(); i++) {
    data[i] = data[i] ^ key [i % key.length ()];
  }
  // Result.
  return data;
}


// Gets a new random string for sessions, encryption, you name it.
string get_new_random_string ()
{
  string u = convert_to_string (filter_date_numerical_microseconds ());
  string s = convert_to_string (filter_date_seconds_since_epoch ());
  string r = convert_to_string (config_globals_int_distribution (config_globals_random_engine));
  return md5 (u + s + r);
}


string unicode_non_breaking_space_entity ()
{
  return "&nbsp;";
}


string unicode_black_up_pointing_triangle ()
{
  return "▲";
}


string unicode_black_right_pointing_triangle ()
{
  return "▶";
}


string unicode_black_down_pointing_triangle ()
{
  return "▼";
}


string unicode_black_left_pointing_triangle ()
{
  return "◀";
}


string emoji_black_right_pointing_triangle ()
{
  return "▶️";
}


string emoji_file_folder ()
{
  return "📁";
}


string emoji_open_book ()
{
  return "📖";
}


string emoji_wastebasket ()
{
  return "🗑";
}


string emoji_smiling_face_with_smiling_eyes ()
{
  return "😊";
}


string emoji_heavy_plus_sign ()
{
  return "➕";
}


// Move the $item $up (towards the beginning), or else down (towards the end).
void array_move_up_down (vector <string> & container, size_t item, bool up)
{
  if (up) {
    if (item > 0) {
      if (item < container.size ()) {
        string s = container [item - 1];
        container [item - 1] = container [item];
        container [item] = s;
      }
    }
  } else {
    if (item < (container.size () - 1)) {
      string s = container [item + 1];
      container [item + 1] = container [item];
      container [item] = s;
    }
  }
}


// Move the item in the $container from position $from to position $to.
void array_move_from_to (vector <string> & container, size_t from, size_t to)
{
  // Check on validity of where moving from and where moving to.
  if (from == to) return;
  if (from >= container.size ()) return;
  if (to >= container.size ()) return;
  
  // Put the data into a map where the keys are multiplied by two.
  map <int, string> mapped_container;
  for (unsigned int i = 0; i < container.size(); i++) {
    mapped_container [i * 2] = container [i];
  }

  // Direction of moving.
  bool move_up = to > from;
  
  // Updated keys.
  from *= 2;
  to *= 2;
  
  // Remove the item, and insert it by a key that puts it at the desired position.
  string moving_item = mapped_container [from];
  mapped_container.erase (from);
  if (move_up) to++;
  else to--;
  mapped_container [to] = moving_item;
  
  // Since the map sorts by key,
  // transfer its data back to the original container.
  container.clear ();
  for (auto & element : mapped_container) {
    container.push_back (element.second);
  }
}


const char * english ()
{
  return "English";
}


#ifdef HAVE_WINDOWS
wstring string2wstring(const string& str)
{
  using convert_typeX = codecvt_utf8<wchar_t>;
  wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.from_bytes(str);
}
#endif


#ifdef HAVE_WINDOWS
string wstring2string(const wstring& wstr)
{
  using convert_typeX = codecvt_utf8<wchar_t>;
  wstring_convert<convert_typeX, wchar_t> converterX;
  return converterX.to_bytes(wstr);
}
#endif


// Converts any line feed character in $str to carriage return + line feed characters,
// basically adding the appropriate carriage return characters.
string lf2crlf (string str)
{
  return filter_string_str_replace ("\n", "\r\n", str);
}


// Converts any carriage return + line feed characters to a line feed character,
// essentially removing any carriage return characters.
string crlf2lf (string str)
{
  return filter_string_str_replace ("\r\n", "\n", str);
}


// Gets the <element> ... </element> part of the input $html.
string filter_text_html_get_element (string html, string element)
{
  size_t pos = html.find ("<" + element);
  if (pos != string::npos) {
    html.erase (0, pos);
    pos = html.find ("</" + element + ">");
    if (pos != string::npos) {
      html.erase (pos + 7);
    }
  }
  return html;
}

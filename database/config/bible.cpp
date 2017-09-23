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


#include <database/config/bible.h>
#include <filter/url.h>
#include <filter/string.h>
#include <styles/logic.h>


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
map <string, string> database_config_bible_cache;


// Functions for getting and setting values or lists of values follow now:


// The path to the folder for storing the settings for the $bible.
string Database_Config_Bible::file (string bible)
{
  return filter_url_create_root_path ("databases", "config", "bible", bible);
}


// The path to the file that contains this setting.
string Database_Config_Bible::file (string bible, const char * key)
{
  return filter_url_create_path (file (bible), key);
}


// The key in the cache for this setting.
string Database_Config_Bible::mapkey (string bible, const char * key)
{
  return bible + key;
}


string Database_Config_Bible::getValue (string bible, const char * key, const char * default_value)
{
  // Check the memory cache.
  string cachekey = mapkey (bible, key);
  if (database_config_bible_cache.count (cachekey)) {
    return database_config_bible_cache [cachekey];
  }
  // Get the setting from file.
  string value;
  string filename = file (bible, key);
  if (file_or_dir_exists (filename)) value = filter_url_file_get_contents (filename);
  else value = default_value;
  // Cache it.
  database_config_bible_cache [cachekey] = value;
  // Done.
  return value;
}


void Database_Config_Bible::setValue (string bible, const char * key, string value)
{
  if (bible.empty ()) return;
  // Store in memory cache.
  database_config_bible_cache [mapkey (bible, key)] = value;
  // Store on disk.
  string filename = file (bible, key);
  string dirname = filter_url_dirname (filename);
  if (!file_or_dir_exists (dirname)) filter_url_mkdir (dirname);
  filter_url_file_put_contents (filename, value);
}


bool Database_Config_Bible::getBValue (string bible, const char * key, bool default_value)
{
  return convert_to_bool (getValue (bible, key, convert_to_string (default_value).c_str()));
}


void Database_Config_Bible::setBValue (string bible, const char * key, bool value)
{
  setValue (bible, key, convert_to_string (value));
}


int Database_Config_Bible::getIValue (string bible, const char * key, int default_value)
{
  return convert_to_int (getValue (bible, key, convert_to_string (default_value).c_str()));
}


void Database_Config_Bible::setIValue (string bible, const char * key, int value)
{
  setValue (bible, key, convert_to_string (value));
}


void Database_Config_Bible::remove (string bible)
{
  // Remove from disk.
  string folder = file (bible);
  filter_url_rmdir (folder);
  // Clear cache.
  database_config_bible_cache.clear ();
}


// Named configuration functions.


string Database_Config_Bible::getRemoteRepositoryUrl (string bible)
{
  return getValue (bible, "remote-repo-url", "");
}
void Database_Config_Bible::setRemoteRepositoryUrl (string bible, string url)
{
  setValue (bible, "remote-repo-url", url);
}


bool Database_Config_Bible::getCheckDoubleSpacesUsfm (string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "double-spaces-usfm", standard);
}
void Database_Config_Bible::setCheckDoubleSpacesUsfm (string bible, bool value)
{
  setBValue (bible, "double-spaces-usfm", value);
}


bool Database_Config_Bible::getCheckFullStopInHeadings (string bible)
{
  return getBValue (bible, "full-stop-headings", false);
}
void Database_Config_Bible::setCheckFullStopInHeadings (string bible, bool value)
{
  setBValue (bible, "full-stop-headings", value);
}


bool Database_Config_Bible::getCheckSpaceBeforePunctuation (string bible)
{
  return getBValue (bible, "space-before-punctuation", false);
}
void Database_Config_Bible::setCheckSpaceBeforePunctuation (string bible, bool value)
{
  setBValue (bible, "space-before-punctuation", value);
}


bool Database_Config_Bible::getCheckSentenceStructure (string bible)
{
  return getBValue (bible, "sentence-structure", false);
}
void Database_Config_Bible::setCheckSentenceStructure (string bible, bool value)
{
  setBValue (bible, "sentence-structure", value);
}


bool Database_Config_Bible::getCheckParagraphStructure (string bible)
{
  return getBValue (bible, "paragraph-structure", false);
}
void Database_Config_Bible::setCheckParagraphStructure (string bible, bool value)
{
  setBValue (bible, "paragraph-structure", value);
}


bool Database_Config_Bible::getCheckBooksVersification (string bible)
{
  return getBValue (bible, "check-books-versification", false);
}
void Database_Config_Bible::setCheckBooksVersification (string bible, bool value)
{
  setBValue (bible, "check-books-versification", value);
}


bool Database_Config_Bible::getCheckChaptesVersesVersification (string bible)
{
  return getBValue (bible, "check-chapters-verses-versification", false);
}
void Database_Config_Bible::setCheckChaptesVersesVersification (string bible, bool value)
{
  setBValue (bible, "check-chapters-verses-versification", value);
}


bool Database_Config_Bible::getCheckWellFormedUsfm (string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "check-well-formed-usfm", standard);
}
void Database_Config_Bible::setCheckWellFormedUsfm (string bible, bool value)
{
  setBValue (bible, "check-well-formed-usfm", value);
}


bool Database_Config_Bible::getCheckMissingPunctuationEndVerse (string bible)
{
  return getBValue (bible, "missing-punctuation-end-verse", false);
}
void Database_Config_Bible::setCheckMissingPunctuationEndVerse (string bible, bool value)
{
  setBValue (bible, "missing-punctuation-end-verse", value);
}


bool Database_Config_Bible::getCheckPatterns (string bible)
{
  return getBValue (bible, "check_patterns", false);
}
void Database_Config_Bible::setCheckPatterns (string bible, bool value)
{
  setBValue (bible, "check_patterns", value);
}


string Database_Config_Bible::getCheckingPatterns (string bible) 
{
  return getValue (bible, "checking-patterns", "");
}
void Database_Config_Bible::setCheckingPatterns (string bible, string value) 
{
  setValue (bible, "checking-patterns", value);
}


string Database_Config_Bible::getSentenceStructureCapitals (string bible)
{
  return getValue (bible, "sentence-structure-capitals", "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
void Database_Config_Bible::setSentenceStructureCapitals (string bible, string value) 
{
  setValue (bible, "sentence-structure-capitals", value);
}


string Database_Config_Bible::getSentenceStructureSmallLetters (string bible)
{
  return getValue (bible, "sentence-structure-small-letters", "a b c d e f g h i j k l m n o p q r s t u v w x y z");
}
void Database_Config_Bible::setSentenceStructureSmallLetters (string bible, string value) 
{
  setValue (bible, "sentence-structure-small-letters", value);
}


string Database_Config_Bible::getSentenceStructureEndPunctuation (string bible) 
{
  return getValue (bible, "sentence-structure-end-punctuation", ". ! ? :");
}
void Database_Config_Bible::setSentenceStructureEndPunctuation (string bible, string value) 
{
  setValue (bible, "sentence-structure-end-punctuation", value);
}


string Database_Config_Bible::getSentenceStructureMiddlePunctuation (string bible) 
{
  return getValue (bible, "sentence-structure-middle-punctuation", ", ;");
}
void Database_Config_Bible::setSentenceStructureMiddlePunctuation (string bible, string value) 
{
  setValue (bible, "sentence-structure-middle-punctuation", value);
}


string Database_Config_Bible::getSentenceStructureDisregards (string bible) 
{
  return getValue (bible, "sentence-structure-disregards", "( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
}
void Database_Config_Bible::setSentenceStructureDisregards (string bible, string value) 
{
  setValue (bible, "sentence-structure-disregards", value);
}


string Database_Config_Bible::getSentenceStructureNames (string bible) 
{
  return getValue (bible, "sentence-structure-names", "");
}
void Database_Config_Bible::setSentenceStructureNames (string bible, string value) 
{
  setValue (bible, "sentence-structure-names", value);
}


string Database_Config_Bible::getSentenceStructureWithinSentenceMarkers (string bible)
{
  return getValue (bible, "sentence-structure-within_sentence-markers", "q q1 q2 q3");
}
void Database_Config_Bible::setSentenceStructureWithinSentenceMarkers (string bible, string value)
{
  setValue (bible, "sentence-structure-within_sentence-markers", value);
}


bool Database_Config_Bible::getCheckMatchingPairs (string bible)
{
  return getBValue (bible, "check-matching-pairs", false);
}
void Database_Config_Bible::setCheckMatchingPairs (string bible, bool value)
{
  setBValue (bible, "check-matching-pairs", value);
}


string Database_Config_Bible::getMatchingPairs (string bible)
{
  return getValue (bible, "matching-pairs", "[] () {} “” ‘’ «» ‹›");
}
void Database_Config_Bible::setMatchingPairs (string bible, string value)
{
  setValue (bible, "matching-pairs", value);
}


bool Database_Config_Bible::getCheckSpaceEndVerse (string bible)
{
  return getBValue (bible, "check-space-end-verse", true);
}
void Database_Config_Bible::setCheckSpaceEndVerse (string bible, bool value)
{
  setBValue (bible, "check-space-end-verse", value);
}


bool Database_Config_Bible::getCheckFrenchPunctuation (string bible)
{
  return getBValue (bible, "check-french-punctuation", false);
}
void Database_Config_Bible::setCheckFrenchPunctuation (string bible, bool value)
{
  setBValue (bible, "check-french-punctuation", value);
}


string Database_Config_Bible::getSprintTaskCompletionCategories (string bible)
{
  return getValue (bible, "sprint-task-completion-categories", "Translate\nCheck\nHebrew/Greek\nDiscussions");
}
void Database_Config_Bible::setSprintTaskCompletionCategories (string bible, string value) 
{
  setValue (bible, "sprint-task-completion-categories", value);
}


int Database_Config_Bible::getRepeatSendReceive (string bible)
{
  return getIValue (bible, "repeat-send-receive", 0);
}
void Database_Config_Bible::setRepeatSendReceive (string bible, int value)
{
  setIValue (bible, "repeat-send-receive", value);
}


bool Database_Config_Bible::getExportChapterDropCapsFrames (string bible)
{
  return getBValue (bible, "export-chapter-drop-caps-frames", false);
}
void Database_Config_Bible::setExportChapterDropCapsFrames (string bible, bool value) 
{
  setBValue (bible, "export-chapter-drop-caps-frames", value);
}


string Database_Config_Bible::getPageWidth (string bible) 
{
  return getValue (bible, "page-width", "210");
}
void Database_Config_Bible::setPageWidth  (string bible, string value) 
{
  setValue (bible, "page-width", value);
}


string Database_Config_Bible::getPageHeight (string bible) 
{
  return getValue (bible, "page-height", "297");
}
void Database_Config_Bible::setPageHeight  (string bible, string value) 
{
  setValue (bible, "page-height", value);
}


string Database_Config_Bible::getInnerMargin (string bible) 
{
  return getValue (bible, "inner-margin", "20");
}
void Database_Config_Bible::setInnerMargin  (string bible, string value) 
{
  setValue (bible, "inner-margin", value);
}


string Database_Config_Bible::getOuterMargin (string bible) 
{
  return getValue (bible, "outer-margin", "10");
}
void Database_Config_Bible::setOuterMargin  (string bible, string value) 
{
  setValue (bible, "outer-margin", value);
}


string Database_Config_Bible::getTopMargin (string bible)
{
  return getValue (bible, "top-margin", "10");
}
void Database_Config_Bible::setTopMargin  (string bible, string value) 
{
  setValue (bible, "top-margin", value);
}


string Database_Config_Bible::getBottomMargin (string bible) 
{
  return getValue (bible, "bottom-margin", "10");
}
void Database_Config_Bible::setBottomMargin  (string bible, string value) 
{
  setValue (bible, "bottom-margin", value);
}


bool Database_Config_Bible::getDateInHeader (string bible) 
{
  return getBValue (bible, "date-in-header", false);
}
void Database_Config_Bible::setDateInHeader  (string bible, bool value) 
{
  setBValue (bible, "date-in-header", value);
}


string Database_Config_Bible::getHyphenationFirstSet (string bible) 
{
  return getValue (bible, "hyphenation-first-set", "");
}
void Database_Config_Bible::setHyphenationFirstSet (string bible, string value) 
{
  setValue (bible, "hyphenation-first-set", value);
}


string Database_Config_Bible::getHyphenationSecondSet (string bible)
{
  return getValue (bible, "hyphenation-second-set", "");
}
void Database_Config_Bible::setHyphenationSecondSet (string bible, string value) 
{
  setValue (bible, "hyphenation-second-set", value);
}


string Database_Config_Bible::getExportStylesheet (string bible)
{
  return getValue (bible, "export-stylesheet", styles_logic_standard_sheet ().c_str());
}
void Database_Config_Bible::setExportStylesheet (string bible, string value)
{
  setValue (bible, "export-stylesheet", value);
}


string Database_Config_Bible::getVersificationSystem (string bible)
{
  return getValue (bible, "versification-system", english ());
}
void Database_Config_Bible::setVersificationSystem (string bible, string value)
{
  setValue (bible, "versification-system", value);
}


string Database_Config_Bible::getVerseMapping (string bible)
{
  return getValue (bible, "verse-mapping", english ());
}
void Database_Config_Bible::setVerseMapping (string bible, string value)
{
  setValue (bible, "verse-mapping", value);
}


string Database_Config_Bible::getBookAbbreviations (string bible)
{
  return getValue (bible, "book-abbreviations", "");
}
void Database_Config_Bible::setBookAbbreviations (string bible, string value)
{
  setValue (bible, "book-abbreviations", value);
}


bool Database_Config_Bible::getExportWebDuringNight (string bible)
{
  return getBValue (bible, "export-web-during-night", false);
}
void Database_Config_Bible::setExportWebDuringNight (string bible, bool value)
{
  setBValue (bible, "export-web-during-night", value);
}


bool Database_Config_Bible::getExportHtmlDuringNight (string bible)
{
  return getBValue (bible, "export-html-during-night", false);
}
void Database_Config_Bible::setExportHtmlDuringNight (string bible, bool value)
{
  setBValue (bible, "export-html-during-night", value);
}


bool Database_Config_Bible::getExportUsfmDuringNight (string bible)
{
  return getBValue (bible, "export-usfm-during-night", false);
}
void Database_Config_Bible::setExportUsfmDuringNight (string bible, bool value)
{
  setBValue (bible, "export-usfm-during-night", value);
}


bool Database_Config_Bible::getExportTextDuringNight (string bible)
{
  return getBValue (bible, "export-text-during-night", false);
}
void Database_Config_Bible::setExportTextDuringNight (string bible, bool value)
{
  setBValue (bible, "export-text-during-night", value);
}


bool Database_Config_Bible::getExportOdtDuringNight (string bible)
{
  return getBValue (bible, "export-odt-during-night", false);
}
void Database_Config_Bible::setExportOdtDuringNight (string bible, bool value)
{
  setBValue (bible, "export-odt-during-night", value);
}


bool Database_Config_Bible::getGenerateInfoDuringNight (string bible)
{
  return getBValue (bible, "generate-info-during-night", false);
}
void Database_Config_Bible::setGenerateInfoDuringNight (string bible, bool value)
{
  setBValue (bible, "generate-info-during-night", value);
}


bool Database_Config_Bible::getExportESwordDuringNight (string bible)
{
  return getBValue (bible, "export-esword-during-night", false);
}
void Database_Config_Bible::setExportESwordDuringNight (string bible, bool value)
{
  setBValue (bible, "export-esword-during-night", value);
}


bool Database_Config_Bible::getExportOnlineBibleDuringNight (string bible)
{
  return getBValue (bible, "export-onlinebible-during-night", false);
}
void Database_Config_Bible::setExportOnlineBibleDuringNight (string bible, bool value)
{
  setBValue (bible, "export-onlinebible-during-night", value);
}


bool Database_Config_Bible::getExportQuickBibleDuringNight (string bible)
{
  return getBValue (bible, "export-quickbible-during-night", false);
}
void Database_Config_Bible::setExportQuickBibleDuringNight (string bible, bool value)
{
  setBValue (bible, "export-quickbible-during-night", value);
}


string Database_Config_Bible::getExportPassword (string bible)
{
  return getValue (bible, "export-password", "");
}
void Database_Config_Bible::setExportPassword (string bible, string value) 
{
  setValue (bible, "export-password", value);
}


bool Database_Config_Bible::getSecureUsfmExport (string bible)
{
  return getBValue (bible, "secure-usfm-export", false);
}
void Database_Config_Bible::setSecureUsfmExport (string bible, bool value)
{
  setBValue (bible, "secure-usfm-export", value);
}


bool Database_Config_Bible::getSecureOdtExport (string bible)
{
  return getBValue (bible, "secure-odt-export", false);
}
void Database_Config_Bible::setSecureOdtExport (string bible, bool value)
{
  setBValue (bible, "secure-odt-export", value);
}


string Database_Config_Bible::getBookOrder (string bible)
{
  return getValue (bible, "book-order", "");
}
void Database_Config_Bible::setBookOrder (string bible, string value) 
{
  setValue (bible, "book-order", value);
}


int Database_Config_Bible::getTextDirection (string bible)
{
  return getIValue (bible, "text-direction", 0);
}
void Database_Config_Bible::setTextDirection (string bible, int value)
{
  setIValue (bible, "text-direction", value);
}


string Database_Config_Bible::getTextFont (string bible)
{
  return getValue (bible, "text-font", "");
}
void Database_Config_Bible::setTextFont (string bible, string value) 
{
  setValue (bible, "text-font", value);
}


string Database_Config_Bible::getTextFontClient (string bible)
{
  return getValue (bible, "text-font-client", "");
}
void Database_Config_Bible::setTextFontClient (string bible, string value)
{
  setValue (bible, "text-font-client", value);
}


string Database_Config_Bible::getParatextProject (string bible)
{
  return getValue (bible, "paratext-project", "");
}
void Database_Config_Bible::setParatextProject (string bible, string value)
{
  setValue (bible, "paratext-project", value);
}


bool Database_Config_Bible::getParatextCollaborationEnabled (string bible)
{
  return getBValue (bible, "paratext-collaboration-enabled", false);
}
void Database_Config_Bible::setParatextCollaborationEnabled (string bible, bool value)
{
  setBValue (bible, "paratext-collaboration-enabled", value);
}


int Database_Config_Bible::getLineHeight (string bible)
{
  return getIValue (bible, "line-height", 100);
}
void Database_Config_Bible::setLineHeight (string bible, int value)
{
  setIValue (bible, "line-height", value);
}


int Database_Config_Bible::getLetterSpacing (string bible)
{
  return getIValue (bible, "letter-spacing", 0);
}
void Database_Config_Bible::setLetterSpacing (string bible, int value)
{
  setIValue (bible, "letter-spacing", value);
}


bool Database_Config_Bible::getPublicFeedbackEnabled (string bible)
{
  return getBValue (bible, "public-feedback-enabled", true);
}
void Database_Config_Bible::setPublicFeedbackEnabled (string bible, bool value)
{
  setBValue (bible, "public-feedback-enabled", value);
}


bool Database_Config_Bible::getReadFromGit (string bible)
{
  return getBValue (bible, "read-from-git", false);
}
void Database_Config_Bible::setReadFromGit (string bible, bool value)
{
  setBValue (bible, "read-from-git", value);
}


const char * send_changes_to_rss_key ()
{
  return "send-changes-to-rss";
}
bool Database_Config_Bible::getSendChangesToRSS (string bible)
{
  return getBValue (bible, send_changes_to_rss_key (), false);
}
void Database_Config_Bible::setSendChangesToRSS (string bible, bool value)
{
  setBValue (bible, send_changes_to_rss_key (), value);
}

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


#include <database/config/bible.h>
#include <filter/url.h>
#include <filter/string.h>
#include <styles/logic.h>
#include <database/logic.h>


namespace database::config::bible {


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
static std::map <std::string, std::string> database_config_bible_cache;


// Functions for getting and setting values or lists of values follow now:


// The path to the folder for storing the settings for the $bible.
static std::string file (const std::string& bible)
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "bible", bible});
}


// The path to the file that contains this setting.
static std::string file (const std::string& bible, const char * key)
{
  return filter_url_create_path ({file (bible), key});
}


// The key in the cache for this setting.
static std::string mapkey (const std::string& bible, const char * key)
{
  return bible + key;
}


std::string getValue (const std::string& bible, const char * key, const char * default_value)
{
  // Check the memory cache.
  const std::string cachekey = mapkey (bible, key);
  if (database_config_bible_cache.count (cachekey)) {
    return database_config_bible_cache [cachekey];
  }
  // Get the setting from file.
  std::string value;
  const std::string filename = file (bible, key);
  if (file_or_dir_exists (filename))
    value = filter_url_file_get_contents (filename);
  else 
    value = default_value;
  // Cache it.
  database_config_bible_cache [cachekey] = value;
  // Done.
  return value;
}


static void setValue (const std::string& bible, const char * key, const std::string& value)
{
  if (bible.empty ()) 
    return;
  // Store in memory cache.
  database_config_bible_cache [mapkey (bible, key)] = value;
  // Store on disk.
  const std::string filename = file (bible, key);
  const std::string dirname = filter_url_dirname (filename);
  if (!file_or_dir_exists (dirname))
    filter_url_mkdir (dirname);
  filter_url_file_put_contents (filename, value);
}


static bool getBValue (const std::string& bible, const char * key, bool default_value)
{
  return filter::strings::convert_to_bool (getValue (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


static void setBValue (const std::string& bible, const char * key, bool value)
{
  setValue (bible, key, filter::strings::convert_to_string (value));
}


static int getIValue (const std::string& bible, const char * key, int default_value)
{
  return filter::strings::convert_to_int (getValue (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


static void setIValue (const std::string& bible, const char * key, int value)
{
  setValue (bible, key, filter::strings::convert_to_string (value));
}


void remove (const std::string& bible)
{
  // Remove from disk.
  const std::string folder = file (bible);
  filter_url_rmdir (folder);
  // Clear cache.
  database_config_bible_cache.clear ();
}


// Named configuration functions.


std::string getRemoteRepositoryUrl (std::string bible)
{
  return getValue (bible, "remote-repo-url", "");
}
void setRemoteRepositoryUrl (std::string bible, std::string url)
{
  setValue (bible, "remote-repo-url", url);
}


bool getCheckDoubleSpacesUsfm (std::string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "double-spaces-usfm", standard);
}
void setCheckDoubleSpacesUsfm (std::string bible, bool value)
{
  setBValue (bible, "double-spaces-usfm", value);
}


bool getCheckFullStopInHeadings (std::string bible)
{
  return getBValue (bible, "full-stop-headings", false);
}
void setCheckFullStopInHeadings (std::string bible, bool value)
{
  setBValue (bible, "full-stop-headings", value);
}


bool getCheckSpaceBeforePunctuation (std::string bible)
{
  return getBValue (bible, "space-before-punctuation", false);
}
void setCheckSpaceBeforePunctuation (std::string bible, bool value)
{
  setBValue (bible, "space-before-punctuation", value);
}


const char * space_before_final_note_marker_key ()
{
  return "space-before-final-note-marker";
}
bool getCheckSpaceBeforeFinalNoteMarker (std::string bible)
{
  return getBValue (bible, space_before_final_note_marker_key (), false);
}
void setCheckSpaceBeforeFinalNoteMarker (std::string bible, bool value)
{
  setBValue (bible, space_before_final_note_marker_key (), value);
}


bool getCheckSentenceStructure (std::string bible)
{
  return getBValue (bible, "sentence-structure", false);
}
void setCheckSentenceStructure (std::string bible, bool value)
{
  setBValue (bible, "sentence-structure", value);
}


bool getCheckParagraphStructure (std::string bible)
{
  return getBValue (bible, "paragraph-structure", false);
}
void setCheckParagraphStructure (std::string bible, bool value)
{
  setBValue (bible, "paragraph-structure", value);
}


bool getCheckBooksVersification (std::string bible)
{
  return getBValue (bible, "check-books-versification", false);
}
void setCheckBooksVersification (std::string bible, bool value)
{
  setBValue (bible, "check-books-versification", value);
}


bool getCheckChaptesVersesVersification (std::string bible)
{
  return getBValue (bible, "check-chapters-verses-versification", false);
}
void setCheckChaptesVersesVersification (std::string bible, bool value)
{
  setBValue (bible, "check-chapters-verses-versification", value);
}


bool getCheckWellFormedUsfm (std::string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "check-well-formed-usfm", standard);
}
void setCheckWellFormedUsfm (std::string bible, bool value)
{
  setBValue (bible, "check-well-formed-usfm", value);
}


bool getCheckMissingPunctuationEndVerse (std::string bible)
{
  return getBValue (bible, "missing-punctuation-end-verse", false);
}
void setCheckMissingPunctuationEndVerse (std::string bible, bool value)
{
  setBValue (bible, "missing-punctuation-end-verse", value);
}


bool getCheckPatterns (std::string bible)
{
  return getBValue (bible, "check_patterns", false);
}
void setCheckPatterns (std::string bible, bool value)
{
  setBValue (bible, "check_patterns", value);
}


std::string getCheckingPatterns (std::string bible)
{
  return getValue (bible, "checking-patterns", "");
}
void setCheckingPatterns (std::string bible, std::string value)
{
  setValue (bible, "checking-patterns", value);
}


std::string getSentenceStructureCapitals (std::string bible)
{
  return getValue (bible, "sentence-structure-capitals", "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
void setSentenceStructureCapitals (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-capitals", value);
}


std::string getSentenceStructureSmallLetters (std::string bible)
{
  return getValue (bible, "sentence-structure-small-letters", "a b c d e f g h i j k l m n o p q r s t u v w x y z");
}
void setSentenceStructureSmallLetters (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-small-letters", value);
}


std::string getSentenceStructureEndPunctuation (std::string bible)
{
  return getValue (bible, "sentence-structure-end-punctuation", ". ! ? :");
}
void setSentenceStructureEndPunctuation (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-end-punctuation", value);
}


std::string getSentenceStructureMiddlePunctuation (std::string bible)
{
  return getValue (bible, "sentence-structure-middle-punctuation", ", ;");
}
void setSentenceStructureMiddlePunctuation (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-middle-punctuation", value);
}


std::string getSentenceStructureDisregards (std::string bible)
{
  return getValue (bible, "sentence-structure-disregards", "( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
}
void setSentenceStructureDisregards (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-disregards", value);
}


std::string getSentenceStructureNames (std::string bible)
{
  return getValue (bible, "sentence-structure-names", "");
}
void setSentenceStructureNames (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-names", value);
}


std::string getSentenceStructureWithinSentenceMarkers (std::string bible)
{
  return getValue (bible, "sentence-structure-within_sentence-markers", "q q1 q2 q3");
}
void setSentenceStructureWithinSentenceMarkers (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-within_sentence-markers", value);
}


bool getCheckMatchingPairs (std::string bible)
{
  return getBValue (bible, "check-matching-pairs", false);
}
void setCheckMatchingPairs (std::string bible, bool value)
{
  setBValue (bible, "check-matching-pairs", value);
}


std::string getMatchingPairs (std::string bible)
{
  return getValue (bible, "matching-pairs", "[] () {} “” ‘’ «» ‹›");
}
void setMatchingPairs (std::string bible, std::string value)
{
  setValue (bible, "matching-pairs", value);
}


bool getCheckSpaceEndVerse (std::string bible)
{
  return getBValue (bible, "check-space-end-verse", true);
}
void setCheckSpaceEndVerse (std::string bible, bool value)
{
  setBValue (bible, "check-space-end-verse", value);
}


bool getCheckFrenchPunctuation (std::string bible)
{
  return getBValue (bible, "check-french-punctuation", false);
}
void setCheckFrenchPunctuation (std::string bible, bool value)
{
  setBValue (bible, "check-french-punctuation", value);
}


const char * check_french_citation_style_key ()
{
  return "check-french-citation-style";
}
bool getCheckFrenchCitationStyle (std::string bible)
{
  return getBValue (bible, check_french_citation_style_key (), false);
}
void setCheckFrenchCitationStyle (std::string bible, bool value)
{
  setBValue (bible, check_french_citation_style_key (), value);
}


const char * transpose_fix_spaces_notes_key ()
{
  return "transpose-fix-spaces-notes";
}
bool getTransposeFixSpacesNotes (std::string bible)
{
  return getBValue (bible, transpose_fix_spaces_notes_key (), false);
}
void setTransposeFixSpacesNotes (std::string bible, bool value)
{
  setBValue (bible, transpose_fix_spaces_notes_key (), value);
}


const char * check_valid_utf8_text_key ()
{
  return "check-valid-utf8-text";
}
bool getCheckValidUTF8Text (std::string bible)
{
  return getBValue (bible, check_valid_utf8_text_key (), false);
}
void setCheckValidUTF8Text (std::string bible, bool value)
{
  setBValue (bible, check_valid_utf8_text_key (), value);
}


std::string getSprintTaskCompletionCategories (std::string bible)
{
  return getValue (bible, "sprint-task-completion-categories", "Translate\nCheck\nHebrew/Greek\nDiscussions");
}
void setSprintTaskCompletionCategories (std::string bible, std::string value)
{
  setValue (bible, "sprint-task-completion-categories", value);
}


int get_repeat_send_receive (std::string bible)
{
  return getIValue (bible, "repeat-send-receive", 0);
}
void set_repeat_send_receive (std::string bible, int value)
{
  setIValue (bible, "repeat-send-receive", value);
}


bool getExportChapterDropCapsFrames (std::string bible)
{
  return getBValue (bible, "export-chapter-drop-caps-frames", false);
}
void setExportChapterDropCapsFrames (std::string bible, bool value)
{
  setBValue (bible, "export-chapter-drop-caps-frames", value);
}


std::string getPageWidth (std::string bible)
{
  return getValue (bible, "page-width", "210");
}
void setPageWidth  (std::string bible, std::string value)
{
  setValue (bible, "page-width", value);
}


std::string getPageHeight (std::string bible)
{
  return getValue (bible, "page-height", "297");
}
void setPageHeight  (std::string bible, std::string value)
{
  setValue (bible, "page-height", value);
}


std::string getInnerMargin (std::string bible)
{
  return getValue (bible, "inner-margin", "20");
}
void setInnerMargin  (std::string bible, std::string value)
{
  setValue (bible, "inner-margin", value);
}


std::string getOuterMargin (std::string bible)
{
  return getValue (bible, "outer-margin", "10");
}
void setOuterMargin  (std::string bible, std::string value)
{
  setValue (bible, "outer-margin", value);
}


std::string getTopMargin (std::string bible)
{
  return getValue (bible, "top-margin", "10");
}
void setTopMargin  (std::string bible, std::string value)
{
  setValue (bible, "top-margin", value);
}


std::string getBottomMargin (std::string bible)
{
  return getValue (bible, "bottom-margin", "10");
}
void setBottomMargin  (std::string bible, std::string value)
{
  setValue (bible, "bottom-margin", value);
}


bool getDateInHeader (std::string bible)
{
  return getBValue (bible, "date-in-header", false);
}
void setDateInHeader  (std::string bible, bool value)
{
  setBValue (bible, "date-in-header", value);
}


std::string getHyphenationFirstSet (std::string bible)
{
  return getValue (bible, "hyphenation-first-set", "");
}
void setHyphenationFirstSet (std::string bible, std::string value)
{
  setValue (bible, "hyphenation-first-set", value);
}


std::string getHyphenationSecondSet (std::string bible)
{
  return getValue (bible, "hyphenation-second-set", "");
}
void setHyphenationSecondSet (std::string bible, std::string value)
{
  setValue (bible, "hyphenation-second-set", value);
}


std::string getEditorStylesheet (std::string bible)
{
  return getValue (bible, "editor-stylesheet", styles_logic_standard_sheet ().c_str());
}
void setEditorStylesheet (std::string bible, std::string value)
{
  setValue (bible, "editor-stylesheet", value);
}


std::string getExportStylesheet (std::string bible)
{
  return getValue (bible, "export-stylesheet", styles_logic_standard_sheet ().c_str());
}
void setExportStylesheet (std::string bible, std::string value)
{
  setValue (bible, "export-stylesheet", value);
}


std::string getVersificationSystem (std::string bible)
{
  return getValue (bible, "versification-system", filter::strings::english ());
}
void setVersificationSystem (std::string bible, std::string value)
{
  setValue (bible, "versification-system", value);
}


bool getExportWebDuringNight (std::string bible)
{
  return getBValue (bible, "export-web-during-night", false);
}
void setExportWebDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-web-during-night", value);
}


bool getExportHtmlDuringNight (std::string bible)
{
  return getBValue (bible, "export-html-during-night", false);
}
void setExportHtmlDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-html-during-night", value);
}


const char * export_html_notes_on_hover_key ()
{
  return "export-html-notes-on-hover";
}
bool getExportHtmlNotesOnHover (std::string bible)
{
  return getBValue (bible, export_html_notes_on_hover_key (), false);
}
void setExportHtmlNotesOnHover (std::string bible, bool value)
{
  setBValue (bible, export_html_notes_on_hover_key (), value);
}


bool getExportUsfmDuringNight (std::string bible)
{
  return getBValue (bible, "export-usfm-during-night", false);
}
void setExportUsfmDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-usfm-during-night", value);
}


bool getExportTextDuringNight (std::string bible)
{
  return getBValue (bible, "export-text-during-night", false);
}
void setExportTextDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-text-during-night", value);
}


bool getExportOdtDuringNight (std::string bible)
{
  return getBValue (bible, "export-odt-during-night", false);
}
void setExportOdtDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-odt-during-night", value);
}


bool getGenerateInfoDuringNight (std::string bible)
{
  return getBValue (bible, "generate-info-during-night", false);
}
void setGenerateInfoDuringNight (std::string bible, bool value)
{
  setBValue (bible, "generate-info-during-night", value);
}


bool getExportESwordDuringNight (std::string bible)
{
  return getBValue (bible, "export-esword-during-night", false);
}
void setExportESwordDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-esword-during-night", value);
}


bool getExportOnlineBibleDuringNight (std::string bible)
{
  return getBValue (bible, "export-onlinebible-during-night", false);
}
void setExportOnlineBibleDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-onlinebible-during-night", value);
}


std::string getExportPassword (std::string bible)
{
  return getValue (bible, "export-password", "");
}
void setExportPassword (std::string bible, std::string value)
{
  setValue (bible, "export-password", value);
}


bool getSecureUsfmExport (std::string bible)
{
  return getBValue (bible, "secure-usfm-export", false);
}
void setSecureUsfmExport (std::string bible, bool value)
{
  setBValue (bible, "secure-usfm-export", value);
}


bool getSecureOdtExport (std::string bible)
{
  return getBValue (bible, "secure-odt-export", false);
}
void setSecureOdtExport (std::string bible, bool value)
{
  setBValue (bible, "secure-odt-export", value);
}


const char * export_font_key ()
{
  return "export-font";
}
std::string getExportFont (std::string bible)
{
  return getValue (bible, export_font_key (), "");
}
void setExportFont (std::string bible, std::string value)
{
  setValue (bible, export_font_key (), value);
}


const char * export_feedback_email_key ()
{
  return "export-feedback-email";
}
std::string getExportFeedbackEmail (std::string bible)
{
  return getValue (bible, export_feedback_email_key (), "");
}
void setExportFeedbackEmail (std::string bible, std::string value)
{
  setValue (bible, export_feedback_email_key (), value);
}


std::string getBookOrder (std::string bible)
{
  return getValue (bible, "book-order", "");
}
void setBookOrder (std::string bible, std::string value)
{
  setValue (bible, "book-order", value);
}


int getTextDirection (std::string bible)
{
  return getIValue (bible, "text-direction", 0);
}
void setTextDirection (std::string bible, int value)
{
  setIValue (bible, "text-direction", value);
}


std::string getTextFont (std::string bible)
{
  return getValue (bible, "text-font", "");
}
void setTextFont (std::string bible, std::string value)
{
  setValue (bible, "text-font", value);
}


std::string getTextFontClient (std::string bible)
{
  return getValue (bible, "text-font-client", "");
}
void setTextFontClient (std::string bible, std::string value)
{
  setValue (bible, "text-font-client", value);
}


std::string getParatextProject (std::string bible)
{
  return getValue (bible, "paratext-project", "");
}
void setParatextProject (std::string bible, std::string value)
{
  setValue (bible, "paratext-project", value);
}


bool getParatextCollaborationEnabled (std::string bible)
{
  return getBValue (bible, "paratext-collaboration-enabled", false);
}
void setParatextCollaborationEnabled (std::string bible, bool value)
{
  setBValue (bible, "paratext-collaboration-enabled", value);
}


int getLineHeight (std::string bible)
{
  return getIValue (bible, "line-height", 100);
}
void setLineHeight (std::string bible, int value)
{
  setIValue (bible, "line-height", value);
}


int getLetterSpacing (std::string bible)
{
  return getIValue (bible, "letter-spacing", 0);
}
void setLetterSpacing (std::string bible, int value)
{
  setIValue (bible, "letter-spacing", value);
}


bool getPublicFeedbackEnabled (std::string bible)
{
  return getBValue (bible, "public-feedback-enabled", true);
}
void setPublicFeedbackEnabled (std::string bible, bool value)
{
  setBValue (bible, "public-feedback-enabled", value);
}


bool getReadFromGit (std::string bible)
{
  return getBValue (bible, "read-from-git", false);
}
void setReadFromGit (std::string bible, bool value)
{
  setBValue (bible, "read-from-git", value);
}


const char * send_changes_to_rss_key ()
{
  return "send-changes-to-rss";
}
bool getSendChangesToRSS (std::string bible)
{
  return getBValue (bible, send_changes_to_rss_key (), false);
}
void setSendChangesToRSS (std::string bible, bool value)
{
  setBValue (bible, send_changes_to_rss_key (), value);
}


const char * odt_space_after_verse_key ()
{
  return "odt-space-after-verse";
}
std::string getOdtSpaceAfterVerse (std::string bible)
{
  return getValue (bible, odt_space_after_verse_key (), " ");
}
void setOdtSpaceAfterVerse (std::string bible, std::string value)
{
  setValue (bible, odt_space_after_verse_key (), value);
}


const char * daily_checks_enabled_key ()
{
  return "daily-checks-enabled";
}
bool getDailyChecksEnabled (std::string bible)
{
  return getBValue (bible, daily_checks_enabled_key (), true);
}
void setDailyChecksEnabled (std::string bible, bool value)
{
  setBValue (bible, daily_checks_enabled_key (), value);
}


const char * odt_poetry_verses_left_key ()
{
  return "odt-poetry-verses-left";
}
bool getOdtPoetryVersesLeft (std::string bible)
{
  return getBValue (bible, odt_poetry_verses_left_key(), false);
}
void setOdtPoetryVersesLeft (std::string bible, bool value)
{
  setBValue (bible, odt_poetry_verses_left_key(), value);
}


const char * odt_automatic_note_caller_key ()
{
  return "odt-automatic-note-caller";
}
bool getOdtAutomaticNoteCaller (std::string bible)
{
  return getBValue (bible, odt_automatic_note_caller_key(), false);
}
void setOdtAutomaticNoteCaller (std::string bible, bool value)
{
  setBValue (bible, odt_automatic_note_caller_key(), value);
}

}

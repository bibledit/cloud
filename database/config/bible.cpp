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


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
std::map <std::string, std::string> database_config_bible_cache;


// Functions for getting and setting values or lists of values follow now:


// The path to the folder for storing the settings for the $bible.
std::string Database_Config_Bible::file (std::string bible)
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "bible", bible});
}


// The path to the file that contains this setting.
std::string Database_Config_Bible::file (std::string bible, const char * key)
{
  return filter_url_create_path ({file (bible), key});
}


// The key in the cache for this setting.
std::string Database_Config_Bible::mapkey (std::string bible, const char * key)
{
  return bible + key;
}


std::string Database_Config_Bible::getValue (std::string bible, const char * key, const char * default_value)
{
  // Check the memory cache.
  std::string cachekey = mapkey (bible, key);
  if (database_config_bible_cache.count (cachekey)) {
    return database_config_bible_cache [cachekey];
  }
  // Get the setting from file.
  std::string value;
  std::string filename = file (bible, key);
  if (file_or_dir_exists (filename)) value = filter_url_file_get_contents (filename);
  else value = default_value;
  // Cache it.
  database_config_bible_cache [cachekey] = value;
  // Done.
  return value;
}


void Database_Config_Bible::setValue (std::string bible, const char * key, std::string value)
{
  if (bible.empty ()) return;
  // Store in memory cache.
  database_config_bible_cache [mapkey (bible, key)] = value;
  // Store on disk.
  std::string filename = file (bible, key);
  std::string dirname = filter_url_dirname (filename);
  if (!file_or_dir_exists (dirname)) filter_url_mkdir (dirname);
  filter_url_file_put_contents (filename, value);
}


bool Database_Config_Bible::getBValue (std::string bible, const char * key, bool default_value)
{
  return filter::strings::convert_to_bool (getValue (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


void Database_Config_Bible::setBValue (std::string bible, const char * key, bool value)
{
  setValue (bible, key, filter::strings::convert_to_string (value));
}


int Database_Config_Bible::getIValue (std::string bible, const char * key, int default_value)
{
  return filter::strings::convert_to_int (getValue (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


void Database_Config_Bible::setIValue (std::string bible, const char * key, int value)
{
  setValue (bible, key, filter::strings::convert_to_string (value));
}


void Database_Config_Bible::remove (std::string bible)
{
  // Remove from disk.
  std::string folder = file (bible);
  filter_url_rmdir (folder);
  // Clear cache.
  database_config_bible_cache.clear ();
}


// Named configuration functions.


std::string Database_Config_Bible::getRemoteRepositoryUrl (std::string bible)
{
  return getValue (bible, "remote-repo-url", "");
}
void Database_Config_Bible::setRemoteRepositoryUrl (std::string bible, std::string url)
{
  setValue (bible, "remote-repo-url", url);
}


bool Database_Config_Bible::getCheckDoubleSpacesUsfm (std::string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "double-spaces-usfm", standard);
}
void Database_Config_Bible::setCheckDoubleSpacesUsfm (std::string bible, bool value)
{
  setBValue (bible, "double-spaces-usfm", value);
}


bool Database_Config_Bible::getCheckFullStopInHeadings (std::string bible)
{
  return getBValue (bible, "full-stop-headings", false);
}
void Database_Config_Bible::setCheckFullStopInHeadings (std::string bible, bool value)
{
  setBValue (bible, "full-stop-headings", value);
}


bool Database_Config_Bible::getCheckSpaceBeforePunctuation (std::string bible)
{
  return getBValue (bible, "space-before-punctuation", false);
}
void Database_Config_Bible::setCheckSpaceBeforePunctuation (std::string bible, bool value)
{
  setBValue (bible, "space-before-punctuation", value);
}


const char * space_before_final_note_marker_key ()
{
  return "space-before-final-note-marker";
}
bool Database_Config_Bible::getCheckSpaceBeforeFinalNoteMarker (std::string bible)
{
  return getBValue (bible, space_before_final_note_marker_key (), false);
}
void Database_Config_Bible::setCheckSpaceBeforeFinalNoteMarker (std::string bible, bool value)
{
  setBValue (bible, space_before_final_note_marker_key (), value);
}


bool Database_Config_Bible::getCheckSentenceStructure (std::string bible)
{
  return getBValue (bible, "sentence-structure", false);
}
void Database_Config_Bible::setCheckSentenceStructure (std::string bible, bool value)
{
  setBValue (bible, "sentence-structure", value);
}


bool Database_Config_Bible::getCheckParagraphStructure (std::string bible)
{
  return getBValue (bible, "paragraph-structure", false);
}
void Database_Config_Bible::setCheckParagraphStructure (std::string bible, bool value)
{
  setBValue (bible, "paragraph-structure", value);
}


bool Database_Config_Bible::getCheckBooksVersification (std::string bible)
{
  return getBValue (bible, "check-books-versification", false);
}
void Database_Config_Bible::setCheckBooksVersification (std::string bible, bool value)
{
  setBValue (bible, "check-books-versification", value);
}


bool Database_Config_Bible::getCheckChaptesVersesVersification (std::string bible)
{
  return getBValue (bible, "check-chapters-verses-versification", false);
}
void Database_Config_Bible::setCheckChaptesVersesVersification (std::string bible, bool value)
{
  setBValue (bible, "check-chapters-verses-versification", value);
}


bool Database_Config_Bible::getCheckWellFormedUsfm (std::string bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return getBValue (bible, "check-well-formed-usfm", standard);
}
void Database_Config_Bible::setCheckWellFormedUsfm (std::string bible, bool value)
{
  setBValue (bible, "check-well-formed-usfm", value);
}


bool Database_Config_Bible::getCheckMissingPunctuationEndVerse (std::string bible)
{
  return getBValue (bible, "missing-punctuation-end-verse", false);
}
void Database_Config_Bible::setCheckMissingPunctuationEndVerse (std::string bible, bool value)
{
  setBValue (bible, "missing-punctuation-end-verse", value);
}


bool Database_Config_Bible::getCheckPatterns (std::string bible)
{
  return getBValue (bible, "check_patterns", false);
}
void Database_Config_Bible::setCheckPatterns (std::string bible, bool value)
{
  setBValue (bible, "check_patterns", value);
}


std::string Database_Config_Bible::getCheckingPatterns (std::string bible)
{
  return getValue (bible, "checking-patterns", "");
}
void Database_Config_Bible::setCheckingPatterns (std::string bible, std::string value) 
{
  setValue (bible, "checking-patterns", value);
}


std::string Database_Config_Bible::getSentenceStructureCapitals (std::string bible)
{
  return getValue (bible, "sentence-structure-capitals", "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
void Database_Config_Bible::setSentenceStructureCapitals (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-capitals", value);
}


std::string Database_Config_Bible::getSentenceStructureSmallLetters (std::string bible)
{
  return getValue (bible, "sentence-structure-small-letters", "a b c d e f g h i j k l m n o p q r s t u v w x y z");
}
void Database_Config_Bible::setSentenceStructureSmallLetters (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-small-letters", value);
}


std::string Database_Config_Bible::getSentenceStructureEndPunctuation (std::string bible)
{
  return getValue (bible, "sentence-structure-end-punctuation", ". ! ? :");
}
void Database_Config_Bible::setSentenceStructureEndPunctuation (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-end-punctuation", value);
}


std::string Database_Config_Bible::getSentenceStructureMiddlePunctuation (std::string bible)
{
  return getValue (bible, "sentence-structure-middle-punctuation", ", ;");
}
void Database_Config_Bible::setSentenceStructureMiddlePunctuation (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-middle-punctuation", value);
}


std::string Database_Config_Bible::getSentenceStructureDisregards (std::string bible)
{
  return getValue (bible, "sentence-structure-disregards", "( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
}
void Database_Config_Bible::setSentenceStructureDisregards (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-disregards", value);
}


std::string Database_Config_Bible::getSentenceStructureNames (std::string bible)
{
  return getValue (bible, "sentence-structure-names", "");
}
void Database_Config_Bible::setSentenceStructureNames (std::string bible, std::string value) 
{
  setValue (bible, "sentence-structure-names", value);
}


std::string Database_Config_Bible::getSentenceStructureWithinSentenceMarkers (std::string bible)
{
  return getValue (bible, "sentence-structure-within_sentence-markers", "q q1 q2 q3");
}
void Database_Config_Bible::setSentenceStructureWithinSentenceMarkers (std::string bible, std::string value)
{
  setValue (bible, "sentence-structure-within_sentence-markers", value);
}


bool Database_Config_Bible::getCheckMatchingPairs (std::string bible)
{
  return getBValue (bible, "check-matching-pairs", false);
}
void Database_Config_Bible::setCheckMatchingPairs (std::string bible, bool value)
{
  setBValue (bible, "check-matching-pairs", value);
}


std::string Database_Config_Bible::getMatchingPairs (std::string bible)
{
  return getValue (bible, "matching-pairs", "[] () {} “” ‘’ «» ‹›");
}
void Database_Config_Bible::setMatchingPairs (std::string bible, std::string value)
{
  setValue (bible, "matching-pairs", value);
}


bool Database_Config_Bible::getCheckSpaceEndVerse (std::string bible)
{
  return getBValue (bible, "check-space-end-verse", true);
}
void Database_Config_Bible::setCheckSpaceEndVerse (std::string bible, bool value)
{
  setBValue (bible, "check-space-end-verse", value);
}


bool Database_Config_Bible::getCheckFrenchPunctuation (std::string bible)
{
  return getBValue (bible, "check-french-punctuation", false);
}
void Database_Config_Bible::setCheckFrenchPunctuation (std::string bible, bool value)
{
  setBValue (bible, "check-french-punctuation", value);
}


const char * check_french_citation_style_key ()
{
  return "check-french-citation-style";
}
bool Database_Config_Bible::getCheckFrenchCitationStyle (std::string bible)
{
  return getBValue (bible, check_french_citation_style_key (), false);
}
void Database_Config_Bible::setCheckFrenchCitationStyle (std::string bible, bool value)
{
  setBValue (bible, check_french_citation_style_key (), value);
}


const char * transpose_fix_spaces_notes_key ()
{
  return "transpose-fix-spaces-notes";
}
bool Database_Config_Bible::getTransposeFixSpacesNotes (std::string bible)
{
  return getBValue (bible, transpose_fix_spaces_notes_key (), false);
}
void Database_Config_Bible::setTransposeFixSpacesNotes (std::string bible, bool value)
{
  setBValue (bible, transpose_fix_spaces_notes_key (), value);
}


const char * check_valid_utf8_text_key ()
{
  return "check-valid-utf8-text";
}
bool Database_Config_Bible::getCheckValidUTF8Text (std::string bible)
{
  return getBValue (bible, check_valid_utf8_text_key (), false);
}
void Database_Config_Bible::setCheckValidUTF8Text (std::string bible, bool value)
{
  setBValue (bible, check_valid_utf8_text_key (), value);
}


std::string Database_Config_Bible::getSprintTaskCompletionCategories (std::string bible)
{
  return getValue (bible, "sprint-task-completion-categories", "Translate\nCheck\nHebrew/Greek\nDiscussions");
}
void Database_Config_Bible::setSprintTaskCompletionCategories (std::string bible, std::string value) 
{
  setValue (bible, "sprint-task-completion-categories", value);
}


int Database_Config_Bible::getRepeatSendReceive (std::string bible)
{
  return getIValue (bible, "repeat-send-receive", 0);
}
void Database_Config_Bible::setRepeatSendReceive (std::string bible, int value)
{
  setIValue (bible, "repeat-send-receive", value);
}


bool Database_Config_Bible::getExportChapterDropCapsFrames (std::string bible)
{
  return getBValue (bible, "export-chapter-drop-caps-frames", false);
}
void Database_Config_Bible::setExportChapterDropCapsFrames (std::string bible, bool value) 
{
  setBValue (bible, "export-chapter-drop-caps-frames", value);
}


std::string Database_Config_Bible::getPageWidth (std::string bible)
{
  return getValue (bible, "page-width", "210");
}
void Database_Config_Bible::setPageWidth  (std::string bible, std::string value) 
{
  setValue (bible, "page-width", value);
}


std::string Database_Config_Bible::getPageHeight (std::string bible)
{
  return getValue (bible, "page-height", "297");
}
void Database_Config_Bible::setPageHeight  (std::string bible, std::string value) 
{
  setValue (bible, "page-height", value);
}


std::string Database_Config_Bible::getInnerMargin (std::string bible)
{
  return getValue (bible, "inner-margin", "20");
}
void Database_Config_Bible::setInnerMargin  (std::string bible, std::string value) 
{
  setValue (bible, "inner-margin", value);
}


std::string Database_Config_Bible::getOuterMargin (std::string bible)
{
  return getValue (bible, "outer-margin", "10");
}
void Database_Config_Bible::setOuterMargin  (std::string bible, std::string value) 
{
  setValue (bible, "outer-margin", value);
}


std::string Database_Config_Bible::getTopMargin (std::string bible)
{
  return getValue (bible, "top-margin", "10");
}
void Database_Config_Bible::setTopMargin  (std::string bible, std::string value) 
{
  setValue (bible, "top-margin", value);
}


std::string Database_Config_Bible::getBottomMargin (std::string bible)
{
  return getValue (bible, "bottom-margin", "10");
}
void Database_Config_Bible::setBottomMargin  (std::string bible, std::string value) 
{
  setValue (bible, "bottom-margin", value);
}


bool Database_Config_Bible::getDateInHeader (std::string bible) 
{
  return getBValue (bible, "date-in-header", false);
}
void Database_Config_Bible::setDateInHeader  (std::string bible, bool value) 
{
  setBValue (bible, "date-in-header", value);
}


std::string Database_Config_Bible::getHyphenationFirstSet (std::string bible)
{
  return getValue (bible, "hyphenation-first-set", "");
}
void Database_Config_Bible::setHyphenationFirstSet (std::string bible, std::string value) 
{
  setValue (bible, "hyphenation-first-set", value);
}


std::string Database_Config_Bible::getHyphenationSecondSet (std::string bible)
{
  return getValue (bible, "hyphenation-second-set", "");
}
void Database_Config_Bible::setHyphenationSecondSet (std::string bible, std::string value) 
{
  setValue (bible, "hyphenation-second-set", value);
}


std::string Database_Config_Bible::getEditorStylesheet (std::string bible)
{
  return getValue (bible, "editor-stylesheet", styles_logic_standard_sheet ().c_str());
}
void Database_Config_Bible::setEditorStylesheet (std::string bible, std::string value)
{
  setValue (bible, "editor-stylesheet", value);
}


std::string Database_Config_Bible::getExportStylesheet (std::string bible)
{
  return getValue (bible, "export-stylesheet", styles_logic_standard_sheet ().c_str());
}
void Database_Config_Bible::setExportStylesheet (std::string bible, std::string value)
{
  setValue (bible, "export-stylesheet", value);
}


std::string Database_Config_Bible::getVersificationSystem (std::string bible)
{
  return getValue (bible, "versification-system", filter::strings::english ());
}
void Database_Config_Bible::setVersificationSystem (std::string bible, std::string value)
{
  setValue (bible, "versification-system", value);
}


bool Database_Config_Bible::getExportWebDuringNight (std::string bible)
{
  return getBValue (bible, "export-web-during-night", false);
}
void Database_Config_Bible::setExportWebDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-web-during-night", value);
}


bool Database_Config_Bible::getExportHtmlDuringNight (std::string bible)
{
  return getBValue (bible, "export-html-during-night", false);
}
void Database_Config_Bible::setExportHtmlDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-html-during-night", value);
}


const char * export_html_notes_on_hover_key ()
{
  return "export-html-notes-on-hover";
}
bool Database_Config_Bible::getExportHtmlNotesOnHover (std::string bible)
{
  return getBValue (bible, export_html_notes_on_hover_key (), false);
}
void Database_Config_Bible::setExportHtmlNotesOnHover (std::string bible, bool value)
{
  setBValue (bible, export_html_notes_on_hover_key (), value);
}


bool Database_Config_Bible::getExportUsfmDuringNight (std::string bible)
{
  return getBValue (bible, "export-usfm-during-night", false);
}
void Database_Config_Bible::setExportUsfmDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-usfm-during-night", value);
}


bool Database_Config_Bible::getExportTextDuringNight (std::string bible)
{
  return getBValue (bible, "export-text-during-night", false);
}
void Database_Config_Bible::setExportTextDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-text-during-night", value);
}


bool Database_Config_Bible::getExportOdtDuringNight (std::string bible)
{
  return getBValue (bible, "export-odt-during-night", false);
}
void Database_Config_Bible::setExportOdtDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-odt-during-night", value);
}


bool Database_Config_Bible::getGenerateInfoDuringNight (std::string bible)
{
  return getBValue (bible, "generate-info-during-night", false);
}
void Database_Config_Bible::setGenerateInfoDuringNight (std::string bible, bool value)
{
  setBValue (bible, "generate-info-during-night", value);
}


bool Database_Config_Bible::getExportESwordDuringNight (std::string bible)
{
  return getBValue (bible, "export-esword-during-night", false);
}
void Database_Config_Bible::setExportESwordDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-esword-during-night", value);
}


bool Database_Config_Bible::getExportOnlineBibleDuringNight (std::string bible)
{
  return getBValue (bible, "export-onlinebible-during-night", false);
}
void Database_Config_Bible::setExportOnlineBibleDuringNight (std::string bible, bool value)
{
  setBValue (bible, "export-onlinebible-during-night", value);
}


std::string Database_Config_Bible::getExportPassword (std::string bible)
{
  return getValue (bible, "export-password", "");
}
void Database_Config_Bible::setExportPassword (std::string bible, std::string value) 
{
  setValue (bible, "export-password", value);
}


bool Database_Config_Bible::getSecureUsfmExport (std::string bible)
{
  return getBValue (bible, "secure-usfm-export", false);
}
void Database_Config_Bible::setSecureUsfmExport (std::string bible, bool value)
{
  setBValue (bible, "secure-usfm-export", value);
}


bool Database_Config_Bible::getSecureOdtExport (std::string bible)
{
  return getBValue (bible, "secure-odt-export", false);
}
void Database_Config_Bible::setSecureOdtExport (std::string bible, bool value)
{
  setBValue (bible, "secure-odt-export", value);
}


const char * export_font_key ()
{
  return "export-font";
}
std::string Database_Config_Bible::getExportFont (std::string bible)
{
  return getValue (bible, export_font_key (), "");
}
void Database_Config_Bible::setExportFont (std::string bible, std::string value)
{
  setValue (bible, export_font_key (), value);
}


const char * export_feedback_email_key ()
{
  return "export-feedback-email";
}
std::string Database_Config_Bible::getExportFeedbackEmail (std::string bible)
{
  return getValue (bible, export_feedback_email_key (), "");
}
void Database_Config_Bible::setExportFeedbackEmail (std::string bible, std::string value)
{
  setValue (bible, export_feedback_email_key (), value);
}


std::string Database_Config_Bible::getBookOrder (std::string bible)
{
  return getValue (bible, "book-order", "");
}
void Database_Config_Bible::setBookOrder (std::string bible, std::string value) 
{
  setValue (bible, "book-order", value);
}


int Database_Config_Bible::getTextDirection (std::string bible)
{
  return getIValue (bible, "text-direction", 0);
}
void Database_Config_Bible::setTextDirection (std::string bible, int value)
{
  setIValue (bible, "text-direction", value);
}


std::string Database_Config_Bible::getTextFont (std::string bible)
{
  return getValue (bible, "text-font", "");
}
void Database_Config_Bible::setTextFont (std::string bible, std::string value) 
{
  setValue (bible, "text-font", value);
}


std::string Database_Config_Bible::getTextFontClient (std::string bible)
{
  return getValue (bible, "text-font-client", "");
}
void Database_Config_Bible::setTextFontClient (std::string bible, std::string value)
{
  setValue (bible, "text-font-client", value);
}


std::string Database_Config_Bible::getParatextProject (std::string bible)
{
  return getValue (bible, "paratext-project", "");
}
void Database_Config_Bible::setParatextProject (std::string bible, std::string value)
{
  setValue (bible, "paratext-project", value);
}


bool Database_Config_Bible::getParatextCollaborationEnabled (std::string bible)
{
  return getBValue (bible, "paratext-collaboration-enabled", false);
}
void Database_Config_Bible::setParatextCollaborationEnabled (std::string bible, bool value)
{
  setBValue (bible, "paratext-collaboration-enabled", value);
}


int Database_Config_Bible::getLineHeight (std::string bible)
{
  return getIValue (bible, "line-height", 100);
}
void Database_Config_Bible::setLineHeight (std::string bible, int value)
{
  setIValue (bible, "line-height", value);
}


int Database_Config_Bible::getLetterSpacing (std::string bible)
{
  return getIValue (bible, "letter-spacing", 0);
}
void Database_Config_Bible::setLetterSpacing (std::string bible, int value)
{
  setIValue (bible, "letter-spacing", value);
}


bool Database_Config_Bible::getPublicFeedbackEnabled (std::string bible)
{
  return getBValue (bible, "public-feedback-enabled", true);
}
void Database_Config_Bible::setPublicFeedbackEnabled (std::string bible, bool value)
{
  setBValue (bible, "public-feedback-enabled", value);
}


bool Database_Config_Bible::getReadFromGit (std::string bible)
{
  return getBValue (bible, "read-from-git", false);
}
void Database_Config_Bible::setReadFromGit (std::string bible, bool value)
{
  setBValue (bible, "read-from-git", value);
}


const char * send_changes_to_rss_key ()
{
  return "send-changes-to-rss";
}
bool Database_Config_Bible::getSendChangesToRSS (std::string bible)
{
  return getBValue (bible, send_changes_to_rss_key (), false);
}
void Database_Config_Bible::setSendChangesToRSS (std::string bible, bool value)
{
  setBValue (bible, send_changes_to_rss_key (), value);
}


const char * odt_space_after_verse_key ()
{
  return "odt-space-after-verse";
}
std::string Database_Config_Bible::getOdtSpaceAfterVerse (std::string bible)
{
  return getValue (bible, odt_space_after_verse_key (), " ");
}
void Database_Config_Bible::setOdtSpaceAfterVerse (std::string bible, std::string value)
{
  setValue (bible, odt_space_after_verse_key (), value);
}


const char * daily_checks_enabled_key ()
{
  return "daily-checks-enabled";
}
bool Database_Config_Bible::getDailyChecksEnabled (std::string bible)
{
  return getBValue (bible, daily_checks_enabled_key (), true);
}
void Database_Config_Bible::setDailyChecksEnabled (std::string bible, bool value)
{
  setBValue (bible, daily_checks_enabled_key (), value);
}


const char * odt_poetry_verses_left_key ()
{
  return "odt-poetry-verses-left";
}
bool Database_Config_Bible::getOdtPoetryVersesLeft (std::string bible)
{
  return getBValue (bible, odt_poetry_verses_left_key(), false);
}
void Database_Config_Bible::setOdtPoetryVersesLeft (std::string bible, bool value)
{
  setBValue (bible, odt_poetry_verses_left_key(), value);
}


const char * odt_automatic_note_caller_key ()
{
  return "odt-automatic-note-caller";
}
bool Database_Config_Bible::getOdtAutomaticNoteCaller (std::string bible)
{
  return getBValue (bible, odt_automatic_note_caller_key(), false);
}
void Database_Config_Bible::setOdtAutomaticNoteCaller (std::string bible, bool value)
{
  setBValue (bible, odt_automatic_note_caller_key(), value);
}

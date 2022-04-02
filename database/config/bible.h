/*
Copyright (©) 2003-2022 Teus Benschop.

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


#pragma once

#include <config/libraries.h>

class Database_Config_Bible
{
public:
  static void remove (string bible);
  static string getRemoteRepositoryUrl (string bible);
  static void setRemoteRepositoryUrl (string bible, string url);
  static bool getCheckDoubleSpacesUsfm (string bible);
  static void setCheckDoubleSpacesUsfm (string bible, bool value);
  static bool getCheckFullStopInHeadings (string bible);
  static void setCheckFullStopInHeadings (string bible, bool value);
  static bool getCheckSpaceBeforePunctuation (string bible);
  static void setCheckSpaceBeforePunctuation (string bible, bool value);
  static bool getCheckSpaceBeforeFinalNoteMarker (string bible);
  static void setCheckSpaceBeforeFinalNoteMarker (string bible, bool value);
  static bool getCheckSentenceStructure (string bible);
  static void setCheckSentenceStructure (string bible, bool value);
  static bool getCheckParagraphStructure (string bible);
  static void setCheckParagraphStructure (string bible, bool value);
  static bool getCheckBooksVersification (string bible);
  static void setCheckBooksVersification (string bible, bool value);
  static bool getCheckChaptesVersesVersification (string bible);
  static void setCheckChaptesVersesVersification (string bible, bool value);
  static bool getCheckWellFormedUsfm (string bible);
  static void setCheckWellFormedUsfm (string bible, bool value);
  static bool getCheckMissingPunctuationEndVerse (string bible);
  static void setCheckMissingPunctuationEndVerse (string bible, bool value);
  static bool getCheckPatterns (string bible);
  static void setCheckPatterns (string bible, bool value);
  static string getCheckingPatterns (string bible);
  static void setCheckingPatterns (string bible, string value);
  static string getSentenceStructureCapitals (string bible);
  static void setSentenceStructureCapitals (string bible, string value);
  static string getSentenceStructureSmallLetters (string bible);
  static void setSentenceStructureSmallLetters (string bible, string value);
  static string getSentenceStructureEndPunctuation (string bible);
  static void setSentenceStructureEndPunctuation (string bible, string value);
  static string getSentenceStructureMiddlePunctuation (string bible);
  static void setSentenceStructureMiddlePunctuation (string bible, string value);
  static string getSentenceStructureDisregards (string bible);
  static void setSentenceStructureDisregards (string bible, string value);
  static string getSentenceStructureNames (string bible);
  static void setSentenceStructureNames (string bible, string value);
  static string getSentenceStructureWithinSentenceMarkers (string bible);
  static void setSentenceStructureWithinSentenceMarkers (string bible, string value);
  static bool getCheckMatchingPairs (string bible);
  static void setCheckMatchingPairs (string bible, bool value);
  static string getMatchingPairs (string bible);
  static void setMatchingPairs (string bible, string value);
  static bool getCheckSpaceEndVerse (string bible);
  static void setCheckSpaceEndVerse (string bible, bool value);
  static bool getCheckFrenchPunctuation (string bible);
  static void setCheckFrenchPunctuation (string bible, bool value);
  static bool getCheckFrenchCitationStyle (string bible);
  static void setCheckFrenchCitationStyle (string bible, bool value);
  static bool getTransposeFixSpacesNotes (string bible);
  static void setTransposeFixSpacesNotes (string bible, bool value);
  static bool getCheckValidUTF8Text (string bible);
  static void setCheckValidUTF8Text (string bible, bool value);
  static string getSprintTaskCompletionCategories (string bible);
  static void setSprintTaskCompletionCategories (string bible, string value);
  static int getRepeatSendReceive (string bible);
  static void setRepeatSendReceive (string bible, int value);
  static bool getExportChapterDropCapsFrames (string bible);
  static void setExportChapterDropCapsFrames (string bible, bool value);
  static string getPageWidth (string bible);
  static void setPageWidth  (string bible, string value);
  static string getPageHeight (string bible);
  static void setPageHeight  (string bible, string value);
  static string getInnerMargin (string bible);
  static void setInnerMargin  (string bible, string value);
  static string getOuterMargin (string bible);
  static void setOuterMargin  (string bible, string value);
  static string getTopMargin (string bible);
  static void setTopMargin  (string bible, string value);
  static string getBottomMargin (string bible);
  static void setBottomMargin  (string bible, string value);
  static bool getDateInHeader (string bible);
  static void setDateInHeader  (string bible, bool value);
  static string getHyphenationFirstSet (string bible);
  static void setHyphenationFirstSet (string bible, string value);
  static string getHyphenationSecondSet (string bible);
  static void setHyphenationSecondSet (string bible, string value);
  static string getEditorStylesheet (string bible);
  static void setEditorStylesheet (string bible, string value);
  static string getExportStylesheet (string bible);
  static void setExportStylesheet (string bible, string value);
  static string getVersificationSystem (string bible);
  static void setVersificationSystem (string bible, string value);
  static bool getExportWebDuringNight (string bible);
  static void setExportWebDuringNight (string bible, bool value);
  static bool getExportHtmlDuringNight (string bible);
  static void setExportHtmlDuringNight (string bible, bool value);
  static bool getExportHtmlNotesOnHover (string bible);
  static void setExportHtmlNotesOnHover (string bible, bool value);
  static bool getExportUsfmDuringNight (string bible);
  static void setExportUsfmDuringNight (string bible, bool value);
  static bool getExportTextDuringNight (string bible);
  static void setExportTextDuringNight (string bible, bool value);
  static bool getExportOdtDuringNight (string bible);
  static void setExportOdtDuringNight (string bible, bool value);
  static bool getGenerateInfoDuringNight (string bible);
  static void setGenerateInfoDuringNight (string bible, bool value);
  static bool getExportESwordDuringNight (string bible);
  static void setExportESwordDuringNight (string bible, bool value);
  static bool getExportOnlineBibleDuringNight (string bible);
  static void setExportOnlineBibleDuringNight (string bible, bool value);
  static string getExportPassword (string bible);
  static void setExportPassword (string bible, string value);
  static bool getSecureUsfmExport (string bible);
  static void setSecureUsfmExport (string bible, bool value);
  static bool getSecureOdtExport (string bible);
  static void setSecureOdtExport (string bible, bool value);
  static string getExportFont (string bible);
  static void setExportFont (string bible, string value);
  static string getExportFeedbackEmail (string bible);
  static void setExportFeedbackEmail (string bible, string value);
  static string getBookOrder (string bible);
  static void setBookOrder (string bible, string value);
  static int getTextDirection (string bible);
  static void setTextDirection (string bible, int value);
  static string getTextFont (string bible);
  static void setTextFont (string bible, string value);
  static string getTextFontClient (string bible);
  static void setTextFontClient (string bible, string value);
  static string getParatextProject (string bible);
  static void setParatextProject (string bible, string value);
  static bool getParatextCollaborationEnabled (string bible);
  static void setParatextCollaborationEnabled (string bible, bool value);
  static int getLineHeight (string bible);
  static void setLineHeight (string bible, int value);
  static int getLetterSpacing (string bible);
  static void setLetterSpacing (string bible, int value);
  static bool getPublicFeedbackEnabled (string bible);
  static void setPublicFeedbackEnabled (string bible, bool value);
  static bool getReadFromGit (string bible);
  static void setReadFromGit (string bible, bool value);
  static bool getSendChangesToRSS (string bible);
  static void setSendChangesToRSS (string bible, bool value);
  static string getOdtSpaceAfterVerse (string bible);
  static void setOdtSpaceAfterVerse (string bible, string value);
  static bool getDailyChecksEnabled (string bible);
  static void setDailyChecksEnabled (string bible, bool value);
  static bool getOdtPoetryVersesLeft (string bible);
  static void setOdtPoetryVersesLeft (string bible, bool value);
  static bool getOdtAutomaticNoteCaller (string bible);
  static void setOdtAutomaticNoteCaller (string bible, bool value);
private:
  static string file (string bible);
  static string file (string bible, const char * key);
  static string mapkey (string bible, const char * key);
  static string getValue (string bible, const char * key, const char * default_value);
  static void setValue (string bible, const char * key, string value);
  static bool getBValue (string bible, const char * key, bool default_value);
  static void setBValue (string bible, const char * key, bool value);
  static int getIValue (string bible, const char * key, int default_value);
  static void setIValue (string bible, const char * key, int value);
};

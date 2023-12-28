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


#pragma once

#include <config/libraries.h>

class Database_Config_Bible
{
public:
  static void remove (std::string bible);
  static std::string getRemoteRepositoryUrl (std::string bible);
  static void setRemoteRepositoryUrl (std::string bible, std::string url);
  static bool getCheckDoubleSpacesUsfm (std::string bible);
  static void setCheckDoubleSpacesUsfm (std::string bible, bool value);
  static bool getCheckFullStopInHeadings (std::string bible);
  static void setCheckFullStopInHeadings (std::string bible, bool value);
  static bool getCheckSpaceBeforePunctuation (std::string bible);
  static void setCheckSpaceBeforePunctuation (std::string bible, bool value);
  static bool getCheckSpaceBeforeFinalNoteMarker (std::string bible);
  static void setCheckSpaceBeforeFinalNoteMarker (std::string bible, bool value);
  static bool getCheckSentenceStructure (std::string bible);
  static void setCheckSentenceStructure (std::string bible, bool value);
  static bool getCheckParagraphStructure (std::string bible);
  static void setCheckParagraphStructure (std::string bible, bool value);
  static bool getCheckBooksVersification (std::string bible);
  static void setCheckBooksVersification (std::string bible, bool value);
  static bool getCheckChaptesVersesVersification (std::string bible);
  static void setCheckChaptesVersesVersification (std::string bible, bool value);
  static bool getCheckWellFormedUsfm (std::string bible);
  static void setCheckWellFormedUsfm (std::string bible, bool value);
  static bool getCheckMissingPunctuationEndVerse (std::string bible);
  static void setCheckMissingPunctuationEndVerse (std::string bible, bool value);
  static bool getCheckPatterns (std::string bible);
  static void setCheckPatterns (std::string bible, bool value);
  static std::string getCheckingPatterns (std::string bible);
  static void setCheckingPatterns (std::string bible, std::string value);
  static std::string getSentenceStructureCapitals (std::string bible);
  static void setSentenceStructureCapitals (std::string bible, std::string value);
  static std::string getSentenceStructureSmallLetters (std::string bible);
  static void setSentenceStructureSmallLetters (std::string bible, std::string value);
  static std::string getSentenceStructureEndPunctuation (std::string bible);
  static void setSentenceStructureEndPunctuation (std::string bible, std::string value);
  static std::string getSentenceStructureMiddlePunctuation (std::string bible);
  static void setSentenceStructureMiddlePunctuation (std::string bible, std::string value);
  static std::string getSentenceStructureDisregards (std::string bible);
  static void setSentenceStructureDisregards (std::string bible, std::string value);
  static std::string getSentenceStructureNames (std::string bible);
  static void setSentenceStructureNames (std::string bible, std::string value);
  static std::string getSentenceStructureWithinSentenceMarkers (std::string bible);
  static void setSentenceStructureWithinSentenceMarkers (std::string bible, std::string value);
  static bool getCheckMatchingPairs (std::string bible);
  static void setCheckMatchingPairs (std::string bible, bool value);
  static std::string getMatchingPairs (std::string bible);
  static void setMatchingPairs (std::string bible, std::string value);
  static bool getCheckSpaceEndVerse (std::string bible);
  static void setCheckSpaceEndVerse (std::string bible, bool value);
  static bool getCheckFrenchPunctuation (std::string bible);
  static void setCheckFrenchPunctuation (std::string bible, bool value);
  static bool getCheckFrenchCitationStyle (std::string bible);
  static void setCheckFrenchCitationStyle (std::string bible, bool value);
  static bool getTransposeFixSpacesNotes (std::string bible);
  static void setTransposeFixSpacesNotes (std::string bible, bool value);
  static bool getCheckValidUTF8Text (std::string bible);
  static void setCheckValidUTF8Text (std::string bible, bool value);
  static std::string getSprintTaskCompletionCategories (std::string bible);
  static void setSprintTaskCompletionCategories (std::string bible, std::string value);
  static int getRepeatSendReceive (std::string bible);
  static void setRepeatSendReceive (std::string bible, int value);
  static bool getExportChapterDropCapsFrames (std::string bible);
  static void setExportChapterDropCapsFrames (std::string bible, bool value);
  static std::string getPageWidth (std::string bible);
  static void setPageWidth  (std::string bible, std::string value);
  static std::string getPageHeight (std::string bible);
  static void setPageHeight  (std::string bible, std::string value);
  static std::string getInnerMargin (std::string bible);
  static void setInnerMargin  (std::string bible, std::string value);
  static std::string getOuterMargin (std::string bible);
  static void setOuterMargin  (std::string bible, std::string value);
  static std::string getTopMargin (std::string bible);
  static void setTopMargin  (std::string bible, std::string value);
  static std::string getBottomMargin (std::string bible);
  static void setBottomMargin  (std::string bible, std::string value);
  static bool getDateInHeader (std::string bible);
  static void setDateInHeader  (std::string bible, bool value);
  static std::string getHyphenationFirstSet (std::string bible);
  static void setHyphenationFirstSet (std::string bible, std::string value);
  static std::string getHyphenationSecondSet (std::string bible);
  static void setHyphenationSecondSet (std::string bible, std::string value);
  static std::string getEditorStylesheet (std::string bible);
  static void setEditorStylesheet (std::string bible, std::string value);
  static std::string getExportStylesheet (std::string bible);
  static void setExportStylesheet (std::string bible, std::string value);
  static std::string getVersificationSystem (std::string bible);
  static void setVersificationSystem (std::string bible, std::string value);
  static bool getExportWebDuringNight (std::string bible);
  static void setExportWebDuringNight (std::string bible, bool value);
  static bool getExportHtmlDuringNight (std::string bible);
  static void setExportHtmlDuringNight (std::string bible, bool value);
  static bool getExportHtmlNotesOnHover (std::string bible);
  static void setExportHtmlNotesOnHover (std::string bible, bool value);
  static bool getExportUsfmDuringNight (std::string bible);
  static void setExportUsfmDuringNight (std::string bible, bool value);
  static bool getExportTextDuringNight (std::string bible);
  static void setExportTextDuringNight (std::string bible, bool value);
  static bool getExportOdtDuringNight (std::string bible);
  static void setExportOdtDuringNight (std::string bible, bool value);
  static bool getGenerateInfoDuringNight (std::string bible);
  static void setGenerateInfoDuringNight (std::string bible, bool value);
  static bool getExportESwordDuringNight (std::string bible);
  static void setExportESwordDuringNight (std::string bible, bool value);
  static bool getExportOnlineBibleDuringNight (std::string bible);
  static void setExportOnlineBibleDuringNight (std::string bible, bool value);
  static std::string getExportPassword (std::string bible);
  static void setExportPassword (std::string bible, std::string value);
  static bool getSecureUsfmExport (std::string bible);
  static void setSecureUsfmExport (std::string bible, bool value);
  static bool getSecureOdtExport (std::string bible);
  static void setSecureOdtExport (std::string bible, bool value);
  static std::string getExportFont (std::string bible);
  static void setExportFont (std::string bible, std::string value);
  static std::string getExportFeedbackEmail (std::string bible);
  static void setExportFeedbackEmail (std::string bible, std::string value);
  static std::string getBookOrder (std::string bible);
  static void setBookOrder (std::string bible, std::string value);
  static int getTextDirection (std::string bible);
  static void setTextDirection (std::string bible, int value);
  static std::string getTextFont (std::string bible);
  static void setTextFont (std::string bible, std::string value);
  static std::string getTextFontClient (std::string bible);
  static void setTextFontClient (std::string bible, std::string value);
  static std::string getParatextProject (std::string bible);
  static void setParatextProject (std::string bible, std::string value);
  static bool getParatextCollaborationEnabled (std::string bible);
  static void setParatextCollaborationEnabled (std::string bible, bool value);
  static int getLineHeight (std::string bible);
  static void setLineHeight (std::string bible, int value);
  static int getLetterSpacing (std::string bible);
  static void setLetterSpacing (std::string bible, int value);
  static bool getPublicFeedbackEnabled (std::string bible);
  static void setPublicFeedbackEnabled (std::string bible, bool value);
  static bool getReadFromGit (std::string bible);
  static void setReadFromGit (std::string bible, bool value);
  static bool getSendChangesToRSS (std::string bible);
  static void setSendChangesToRSS (std::string bible, bool value);
  static std::string getOdtSpaceAfterVerse (std::string bible);
  static void setOdtSpaceAfterVerse (std::string bible, std::string value);
  static bool getDailyChecksEnabled (std::string bible);
  static void setDailyChecksEnabled (std::string bible, bool value);
  static bool getOdtPoetryVersesLeft (std::string bible);
  static void setOdtPoetryVersesLeft (std::string bible, bool value);
  static bool getOdtAutomaticNoteCaller (std::string bible);
  static void setOdtAutomaticNoteCaller (std::string bible, bool value);
private:
  static std::string file (std::string bible);
  static std::string file (std::string bible, const char * key);
  static std::string mapkey (std::string bible, const char * key);
  static std::string getValue (std::string bible, const char * key, const char * default_value);
  static void setValue (std::string bible, const char * key, std::string value);
  static bool getBValue (std::string bible, const char * key, bool default_value);
  static void setBValue (std::string bible, const char * key, bool value);
  static int getIValue (std::string bible, const char * key, int default_value);
  static void setIValue (std::string bible, const char * key, int value);
};

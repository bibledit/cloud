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

namespace database::config::bible {

void remove (const std::string& bible);
std::string getRemoteRepositoryUrl (std::string bible);
void setRemoteRepositoryUrl (std::string bible, std::string url);
bool getCheckDoubleSpacesUsfm (std::string bible);
void setCheckDoubleSpacesUsfm (std::string bible, bool value);
bool getCheckFullStopInHeadings (std::string bible);
void setCheckFullStopInHeadings (std::string bible, bool value);
bool getCheckSpaceBeforePunctuation (std::string bible);
void setCheckSpaceBeforePunctuation (std::string bible, bool value);
bool getCheckSpaceBeforeFinalNoteMarker (std::string bible);
void setCheckSpaceBeforeFinalNoteMarker (std::string bible, bool value);
bool getCheckSentenceStructure (std::string bible);
void setCheckSentenceStructure (std::string bible, bool value);
bool getCheckParagraphStructure (std::string bible);
void setCheckParagraphStructure (std::string bible, bool value);
bool getCheckBooksVersification (std::string bible);
void setCheckBooksVersification (std::string bible, bool value);
bool getCheckChaptesVersesVersification (std::string bible);
void setCheckChaptesVersesVersification (std::string bible, bool value);
bool getCheckWellFormedUsfm (std::string bible);
void setCheckWellFormedUsfm (std::string bible, bool value);
bool getCheckMissingPunctuationEndVerse (std::string bible);
void setCheckMissingPunctuationEndVerse (std::string bible, bool value);
bool getCheckPatterns (std::string bible);
void setCheckPatterns (std::string bible, bool value);
std::string getCheckingPatterns (std::string bible);
void setCheckingPatterns (std::string bible, std::string value);
std::string getSentenceStructureCapitals (std::string bible);
void setSentenceStructureCapitals (std::string bible, std::string value);
std::string getSentenceStructureSmallLetters (std::string bible);
void setSentenceStructureSmallLetters (std::string bible, std::string value);
std::string getSentenceStructureEndPunctuation (std::string bible);
void setSentenceStructureEndPunctuation (std::string bible, std::string value);
std::string getSentenceStructureMiddlePunctuation (std::string bible);
void setSentenceStructureMiddlePunctuation (std::string bible, std::string value);
std::string getSentenceStructureDisregards (std::string bible);
void setSentenceStructureDisregards (std::string bible, std::string value);
std::string getSentenceStructureNames (std::string bible);
void setSentenceStructureNames (std::string bible, std::string value);
std::string getSentenceStructureWithinSentenceMarkers (std::string bible);
void setSentenceStructureWithinSentenceMarkers (std::string bible, std::string value);
bool getCheckMatchingPairs (std::string bible);
void setCheckMatchingPairs (std::string bible, bool value);
std::string getMatchingPairs (std::string bible);
void setMatchingPairs (std::string bible, std::string value);
bool getCheckSpaceEndVerse (std::string bible);
void setCheckSpaceEndVerse (std::string bible, bool value);
bool getCheckFrenchPunctuation (std::string bible);
void setCheckFrenchPunctuation (std::string bible, bool value);
bool getCheckFrenchCitationStyle (std::string bible);
void setCheckFrenchCitationStyle (std::string bible, bool value);
bool getTransposeFixSpacesNotes (std::string bible);
void setTransposeFixSpacesNotes (std::string bible, bool value);
bool getCheckValidUTF8Text (std::string bible);
void setCheckValidUTF8Text (std::string bible, bool value);
std::string getSprintTaskCompletionCategories (std::string bible);
void setSprintTaskCompletionCategories (std::string bible, std::string value);
int get_repeat_send_receive (std::string bible);
void set_repeat_send_receive (std::string bible, int value);
bool getExportChapterDropCapsFrames (std::string bible);
void setExportChapterDropCapsFrames (std::string bible, bool value);
std::string getPageWidth (std::string bible);
void setPageWidth  (std::string bible, std::string value);
std::string getPageHeight (std::string bible);
void setPageHeight  (std::string bible, std::string value);
std::string getInnerMargin (std::string bible);
void setInnerMargin  (std::string bible, std::string value);
std::string getOuterMargin (std::string bible);
void setOuterMargin  (std::string bible, std::string value);
std::string getTopMargin (std::string bible);
void setTopMargin  (std::string bible, std::string value);
std::string getBottomMargin (std::string bible);
void setBottomMargin  (std::string bible, std::string value);
bool getDateInHeader (std::string bible);
void setDateInHeader  (std::string bible, bool value);
std::string getHyphenationFirstSet (std::string bible);
void setHyphenationFirstSet (std::string bible, std::string value);
std::string getHyphenationSecondSet (std::string bible);
void setHyphenationSecondSet (std::string bible, std::string value);
std::string getEditorStylesheet (std::string bible);
void setEditorStylesheet (std::string bible, std::string value);
std::string getExportStylesheet (std::string bible);
void setExportStylesheet (std::string bible, std::string value);
std::string getVersificationSystem (std::string bible);
void setVersificationSystem (std::string bible, std::string value);
bool getExportWebDuringNight (std::string bible);
void setExportWebDuringNight (std::string bible, bool value);
bool getExportHtmlDuringNight (std::string bible);
void setExportHtmlDuringNight (std::string bible, bool value);
bool getExportHtmlNotesOnHover (std::string bible);
void setExportHtmlNotesOnHover (std::string bible, bool value);
bool getExportUsfmDuringNight (std::string bible);
void setExportUsfmDuringNight (std::string bible, bool value);
bool getExportTextDuringNight (std::string bible);
void setExportTextDuringNight (std::string bible, bool value);
bool getExportOdtDuringNight (std::string bible);
void setExportOdtDuringNight (std::string bible, bool value);
bool getGenerateInfoDuringNight (std::string bible);
void setGenerateInfoDuringNight (std::string bible, bool value);
bool getExportESwordDuringNight (std::string bible);
void setExportESwordDuringNight (std::string bible, bool value);
bool getExportOnlineBibleDuringNight (std::string bible);
void setExportOnlineBibleDuringNight (std::string bible, bool value);
std::string getExportPassword (std::string bible);
void setExportPassword (std::string bible, std::string value);
bool getSecureUsfmExport (std::string bible);
void setSecureUsfmExport (std::string bible, bool value);
bool getSecureOdtExport (std::string bible);
void setSecureOdtExport (std::string bible, bool value);
std::string getExportFont (std::string bible);
void setExportFont (std::string bible, std::string value);
std::string getExportFeedbackEmail (std::string bible);
void setExportFeedbackEmail (std::string bible, std::string value);
std::string getBookOrder (std::string bible);
void setBookOrder (std::string bible, std::string value);
int getTextDirection (std::string bible);
void setTextDirection (std::string bible, int value);
std::string getTextFont (std::string bible);
void setTextFont (std::string bible, std::string value);
std::string getTextFontClient (std::string bible);
void setTextFontClient (std::string bible, std::string value);
std::string getParatextProject (std::string bible);
void setParatextProject (std::string bible, std::string value);
bool getParatextCollaborationEnabled (std::string bible);
void setParatextCollaborationEnabled (std::string bible, bool value);
int getLineHeight (std::string bible);
void setLineHeight (std::string bible, int value);
int getLetterSpacing (std::string bible);
void setLetterSpacing (std::string bible, int value);
bool getPublicFeedbackEnabled (std::string bible);
void setPublicFeedbackEnabled (std::string bible, bool value);
bool getReadFromGit (std::string bible);
void setReadFromGit (std::string bible, bool value);
bool getSendChangesToRSS (std::string bible);
void setSendChangesToRSS (std::string bible, bool value);
std::string getOdtSpaceAfterVerse (std::string bible);
void setOdtSpaceAfterVerse (std::string bible, std::string value);
bool getDailyChecksEnabled (std::string bible);
void setDailyChecksEnabled (std::string bible, bool value);
bool getOdtPoetryVersesLeft (std::string bible);
void setOdtPoetryVersesLeft (std::string bible, bool value);
bool getOdtAutomaticNoteCaller (std::string bible);
void setOdtAutomaticNoteCaller (std::string bible, bool value);

}

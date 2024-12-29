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


#pragma once


#include <config/libraries.h>


#define StyleTypeIdentifier            0 
#define StyleTypeNotUsedComment        1 
#define StyleTypeNotUsedRunningHeader  2 
#define StyleTypeStartsParagraph       3 
#define StyleTypeInlineText            4 
#define StyleTypeChapterNumber         5 
#define StyleTypeVerseNumber           6 
#define StyleTypeFootEndNote           7 
#define StyleTypeCrossreference        8 
#define StyleTypePeripheral            9 
#define StyleTypePicture               10 
#define StyleTypePageBreak             11 
#define StyleTypeTableElement          12 
#define StyleTypeWordlistElement       13 


#define IdentifierSubtypeBook                     0 
#define IdentifierSubtypeEncoding                 1 
#define IdentifierSubtypeComment                  2 
#define IdentifierSubtypeRunningHeader            3 
#define IdentifierSubtypeLongTOC                  4 
#define IdentifierSubtypeShortTOC                 5 
#define IdentifierSubtypeBookAbbrev               6 
#define IdentifierSubtypeChapterLabel             7
#define IdentifierSubtypePublishedChapterMarker   8
#define IdentifierSubtypeCommentWithEndmarker     9 
#define IdentifierSubtypePublishedVerseMarker    10


#define FootEndNoteSubtypeFootnote                 0 
#define FootEndNoteSubtypeEndnote                  1 
#define FootEndNoteSubtypeStandardContent          2 
#define FootEndNoteSubtypeContent                  3 
#define FootEndNoteSubtypeContentWithEndmarker     4 
#define FootEndNoteSubtypeParagraph                5 


#define CrossreferenceSubtypeCrossreference        0 
#define CrossreferenceSubtypeStandardContent       1 
#define CrossreferenceSubtypeContent               2 
#define CrossreferenceSubtypeContentWithEndmarker  3 


#define ParagraphSubtypeMainTitle                  0 
#define ParagraphSubtypeSubTitle                   1 
#define ParagraphSubtypeSectionHeading             2 
#define ParagraphSubtypeNormalParagraph            3 


#define PeripheralSubtypePublication     0
#define PeripheralSubtypeTableOfContents 1
#define PeripheralSubtypePreface         2
#define PeripheralSubtypeIntroduction    3
#define PeripheralSubtypeGlossary        4
#define PeripheralSubtypeConcordance     5
#define PeripheralSubtypeIndex           6
#define PeripheralSubtypeMapIndex        7
#define PeripheralSubtypeCover           8
#define PeripheralSubtypeSpine           9
#define PeripheralSubtypeGeneral        10

#define TableElementSubtypeRow             0 
#define TableElementSubtypeHeading         1 
#define TableElementSubtypeCell            2 


#define WorListElementSubtypeWordlistGlossaryDictionary  0 
#define WorListElementSubtypeHebrewWordlistEntry         1 
#define WorListElementSubtypeGreekWordlistEntry          2 
#define WorListElementSubtypeSubjectIndexEntry           3 


#define ooitOff      0 
#define ooitOn       1 
#define ooitInherit  2 
#define ooitToggle   3 


#define AlignmentLeft      0 
#define AlignmentCenter    1 
#define AlignmentRight     2 
#define AlignmentJustify   3 


#define UserBool1None                      0 
#define UserBool1PrintChapterAtFirstVerse  1 
#define UserBool1IdStartsNewPage           2 
#define UserBool1NoteAppliesToApocrypha    3 
#define UserBool1VerseRestartsParagraph    4 


#define UserBool2None                        0 
#define UserBool2IdStartsOddPage             1  // \id: whether to start an odd page number. Not implemented due to limitations in OpenDocument.
#define UserBool2ChapterInLeftRunningHeader  2  // Chapter number (\c): Whether to include it in the left running header.
#define UserBool2RunningHeaderLeft           3  // Running header (\h(#): Whether to include it in the left running header.


#define UserBool3None                          0 
#define UserBool3ChapterInRightRunningHeader   1  // Chapter number (\c): Whether to include it in the right running header.
#define UserBool3RunningHeaderRight            2  // Running header (\h(#): Whether to include it in the right running header.


#define UserInt1None               0 
#define UserInt1NoteNumbering      1  // Applies to footnote, endnote, crossreference.
#define UserInt1TableColumnNumber  2 


#define UserInt2None                   0 
#define UserInt2NoteNumberingRestart   1  // Applies to footnote, crossreference.
#define UserInt2EndnotePosition        2 


#define UserInt3None                   0 


#define UserString1None                    0 
#define UserString1NoteNumberingSequence   1 
#define UserString1WordListEntryAddition   2  // Word list entries: text to add after entry.


#define UserString2None              0 
#define UserString2DumpEndnotesHere  1  // Endnotes: dump notes upon encountering this marker.


#define UserString3None  0 


#define NoteNumbering123   0 
#define NoteNumberingAbc   1 
#define NoteNumberingUser  2 


#define NoteRestartNumberingNever         0 
#define NoteRestartNumberingEveryBook     1 
#define NoteRestartNumberingEveryChapter  2 


#define EndNotePositionAfterBook  0 
#define EndNotePositionVeryEnd    1 
#define EndNotePositionAtMarker   2 


std::string styles_logic_standard_sheet ();
std::string styles_logic_category_text (std::string category);
std::string styles_logic_type_text (int type);
std::string styles_logic_subtype_text (int type, int subtype);
bool styles_logic_fontsize_is_relevant (int type, int subtype);
bool styles_logic_italic_bold_underline_smallcaps_are_relevant (int type, int subtype);
bool styles_logic_italic_bold_underline_smallcaps_are_full (int type, int subtype);
std::string styles_logic_off_on_inherit_toggle_text (int value);
bool styles_logic_superscript_is_relevant (int type, int subtype);
bool styles_logic_paragraph_treats_are_relevant (int type, int subtype);
std::string styles_logic_alignment_text (int value);
bool styles_logic_columns_are_relevant (int type, int subtype);
bool styles_logic_color_is_relevant (int type, int subtype);
bool styles_logic_print_is_relevant (int type, int subtype);
int styles_logic_get_userbool1_function (int type, int subtype);
std::string styles_logic_get_userbool1_text (int function);
int styles_logic_get_userbool2_function (int type, int subtype);
std::string styles_logic_get_userbool2_text (int function);
int styles_logic_get_userbool3_function (int type, int subtype);
std::string styles_logic_get_userbool3_text (int function);
int styles_logic_get_userint1_function (int type, int subtype);
std::string styles_logic_note_numbering_text (int value);
int styles_logic_get_userint2_function (int type, int subtype);
std::string styles_logic_note_restart_numbering_text (int value);
std::string styles_logic_end_note_position_text (int value);
int styles_logic_get_userint3_function (int type, int subtype);
int styles_logic_get_userstring1_function (int type, int subtype);
int styles_logic_get_userstring2_function (int type, int subtype);
int styles_logic_get_userstring3_function (int type, int subtype);
bool styles_logic_starts_new_line_in_usfm (int type, int subtype);

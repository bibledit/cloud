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


#include <styles/logic.h>
#include <locale/translate.h>
#include <filter/string.h>


// The name of the "Standard" stylesheet.
std::string styles_logic_standard_sheet ()
{
  return "Standard";
}


// This contains the styles logic.
// categoryText - Returns the $category as human readable text.
std::string styles_logic_category_text (std::string category)
{
  if (category == "id")  return translate ("Identification information");
  if (category == "ith") return translate ("Introduction titles and headings");
  if (category == "ipp") return translate ("Introduction paragraphs and poetry");
  if (category == "ioe") return translate ("Introduction other elements");
  if (category == "t"  ) return translate ("Titles");
  if (category == "h"  ) return translate ("Headings");
  if (category == "cv" ) return translate ("Chapters and verses");
  if (category == "p"  ) return translate ("Paragraphs");
  if (category == "l"  ) return translate ("Lists");
  if (category == "pe" ) return translate ("Poetry elements");
  if (category == "te" ) return translate ("Table elements");
  if (category == "f"  ) return translate ("Footnotes");
  if (category == "x"  ) return translate ("Crossreferences");
  if (category == "st" ) return translate ("Special text");
  if (category == "cs" ) return translate ("Character styles");
  if (category == "sb" ) return translate ("Spacings and breaks");
  if (category == "sf" ) return translate ("Special features");
  if (category == "pm" ) return translate ("Peripheral materials");
  return translate ("Extra styles");
}


// Returns the $type as human readable text.
std::string styles_logic_type_text (int type)
{
  if (type == StyleTypeStartsParagraph) return translate ("starts a new paragraph");
  if (type == StyleTypeInlineText     ) return translate ("is inline text with endmarker");
  if (type == StyleTypePeripheral     ) return translate ("is a peripheral element");
  if (type == StyleTypePicture        ) return translate ("is a picture");
  if (type == StyleTypeWordlistElement) return translate ("is a word list element");
  return "--";
}


// This returns the $subtype as human readable text.
std::string styles_logic_subtype_text (int type, int subtype)
{
  if (type == StyleTypeStartsParagraph) {
    if (subtype == ParagraphSubtypeMainTitle)       return translate ("is a main title");
    if (subtype == ParagraphSubtypeSubTitle)        return translate ("is a subtitle");
    if (subtype == ParagraphSubtypeSectionHeading)  return translate ("is a section heading");
    if (subtype == ParagraphSubtypeNormalParagraph) return translate ("is a normal paragraph");
  }
  if (type == StyleTypeInlineText) {
  }
  if (type == StyleTypePeripheral) {
    if (subtype == PeripheralSubtypePublication    ) return translate ("starts publication data");
    if (subtype == PeripheralSubtypeTableOfContents) return translate ("starts table of contents");
    if (subtype == PeripheralSubtypePreface        ) return translate ("starts preface");
    if (subtype == PeripheralSubtypeIntroduction   ) return translate ("starts introduction");
    if (subtype == PeripheralSubtypeGlossary       ) return translate ("starts concordance");
    if (subtype == PeripheralSubtypeConcordance    ) return translate ("starts glossary");
    if (subtype == PeripheralSubtypeIndex          ) return translate ("starts index");
    if (subtype == PeripheralSubtypeMapIndex       ) return translate ("starts map index");
    if (subtype == PeripheralSubtypeCover          ) return translate ("starts cover");
    if (subtype == PeripheralSubtypeSpine          ) return translate ("starts spine");
    if (subtype == PeripheralSubtypeGeneral        ) return translate ("starts general peripheral content");
  }
  if (type == StyleTypePicture) {
  }
  if (type == StyleTypeWordlistElement) {
    if (subtype == WorListElementSubtypeWordlistGlossaryDictionary) return translate ("is a wordlist / glossary / dictionary entry");
    if (subtype == WorListElementSubtypeHebrewWordlistEntry       ) return translate ("is a Hebrew wordlist entry");
    if (subtype == WorListElementSubtypeGreekWordlistEntry        ) return translate ("is a Greek wordlist entry");
    if (subtype == WorListElementSubtypeSubjectIndexEntry         ) return translate ("is a subject index entry");
  }
  return "--";
}


// Returns true if the fontsize is relevant for $type.
bool styles_logic_fontsize_is_relevant (int type)
{
  switch (type) {
    case StyleTypeStartsParagraph : return true;
    case StyleTypePicture : return true;
    default: return false;
  }
  return false;
}


// Returns true if the italic, bold, etc. settings are relevant for $type and $subtype.
bool styles_logic_italic_bold_underline_smallcaps_are_relevant (int type)
{
  switch (type) {
    case StyleTypeStartsParagraph : return true;
    case StyleTypeInlineText      : return true;
    case StyleTypePicture         : return true;
    default: return false;
  }
  return false;
}


// Returns true if the italic, bold, etc. settings are fully applicable for $type and $subtype. Full means it also has inherit and toggle values.
bool styles_logic_italic_bold_underline_smallcaps_are_full (int type)
{
  switch (type) {
    case StyleTypeInlineText: return true;
    default: return false;
  }
  return false;
}


// Returns the $value as human readable text.
std::string styles_logic_off_on_inherit_toggle_text (int value)
{
  if (value == ooitOff    ) return translate ("Off");
  if (value == ooitOn     ) return translate ("On");
  if (value == ooitInherit) return translate ("Inherit");
  if (value == ooitToggle ) return translate ("Toggle");
  return "--";
}


// Returns true if the superscript setting is relevant for $type.
bool styles_logic_superscript_is_relevant (int type)
{
  switch (type) {
    case StyleTypeInlineText  : return true;
    default: return false;
  }
  return false;
}


// Returns true if the paragraph treats are relevant for $type.
bool styles_logic_paragraph_treats_are_relevant (int type)
{
  switch (type) {
    case StyleTypeStartsParagraph : return true;
    case StyleTypePicture : return true;
    default: return false;
  }
  return false;
}


// Returns the $value as human readable text.
std::string styles_logic_alignment_text (int value)
{
  if (value == AlignmentLeft   ) return translate ("Left align");
  if (value == AlignmentCenter ) return translate ("Center");
  if (value == AlignmentRight  ) return translate ("Right align");
  if (value == AlignmentJustify) return translate ("Justify");
  return "--";
}


// Returns true if the columns are relevant for $type and $subtype
bool styles_logic_columns_are_relevant (int type, int subtype)
{
  if (subtype) {};
  switch (type) {
    case StyleTypeStartsParagraph : return true;
    default: return false;
  }
  return false;
}


// Returns true if the color is relevant for $type.
bool styles_logic_color_is_relevant (int type)
{
  switch (type) {
    case StyleTypeInlineText  : return true;
    default: return false;
  }
  return false;
}


// Returns true if the print setting is relevant (just now: never).
bool styles_logic_print_is_relevant ()
{
  return false;
}


// Returns the function of userbool1.
int styles_logic_get_userbool1_function ()
{
  return UserBool1None;
}


// Returns the text of userbool1's function.
std::string styles_logic_get_userbool1_text (int function)
{
  switch (function) {
    case UserBool1PrintChapterAtFirstVerse: return translate ("Print chapter number at first verse");
    case UserBool1NoteAppliesToApocrypha: return translate ("Refers to the Apocrypha");
    case UserBool1VerseRestartsParagraph: return translate ("Restart paragraph");
    default: return "--";
  }
  return std::string();
}


// Returns the function of userbool2 for type and subtype
int styles_logic_get_userbool2_function (int type, int subtype)
{
  (void) type;
  (void) subtype;
  return UserBool2None;
}


// Returns the text of userbool2's function.
std::string styles_logic_get_userbool2_text (int function)
{
  switch (function) {
    case UserBool2IdStartsOddPage: return translate ("New page starts with an odd number (not implemented due to limitations in OpenDocument)");
    case UserBool2ChapterInLeftRunningHeader: return translate ("Print chapter number in the running header of the left page");
    default: return std::string();
  }
  return std::string();
}


// Returns the function of userbool3 for type and subtype
int styles_logic_get_userbool3_function (int type, int subtype)
{
  (void) type;
  (void) subtype;
  return UserBool3None;
}


// Returns the text of userbool3's function.
std::string styles_logic_get_userbool3_text (int function)
{
  switch (function) {
    case UserBool3ChapterInRightRunningHeader: return translate ("Print chapter number in the running header of the right page");
    default: return std::string();
  }
  return std::string();
}


// Returns the function of userint1.
int styles_logic_get_userint1_function ()
{
  return UserInt1None;
}


// Returns the value as human readable text for note numbering.
std::string styles_logic_note_numbering_text (int value)
{
  if (value == NoteNumbering123 ) return "1, 2, 3 ...";
  if (value == NoteNumberingAbc ) return "a, b, c ...";
  if (value == NoteNumberingUser) return translate ("User defined sequence");
  return std::to_string (value);
}


// Returns the function of userint2.
int styles_logic_get_userint2_function ()
{
  return UserInt2None;
}


// Returns the value as human readable text for when to restart the note numbering.
std::string styles_logic_note_restart_numbering_text (int value)
{
  if (value == NoteRestartNumberingNever       ) return translate ("Never");
  if (value == NoteRestartNumberingEveryBook   ) return translate ("Every book");
  if (value == NoteRestartNumberingEveryChapter) return translate ("Every chapter");
  return std::to_string (value);
}


// Returns the value as human readable text for the position of the endnotes.
std::string styles_logic_end_note_position_text (int value)
{
  if (value == EndNotePositionAfterBook) return translate ("After each book");
  if (value == EndNotePositionVeryEnd  ) return translate ("After everything else");
  if (value == EndNotePositionAtMarker ) return translate ("Upon encountering a certain marker");
  return std::to_string (value);
}


// Returns the function of userint3 for type and subtype
int styles_logic_get_userint3_function (int type, int subtype)
{
  if (type) {};
  if (subtype) {};
  return UserInt3None;
}


// Returns the function of userstring1 for type.
int styles_logic_get_userstring1_function (int type)
{
  if (type == StyleTypeWordlistElement) {
    return UserString1WordListEntryAddition;
  }
  return UserString1None;
}


// Returns the function of userstring2 for type and subtype
int styles_logic_get_userstring2_function ([[maybe_unused]]int type, [[maybe_unused]]int subtype)
{
  return UserString2None;
}


// Returns the function of userstring3 for type and subtype
int styles_logic_get_userstring3_function (int type, int subtype)
{
  if (type) {};
  if (subtype) {};
  return UserString3None;
}


// It returns true if the combination of type and subtype start a new line in well-formed USFM.
// Otherwise it returns false.
bool styles_logic_starts_new_line_in_usfm (int type, [[maybe_unused]]int subtype)
{
  switch (type) {
    case StyleTypeStartsParagraph :
    {
      return true;
    }
    case StyleTypeInlineText :
    {
      return false;
    }
    case StyleTypePeripheral :
    {
      return true;
    }
    case StyleTypePicture :
    {
      return true;
    }
    case StyleTypeWordlistElement :
    {
      return false;
    }
    default:
      return false;
  }
}


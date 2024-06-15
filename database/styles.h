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

struct Database_Styles_Item
{
  std::string marker {};
  std::string name {};
  std::string info {};
  std::string category {};
  int type {0};
  int subtype {0};
  float fontsize {12.0f};
  int italic {0};
  int bold {0};
  int underline {0};
  int smallcaps {0};
  int superscript {0};
  int justification {0};
  float spacebefore {0.0f};
  float spaceafter {0.0f};
  float leftmargin {0.0f};
  float rightmargin {0.0f};
  float firstlineindent {0.0f};
  bool spancolumns {false};
  std::string color {"#000000"};
  bool print {false};
  bool userbool1 {false};
  bool userbool2 {false};
  bool userbool3 {false};
  int userint1 {0};
  int userint2 {0};
  int userint3 {0};
  std::string userstring1 {};
  std::string userstring2 {};
  std::string userstring3 {};
  std::string backgroundcolor {"#FFFFFF"};
};


class Database_Styles
{
public:
  void create ();
  void createSheet (std::string sheet);
  std::vector <std::string> getSheets ();
  void deleteSheet (std::string sheet);
  void addMarker (std::string sheet, std::string marker);
  void deleteMarker (std::string sheet, std::string marker);
  std::map <std::string, std::string> getMarkersAndNames (std::string sheet);
  std::vector <std::string> getMarkers (std::string sheet);
  Database_Styles_Item getMarkerData (std::string sheet, std::string marker);
  void updateName (std::string sheet, std::string marker, std::string name);
  void updateInfo (std::string sheet, std::string marker, std::string info);
  void updateCategory (std::string sheet, std::string marker, std::string category);
  void updateType (std::string sheet, std::string marker, int type);
  void updateSubType (std::string sheet, std::string marker, int subtype);
  void updateFontsize (std::string sheet, std::string marker, float fontsize);
  void updateItalic (std::string sheet, std::string marker, int italic);
  void updateBold (std::string sheet, std::string marker, int bold);
  void updateUnderline (std::string sheet, std::string marker, int underline);
  void updateSmallcaps (std::string sheet, std::string marker, int smallcaps);
  void updateSuperscript (std::string sheet, std::string marker, int superscript);
  void updateJustification (std::string sheet, std::string marker, int justification);
  void updateSpaceBefore (std::string sheet, std::string marker, float spacebefore);
  void updateSpaceAfter (std::string sheet, std::string marker, float spaceafter);
  void updateLeftMargin (std::string sheet, std::string marker, float leftmargin);
  void updateRightMargin (std::string sheet, std::string marker, float rightmargin);
  void updateFirstLineIndent (std::string sheet, std::string marker, float firstlineindent);
  void updateSpanColumns (std::string sheet, std::string marker, bool spancolumns);
  void updateColor (std::string sheet, std::string marker, std::string color);
  void updatePrint (std::string sheet, std::string marker, bool print);
  void updateUserbool1 (std::string sheet, std::string marker, bool userbool1);
  void updateUserbool2 (std::string sheet, std::string marker, bool userbool2);
  void updateUserbool3 (std::string sheet, std::string marker, bool userbool3);
  void updateUserint1 (std::string sheet, std::string marker, int userint1);
  void updateUserint2 (std::string sheet, std::string marker, int userint2);
  void updateUserstring1 (std::string sheet, std::string marker, std::string userstring1);
  void updateUserstring2 (std::string sheet, std::string marker, std::string userstring2);
  void updateUserstring3 (std::string sheet, std::string marker, std::string userstring3);
  void updateBackgroundColor (std::string sheet, std::string marker, std::string color);
  void grantWriteAccess (std::string user, std::string sheet);
  void revokeWriteAccess (std::string user, std::string sheet);
  bool hasWriteAccess (std::string user, std::string sheet);
private:
  std::string databasefolder ();
  std::string sheetfolder (std::string sheet);
  std::string stylefile (std::string sheet, std::string marker);
  Database_Styles_Item read_item (std::string sheet, std::string marker);
  void write_item (std::string sheet, Database_Styles_Item & item);
  void cache_defaults ();
};

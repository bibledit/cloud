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

class Database_Styles_Item
{
public:
  Database_Styles_Item ();
  string marker {};
  string name {};
  string info {};
  string category {};
  int type {0};
  int subtype {0};
  float fontsize {0.0f};
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
  string color {};
  bool print {false};
  bool userbool1 {false};
  bool userbool2 {false};
  bool userbool3 {false};
  int userint1 {0};
  int userint2 {0};
  int userint3 {0};
  string userstring1 {};
  string userstring2 {};
  string userstring3 {};
  string backgroundcolor {};
};


class Database_Styles
{
public:
  void create ();
  void createSheet (string sheet);
  vector <string> getSheets ();
  void deleteSheet (string sheet);
  void addMarker (string sheet, string marker);
  void deleteMarker (string sheet, string marker);
  map <string, string> getMarkersAndNames (string sheet);
  vector <string> getMarkers (string sheet);
  Database_Styles_Item getMarkerData (string sheet, string marker);
  void updateName (string sheet, string marker, string name);
  void updateInfo (string sheet, string marker, string info);
  void updateCategory (string sheet, string marker, string category);
  void updateType (string sheet, string marker, int type);
  void updateSubType (string sheet, string marker, int subtype);
  void updateFontsize (string sheet, string marker, float fontsize);
  void updateItalic (string sheet, string marker, int italic);
  void updateBold (string sheet, string marker, int bold);
  void updateUnderline (string sheet, string marker, int underline);
  void updateSmallcaps (string sheet, string marker, int smallcaps);
  void updateSuperscript (string sheet, string marker, int superscript);
  void updateJustification (string sheet, string marker, int justification);
  void updateSpaceBefore (string sheet, string marker, float spacebefore);
  void updateSpaceAfter (string sheet, string marker, float spaceafter);
  void updateLeftMargin (string sheet, string marker, float leftmargin);
  void updateRightMargin (string sheet, string marker, float rightmargin);
  void updateFirstLineIndent (string sheet, string marker, float firstlineindent);
  void updateSpanColumns (string sheet, string marker, bool spancolumns);
  void updateColor (string sheet, string marker, string color);
  void updatePrint (string sheet, string marker, bool print);
  void updateUserbool1 (string sheet, string marker, bool userbool1);
  void updateUserbool2 (string sheet, string marker, bool userbool2);
  void updateUserbool3 (string sheet, string marker, bool userbool3);
  void updateUserint1 (string sheet, string marker, int userint1);
  void updateUserint2 (string sheet, string marker, int userint2);
  void updateUserstring1 (string sheet, string marker, string userstring1);
  void updateUserstring2 (string sheet, string marker, string userstring2);
  void updateUserstring3 (string sheet, string marker, string userstring3);
  void updateBackgroundColor (string sheet, string marker, string color);
  void grantWriteAccess (string user, string sheet);
  void revokeWriteAccess (string user, string sheet);
  bool hasWriteAccess (string user, string sheet);
private:
  sqlite3 * connect ();
  string databasefolder ();
  string sheetfolder (string sheet);
  string stylefile (string sheet, string marker);
  Database_Styles_Item read_item (string sheet, string marker);
  void write_item (string sheet, Database_Styles_Item & item);
  void cache_defaults ();
};

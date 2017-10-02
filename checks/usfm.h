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


#ifndef INCLUDED_CHECK_USFM_H
#define INCLUDED_CHECK_USFM_H


#include <config/libraries.h>


class Checks_Usfm
{
public:
  Checks_Usfm (string bible);
  void initialize (int book, int chapter);
  void finalize ();
  void check (string usfm);
  vector <pair<int, string>> getResults ();
private:
  // USFM and text.
  vector <string> usfmMarkersAndText;
  unsigned int usfmMarkersAndTextPointer;
  string usfmItem;
  int bookNumber;
  int chapterNumber;
  int verseNumber;
  
  // Results of the checks.
  vector <pair<int, string>> checkingResults;
  static const int displayNothing = 0;
  static const int displayCurrent = 1;
  static const int displayNext = 2;
  static const int displayFull = 3;
  
  // Stylesheet.
  vector <string> markersStylesheet;
  
  // Matching markers.
  vector <string> markersRequiringEndmarkers;
  vector <string> openMatchingMarkers;
  
  // Table of contents markers and flags.
  string longToc1Marker;
  string shortToc2Marker;
  string abbrevToc3Marker;

  // Methods.
  void malformedVerseNumber ();
  void newLineInUsfm (string usfm);
  void markerInStylesheet ();
  void malformedId ();
  void forwardSlash (string usfm);
  void widowBackSlash ();
  void matchingEndmarker ();
  void toc (string usfm);
  void addResult (string text, int modifier);
};


#endif

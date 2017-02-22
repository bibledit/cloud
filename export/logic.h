/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#ifndef INCLUDED_EXPORT_LOGIC_H
#define INCLUDED_EXPORT_LOGIC_H


#include <config/libraries.h>


class Export_Logic
{
public:
  static void scheduleAll ();
  static void scheduleTextAndBasicUsfm (string bible, bool log);
  static void scheduleUsfm (string bible, bool log);
  static void scheduleOpenDocument (string bible, bool log);
  static void scheduleInfo (string bible, bool log);
  static void scheduleHtml (string bible, bool log);
  static void scheduleWeb (string bible, bool log);
  static void scheduleWebIndex (string bible, bool log);
  static void scheduleOnlineBible (string bible, bool log);
  static void scheduleESword (string bible, bool log);
  static void scheduleQuickBible (string bible, bool log);
  static string mainDirectory ();
  static string bibleDirectory (string bible);
  static string USFMdirectory (string bible, int type);
  static string webDirectory (string bible);
  static string webBackLinkDirectory (string bible);
  static string baseBookFileName (int book);
  static string osisDirectory (string bible);
  static string osisSwordVariantDirectory (string interpreter, int complexity);
  static const int export_needed = 0;
  static const int export_text_and_basic_usfm = 1;
  static const int export_full_usfm = 2;
  static const int export_opendocument = 3;
  static const int export_info = 4;
  static const int export_html = 5;
  static const int export_web = 6;
  static const int export_web_index = 7;
  static const int export_online_bible = 8;
  static const int export_esword = 9;
  static const int export_quick_bible = 10;
  static const int export_end = 11;
private:
};


#endif

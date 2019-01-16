/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#ifndef INCLUDED_NAVIGATION_PASSAGE_H
#define INCLUDED_NAVIGATION_PASSAGE_H


#include <config/libraries.h>
#include <filter/passage.h>


class Navigation_Passage
{
public:
  static string getNavigator (void * webserver_request, string bible);
  static string getBooksFragment (void * webserver_request, string bible);
  static string getChaptersFragment (void * webserver_request, string bible, int book, int chapter);
  static string getVersesFragment (void * webserver_request, string bible, int book, int chapter, int verse);
  static string code (string bible);
  static void setBook (void * webserver_request, int book);
  static void setChapter (void * webserver_request, int chapter);
  static void setVerse (void * webserver_request, int verse);
  static void setPassage (void * webserver_request, string bible, string passage);
  static void gotoNextChapter (void * webserver_request, string bible);
  static void gotoPreviousChapter (void * webserver_request, string bible);
  static void gotoNextVerse (void * webserver_request, string bible);
  static void gotoPreviousVerse (void * webserver_request, string bible);
  static void recordHistory (void * webserver_request, int book, int chapter, int verse);
  static void goBack (void * webserver_request);
  static void goForward (void * webserver_request);
private:
  static void addSelectorLink (string& html, string id, string href, string text, bool selected);
  static Passage getNextChapter (void * webserver_request, string bible, int book, int chapter);
  static Passage getPreviousChapter (void * webserver_request, string bible, int book, int chapter);
  static Passage getNextVerse (void * webserver_request, string bible, int book, int chapter, int verse);
  static Passage getPreviousVerse (void * webserver_request, string bible, int book, int chapter, int verse);
};


#endif

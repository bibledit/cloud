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

#include <string>
#include <vector>
#include <map>
using namespace std;
#include <classes/merge.h>
#include <filter/passage.h>

class Paratext_Logic
{
public:
  static string searchProjectsFolder ();
  static vector <string> searchProjects (string projects_folder);
  static map <int, string> searchBooks (string project_path);
  static int getBook (string filename);
  static void setup (string bible, string master);
  static void copyBibledit2Paratext (string bible);
  static void copyParatext2Bibledit (string bible);
  static string ancestorPath (string bible, int book);
  static vector <string> enabledBibles ();
  static void synchronize ();
  static string synchronize (string ancestor, string bibledit, string paratext,
                             vector <string> & messages,
                             bool & merged, vector <Merge_Conflict> & conflicts);
  static string synchronizeStartText ();
  static string synchronizeReadyText ();
private:
  static string projectFolder (string bible);
  static void ancestor (string bible, int book, string usfm);
  static string ancestor (string bible, int book);
  static string journalTag (string bible, int book, int chapter);
};

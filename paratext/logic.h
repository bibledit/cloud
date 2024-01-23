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
#include <classes/merge.h>
#include <filter/passage.h>
#include <tasks/enums.h>

class Paratext_Logic
{
public:
  static std::string searchProjectsFolder ();
  static std::vector <std::string> searchProjects (std::string projects_folder);
  static std::map <int, std::string> searchBooks (std::string project_path);
  static int getBook (std::string filename);
  static void setup (std::string bible, std::string master);
  static void copyBibledit2Paratext (std::string bible);
  static void copyParatext2Bibledit (std::string bible);
  static std::string ancestorPath (std::string bible, int book);
  static std::vector <std::string> enabledBibles ();
  static void synchronize (tasks::enums::paratext_sync method);
  static std::string synchronize (std::string ancestor, std::string bibledit, std::string paratext,
                                  std::vector <std::string> & messages,
                                  std::vector <Merge_Conflict> & conflicts);
  static std::string synchronizeStartText ();
  static std::string synchronizeReadyText ();
private:
  static std::string projectFolder (std::string bible);
  static void ancestor (std::string bible, int book, std::string usfm);
  static std::string ancestor (std::string bible, int book);
  static std::string journalTag (std::string bible, int book, int chapter);
};

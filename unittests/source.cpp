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


#include <unittests/source.h>
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>
#include <iostream>
#include <set>


void test_source ()
{
  trace_unit_tests (__func__);
  
  // There should be no empty folders in the library, because git won't include them.
  // If there were such empty folders, they would not be included in the git.
  // Apart from any empty folders in the ./git folder itself,
  // and apart from the Xcode project.
  {
    string output;
    int result = filter_shell_run ("find . -type d -empty -not -path './.git/*' -not -path './cloud.xcodeproj/*'", output);
    evaluate (__LINE__, __func__, 0, result);
    evaluate (__LINE__, __func__, string(), output);
  }

  // All of the source code should include the system configuration
  // and the Bibledit configuration.
  // Reason: The configuration is important for #ifdef enclosures.
  {
    vector <string> folders_to_skip = {"unittests", "i18n", "parsewebdata", "pugixml", "jsonxx"};
    vector <string> files_to_skip = { "UriCodec.cpp", "greekstrong.cpp" };
    string folder = filter_url_create_root_path ();
    vector<string> paths;
    filter_url_recursive_scandir (folder, paths);
    vector<string> cpp_files_without_config_h;
    vector<string> cpp_files_without_config_config_h;
    for (auto path : paths) {
      string extension = filter_url_get_extension (path);
      if (extension != "cpp") continue;
      string basename = filter_url_basename (path);
      string dirname = filter_url_basename(filter_url_dirname (path));
      if (in_array (dirname, folders_to_skip)) continue;
      if (in_array (basename, files_to_skip)) continue;
      string sourcefile = filter_url_create_path(dirname, basename);
      string contents = filter_url_file_get_contents(path);
      size_t pos = contents.find("#include <config.h>");
      if (pos == string::npos) {
        cpp_files_without_config_h.push_back(sourcefile);
      }
      pos = contents.find ("#include <config/config.h>");
      if (pos == string::npos) {
        cpp_files_without_config_config_h.push_back(sourcefile);
      }
    }
    evaluate (__LINE__, __func__, 0, cpp_files_without_config_h.size());
    for (auto s : cpp_files_without_config_h) cout << s << endl;
    evaluate (__LINE__, __func__, 0, cpp_files_without_config_config_h.size());
    for (auto s : cpp_files_without_config_config_h) cout << s << endl;
  }
  
}


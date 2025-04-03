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


#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>


std::string testing_directory;


// Puts a fresh and clean copy of Bibledit into the sandbox in the testing directory.
// $displayjournal: Whether to display the journal before clearing it.
// $allowed: Text fragments allowed in the journal: Items containing these won't be displayed.
void refresh_sandbox (bool displayjournal, std::vector <std::string> allowed)
{
  // Display any old journal entries.
  if (displayjournal) {
    std::string directory = filter_url_create_path ({testing_directory, "logbook"});
    std::vector <std::string> files = filter_url_scandir (directory);
    std::string contents;
    for (const auto& file : files) {
      if (file == "gitflag") continue;
      if (!contents.empty())
        contents.append ("\n");
      contents.append (filter_url_file_get_contents (filter_url_create_path ({directory, file})));
    }
    bool display = false;
    for (const auto& allow : allowed) {
      if (contents.find (allow) == std::string::npos) display = true;
    }
    if (display) {
      std::cerr << "Expected log entries:" << std::endl;
      for (const auto& allow : allowed)
        std::cerr << allow << std::endl;
      std::cerr << "Actual log entries:" << std::endl;
      std::cout << contents << std::endl;
    }
  }
  
  // Refresh.
  std::string command = "rsync . -a --delete " + testing_directory;
  int status = system (command.c_str());
  if (status != 0) {
    std::cout << "Error while running " + command << std::endl;
    exit (status);
  }
  
  // Clear caches in memory.
  Webserver_Request request;
  request.database_config_user()->clear_cache ();
}


// Initially a version of odt2txt was used written in C.
// https://github.com/dstosberg/odt2txt
// That worked well for converting een OpenDocument to text format.
// Then there came up a need for a unit test that would catch invalid UTF8.
// LibreOffice would give an error on invalid UTF8.
// The odt2txt written in C would not given an error, it would steamroll on.
// Then a version of odf2txt written in Python was used instead.
// That version does catch invalid UTF-8.
// https://github.com/mwoehlke/odf2txt
int odf2txt (std::string odf, std::string txt) // Todo fix.
{
  const std::string script_path {filter_url_create_root_path ({"unittests", "tests", "odf2txt", "odf2txt.py"})};
  const std::string command = "python " + script_path + " " + std::move(odf) + " > " + std::move(txt) + " 2>&1";
  return system (command.c_str());
}

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


#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>
using namespace std;


string testing_directory;


// Puts a fresh and clean copy of Bibledit into the sandbox in the testing directory.
// $displayjournal: Whether to display the journal before clearing it.
// $allowed: Text fragments allowed in the journal: Items containing these won't be displayed.
void refresh_sandbox (bool displayjournal, vector <string> allowed)
{
  // Display any old journal entries.
  if (displayjournal) {
    bool output = false;
    string directory = filter_url_create_path ({testing_directory, "logbook"});
    vector <string> files = filter_url_scandir (directory);
    for (unsigned int i = 0; i < files.size (); i++) {
      if (files [i] == "gitflag") continue;
      string contents = filter_url_file_get_contents (filter_url_create_path ({directory, files [i]}));
      bool display = true;
      for (auto & allow : allowed) {
        if (contents.find (allow) != std::string::npos) display = false;
      }
      if (display) {
        std::cout << contents << std::endl;
        output = true;
      }
    }
  }
  
  // Refresh.
  string command = "rsync . -a --delete " + testing_directory;
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
// But then there was need for a unit test that would catch invalid UTF8.
// LibreOffice would give an error on invalid UTF8.
// But the odt2txt written in C would not given an error,
// but steamroll on.
// So then a version of odf2txt written in Python was going to be used.
// That version does catch invalid UTF-8.
// https://github.com/mwoehlke/odf2txt
int odf2txt (string odf, string txt)
{
  string script_path = filter_url_create_root_path ({"unittests", "tests", "odf2txt", "odf2txt.py"});
  string command = "python2.7 " + script_path + " " + odf + " > " + txt + " 2>&1";
  int ret = system (command.c_str());
  return ret;
}

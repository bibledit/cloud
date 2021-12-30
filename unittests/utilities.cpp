/*
Copyright (©) 2003-2021 Teus Benschop.

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


string testing_directory;
int error_count;


// Puts a fresh and clean copy of Bibledit into the sandbox in the testing directory.
// $displayjournal: Whether to display the journal before clearing it.
// $allowed: Text fragments allowed in the journal: Items containing these won't be displayed.
void refresh_sandbox (bool displayjournal, vector <string> allowed)
{
  // Display any old journal entries.
  if (displayjournal) {
    bool output = false;
    string directory = filter_url_create_path (testing_directory, "logbook");
    vector <string> files = filter_url_scandir (directory);
    for (unsigned int i = 0; i < files.size (); i++) {
      if (files [i] == "gitflag") continue;
      string contents = filter_url_file_get_contents (filter_url_create_path (directory, files [i]));
      bool display = true;
      for (auto & allow : allowed) {
        if (contents.find (allow) != string::npos) display = false;
      }
      if (display) {
        cout << contents << endl;
        output = true;
      }
    }
    if (output) error_count++;
  }
  
  // Refresh.
  string command = "rsync . -a --delete " + testing_directory;
  int status = system (command.c_str());
  if (status != 0) {
    cout << "Error while running " + command << endl;
    exit (status);
  }
  
  // Clear caches in memory.
  Webserver_Request request;
  request.database_config_user()->clear_cache ();
}


void error_message (int line, string func, string desired, string actual)
{
  string difference;
  if (desired.length () > 1000) {
    filter_url_file_put_contents ("/tmp/desired.txt", desired);
    filter_url_file_put_contents ("/tmp/actual.txt", actual);
    filter_shell_run ("diff /tmp/desired.txt /tmp/actual.txt", difference);
  }
  cout << "Line number:    " << line << endl;
  cout << "Function:       " << func << endl;
  if (!difference.empty ()) {
    cout << "Difference:     " << difference << endl;
  } else {
    cout << "Desired result: " << desired << endl;
    cout << "Actual result:  " << actual << endl;
  }
  cout << endl;
  error_count++;
}


bool evaluate (int line, string func, string desired, string actual)
{
  if (desired != actual) {
    error_message (line, func, desired, actual);
    return false;
  }
  return true;
}


void evaluate (int line, string func, int desired, int actual)
{
  if (desired != actual) error_message (line, func, convert_to_string (desired), convert_to_string (actual));
}


void evaluate (int line, string func, int desired, size_t actual)
{
  if (desired != (int)actual) error_message (line, func, convert_to_string (desired), convert_to_string (actual));
}


void evaluate (int line, string func, unsigned int desired, unsigned int actual)
{
  if (desired != actual) error_message (line, func, convert_to_string ((size_t)desired), convert_to_string ((size_t)actual));
}


void evaluate (int line, string func, bool desired, bool actual)
{
  if (desired != actual) error_message (line, func, desired ? "true" : "false", actual ? "true" : "false");
}


void evaluate (int line, string func, float desired, float actual)
{
  if (desired != actual) error_message (line, func, convert_to_string (desired), convert_to_string (actual));
}


void evaluate (int line, string func, vector <string> desired, vector <string> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  for (size_t i = 0; i < desired.size (); i++) {
    if (desired[i] != actual[i]) error_message (line, func, desired[i], actual[i] + " mismatch at offset " + convert_to_string (i));
  }
}


void evaluate (int line, string func, vector <int> desired, vector <int> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  for (size_t i = 0; i < desired.size (); i++) {
    if (desired[i] != actual[i]) error_message (line, func, convert_to_string (desired[i]), convert_to_string (actual[i]) + " mismatch at offset " + convert_to_string (i));
  }
}


void evaluate (int line, string func, vector <bool> desired, vector <bool> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  for (size_t i = 0; i < desired.size (); i++) {
    if (desired[i] != actual[i]) error_message (line, func, convert_to_string (desired[i]), convert_to_string (actual[i]) + " mismatch at offset " + convert_to_string (i));
  }
}


void evaluate (int line, string func, map <int, string> desired, map <int, string> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  auto desirediterator = desired.begin ();
  auto actualiterator = actual.begin ();
  for (auto iterator = desired.begin(); iterator != desired.end(); iterator++) {
    evaluate (line, func, desirediterator->first, actualiterator->first);
    evaluate (line, func, desirediterator->second, actualiterator->second);
    desirediterator++;
    actualiterator++;
  }
}


void evaluate (int line, string func, map <string, int> desired, map <string, int> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  auto desirediterator = desired.begin ();
  auto actualiterator = actual.begin ();
  for (auto iterator = desired.begin(); iterator != desired.end(); iterator++) {
    evaluate (line, func, desirediterator->first, actualiterator->first);
    evaluate (line, func, desirediterator->second, actualiterator->second);
    desirediterator++;
    actualiterator++;
  }
}


void evaluate (int line, string func, map <int, int> desired, map <int, int> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  auto desirediterator = desired.begin ();
  auto actualiterator = actual.begin ();
  for (auto iterator = desired.begin(); iterator != desired.end(); iterator++) {
    evaluate (line, func, desirediterator->first, actualiterator->first);
    evaluate (line, func, desirediterator->second, actualiterator->second);
    desirediterator++;
    actualiterator++;
  }
}


void evaluate (int line, string func, map <string, string> desired, map <string, string> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  auto desirediterator = desired.begin ();
  auto actualiterator = actual.begin ();
  for (auto iterator = desired.begin(); iterator != desired.end(); iterator++) {
    evaluate (line, func, desirediterator->first, actualiterator->first);
    evaluate (line, func, desirediterator->second, actualiterator->second);
    desirediterator++;
    actualiterator++;
  }
}


void evaluate (int line, string func, vector <pair<int, string>> desired, vector <pair<int, string>> actual)
{
  if (desired.size() != actual.size ()) {
    error_message (line, func, convert_to_string ((int)desired.size ()), convert_to_string ((int)actual.size()) + " size mismatch");
    return;
  }
  auto desirediterator = desired.begin ();
  auto actualiterator = actual.begin ();
  for (auto iterator = desired.begin(); iterator != desired.end(); iterator++) {
    evaluate (line, func, desirediterator->first, actualiterator->first);
    evaluate (line, func, desirediterator->second, actualiterator->second);
    desirediterator++;
    actualiterator++;
  }
}


void trace_unit_tests (string func)
{
  if (error_count) {
    cout << func << endl;
  }
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
  string script_path = filter_url_create_root_path ("unittests", "tests", "odf2txt", "odf2txt.py");
  string command = "python2 " + script_path + " " + odf + " > " + txt + " 2>&1";
  int ret = system (command.c_str());
  return ret;
}

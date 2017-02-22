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


#include <flate/flate.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/logs.h>
#include <locale/translate.h>


// Sets a variable (key and value) for the html template.
void Flate::set_variable (string key, string value)
{
  variables [key] = value;
}


// Enable a zone in the html template.
void Flate::enable_zone (string zone)
{
  zones [zone] = true;
}


// Add $value-s for one iteration to iterator $key.
void Flate::add_iteration (string key, map <string, string> value)
{
  // The $key is the name for the iteration,
  // where to add $value, which is a map of keys and values.
  iterations[key].push_back (value);
}


// Renders the html template.
string Flate::render (string html)
{
  string rendering;
  try {
    if (file_or_dir_exists (html)) {
      rendering = filter_url_file_get_contents (html);
      process_iterations (rendering);
      process_zones (rendering);
      process_variables (rendering);
      process_translate (rendering);
    }
  } catch (...) {
    Database_Logs::log ("Failure to process template " + html);
  }
  // Remove empty lines.
  vector <string> lines = filter_string_explode (rendering, '\n');
  rendering.clear ();
  for (auto & line : lines) {
    line = filter_string_trim (line);
    if (line.empty ()) continue;
    rendering.append (line);
    rendering.append ("\n");
  }
  // Done.
  return rendering;
}


void Flate::process_iterations (string & rendering)
{
  // Limit iteration count.
  int iteration_count = 0;
  // Start processing iterations by locating the first one.
  string beginiteration ("<!-- #BEGINITERATION");
  size_t position = rendering.find (beginiteration);
  // Iterate through the rendering till all have been dealt with.
  while ((position != string::npos) && (iteration_count < 100)) {
    iteration_count++;
    // Position where the opening tag ends.
    size_t pos = rendering.find ("-->", position);
    string iteration_start_line = rendering.substr (position, pos - position + 3);
    // Remove the opening tag for the current iteration.
    rendering.erase (position, iteration_start_line.length ());
    // Name for the current iteration.
    string name = iteration_start_line.substr (21, iteration_start_line.length () - 21 - 4);
    // Assemble the ending line for the current iteration.
    string iterationendline = "<!-- #ENDITERATION " + name + " -->";
    // Locate the ending position.
    size_t iterationendposition = rendering.find (iterationendline);
    // Process if it exists.
    if (iterationendposition != string::npos) {
      // Take the ending line out.
      rendering.erase (iterationendposition, iterationendline.length ());
      // Get and remove the inner contents of this iteration.
      string iterating_fragment = rendering.substr (position, iterationendposition - position);
      rendering.erase (position, iterationendposition - position);
      // The fragment to insert after ready iterating.
      string iterated_fragment;
      // Go through the container for the name of the current iteration.
      vector < map <string, string> > named_iterations = iterations [name];
      for (auto & named_iteration : named_iterations) {
        // Process one iteration.
        string fragment (iterating_fragment);
        for (auto & element : named_iteration) {
          fragment = filter_string_str_replace ("##" + element.first + "##", element.second, fragment);
        }
        // Add the processed fragment.
        iterated_fragment.append ("\n");
        iterated_fragment.append (fragment);
        iterated_fragment.append ("\n");
      }
      // Insert it into the rendering.
      rendering.insert (position, iterated_fragment);
    }
    // Next iteration.
    position = rendering.find (beginiteration);
  }
}


void Flate::process_zones (string& rendering)
{
  // Limit zone iterations.
  int iterations = 0;
  // Start processing zones by locating the first one.
  string beginzone ("<!-- #BEGINZONE");
  size_t position = rendering.find (beginzone);
  // Iterate through the file contents till all zones have been dealt with.
  while ((position != string::npos) && (iterations < 1000)) {
    iterations++;
    // Position where the starting zone ends.
    size_t pos = rendering.find ("-->", position);
    string zonestartline = rendering.substr (position, pos - position + 3);
    // Remove the opening tag for the current zone.
    rendering.erase (position, zonestartline.length ());
    // Name for the current zone.
    string name = zonestartline.substr (16, zonestartline.length () - 16 - 4);
    // Assemble the ending line for the current zone.
    string zoneendline = "<!-- #ENDZONE " + name + " -->";
    // Locate the ending position.
    size_t zoneendposition = rendering.find (zoneendline);
    // Process if it exists.
    if (zoneendposition != string::npos) {
      // Take the ending line out.
      rendering.erase (zoneendposition, zoneendline.length ());
      // If the zone has not been enabled, remove all its contents within.
      if (zones.count (name) == 0) {
        rendering.erase (position, zoneendposition - position);
      }
    }
    // Next zone.
    position = rendering.find (beginzone);
  }
}


void Flate::process_variables (string& rendering)
{
  // Limit variable iterations.
  int iterations = 0;
  // Start processing variables by locating the first one.
  size_t position = rendering.find ("##");
  // Iterate through the contents till all variables have been dealt with.
  while ((position != string::npos) && (iterations < 1000)) {
    iterations++;
    bool correct = true;
    // Check that this is a correct position: It should not have hashes nearby.
    if (correct) if (position > 0) if (rendering.substr (position - 1, 1) == "#") {
      correct = false;
    }
    if (correct) if (position < rendering.size ()) if (rendering.substr (position + 2, 1) == "#") {
      correct = false;
    }
    // Position where the variable ends.
    size_t pos = rendering.find ("##", position + 1);
    if (pos == string::npos) pos = position + 4;
    // Name for the variable zone.
    string name = rendering.substr (position + 2, pos - position - 2);
    // No new line in the variable name.
    if (correct) if (name.find ("\n") != string::npos) correct = false;
    if (correct) {
      // Take the variable out.
      rendering.erase (position, name.length () + 4);
      // Insert the replacement.
      rendering.insert (position, variables [name]);
    }
    // Next zone.
    position = rendering.find ("##", position + 1);
  }
}


void Flate::process_translate (string& rendering)
{
  // Clean up the "translate" (gettext) calls.
  rendering = filter_string_str_replace ("translate (", "translate(", rendering);
  // Gettext markup.
  string gettextopen = "translate(\"";
  string gettextclose = "\")";
  // Limit gettext iterations.
  int iterations = 0;
  // Start processing variables by locating the first one.
  size_t position = rendering.find (gettextopen);
  // Iterate through the contents till all gettext calls have been dealt with.
  while ((position != string::npos) && (iterations < 1000)) {
    iterations++;
    // Remove the gettext opener.
    rendering.erase (position, gettextopen.length());
    // Position where the gettext call ends.
    size_t pos = rendering.find (gettextclose, position);
    if (pos != string::npos) {
      // Take the gettext closer out.
      rendering.erase (pos, gettextclose.length());
      // The English string.
      string english = rendering.substr (position, pos - position);
      // Take the English out.
      rendering.erase (position, pos - position);
      // Insert the localization.
      rendering.insert (position, translate (english));
    }
    // Next gettext call.
    position = rendering.find (gettextopen);
  }
}

/*
Copyright (©) 2003-2026 Teus Benschop.

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
void Flate::set_variable (std::string key, std::string value)
{
  variables [key] = value;
}


// Enable a zone in the html template.
void Flate::enable_zone (std::string zone)
{
  zones [zone] = true;
}


// Add $value-s for one iteration to iterator $key.
void Flate::add_iteration (std::string key, std::map <std::string, std::string> value)
{
  // The $key is the name for the iteration,
  // where to add $value, which is a map of keys and values.
  iterations[key].push_back (value);
}


// Renders the html template.
std::string Flate::render (std::string html)
{
  std::string rendering;
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
  std::vector <std::string> lines = filter::string::explode (rendering, '\n');
  rendering.clear ();
  for (auto & line : lines) {
    line = filter::string::trim (line);
    if (line.empty ()) continue;
    rendering.append (line);
    rendering.append ("\n");
  }
  // Done.
  return rendering;
}


void Flate::process_iterations (std::string & rendering)
{
  // Limit iteration count.
  int iteration_count = 0;
  // Start processing iterations by locating the first one.
  std::string beginiteration ("<!-- #BEGINITERATION");
  size_t position = rendering.find (beginiteration);
  // Iterate through the rendering till all have been dealt with.
  while ((position != std::string::npos) && (iteration_count < 100)) {
    iteration_count++;
    // Position where the opening tag ends.
    size_t pos = rendering.find ("-->", position);
    std::string iteration_start_line = rendering.substr (position, pos - position + 3);
    // Remove the opening tag for the current iteration.
    rendering.erase (position, iteration_start_line.length ());
    // Name for the current iteration.
    std::string name = iteration_start_line.substr (21, iteration_start_line.length () - 21 - 4);
    // Assemble the ending line for the current iteration.
    std::string iterationendline = "<!-- #ENDITERATION " + name + " -->";
    // Locate the ending position.
    size_t iterationendposition = rendering.find (iterationendline);
    // Process if it exists.
    if (iterationendposition != std::string::npos) {
      // Take the ending line out.
      rendering.erase (iterationendposition, iterationendline.length ());
      // Get and remove the inner contents of this iteration.
      std::string iterating_fragment = rendering.substr (position, iterationendposition - position);
      rendering.erase (position, iterationendposition - position);
      // The fragment to insert after ready iterating.
      std::string iterated_fragment;
      // Go through the container for the name of the current iteration.
      std::vector < std::map <std::string, std::string> > named_iterations = iterations [name];
      for (auto & named_iteration : named_iterations) {
        // Process one iteration.
        std::string fragment (iterating_fragment);
        for (auto & element : named_iteration) {
          fragment = filter::string::replace ("##" + element.first + "##", element.second, fragment);
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


void Flate::process_zones (std::string& rendering)
{
  // Limit zone iterations.
  int zone_iteration_count = 0;
  // Start processing zones by locating the first one.
  std::string beginzone ("<!-- #BEGINZONE");
  size_t position = rendering.find (beginzone);
  // Iterate through the file contents till all zones have been dealt with.
  while ((position != std::string::npos) && (zone_iteration_count < 1000)) {
    zone_iteration_count++;
    // Position where the starting zone ends.
    size_t pos = rendering.find ("-->", position);
    std::string zonestartline = rendering.substr (position, pos - position + 3);
    // Remove the opening tag for the current zone.
    rendering.erase (position, zonestartline.length ());
    // Name for the current zone.
    std::string name = zonestartline.substr (16, zonestartline.length () - 16 - 4);
    // Assemble the ending line for the current zone.
    std::string zoneendline = "<!-- #ENDZONE " + name + " -->";
    // Locate the ending position.
    size_t zoneendposition = rendering.find (zoneendline);
    // Process if it exists.
    if (zoneendposition != std::string::npos) {
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


void Flate::process_variables (std::string& rendering)
{
  // Limit variable iterations.
  int variable_iteration_count = 0;
  // Start processing variables by locating the first one.
  size_t position = rendering.find ("##");
  // Iterate through the contents till all variables have been dealt with.
  while ((position != std::string::npos) && (variable_iteration_count < 1000)) {
    variable_iteration_count++;
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
    if (pos == std::string::npos) pos = position + 4;
    // Name for the variable zone.
    std::string name = rendering.substr (position + 2, pos - position - 2);
    // No new line in the variable name.
    if (correct) if (name.find ("\n") != std::string::npos) correct = false;
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


void Flate::process_translate (std::string& rendering)
{
  // Clean up the "translate" (gettext) calls.
  rendering = filter::string::replace ("translate (", "translate(", rendering);
  // Gettext markup.
  std::string gettextopen = R"(translate(")";
  std::string gettextclose = R"("))";
  // Limit gettext iterations.
  int iteration_counter { 0 };
  // Start processing variables by locating the first one.
  size_t position = rendering.find (gettextopen);
  // Iterate through the contents till all gettext calls have been dealt with.
  while ((position != std::string::npos) && (iteration_counter < 1000)) {
    iteration_counter++;
    // Remove the gettext opener.
    rendering.erase (position, gettextopen.length());
    // Position where the gettext call ends.
    size_t pos = rendering.find (gettextclose, position);
    if (pos != std::string::npos) {
      // Take the gettext closer out.
      rendering.erase (pos, gettextclose.length());
      // The English string.
      std::string english = rendering.substr (position, pos - position);
      // Take the English out.
      rendering.erase (position, pos - position);
      // Insert the localization.
      rendering.insert (position, translate (english));
    }
    // Next gettext call.
    position = rendering.find (gettextopen);
  }
}

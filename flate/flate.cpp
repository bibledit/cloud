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


#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <flate/flate.h>
#include <locale/translate.h>


// Sets a variable (key and value) for the html template.
void Flate::set_variable(std::string key, std::string value)
{
    variables[std::move(key)] = std::move(value);
}


// Enable a zone in the HTML template.
void Flate::enable_zone(const std::string& zone)
{
    zones[zone] = true;
}


// Add $value-s for one iteration to iterator $key.
void Flate::add_iteration(std::string key, std::map<std::string, std::string> value)
{
    // The $key is the name for the iteration,
    // where to add $value, which is a map of keys and values.
    iterations[std::move(key)].emplace_back(std::move(value));
}


// Renders the HTML template.
std::string Flate::render(const std::string& html)
{
    std::string rendering;
    try
    {
        if (file_or_dir_exists(html))
        {
            rendering = filter_url_file_get_contents(html);
            process_iterations(rendering);
            process_zones(rendering);
            process_variables(rendering);
            process_translate(rendering);
        }
    }
    catch (...)
    {
        Database_Logs::log("Failure to process template " + html);
    }
    // Remove empty lines.
    std::vector<std::string> lines = filter::string::explode(rendering, '\n');
    rendering.clear();
    for (auto& line : lines)
    {
        line = filter::string::trim(line);
        if (line.empty()) continue;
        rendering.append(line);
        rendering.append("\n");
    }
    // Done.
    return rendering;
}


void Flate::process_iterations(std::string& rendering)
{
    // Limit iteration count.
    int iteration_count = 0;
    // Start processing iterations by locating the first one.
    std::string begin_iteration("<!-- #BEGINITERATION");
    size_t position = rendering.find(begin_iteration);
    // Iterate through the rendering till all have been dealt with.
    while (position != std::string::npos and iteration_count < 100)
    {
        iteration_count++;
        // Position where the opening tag ends.
        size_t pos = rendering.find("-->", position);
        std::string iteration_start_line = rendering.substr(position, pos - position + 3);
        // Remove the opening tag for the current iteration.
        rendering.erase(position, iteration_start_line.length());
        // Name for the current iteration.
        std::string name = iteration_start_line.substr(21, iteration_start_line.length() - 21 - 4);
        // Assemble the ending line for the current iteration.
        std::string iteration_end_line = "<!-- #ENDITERATION " + name + " -->";
        // Locate the ending position and process if it exists.
        if (size_t iteration_end_position = rendering.find(iteration_end_line);
            iteration_end_position != std::string::npos)
        {
            // Take the ending line out.
            rendering.erase(iteration_end_position, iteration_end_line.length());
            // Get and remove the inner contents of this iteration.
            std::string iterating_fragment = rendering.substr(position, iteration_end_position - position);
            rendering.erase(position, iteration_end_position - position);
            // The fragment to insert after ready iterating.
            std::string iterated_fragment;
            // Go through the container for the name of the current iteration.
            std::vector<std::map<std::string, std::string>> named_iterations = iterations[name];
            std::ranges::for_each(named_iterations,
                [&iterating_fragment, &iterated_fragment] (auto& named_iteration){
                // Process one iteration.
                std::string fragment(iterating_fragment);
                std::ranges::for_each(named_iteration, [&fragment](auto& element){
                    fragment = filter::string::replace("##" + element.first + "##", element.second, fragment);
                });
                // Add the processed fragment.
                iterated_fragment.append("\n");
                iterated_fragment.append(fragment);
                iterated_fragment.append("\n");
            });
            // Insert it into the rendering.
            rendering.insert(position, iterated_fragment);
        }
        // Next iteration.
        position = rendering.find(begin_iteration);
    }
}


void Flate::process_zones(std::string& rendering) const
{
    // Limit zone iterations.
    int zone_iteration_count = 0;
    // Start processing zones by locating the first one.
    const std::string begin_zone("<!-- #BEGINZONE");
    size_t position = rendering.find(begin_zone);
    // Iterate through the file contents till all zones have been dealt with.
    while (position != std::string::npos and zone_iteration_count < 1000)
    {
        zone_iteration_count++;
        // Position where the starting zone ends.
        const size_t pos = rendering.find("-->", position);
        std::string zone_start_line = rendering.substr(position, pos - position + 3);
        // Remove the opening tag for the current zone.
        rendering.erase(position, zone_start_line.length());
        // Name for the current zone.
        std::string name = zone_start_line.substr(16, zone_start_line.length() - 16 - 4);
        // Assemble the ending line for the current zone.
        std::string zone_end_line = "<!-- #ENDZONE " + name + " -->";
        // Locate the ending position and process if it exists.
        if (const size_t zone_end_pos = rendering.find(zone_end_line);
            zone_end_pos != std::string::npos)
        {
            // Take the ending line out.
            rendering.erase(zone_end_pos, zone_end_line.length());
            // If the zone has not been enabled, remove all its contents within.
            if (not zones.contains(name))
            {
                rendering.erase(position, zone_end_pos - position);
            }
        }
        // Next zone.
        position = rendering.find(begin_zone);
    }
}


void Flate::process_variables(std::string& rendering)
{
    // Limit variable iterations.
    int variable_iteration_count = 0;
    // Start processing variables by locating the first one.
    size_t position = rendering.find("##");
    // Iterate through the contents till all variables have been dealt with.
    while (position != std::string::npos and variable_iteration_count < 1000)
    {
        variable_iteration_count++;
        bool correct = true;
        // Check that this is a correct position: It should not have hashes nearby.
        if (position > 0 and rendering.substr(position - 1, 1) == "#")
            correct = false;
        if (correct and position < rendering.size() and rendering.substr(position + 2, 1) == "#")
            correct = false;
        // Position where the variable ends.
        size_t pos = rendering.find("##", position + 1);
        if (pos == std::string::npos)
            pos = position + 4;
        // Name for the variable zone.
        std::string name = rendering.substr(position + 2, pos - position - 2);
        // No new line in the variable name.
        if (correct and name.find('\n') != std::string::npos)
            correct = false;
        if (correct)
        {
            // Take the variable out.
            rendering.erase(position, name.length() + 4);
            // Insert the replacement.
            rendering.insert(position, variables[name]);
        }
        // Next zone.
        position = rendering.find("##", position + 1);
    }
}


void Flate::process_translate(std::string& rendering)
{
    // Clean up the "translate" (gettext) calls.
    rendering = filter::string::replace("translate (", "translate(", rendering);
    // Gettext markup.
    const std::string gettext_open = R"(translate(")";
    const std::string gettext_close = R"("))";
    // Limit number of gettext iterations.
    int max_iterations{1000};
    // Start processing variables by locating the first one.
    size_t position = rendering.find(gettext_open);
    // Iterate through the contents till all gettext calls have been dealt with.
    while (position != std::string::npos and max_iterations--)
    {
        // Remove the gettext opener.
        rendering.erase(position, gettext_open.length());
        // Position where the gettext call ends.
        if (const size_t pos = rendering.find(gettext_close, position); pos != std::string::npos)
        {
            // Take the gettext closer out.
            rendering.erase(pos, gettext_close.length());
            // The English string.
            const std::string english = rendering.substr(position, pos - position);
            // Take the English out.
            rendering.erase(position, pos - position);
            // Insert the localization.
            rendering.insert(position, translate(english));
        }
        // Next gettext call.
        position = rendering.find(gettext_open);
    }
}

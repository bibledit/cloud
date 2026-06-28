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
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <journal/logic.h>

#include <utility>


// Bibledit no longer uses a database for storing the journal.
// Reasons that a database is no longer used:
// 1. Simpler system.
// 2. Android has VACUUM errors due to a locked database.


namespace database::logs {
// The folder where to store the records.
std::string folder()
{
    return filter_url_create_root_path({"logbook"});
}


// Records a journal entry.
void log(std::string description, const int level)
{
    // Trim spaces.
    description = filter::string::trim(description);
    // Discard empty line.
    if (description.empty()) return;
    // Truncate very long entry.

    constexpr std::size_t maximum_length {50000};
    if (const std::size_t length = description.length(); length > maximum_length)
    {
        description.erase (50000);
        description.append ("... This entry was too large and has been truncated: " + std::to_string(length) + " bytes");
    }

    // Save this logbook entry to a filename with seconds and microseconds.
    const std::string seconds = std::to_string(filter::date::seconds_since_epoch());
    const std::string time = seconds + filter::string::fill(std::to_string(filter::date::numerical_microseconds()), 8, '0');
    const std::string file = filter_url_create_path({folder(), time});
    // The microseconds granularity depends on the platform.
    // On Windows it is lower than on Linux.
    // There may be the rare case of more than one entry per file.
    // Append the data so it won't overwrite an earlier entry.
    if (file_or_dir_exists(file))
        description.insert(0, " | ");
    else
        description.insert(0, std::to_string(level) + " ");
    filter_url_file_put_contents_append(file, description);
#ifdef HAVE_WINDOWS
    // Delay to cover for lower usec granularity on Windows.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
}


// Records an extended journal entry.
void log(std::string subject, const std::string& body, const int level)
{
    std::string description {std::move(subject)};
    description.append("\n");
    description.append(body);
    log(description, level);
}


void rotate()
{
    std::string directory = folder();
    std::vector<std::string> files = filter_url_scandir(directory);


    // Timestamp for removing older records, depending on whether it's a tiny journal.
#ifdef HAVE_TINY_JOURNAL
    const int old_timestamp = filter::date::seconds_since_epoch() - 14400;
#else
    const int old_timestamp = filter::date::seconds_since_epoch() - 6 * 86400;
#endif


    // Limit the journal entry count in the filesystem.
    // This speeds up subsequent reading of the journal by the users.
    // In previous versions of Bibledit, there were certain conditions
    // that led to an infinite loop, as had been noticed at times,
    // and this quickly exhausted the available inodes on the filesystem.
#ifdef HAVE_TINY_JOURNAL
    const int limit_file_count = static_cast<int>(files.size() - 200);
#else
    const int limit_file_count = static_cast<int>(files.size() - 2000);
#endif


    bool filtered_entries = false;
    for (unsigned int i = 0; i < files.size(); ++i)
    {
        const std::string path = filter_url_create_path({directory, files.at(i)});

        // Limit the number of journal entries.
        if (static_cast<int>(i) < limit_file_count)
        {
            filter_url_unlink(path);
            continue;
        }

        // Remove expired entries.
        if (const int timestamp = filter::string::convert_to_int(files[i].substr(0, 10));
            timestamp < old_timestamp)
        {
            filter_url_unlink(path);
            continue;
        }

        // Filtering of certain entries.
        if (const std::string entry = filter_url_file_get_contents(path);
            journal_logic_filter_entry(entry))
        {
            filtered_entries = true;
            filter_url_unlink(path);
        }
    }

    if (filtered_entries)
        log(journal_logic_filtered_message());
}


// Get the logbook entries.
std::vector<std::string> get(std::string& last_filename)
{
    last_filename = "0";

    // Read the journal records from the filesystem.
    const std::vector<std::string> files = filter_url_scandir(folder());
    for (const auto & file : filter_url_scandir(folder()))
    {
        // Last second gets updated based on the filename.
        last_filename = file;
    }

    // Done.
    return files;
}


// Gets journal entry more recent than "filename".
// Updates "filename" to the item it got.
std::string next(std::string& filename)
{
    for (const auto & file : filter_url_scandir(folder()))
    {
        if (file > filename)
        {
            filename = file;
            return file;
        }
    }
    return {};
}


// Clears all journal entries.
void clear()
{
    for (const std::string directory = folder();
        const auto& file : filter_url_scandir(directory))
    {
        filter_url_unlink(filter_url_create_path({directory, file}));
    }
    log("The journal was cleared");
}
}

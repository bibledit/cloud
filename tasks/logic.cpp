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


#include <tasks/logic.h>
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>


// Folder where the tasks are stored.
std::string tasks_logic_folder()
{
    return filter_url_create_root_path({"processes"});
}


// Queue task $command to run later, with $parameters for that task.
void tasks_logic_queue(std::string command, std::vector<std::string> parameters)
{
    // The file on disk will contain the command on the first line,
    // and any parameters on the following lines, one parameters per line.
    std::vector<std::string> lines;
    lines.push_back(command);
    lines.insert(lines.end(), parameters.begin(), parameters.end());
    // The filename to write to contains seconds and microseconds.
    std::string seconds = std::to_string(filter::date::get_seconds_since_epoch());
    std::string time = seconds + filter::string::fill(std::to_string(filter::date::get_microseconds_within_second()), 8,
                                                      '0');
    std::string file = filter_url_create_path({tasks_logic_folder(), time});
    // On Windows the microtime is not fine enough.
    // This leads to one task overwriting a previous one in case it is queued immediately after.
    // Deal with that problem here: Ensure the filename is unique.
    file = filter_url_unique_path(file);
    // Save it.
    command = filter::string::implode(lines, "\n");
    filter_url_file_put_contents(file, command);
}


// If $command and $parameters are queued as a task, the function returns true.
// Else it returns false.
// It looks for an exact match.
// Parameters left out are not checked.
bool tasks_logic_queued(std::string command, std::vector<std::string> parameters)
{
    // The lines to look for consist of the command followed by the parameters.
    std::vector<std::string> search(parameters);
    search.insert(search.begin(), command);
    // Go through all queued tasks.
    std::vector<std::string> files = filter_url_scandir(tasks_logic_folder());
    for (auto& file : files)
    {
        // Read the task's contents.
        std::string contents = filter_url_file_get_contents(filter_url_create_path({tasks_logic_folder(), file}));
        std::vector<std::string> lines = filter::string::explode(contents, '\n');
        if (lines.empty()) return false;
        // Look for a match.
        bool match = true;
        for (size_t i = 0; i < search.size(); i++)
        {
            if (i < lines.size())
            {
                if (search[i] != lines[i]) match = false;
            }
            else
            {
                match = false;
            }
        }
        if (match) return true;
    }
    // No match found.
    return false;
}


std::string to_string(const tasks::enums::task& task)
{
    switch (task)
    {
    case tasks::enums::task::none:                              return "none";
    case tasks::enums::task::rotate_journal:                    return "rotate journal";
    case tasks::enums::task::receive_email:                     return "receive email";
    case tasks::enums::task::send_email:                        return "send email";
    case tasks::enums::task::reindex_bibles:                    return "reindex bibles";
    case tasks::enums::task::reindex_notes:                     return "reindex notes";
    case tasks::enums::task::create_css:                        return "create css";
    case tasks::enums::task::import_bible:                      return "import bible";
    case tasks::enums::task::import_resource:                   return "import resource";
    case tasks::enums::task::compare_usfm:                      return "compare usfm";
    case tasks::enums::task::maintain_database:                 return "maintain database";
    case tasks::enums::task::clean_tmp_files:                   return "clean tmp files";
    case tasks::enums::task::link_git_repository:               return "link git repository";
    case tasks::enums::task::send_receive_bibles:               return "send receive bibles";
    case tasks::enums::task::sync_bibles:                       return "sync bibles";
    case tasks::enums::task::sync_notes:                        return "sync notes";
    case tasks::enums::task::sync_settings:                     return "sync settings";
    case tasks::enums::task::sync_changes:                      return "sync changes";
    case tasks::enums::task::sync_files:                        return "sync files";
    case tasks::enums::task::sync_resources:                    return "sync resources";
    case tasks::enums::task::clean_demo:                        return "clean demo";
    case tasks::enums::task::convert_bible_to_resource:         return "convert bible to resource";
    case tasks::enums::task::convert_resource_to_bible:         return "convert resource to bible";
    case tasks::enums::task::notes_statistics:                  return "notes statistics";
    case tasks::enums::task::generate_changes:                  return "generate changes";
    case tasks::enums::task::check_bible:                       return "check bible";
    case tasks::enums::task::export_all:                        return "export all";
    case tasks::enums::task::export_text_usfm:                  return "export text usfm";
    case tasks::enums::task::export_usfm:                       return "export usfm";
    case tasks::enums::task::export_odt:                        return "export odt";
    case tasks::enums::task::export_info:                       return "export info";
    case tasks::enums::task::export_html:                       return "export html";
    case tasks::enums::task::export_web_main:                   return "export web main";
    case tasks::enums::task::export_web_index:                  return "export web index";
    case tasks::enums::task::export_online_bible:               return "export online bible";
    case tasks::enums::task::export_esword:                     return "export esword";
    case tasks::enums::task::setup_paratext:                    return "setup paratext";
    case tasks::enums::task::sync_paratext:                     return "sync paratext";
    case tasks::enums::task::refresh_sword_modules:             return "refresh sword modules";
    case tasks::enums::task::install_sword_module:              return "install sword module";
    case tasks::enums::task::update_sword_modules:              return "update sword modules";
    case tasks::enums::task::list_usfm_resources:               return "list usfm resources";
    case tasks::enums::task::create_sample_bible:               return "create sample bible";
    case tasks::enums::task::cache_resources:                   return "cache resources";
    case tasks::enums::task::refresh_web_resources:             return "refresh web resources";
    case tasks::enums::task::produce_bibles_transferfile:       return "produce bibles transferfile";
    case tasks::enums::task::import_bibles_transferfile:        return "import bibles transferfile";
    case tasks::enums::task::produce_notes_transferfile:        return "produce notes transferfile";
    case tasks::enums::task::import_notes_transferfile:         return "import notes transferfile";
    case tasks::enums::task::produce_resources_transferfile:    return "produce resources transferfile";
    case tasks::enums::task::import_resources_transferfile:     return "import resources transferfile";
    case tasks::enums::task::delete_changes:                    return "delete changes";
    case tasks::enums::task::clear_caches:                      return "clear caches";
    case tasks::enums::task::trim_caches:                       return "trim caches";
    case tasks::enums::task::create_empty_bible:                return "create empty bible";
    case tasks::enums::task::import_bible_images:               return "import bible images";
    case tasks::enums::task::get_google_access_token:           return "get google access token";
    default:                                                    return "";
    }
    return "";
}


struct Task
{
    tasks::enums::task task{tasks::enums::task::none};
    std::vector<std::string> parameters;
    constexpr auto operator<=>(const Task&) const = default;
};


static std::ostream& operator<<(std::ostream& os, const Task& task) noexcept
{
    os << std::quoted(to_string(task.task));
    std::ranges::for_each(task.parameters, [&os]<typename P>(P&& parameter)
    {
       os << " " << std::quoted(std::forward<P>(parameter));
    });
    return os;
}


static std::deque<Task> task_queue{};
static std::mutex queue_mutex{};


void tasks_logic_queue(const tasks::enums::task task, std::vector<std::string> parameters)
{
    std::scoped_lock lock(queue_mutex);
    task_queue.emplace_back(task, std::move(parameters));
}


bool tasks_logic_queued(const tasks::enums::task task, std::vector<std::string> parameters)
{
    const Task query_task{
        .task = task,
        .parameters = std::move(parameters)
    };
    std::scoped_lock lock(queue_mutex);
    return std::ranges::find(task_queue, query_task) != task_queue.cend();
}

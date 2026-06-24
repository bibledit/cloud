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


#include <database/ipc.h>
#include <database/logic.h>
#include <filter/date.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>


// Database resilience: Stored in plain file system.


static std::string folder()
{
    return filter_url_create_root_path({database_logic_databases(), "ipc"});
}


static std::string file(const std::string& file)
{
    return filter_url_create_path({folder(), file});
}


// Reads most fields of all data in this database.
// Returns a container with the above.
// The fields are those that can be obtained from the name of the files.
// One file is one entry in the database.
// The filename looks like this: rowid__user__channel__command
static std::vector<Database_Ipc_Item> read_data()
{
    std::vector<Database_Ipc_Item> data;
    const std::vector<std::string> files = filter_url_scandir(folder());
    std::ranges::for_each(files, [&](const std::string& file)
    {
        if (const auto explosion = filter::string::explode(file, '_'); explosion.size() == 7)
        {
            Database_Ipc_Item item {
                .file = file,
                .rowid = filter::string::convert_to_int(explosion.at(0)),
                .user = explosion.at(2),
                .channel = explosion.at(4),
                .command = explosion.at(6),
            };
            data.emplace_back(std::move(item));
        }
    });
    return data;
}


static void write_record(const int rowid, const std::string& user, const std::string& channel, const std::string& command, const std::string& message)
{
    std::string filename = std::to_string(rowid) + "__" + user + "__" + channel + "__" + command;
    filename = file(filename);
    filter_url_file_put_contents(filename, message);
}


// Returns the next available row identifier.
static int get_next_id(const std::vector<Database_Ipc_Item>& data)
{
    auto&& range = data | std::views::transform(&Database_Ipc_Item::rowid);
    const auto it = std::ranges::max_element(range);
    int id = it != range.end() ? *it : 0;
    ++id;
    return id;
}


Database_Ipc::Database_Ipc(Webserver_Request& webserver_request) :
    m_webserver_request(webserver_request)
{
}


void Database_Ipc::trim()
{
    const std::vector<Database_Ipc_Item> data = read_data();
    std::ranges::for_each(data, [](const auto& record)
    {
        if (record.user.empty())
            delete_message(record.rowid);
    });
    const int now = filter::date::seconds_since_epoch();
    std::vector<std::string> files = filter_url_scandir(folder());
    std::ranges::for_each(files, [now] (const std::string& item)
    {
        const std::string path = file(item);
        if (const int modification_time = filter_url_file_modification_time(path);
            now - modification_time > 3600)
            filter_url_unlink(path);
    });
}


void Database_Ipc::store_message(const std::string& user, const std::string& channel, const std::string& command, const std::string& message)
{
    // Load entire database into memory.
    const std::vector<Database_Ipc_Item> data = read_data();

    // Write new information.
    {
        const int rowid = get_next_id(data);
        write_record(rowid, user, channel, command, message);
    }

    // Gather information about records to delete and delete those.
    // Do this after writing new data, to be sure there is always some data on disk.
    if (channel.empty())
    {
        const auto match = [&](auto&& record)
        {
            return record.user == user and record.channel == channel and record.command == command;
        };
        auto&& row_ids = data | std::views::filter(match) | std::views::transform(&Database_Ipc_Item::rowid);
        std::ranges::for_each(row_ids, [](const auto row_id)
        {
            delete_message(row_id);
        });
    }
}


// Retrieves a message if there is any.
// Returns an object with the data.
// The rowid is 0 if there was nothing,
// Else the object's properties are set properly.
Database_Ipc_Message Database_Ipc::retrieve_message(int id, const std::string& user, const std::string& channel, const std::string& command)
{
    int highest_id = 0;
    std::string hit_channel;
    std::string hit_command;
    std::string hit_message;
    for (auto& [record_file, record_id, record_user, record_channel, record_command] : read_data())
    {
        // Selection condition 1: The database record has a message identifier younger than the calling identifier.
        if (record_id > id)
        {
            // Selection condition 2: Channel matches calling channel, or empty channel.
            if (record_channel == channel or record_channel.empty())
            {
                // Selection condition 3: Record user matches calling user, or empty user.
                if (record_user == user or record_user.empty())
                {
                    // Selection condition 4: Matching command.
                    if (record_command == command)
                    {
                        if (record_id > highest_id)
                        {
                            highest_id = record_id;
                            hit_channel = record_channel;
                            hit_command = record_command;
                            hit_message = filter_url_file_get_contents(file(record_file));
                        }
                    }
                }
            }
        }
    }
    Database_Ipc_Message message;
    if (highest_id)
    {
        message.id = highest_id;
        message.channel = hit_channel;
        message.command = hit_command;
        message.message = hit_message;
    }
    return message;
}


void Database_Ipc::delete_message(const int id)
{
    const std::vector<Database_Ipc_Item> data = read_data();
    std::ranges::for_each(data, [id](const auto& record)
    {
        if (record.rowid == id)
            filter_url_unlink(file(record.file));

    });
}


Database_Ipc_Message Database_Ipc::get_note()
{
    const std::string& user = m_webserver_request.session_logic()->get_username();

    int highest_id = 0;
    std::string hit_message;

    const  std::vector<Database_Ipc_Item> data = read_data();
    std::ranges::for_each(data, [&](const auto& record)
    {
        // Conditions: Command is "opennote", and matching user.
        if (record.command == "opennote" and record.user == user and record.rowid > highest_id)
        {
            highest_id = record.rowid;
            hit_message = filter_url_file_get_contents(file(record.file));
        }
    });

    Database_Ipc_Message note;
    if (highest_id)
    {
        note.id = highest_id;
        note.message = hit_message;
    }

    return note;
}


bool Database_Ipc::get_notes_alive()
{
    const std::string& user = m_webserver_request.session_logic()->get_username();

    int highest_id = 0;
    std::string hit_message;

    const std::vector<Database_Ipc_Item> data = read_data();
    std::ranges::for_each(data, [&](const auto& record)
    {
        // Conditions: Command is "notesalive", and matching user.
        if (record.command == "notesalive" and record.user == user and record.rowid > highest_id)
        {
            highest_id = record.rowid;
            hit_message = filter_url_file_get_contents(file(record.file));
        }
    });

    if (highest_id)
        return filter::string::convert_to_bool(hit_message);

    return false;
}

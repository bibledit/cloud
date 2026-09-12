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


#include <bb/import_run.h>
#include <bb/logic.h>
#include <changes/logic.h>
#include <changes/modifications.h>
#include <checks/run.h>
#include <client/logic.h>
#include <collaboration/link.h>
#include <compare/compare.h>
#include <database/cache.h>
#include <database/logs.h>
#include <database/maintenance.h>
#include <demo/logic.h>
#include <email/receive.h>
#include <email/send.h>
#include <export/esword.h>
#include <export/html.h>
#include <export/index.h>
#include <export/info.h>
#include <export/odt.h>
#include <export/onlinebible.h>
#include <export/textusfm.h>
#include <export/usfm.h>
#include <export/web.h>
#include <filter/google.h>
#include <filter/string.h>
#include <images/logic.h>
#include <notes/logic.h>
#include <paratext/logic.h>
#include <resource/convert2bible.h>
#include <resource/convert2resource.h>
#include <resource/logic.h>
#include <search/rebibles.h>
#include <search/renotes.h>
#include <sendreceive/bibles.h>
#include <sendreceive/changes.h>
#include <sendreceive/files.h>
#include <sendreceive/notes.h>
#include <sendreceive/resources.h>
#include <sendreceive/sendreceive.h>
#include <sendreceive/settings.h>
#include <statistics/statistics.h>
#include <styles/sheets.h>
#include <sword/logic.h>
#include <system/logic.h>
#include <tasks/enums.h>
#include <tasks/logic.h>
#include <tmp/tmp.h>
#include <user/logic.h>
#include "database/tasks.h"


namespace tasks {
std::string_view to_string(const enums::task task)
{
    switch (task)
    {
    case enums::task::none: return "none";
    case enums::task::rotate_journal: return "rotate journal";
    case enums::task::receive_email: return "receive email";
    case enums::task::send_email: return "send email";
    case enums::task::reindex_bibles: return "reindex bibles";
    case enums::task::reindex_notes: return "reindex notes";
    case enums::task::create_css: return "create css";
    case enums::task::import_bible: return "import bible";
    case enums::task::import_resource: return "import resource";
    case enums::task::compare_usfm: return "compare usfm";
    case enums::task::maintain_database: return "maintain database";
    case enums::task::clean_tmp_files: return "clean tmp files";
    case enums::task::link_git_repository: return "link git repository";
    case enums::task::send_receive_bibles: return "send receive bibles";
    case enums::task::sync_bibles: return "sync bibles";
    case enums::task::sync_notes: return "sync notes";
    case enums::task::sync_settings: return "sync settings";
    case enums::task::sync_changes: return "sync changes";
    case enums::task::sync_files: return "sync files";
    case enums::task::sync_resources: return "sync resources";
    case enums::task::clean_demo: return "clean demo";
    case enums::task::convert_bible_to_resource: return "convert bible to resource";
    case enums::task::convert_resource_to_bible: return "convert resource to bible";
    case enums::task::notes_statistics: return "notes statistics";
    case enums::task::generate_changes: return "generate changes";
    case enums::task::check_bible: return "check bible";
    case enums::task::export_all: return "export all";
    case enums::task::export_text_usfm: return "export text usfm";
    case enums::task::export_usfm: return "export usfm";
    case enums::task::export_odt: return "export odt";
    case enums::task::export_info: return "export info";
    case enums::task::export_html: return "export html";
    case enums::task::export_web_main: return "export web main";
    case enums::task::export_web_index: return "export web index";
    case enums::task::export_online_bible: return "export online bible";
    case enums::task::export_esword: return "export esword";
    case enums::task::setup_paratext: return "setup paratext";
    case enums::task::sync_paratext: return "sync paratext";
    case enums::task::refresh_sword_modules: return "refresh sword modules";
    case enums::task::install_sword_module: return "install sword module";
    case enums::task::update_sword_modules: return "update sword modules";
    case enums::task::list_usfm_resources: return "list usfm resources";
    case enums::task::create_sample_bible: return "create sample bible";
    case enums::task::cache_resources: return "cache resources";
    case enums::task::refresh_web_resources: return "refresh web resources";
    case enums::task::produce_bibles_transferfile: return "produce bibles transferfile";
    case enums::task::import_bibles_transferfile: return "import bibles transferfile";
    case enums::task::produce_notes_transferfile: return "produce notes transferfile";
    case enums::task::import_notes_transferfile: return "import notes transferfile";
    case enums::task::produce_resources_transferfile: return "produce resources transferfile";
    case enums::task::import_resources_transferfile: return "import resources transferfile";
    case enums::task::delete_changes: return "delete changes";
    case enums::task::clear_caches: return "clear caches";
    case enums::task::trim_caches: return "trim caches";
    case enums::task::create_empty_bible: return "create empty bible";
    case enums::task::import_bible_images: return "import bible images";
    case enums::task::get_google_access_token: return "get google access token";
    default: return {};
    }
}


static std::ostream& operator<<(std::ostream& os, const database::tasks::Task& task)
{
    os << std::quoted(to_string(task.task));
    for (const auto& parameter : task.parameters)
        os << ' ' << std::quoted(parameter);
    return os;
}


static std::deque<database::tasks::Task> task_queue{};
static std::mutex queue_mutex{};
static std::vector<std::thread> thread_pool;
static std::condition_variable thread_cv;
static std::atomic run_pool{false};
static std::atomic running_tasks(0);


void tasks_logic_queue(const enums::task task, std::vector<std::string> parameters)
{
    {
        std::lock_guard lock(queue_mutex);
        task_queue.emplace_back(task, std::move(parameters));
    }
    thread_cv.notify_one();
}


bool tasks_logic_queued(const enums::task task, const std::vector<std::string>& parameters)
{
    std::lock_guard lock(queue_mutex);
    return std::ranges::any_of(task_queue, [&](const database::tasks::Task& t) noexcept {
        return t.task == task && t.parameters == parameters;
    });
}


static void tasks_logic_run_one(database::tasks::Task task)
{
    const auto [parameter1, parameter2, parameter3, parameter4] = extract(task.parameters);

    switch (task.task)
    {
    case enums::task::none:
        {
            std::ostringstream oss;
            oss << task;
            throw std::runtime_error(std::move(oss).str());
        }
    case enums::task::rotate_journal:
        {
            database::logs::rotate();
            break;
        }
    case enums::task::receive_email:
        {
            email::receive();
            break;
        }
    case enums::task::send_email:
        {
            email::send();
            break;
        }
    case enums::task::reindex_bibles:
        {
            search_reindex_bibles(filter::string::convert_to_bool(parameter1));
            break;
        }
    case enums::task::reindex_notes:
        {
            search_reindex_notes();
            break;
        }
    case enums::task::create_css:
        {
            styles::sheets::create_all_run();
            break;
        }
    case enums::task::import_bible:
        {
            bible_import_run(parameter1, parameter2, filter::string::convert_to_int(parameter3),
                             filter::string::convert_to_int(parameter4));
            break;
        }
    case enums::task::import_resource:
        {
            bible_logic::import_resource(parameter1, parameter2);
            break;
        }
    case enums::task::compare_usfm:
        {
            compare_compare(parameter1, parameter2, filter::string::convert_to_int(parameter3));
            break;
        }
    case enums::task::maintain_database:
        {
            database_maintenance();
            break;
        }
    case enums::task::clean_tmp_files:
        {
            tmp_tmp();
            break;
        }
    case enums::task::link_git_repository:
        {
            collaboration_link(parameter1, filter::string::convert_to_int(parameter2), parameter3);
            break;
        }
    case enums::task::send_receive_bibles:
        {
            sendreceive_sendreceive(parameter1);
            break;
        }
    case enums::task::sync_notes:
        {
            sendreceive_notes();
            break;
        }
    case enums::task::sync_bibles:
        {
            sendreceive_bibles();
            break;
        }
    case enums::task::sync_settings:
        {
            sendreceive_settings();
            break;
        }
    case enums::task::sync_changes:
        {
            sendreceive_changes();
            break;
        }
    case enums::task::sync_files:
        {
            sendreceive_files();
            break;
        }
    case enums::task::sync_resources:
        {
            sendreceive_resources();
            break;
        }
    case enums::task::clean_demo:
        {
            demo_clean_data();
            break;
        }
    case enums::task::convert_bible_to_resource:
        {
            convert_bible_to_resource(parameter1);
            break;
        }
    case enums::task::convert_resource_to_bible:
        {
            convert_resource_to_bible(parameter1);
            break;
        }
    case enums::task::notes_statistics:
        {
            statistics_statistics();
            break;
        }
    case enums::task::generate_changes:
        {
            changes_modifications();
            break;
        }
    case enums::task::check_bible:
        {
            checks_run(parameter1);
            break;
        }
    case enums::task::export_all:
        {
            export_index();
            break;
        }
    case enums::task::export_web_main:
        {
            export_web_book(parameter1, filter::string::convert_to_int(parameter2),
                            filter::string::convert_to_bool(parameter3));
            break;
        }
    case enums::task::export_web_index:
        {
            export_web_index(parameter1, filter::string::convert_to_bool(parameter2));
            break;
        }
    case enums::task::export_html:
        {
            export_html_book(parameter1, filter::string::convert_to_int(parameter2),
                             filter::string::convert_to_bool(parameter3));
            break;
        }
    case enums::task::export_usfm:
        {
            export_usfm(parameter1, filter::string::convert_to_bool(parameter2));
            break;
        }
    case enums::task::export_text_usfm:
        {
            export_text_usfm_book(parameter1, filter::string::convert_to_int(parameter2),
                                  filter::string::convert_to_bool(parameter3));
            break;
        }
    case enums::task::export_odt:
        {
            export_odt_book(parameter1, filter::string::convert_to_int(parameter2),
                            filter::string::convert_to_bool(parameter3));
            break;
        }
    case enums::task::export_info:
        {
            export_info(parameter1, filter::string::convert_to_bool(parameter2));
            break;
        }
    case enums::task::export_esword:
        {
            export_esword(parameter1, filter::string::convert_to_bool(parameter2));
            break;
        }
    case enums::task::export_online_bible:
        {
            export_onlinebible(parameter1, filter::string::convert_to_bool(parameter2));
            break;
        }
    case enums::task::setup_paratext:
        {
            Paratext_Logic::setup(parameter1, parameter2);
            break;
        }
    case enums::task::sync_paratext:
        {
            int imethod = filter::string::convert_to_int(parameter1);
            auto method = static_cast<enums::paratext_sync>(imethod);
            Paratext_Logic::synchronize(method);
            break;
        }
    case enums::task::refresh_sword_modules:
        {
            sword_logic_refresh_module_list();
            break;
        }
    case enums::task::install_sword_module:
        {
            sword_logic_run_scheduled_module_install(parameter1, parameter2);
            break;
        }
    case enums::task::update_sword_modules:
        {
            sword_logic_update_installed_modules();
            break;
        }
    case enums::task::list_usfm_resources:
        {
            client_logic_usfm_resources_update();
            break;
        }
    case enums::task::create_sample_bible:
        {
            demo_create_sample_bible();
            break;
        }
    case enums::task::cache_resources:
        {
            resource_logic_create_cache();
            break;
        }
    case enums::task::refresh_web_resources:
        {
            resource_logic_bible_gateway_module_list_refresh();
            resource_logic_study_light_module_list_refresh();
            break;
        }
    case enums::task::produce_bibles_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_produce_bibles_file(filter::string::convert_to_int(parameter1));
#endif
            break;
        }
    case enums::task::import_bibles_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_import_bibles_file(parameter1);
#endif
            break;
        }
    case enums::task::produce_notes_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_produce_notes_file(filter::string::convert_to_int(parameter1));
#endif
            break;
        }
    case enums::task::import_notes_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_import_notes_file(parameter1);
#endif
            break;
        }
    case enums::task::produce_resources_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_produce_resources_file(filter::string::convert_to_int(parameter1));
#endif
            break;
        }
    case enums::task::import_resources_transferfile:
        {
#ifdef HAVE_CLIENT
            system_logic_import_resources_file(parameter1);
#endif
            break;
        }
    case enums::task::delete_changes:
        {
            changes_clear_notifications_user(parameter1, parameter2);
            break;
        }
    case enums::task::clear_caches:
        {
            database::cache::file::trim(true);
            break;
        }
    case enums::task::trim_caches:
        {
            database::cache::file::trim(false);
            break;
        }
    case enums::task::create_empty_bible:
        {
            bible_logic::create_empty_bible(parameter1);
            break;
        }
    case enums::task::import_bible_images:
        {
            images_logic_import_images(parameter1);
            break;
        }
    case enums::task::get_google_access_token:
        {
            filter::google::refresh_access_token();
            break;
        }
    default:
        {
            throw std::runtime_error("Unknown task");
        }
    }
}


void tasks_logic_start_thread_pool(const std::size_t num_threads)
{
    // Guard against double starting.
    if (run_pool)
        return;
    // Flag to run.
    run_pool = true;
    // Creating worker threads.
    for (size_t i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back([]
        {
            while (true)
            {
                database::tasks::Task task{};
                // The reason for putting the below code here is to unlock the queue
                // before executing the task so that other threads can perform enqueue tasks.
                {
                    // Locking the queue so that data can be shared safely.
                    std::unique_lock lock(queue_mutex);

                    // Waiting until there is a task to execute or the pool is stopped.
                    // While in .wait it unlocks the mutex on the queue.
                    thread_cv.wait(lock, []
                    {
                        return not task_queue.empty() or not run_pool;
                    });

                    // Exit the thread in case the pool is stopped, disregarding pending http requests.
                    if (not run_pool)
                        return;

                    // Get the next task from the queue.
                    task = std::move(task_queue.front());
                    task_queue.pop_front();
                }

                // Run the task in this thread.
                try
                {
                    struct RunningTasks
                    {
                        explicit RunningTasks() { ++running_tasks; }
                        ~RunningTasks() { --running_tasks; }
                    };
                    RunningTasks running_tasks_raii_wrapper;
                    tasks_logic_run_one(task);
                }
                catch (const std::exception& exception)
                {
                    database::logs::log("Error running background task:", exception.what());
                }
            }
        });
    }
}


void tasks_logic_stop_thread_pool()
{
    // Guard against double stopping.
    if (not run_pool)
        return;

    // Indicate stop.
    run_pool = false;

    // Notify all threads
    thread_cv.notify_all();

    // Join all worker threads, if possible, to ensure they have completed their tasks.
    std::ranges::for_each(thread_pool, [](std::thread& t)
    {
        if (t.joinable())
            t.join();
    });

    // Clear them so they are no longer available on a possible subsequent shutdown.
    thread_pool.clear();
}


int tasks_logic_queue_size()
{
    std::scoped_lock lock(queue_mutex);
    return static_cast<decltype(tasks::tasks_logic_queue_size())>(task_queue.size());
}


int tasks_logic_active_jobs_count ()
{
    return running_tasks;
}


Parameters extract(std::vector<std::string>& task_parameters)
{
    size_t index {0};
    const auto get_parameter = [&task_parameters, &index](std::string& parameter)
    {
        if (index < task_parameters.size())
            parameter = std::move(task_parameters[index]);
        ++index;
    };
    Parameters parameters;
    get_parameter(parameters.p1);
    get_parameter(parameters.p2);
    get_parameter(parameters.p3);
    get_parameter(parameters.p4);
    return parameters;
}


} //namespace
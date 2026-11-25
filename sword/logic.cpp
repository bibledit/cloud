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


#include <client/logic.h>
#include <database/books.h>
#include <database/cache.h>
#include <database/logic.h>
#include <database/logs.h>
#include <database/sword.h>
#include <database/config/general.h>
#include <demo/logic.h>
#include <developer/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <sword/logic.h>
#include <sync/resources.h>
#include <tasks/logic.h>
#ifdef HAVE_SWORD
#include <filemgr.h>
#include <installmgr.h>
#include <remotetrans.h>
#include <swbuf.h>
#include <swmgr.h>
#include <swmodule.h>
#include <versekey.h>
#endif


#ifdef HAVE_CLOUD
static std::atomic extracting_module{false};
#endif


#ifdef HAVE_CLOUD
[[nodiscard]] static std::string get_sword_install_mgr_path()
{
    std::string home_directory{"."};
    if (const char* home = getenv("HOME"))
        home_directory = home;
    return std::filesystem::path{home_directory} / ".sword" / "InstallMgr";
}
#endif


[[nodiscard]] static std::string sword_logic_module_list_path()
{
    return filter_url_create_root_path({database_logic_databases(), "client", "sword_modules.txt"});
}


#ifdef HAVE_CLOUD
static void initialize_sword_structures()
{
    // Initialize SWORD directory structure.
    const std::string sword_path = get_sword_install_mgr_path();
    const std::filesystem::path path(sword_path);
    std::filesystem::create_directories(path);
    const sword::SWBuf base_dir = get_sword_install_mgr_path().c_str();

    // The installmgr needs these directories to exist:
    sword::FileMgr::createParent(base_dir + "/mods.d/placeholder");
    sword::FileMgr::createParent(base_dir + "/modules/placeholder");

    // The constructor creates the InstallMgr config directory and reads InstallMgr.conf.
    // If there is none, it falls back to the default CrossWire source.
    sword::InstallMgr installMgr(base_dir, nullptr);

    // Equivalent of --allow-internet-access-and-risk-tracing-and-jail-or-martyrdom.
    installMgr.setUserDisclaimerConfirmed(true);

    // Write InstallMgr.conf to disk. This is what "-init" boils down to.
    installMgr.saveInstallConf();
    database::logs::log("Initialized SWORD install manager in", base_dir);

    // Sync the configuration with the online known remote repository list.
    if (installMgr.refreshRemoteSourceConfiguration())
        database::logs::log("Could not refresh the remote source configuration");
}
#endif


void sword_logic_refresh_module_list()
{
#ifdef HAVE_CLOUD
    initialize_sword_structures();

    database::logs::log("Refreshing list of SWORD modules");

    const sword::SWBuf base_dir = get_sword_install_mgr_path().c_str();
    sword::InstallMgr install_manager(base_dir, nullptr);
    install_manager.setUserDisclaimerConfirmed(true);

    // Get the remote sources.
    std::vector<std::string> remote_sources{};
    for (const sword::InstallSource* install_source : install_manager.sources | std::views::values)
        remote_sources.emplace_back(install_source->caption.c_str());
    database::logs::log("Remote sources:", filter::string::implode(remote_sources, " - "));

    std::vector<std::string> sword_modules{};

    for (const std::string& remote_source : remote_sources)
    {
        auto source_iter = install_manager.sources.find(remote_source.c_str());
        if (source_iter == install_manager.sources.cend())
            database::logs::log("Could not find remote source", remote_source);
        // Download the module list of this source.
        if (const int error = install_manager.refreshRemoteSource(source_iter->second))
        {
            database::logs::log("Could not refresh remote source", remote_source, "error", error);
            continue;
        }
        sword::SWMgr sword_manager(base_dir);
        sword::SWMgr* other_manager = source_iter->second->getMgr();
        if (not other_manager)
            other_manager = std::addressof(sword_manager);
        // ReSharper disable once CppTooWideScopeInitStatement
        const std::map<sword::SWModule*, int> mods = sword::InstallMgr::getModuleStatus(sword_manager, *other_manager);
        std::size_t usable_module_count{};
        std::size_t encrypted_module_count{};
        std::size_t non_verse_based_module_count{};
        for (const auto& module_ptr : mods | std::views::keys)
        {
            const sword::SWModule* module = module_ptr;

            // Get module properties.
            std::string name = module->getName();
            const auto get_version = [module]
            {
                if (const char* version = module->getConfigEntry("Version"))
                    return version;
                return "";
            };
            std::string version = get_version();
            std::string description = module->getDescription();

            // No CipherKey entry: the module is not encrypted.
            // Skip encrypted module (no CipherKey to decrypt it).
            if (module->getConfigEntry("CipherKey"))
            {
                ++encrypted_module_count;
                continue;
            }

            // Skip module if it is not verse-based (Bible or commentary).
            const auto verse_based = [module]
            {
                const std::string module_type = module->getType();
                if (module_type == sword::SWMgr::MODTYPE_BIBLES)
                    return true;
                if (module_type == sword::SWMgr::MODTYPE_COMMENTARIES)
                    return true;
                return false;
            };
            if (not verse_based())
            {
                ++non_verse_based_module_count;
                continue;
            }

            std::ostringstream oss{};
            oss << "[" << remote_source << "] *[" << name << "]  (" << version << ") - " << description;
            ++usable_module_count;
            sword_modules.push_back(std::move(oss).str());
        }
        database::logs::log("Remote source", remote_source, "has", usable_module_count, "modules excluding",
                            encrypted_module_count, "encrypted and", non_verse_based_module_count, "non verse-based");
    }

    // Store the list of remote sources and their modules.
    // It is stored in the client files area.
    // Clients can access it from there too.
    filter_url_file_put_contents(sword_logic_module_list_path(), filter::string::implode(sword_modules, "\n"));

    database::logs::log("Ready refreshing SWORD module list");
#endif
}


// Gets the name of the remote source of the $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_source(std::string line)
{
    if (line.length() < 10)
        return {};
    line.erase(0, 1);
    const size_t pos = line.find(']');
    if (pos == std::string::npos)
        return {};
    line.erase(pos);
    return line;
}


// Gets the module name of the $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_remote_module(std::string line)
{
    if (line.length() < 10)
        return {};
    line.erase(0, 2);
    if (line.length() < 10)
        return {};
    size_t pos = line.find('[');
    if (pos == std::string::npos)
        return {};
    line.erase(0, pos + 1);
    pos = line.find(']');
    if (pos == std::string::npos)
        return {};
    line.erase(pos);
    return line;
}


// Gets the module name of the $line like this:
// [Shona]  (1.1)  - Shona Bible
std::string sword_logic_get_installed_module(std::string line)
{
    line = filter::string::trim(line);
    if (line.length() > 10)
    {
        line.erase(0, 1);
        if (const size_t pos = line.find(']'); pos != std::string::npos)
            line.erase(pos);
    }
    return line;
}


// Gets the version number of a module of the $line like this:
// [Shona]  (1.1)  - Shona Bible
std::string sword_logic_get_version(std::string line)
{
    line = filter::string::trim(line);
    if (line.length() > 10)
        line.erase(0, 3);
    if (line.length() > 10)
    {
        size_t pos = line.find('(');
        if (pos != std::string::npos)
            line.erase(0, pos + 1);
        pos = line.find(')');
        if (pos != std::string::npos)
            line.erase(pos);
    }
    return line;
}


// Gets the human-readable name of a $line like this:
// [CrossWire] *[Shona] (1.1) - Shona Bible
std::string sword_logic_get_name(std::string line)
{
    std::vector<std::string> bits = filter::string::explode(line, '-');
    if (bits.size() >= 2)
        bits.erase(bits.begin());
    line = filter::string::implode(bits, "-");
    line = filter::string::trim(line);
    return line;
}


// Schedule SWORD module extraction.
void sword_logic_schedule_module_extraction([[maybe_unused]] const std::string& source, [[maybe_unused]] const std::string& module)
{
#ifdef HAVE_CLOUD
    // No source: Done.
    if (source.empty())
        return;

    // No module: Done.
    // There have been cases with more than 6000 scheduled SWORD module installation tasks,
    // all trying to install an empty $module.
    // So it's important to check on that.
    if (module.empty())
        return;

    // Schedule the module installation and extraction
    // if it has not yet been scheduled and has not yet been extracted.
    if (tasks::tasks_logic_queued_or_running(tasks::enums::task::extract_sword_module, {source, module}))
        return;
    if (database::sword::exists(module))
        return;
    tasks::tasks_logic_queue(tasks::enums::task::extract_sword_module, {source, module});
#endif
}


#ifdef HAVE_CLOUD
static void sword_logic_install_module(const std::string& source_name, const std::string& module_name)
{
    database::logs::log("Installing SWORD module", module_name, "from source", source_name);
    const sword::SWBuf base_dir{get_sword_install_mgr_path().c_str()};
    sword::SWMgr sword_mgr(base_dir); // Only installs if path set.
    sword::InstallMgr installMgr(base_dir, nullptr);
    installMgr.setUserDisclaimerConfirmed(true);
    const auto source = installMgr.sources.find(source_name.c_str());
    if (source == installMgr.sources.end())
    {
        database::logs::log("Could not find remote source", source_name);
        return;
    }
    sword::InstallSource* install_source = source->second;
    sword::SWMgr* remote_manager = install_source->getMgr();
    const auto module_iter = remote_manager->Modules.find(module_name.c_str());
    if (module_iter == remote_manager->Modules.end())
    {
        database::logs::log("Remote source", source_name, "does not make available module", module_name);
        return;
    }
    const sword::SWModule* module = module_iter->second;
    if (const int error = installMgr.installModule(&sword_mgr, nullptr, module->getName(), install_source);
        error != 0)
    {
        database::logs::log("Error installing SWORD module", module_name);
        return;
    }
    database::logs::log("Completed installing SWORD module", module_name);
}
#endif



#ifdef HAVE_CLOUD
static void sword_logic_uninstall_module(const std::string& module)
{
    database::logs::log("Uninstalling SWORD module", module);
    const sword::SWBuf base_dir{get_sword_install_mgr_path().c_str()};
    sword::SWMgr sword_mgr(base_dir);
    sword::InstallMgr installMgr(base_dir, nullptr);
    const auto module_iter = sword_mgr.Modules.find(module.c_str());
    if (module_iter == sword_mgr.Modules.end())
    {
        database::logs::log("SWORD module", module, "is not installed");
        return;
    }
    const sword::SWModule* sw_module = module_iter->second;
    if (const int error = installMgr.removeModule(&sword_mgr, sw_module->getName()); error != 0)
    {
        database::logs::log("Error uninstalling SWORD module", module);
        return;
    }
    database::logs::log("Uninstalled SWORD module", module);
}
#endif


// Get available SWORD modules.
std::vector<std::string> sword_logic_get_available()
{
    const std::string contents = filter_url_file_get_contents(sword_logic_module_list_path());
    return filter::string::explode(contents, '\n');
}


std::string sword_logic_get_text(const std::string& source, const std::string& module,
                                 const int book, const int chapter, const int verse)
{
#ifndef HAVE_CLOUD

    // The resource name consists of source and module, e.g. [CrossWire][NET].
    std::string resource = sword_logic_get_resource_name(source, module);

    // Client checks for and optionally creates the cache for this SWORD source/module.
    if (not database::cache::sql::exists(resource, book))
    {
        database::cache::sql::create(resource, book);
    }

    // If this module/passage exists in the cache, return it (it updates the access days in the cache).
    if (database::cache::sql::exists(resource, book, chapter, verse))
    {
        return database::cache::sql::retrieve(resource, book, chapter, verse);
    }

    // Fetch this SWORD resource from the server.
    std::string address = database::config::general::get_server_address();
    int port = database::config::general::get_server_port();
    if (!client_logic_client_enabled())
    {
        // If the client has not been connected to a cloud instance,
        // fetch the SWORD content from the Bibledit Cloud demo.
        address = demo_address();
        port = demo_port();
    }
    const std::string url = filter_url_build_http_query(client_logic_url(address, port, sync_resources_url()), {
                                                            {"r", filter_url_url_encode(resource)}, // May contain space.
                                                            {"b", std::to_string(book)},
                                                            {"c", std::to_string(chapter)},
                                                            {"v", std::to_string(verse)},
                                                        });
    std::string error{};
    std::string html = filter_url_http_get(url, error, true);

    // In case of an error, don't cache that error, but let the user see it.
    if (!error.empty()) return error;

    // Client caches this info for later.
    // Except in case of predefined responses from the Cloud.
    if (html != sword_logic_installing_module_text())
        database::cache::sql::cache(resource, book, chapter, verse, html);

    return html;

#else

    // This it the Bibledit Cloud side (not the client).

    if (database::sword::exists(module))
        return database::sword::get_verse(module, book, chapter, verse);

    // At this point the extracted SWORD text is not available.
    // Check if the module exists, if so, schedule its installation and extraction.
    // Return appropriate message to the caller.

    if (const std::string modules = filter::string::implode(sword_logic_get_available(), std::string());
        modules.find("[" + module + "]") != std::string::npos)
    {
        // Schedule SWORD module installation and extraction.
        sword_logic_schedule_module_extraction(source, module);
        // Return standard 'installing' information. Client knows not to cache this.
        return sword_logic_installing_module_text();
    }
    return "Cannot find SWORD module " + module; // Sorry, can't help you.

#endif
}


// Text saying that the Cloud will install the requested SWORD module.
// Client knows not to cache this.
std::string sword_logic_installing_module_text()
{
    return
        "The requested SWORD module is not yet installed. Bibledit Cloud will install it shortly. Please check back after a few minutes.";
}


// The function runs a scheduled module installation and extraction.
// The purpose of this function is that only one such operation occurs at a time,
// rather than simultaneously installing modules, which clogs the system.
void sword_logic_run_scheduled_module_extract([[maybe_unused]] const std::string& source, [[maybe_unused]] const std::string& module)
{
#ifdef HAVE_CLOUD
    // If another module installation and extraction is already in progress,
    // postpone this task by re-scheduling it.
    // Only one task can run at a time:
    // This task must wait until the current one has completed.
    if (extracting_module)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Prevent rapid rescheduling multiple tasks.
        tasks::tasks_logic_queue(tasks::enums::task::extract_sword_module, {source, module});
        return;
    }

    // Set flag for current module extraction running, clear it on scope end.
    extracting_module = true;
    using flag_destructor = decltype([](void*) { extracting_module = false; });
    const auto flag_guard{std::unique_ptr<void, flag_destructor>(reinterpret_cast<void*>(1))};

    // Install the module.
    sword_logic_install_module(source, module);

    database::logs::log("Importing SWORD module", module);

    const bool keep_osis_content = database::config::general::get_keep_osis_content_in_sword_resources();

    // The SWORD manager should be pointed to the path of the library, else it fails.
    sword::SWMgr sword_manager(get_sword_install_mgr_path().c_str());

    // Set options that influence the text content.
    sword_manager.setGlobalOption("Footnotes", "Off");
    sword_manager.setGlobalOption("Headings", "Off");
    sword_manager.setGlobalOption("Strong's Numbers", "Off");
    sword_manager.setGlobalOption("Morphological Tags", "Off");
    sword_manager.setGlobalOption("Hebrew Cantillation", "On");
    sword_manager.setGlobalOption("Hebrew Vowel Points", "On");
    sword_manager.setGlobalOption("Greek Accents", "On");
    sword_manager.setGlobalOption("Lemmas", "Off");
    sword_manager.setGlobalOption("Cross-references", "Off");
    sword_manager.setGlobalOption("Words of Christ in Red", "Off");
    sword_manager.setGlobalOption("Arabic Vowel Points", "On");
    sword_manager.setGlobalOption("Glosses", "Off");
    sword_manager.setGlobalOption("Transliterated Forms", "Off");
    sword_manager.setGlobalOption("Enumerations", "Off");
    sword_manager.setGlobalOption("Transliteration", "Off");
    sword_manager.setGlobalOption("Textual Variants", "All Readings");
    //manager.setGlobalOption("Textual Variants", "Secondary Reading");
    //manager.setGlobalOption("Textual Variants", "Primary Reading");

    sword::SWModule* sw_module = sword_manager.getModule(module.c_str());
    if (not sw_module)
    {
        database::logs::log("SWORD module", module, "is not available");
        return;
    }

    // Use the module's own key, so the versification matches the module.
    auto* verse_key = dynamic_cast<sword::VerseKey*>(sw_module->getKey());
    if (not verse_key)
    {
        database::logs::log("SWORD module", module, "does not use a verse key");
        return;
    }

    // Don't let setters jump to neighboring books.
    verse_key->setAutoNormalize(false);

    // Container with verse content.
    std::vector<database::sword::VerseText> verse_texts;

    // Iterate over both Testaments.
    // The Old Testament may also contain Apocrypha for some verse systems.
    for (const char testament : std::initializer_list<char>{1, 2})
    {
        verse_key->setTestament(testament);
        // Go over the books in this testament.
        for (int book = 1; book <= verse_key->getBookMax(); ++book)
        {
            verse_key->setBook(static_cast<char>(book));

            // Info about book: name and id.
            //const std::string name = verse_key->getBookName();
            const std::string module_osis_book_name{verse_key->getOSISBookName()};
            //database::logs::log("Extracting", module_osis_book_name, "from SWORD module", module);
            const int book_id = static_cast<int>(database::books::get_id_from_osis(module_osis_book_name));

            // Go over the chapters in this book.
            for (int chapter = 1; chapter <= verse_key->getChapterMax(); ++chapter)
            {
                verse_key->setChapter(chapter);

                // Go over the verses in this chapter.
                for (int verse = 1; verse <= verse_key->getVerseMax(); ++verse)
                {
                    verse_key->setVerse(verse);
                    // The versification may define books the module does not contain, e.g. an NT-only module.
                    if (not sw_module->hasEntry(verse_key))
                        continue;

                    // Get verse text.
                    std::string rendering = sw_module->renderText().c_str();

                    // Remove any OSIS elements or make those elements displayable.
                    if (keep_osis_content)
                        rendering = filter::string::escape_special_xml_characters(rendering);
                    else
                        filter::string::replace_between(rendering, "<", ">", "");

                    // Remove double spaces and lean whitespace away.
                    rendering = filter::string::replace("  ", " ", rendering);
                    rendering = filter::string::trim(rendering);

                    // Store it.
                    verse_texts.emplace_back(
                        database::sword::VerseText{
                            .book = book_id,
                            .chapter = chapter,
                            .verse = verse,
                            .text = rendering,
                        }
                    );
                }
            }
        }
    }

    // Save it in the SWORD database.
    database::sword::store_all(module, verse_texts);
    database::logs::log("Completed importing SWORD module", module);

    // Uninstall the module again.
    sword_logic_uninstall_module(module);
#endif
}


// Take the SWORD $source and SWORD $module and form it into a canonical resource name.
std::string sword_logic_get_resource_name(const std::string& source, const std::string& module)
{
    return "[" + source + "][" + module + "]";
}

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
   

#include <assets/external.h>
#include <bb/book.h>
#include <bb/chapter.h>
#include <bb/css.h>
#include <bb/import.h>
#include <bb/manage.h>
#include <bb/order.h>
#include <bb/settings.h>
#include <bootstrap/bootstrap.h>
#include <changes/change.h>
#include <changes/changes.h>
#include <changes/manage.h>
#include <changes/statistics.h>
#include <checks/index.h>
#include <checks/settings.h>
#include <checks/settingspairs.h>
#include <checks/settingspatterns.h>
#include <checks/settingssentences.h>
#include <checks/suppress.h>
#include <client/index.h>
#include <collaboration/index.h>
#include <collaboration/settings.h>
#include <compare/index.h>
#include <config/globals.h>
#include <consistency/index.h>
#include <consistency/input.h>
#include <consistency/poll.h>
#include <database/cache.h>
#include <database/config/general.h>
#include <developer/delay.h>
#include <developer/index.h>
#include <edit/edit.h>
#include <edit/id.h>
#include <edit/index.h>
#include <edit/load.h>
#include <edit/navigate.h>
#include <edit/position.h>
#include <edit/preview.h>
#include <edit/save.h>
#include <edit/styles.h>
#include <edit/update.h>
#include <editone/index.h>
#include <editone/load.h>
#include <editone/save.h>
#include <editone/update.h>
#include <editone/verse.h>
#include <editor/id.h>
#include <editor/select.h>
#include <editor/style.h>
#include <editusfm/focus.h>
#include <editusfm/index.h>
#include <editusfm/load.h>
#include <editusfm/offset.h>
#include <editusfm/save.h>
#include <email/index.h>
#include <filter/url.h>
#include <fonts/logic.h>
#include <help/index.h>
#include <images/fetch.h>
#include <images/index.h>
#include <images/view.h>
#include <index/index.h>
#include <index/listing.h>
#include <jobs/index.h>
#include <journal/index.h>
#include <lexicon/definition.h>
#include <manage/bibles.h>
#include <manage/exports.h>
#include <manage/index.h>
#include <manage/privileges.h>
#include <manage/users.h>
#include <manage/write.h>
#include <mapping/index.h>
#include <mapping/map.h>
#include <menu/index.h>
#include <navigation/paratext.h>
#include <navigation/poll.h>
#include <navigation/update.h>
#include <notes/actions.h>
#include <notes/assign-1.h>
#include <notes/assign-n.h>
#include <notes/bb-1.h>
#include <notes/bb-n.h>
#include <notes/bulk.h>
#include <notes/click.h>
#include <notes/comment.h>
#include <notes/create.h>
#include <notes/edit.h>
#include <notes/index.h>
#include <notes/note.h>
#include <notes/notes.h>
#include <notes/poll.h>
#include <notes/select.h>
#include <notes/severity-1.h>
#include <notes/severity-n.h>
#include <notes/status-1.h>
#include <notes/status-n.h>
#include <notes/summary.h>
#include <notes/unassign-n.h>
#include <notes/verses.h>
#include <paratext/index.h>
#include <personalize/index.h>
#include <public/chapter.h>
#include <public/comment.h>
#include <public/create.h>
#include <public/index.h>
#include <public/login.h>
#include <public/new.h>
#include <public/note.h>
#include <public/notes.h>
#include <read/index.h>
#include <read/load.h>
#include <read/verse.h>
#include <resource/bb2resource.h>
#include <resource/bbgateway.h>
#include <resource/cache.h>
#include <resource/comparative1edit.h>
#include <resource/comparative9edit.h>
#include <resource/divider.h>
#include <resource/download.h>
#include <resource/get.h>
#include <resource/index.h>
#include <resource/manage.h>
#include <resource/organize.h>
#include <resource/select.h>
#include <resource/studylight.h>
#include <resource/sword.h>
#include <resource/translated1edit.h>
#include <resource/translated9edit.h>
#include <resource/unload.h>
#include <resource/user1edit.h>
#include <resource/user1view.h>
#include <resource/user9edit.h>
#include <resource/user9view.h>
#include <search/all.h>
#include <search/getids.h>
#include <search/getids2.h>
#include <search/index.h>
#include <search/originals.h>
#include <search/replace.h>
#include <search/replace2.h>
#include <search/replacego.h>
#include <search/replacego2.h>
#include <search/replacepre.h>
#include <search/replacepre2.h>
#include <search/search2.h>
#include <search/similar.h>
#include <search/strong.h>
#include <search/strongs.h>
#include <sendreceive/index.h>
#include <session/confirm.h>
#include <session/login.h>
#include <session/logout.h>
#include <session/password.h>
#include <session/signup.h>
#include <session/switch.h>
#include <setup/index.h>
#include <styles/indexm.h>
#include <styles/new.h>
#include <styles/sheetm.h>
#include <styles/view.h>
#include <sync/bibles.h>
#include <sync/changes.h>
#include <sync/files.h>
#include <sync/mail.h>
#include <sync/notes.h>
#include <sync/resources.h>
#include <sync/settings.h>
#include <sync/setup.h>
#include <sync/usfmresources.h>
#include <system/googletranslate.h>
#include <system/index.h>
#include <user/account.h>
#include <user/notifications.h>
#include <versification/index.h>
#include <versification/system.h>
#include <webbb/search.h>
#include <webserver/http.h>
#include <webserver/request.h>
#include <workspace/index.h>
#include <workspace/organize.h>
#include <workspace/settings.h>


// Check whether a request coming from the browser is considered secure enough.
// It returns true if the security is okay.
static bool browser_request_security_okay(const Webserver_Request& webserver_request)
{
    // If the request is made via https, the security is OK.
    if (webserver_request.secure)
        return true;

    // At this stage the request is made via plain http.
    // If https is not enforced for the browser, the security is OK.
    if (!config_globals_enforce_https_browser)
        return true;

    // Not secure enough:
    return false;
}


using AclFunc = bool (*)(Webserver_Request&);
using HandlerFunc = std::string (*)(Webserver_Request&);

namespace {
struct Route { AclFunc acl; HandlerFunc handler; };
}

static const std::unordered_map<std::string, Route>& get_route_table()
{
    static const std::unordered_map<std::string, Route> route_table = [] {
        std::unordered_map<std::string, Route> t;
        const auto add = [&t](const std::string& page_url, const AclFunc acl, const HandlerFunc handler) {
            t.emplace(page_url, Route{.acl = acl, .handler = handler});
            // The .emplace keeps the first entry on duplicates.
        };

        // Macro to cut repetition and so eliminate some programmer mistakes.
        // The ## is the preprocessor's token-pasting operator.
#define ROUTE(name) add(name##_url(), name##_acl, name)

        ROUTE(index_index);
        ROUTE(menu_index);
        ROUTE(session_login);
        ROUTE(session_logout);
        ROUTE(session_password);
        ROUTE(session_signup);
        ROUTE(session_switch);
        ROUTE(bible_manage);
        ROUTE(bible_settings);
        ROUTE(bible_book);
        ROUTE(bible_chapter);
        ROUTE(bible_import);
        ROUTE(compare_index);
        ROUTE(bible_order);
        ROUTE(bible_css);
        ROUTE(editusfm_index);
        ROUTE(edit_index);
        ROUTE(edit_position);
        ROUTE(edit_navigate);
        ROUTE(search_index);
        ROUTE(workspace_index);
        ROUTE(workspace_organize);
        ROUTE(resource_bible2resource);
        ROUTE(checks_index);
        ROUTE(checks_settings);
        ROUTE(consistency_index);
        ROUTE(notes_index);
        ROUTE(notes_create);
        ROUTE(notes_select);
        ROUTE(notes_note);
        ROUTE(notes_comment);
        ROUTE(notes_actions);
        ROUTE(notes_assign_1);
        ROUTE(notes_assign_n);
        ROUTE(notes_unassign_n);
        ROUTE(notes_status_1);
        ROUTE(notes_status_n);
        ROUTE(notes_verses);
        ROUTE(notes_severity_1);
        ROUTE(notes_severity_n);
        ROUTE(notes_bible_1);
        ROUTE(notes_bible_n);
        ROUTE(notes_bulk);
        ROUTE(notes_edit);
        ROUTE(notes_summary);
        ROUTE(resource_index);
        ROUTE(resource_organize);
        ROUTE(resource_manage);
        ROUTE(resource_download);
        ROUTE(resource_sword);
        ROUTE(resource_select);
        ROUTE(resource_cache);
        ROUTE(resource_user9edit);
        ROUTE(resource_user1edit);
        ROUTE(resource_user9view);
        ROUTE(resource_user1view);
        ROUTE(resource_biblegateway);
        ROUTE(resource_studylight);
        ROUTE(journal_index);
        ROUTE(changes_changes);
        ROUTE(changes_change);
        ROUTE(changes_manage);
        ROUTE(changes_statistics);
        ROUTE(sendreceive_index);
        ROUTE(manage_exports);
        ROUTE(developer_index);
        ROUTE(personalize_index);
        ROUTE(manage_users);
        ROUTE(manage_index);
        ROUTE(system_index);
        ROUTE(system_googletranslate);
        ROUTE(email_index);
        ROUTE(styles_indexm);
        ROUTE(styles_new);
        ROUTE(styles_sheetm);
        ROUTE(styles_view);
        ROUTE(versification_index);
        ROUTE(versification_system);
        ROUTE(collaboration_index);
        ROUTE(client_index);
        ROUTE(mapping_index);
        ROUTE(mapping_map);
        ROUTE(paratext_index);
        ROUTE(user_notifications);
        ROUTE(user_account);
        ROUTE(public_index);
        ROUTE(public_login);
        ROUTE(public_chapter);
        ROUTE(public_notes);
        ROUTE(public_new);
        ROUTE(public_create);
        ROUTE(public_note);
        ROUTE(public_comment);
        ROUTE(jobs_index);
        ROUTE(search_all);
        ROUTE(search_replace);
        ROUTE(search_search2);
        ROUTE(search_replace2);
        ROUTE(search_similar);
        ROUTE(search_strongs);
        ROUTE(search_strong);
        ROUTE(search_originals);
        ROUTE(workspace_settings);
        ROUTE(collaboration_settings);
        ROUTE(checks_settingspatterns);
        ROUTE(checks_settingssentences);
        ROUTE(checks_settingspairs);
        ROUTE(checks_suppress);
        ROUTE(webbible_search);
        ROUTE(manage_write);
        ROUTE(manage_bibles);
        ROUTE(manage_privileges);
        ROUTE(editor_select);
        ROUTE(navigation_update);
        ROUTE(navigation_poll);
        ROUTE(edit_preview);
        ROUTE(editusfm_focus);
        ROUTE(editusfm_load);
        ROUTE(editusfm_offset);
        ROUTE(editusfm_save);
        ROUTE(edit_edit);
        ROUTE(edit_id);
        ROUTE(edit_load);
        ROUTE(edit_save);
        ROUTE(edit_styles);
        ROUTE(search_getids);
        ROUTE(search_replacepre);
        ROUTE(search_replacego);
        ROUTE(search_replacepre2);
        ROUTE(search_getids2);
        ROUTE(search_replacego2);
        ROUTE(resource_get);
        ROUTE(resource_unload);
        ROUTE(notes_poll);
        ROUTE(notes_notes);
        ROUTE(notes_click);
        ROUTE(consistency_poll);
        ROUTE(consistency_input);
        ROUTE(lexicon_definition);
        ROUTE(edit_update);
        ROUTE(editor_id);
        ROUTE(editor_style);
        ROUTE(editone_index);
        ROUTE(editone_load);
        ROUTE(editone_save);
        ROUTE(editone_verse);
        ROUTE(editone_update);
        ROUTE(read_index);
        ROUTE(read_load);
        ROUTE(read_verse);
        ROUTE(resource_divider);
        ROUTE(session_confirm);
        ROUTE(resource_comparative9edit);
        ROUTE(resource_comparative1edit);
        ROUTE(resource_translated9edit);
        ROUTE(resource_translated1edit);
        ROUTE(images_index);
        ROUTE(images_view);
        ROUTE(images_fetch);

        return t;
    }();
    return route_table;
}




// This function is the first function to be called after a client requests a page or file.
// Based on the request from the client,
// it decides which functions to call to obtain the response.
void bootstrap_index(Webserver_Request& webserver_request)
{
    // [[maybe_unused]] std::shared_ptr<Developer_Logic_Tracer> developer_logic_tracer{nullptr};
    // if (config_globals_log_network)
    // {
    //     developer_logic_tracer = std::make_shared<Developer_Logic_Tracer>(webserver_request);
    // }
    // Record the POST request made to the web server.
    // This can be used for debugging.
    // if (!webserver_request.post.empty () and config_logic_demo_enabled ()) {
    //   std::string contents;
    //   int seconds = filter::date::get_seconds_since_epoch ();
    //   std::string rfc822time = filter::date::rfc822 (seconds);
    //   contents.append (rfc822time + "\n");
    //   contents.append (webserver_request.get + "\n");
    //   std::string query;
    //   for (auto element : webserver_request.query) {
    //     query.append (element.first + "=" + element.second + " and ");
    //   }
    //   std::string post;
    //   for (auto element : webserver_request.post) {
    //     post.append (element.first + "=" + element.second + " and ");
    //   }
    //   contents.append ("query: " + query + "\n");
    //   contents.append ("post: " + post + "\n");
    //   std::string filename;
    //   filename = filter_url_create_root_path (filter_url_temp_dir (), "http-post-trace.txt");
    //   filter_url_file_put_contents_append (filename, contents);
    // }

    const std::string extension = filter_url_get_extension(webserver_request.get);
    const std::string url = webserver_request.get.substr(1);

    // Serve graphics, stylesheets, JavaScript, fonts, and so on.
    // Use direct streaming for low memory usage.
    if (extension == "ico"
        or filter_url_is_image(extension)
        or extension == "css"
        or extension == "js"
        or fonts::logic::is_font(extension)
        or extension == "sh"
        or extension == "map"
        or extension == "webmanifest"
    )
    {
        http_stream_file(webserver_request, true);
        return;
    }

    // Serve resource downloads.
    if (extension == "sqlite" and webserver_request.get.find(database::cache::sql::fragment()) != std::string::npos)
    {
        http_stream_file(webserver_request, false);
        return;
    }

    // Serve initialization notice.
    if (config::logic::version() != database::config::general::get_installed_database_version())
    {
        webserver_request.reply = setup_initialization_notice();
        return;
    }

    // Force setup.
    if (config::logic::version() != database::config::general::get_installed_interface_version())
    {
        webserver_request.reply = setup_index(webserver_request);
        return;
    }

    // Help menu.
    if (help_index_url(url) and browser_request_security_okay(webserver_request) and help_index_acl(webserver_request))
    {
        webserver_request.reply = help_index(webserver_request, url);
        return;
    }

    const auto& route_table = get_route_table();

    if (const auto it = route_table.find(url); it != route_table.cend()) {
        if (browser_request_security_okay(webserver_request) and it->second.acl(webserver_request)) {
            webserver_request.reply = it->second.handler(webserver_request);
            return;
        }
    }

    // Downloads
    if (url == index_listing_url(url) and browser_request_security_okay(webserver_request) and index_listing_acl(
        webserver_request, url))
    {
        webserver_request.reply = index_listing(webserver_request, url);
        return;
    }

#ifdef HAVE_CLIENT
    if (extension == "tar")
    {
        http_stream_file(webserver_request, false);
        return;
    }
#endif

    if (url == sync_setup_url())
    {
        webserver_request.reply = sync_setup(webserver_request);
        return;
    }
    if (url == sync_settings_url())
    {
        webserver_request.reply = sync_settings(webserver_request);
        return;
    }
    if (url == sync_bibles_url())
    {
        webserver_request.reply = sync_bibles(webserver_request);
        return;
    }
    if (url == sync_notes_url())
    {
        webserver_request.reply = sync_notes(webserver_request);
        return;
    }
    if (extension == "sqlite")
    {
        if (filter_url_dirname(url) == filter_url_temp_dir())
        {
            http_stream_file(webserver_request, false);
            return;
        }
    }
    if (url == sync_usfmresources_url())
    {
        webserver_request.reply = sync_usfmresources(webserver_request);
        return;
    }
    if (url == sync_changes_url())
    {
        webserver_request.reply = sync_changes(webserver_request);
        return;
    }
    if (url == sync_files_url())
    {
        webserver_request.reply = sync_files(webserver_request);
        return;
    }
    if (url == sync_resources_url())
    {
        webserver_request.reply = sync_resources(webserver_request);
        return;
    }
    if (url == sync_mail_url())
    {
        webserver_request.reply = sync_mail(webserver_request);
        return;
    }

#ifdef HAVE_WINDOWS
    if (url == navigation_paratext_url())
    {
        webserver_request.reply = navigation_paratext(webserver_request);
        return;
    }
#endif

#ifdef HAVE_CLIENT
    // For security reasons, this is only available in a client configuration.
    if (url == assets_external_url())
    {
        webserver_request.reply = assets_external(webserver_request);
        return;
    }
#endif

    if (url == developer_delay_url() and developer_delay_acl())
    {
        webserver_request.reply = developer_delay();
        return;
    }

    // Forward the browser to the default home page.
    redirect_browser(webserver_request, index_index_url());
}

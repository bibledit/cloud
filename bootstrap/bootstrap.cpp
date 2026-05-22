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
#include <developer/logic.h>
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
#include <rss/feed.h>
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
#include <sprint/index.h>
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
bool browser_request_security_okay(const Webserver_Request& webserver_request)
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


// This function is the first function to be called after a client requests a page or file.
// Based on the request from the client,
// it decides which functions to call to obtain the response.
void bootstrap_index(Webserver_Request& webserver_request)
{
    // ReSharper disable once CppEntityAssignedButNoRead
    [[maybe_unused]] std::shared_ptr<Developer_Logic_Tracer> developer_logic_tracer{nullptr};
    if (config_globals_log_network)
    {
        developer_logic_tracer = std::make_shared<Developer_Logic_Tracer>(webserver_request);
    }
    // Record the POST request made to the web server.
    // This can be used for debugging.
    /*
    if (!webserver_request.post.empty () and config_logic_demo_enabled ()) {
      std::string contents;
      int seconds = filter::date::seconds_since_epoch ();
      std::string rfc822time = filter::date::rfc822 (seconds);
      contents.append (rfc822time + "\n");
      contents.append (webserver_request.get + "\n");
      std::string query;
      for (auto element : webserver_request.query) {
        query.append (element.first + "=" + element.second + " and ");
      }
      std::string post;
      for (auto element : webserver_request.post) {
        post.append (element.first + "=" + element.second + " and ");
      }
      contents.append ("query: " + query + "\n");
      contents.append ("post: " + post + "\n");
      std::string filename;
      filename = filter_url_create_root_path (filter_url_temp_dir (), "http-post-trace.txt");
      filter_url_file_put_contents_append (filename, contents);
    }
    */

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
    if (config::logic::version() != database::config::general::getInstalledDatabaseVersion())
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

    const auto request_matcher = [&url, &webserver_request] (const std::string& page_url,
        std::function<bool(Webserver_Request&)> acl)
    {
        return url == page_url and browser_request_security_okay(webserver_request) and acl(
        webserver_request);
    };

    // Home page and menu.

    if (request_matcher(index_index_url(), index_index_acl))
    {
        webserver_request.reply = index_index(webserver_request);
        return;
    }

    if (request_matcher(menu_index_url(), menu_index_acl))
    {
        webserver_request.reply = menu_index(webserver_request);
        return;
    }

    // Login and logout.
    if (request_matcher(session_login_url(), session_login_acl))
    {
        webserver_request.reply = session_login(webserver_request);
        return;
    }

    if (request_matcher(session_logout_url(), session_logout_acl))
    {
        webserver_request.reply = session_logout(webserver_request);
        return;
    }

    if (request_matcher(session_password_url(), session_password_acl))
    {
        webserver_request.reply = session_password(webserver_request);
        return;
    }

    if (request_matcher(session_signup_url(), session_signup_acl))
    {
        webserver_request.reply = session_signup(webserver_request);
        return;
    }

    if (request_matcher(session_switch_url(), session_switch_acl))
    {
        webserver_request.reply = session_switch(webserver_request);
        return;
    }

    // Bible menu.
    if (request_matcher(bible_manage_url(), bible_manage_acl))
    {
        webserver_request.reply = bible_manage(webserver_request);
        return;
    }

    if (request_matcher(bible_settings_url(), bible_settings_acl))
    {
        webserver_request.reply = bible_settings(webserver_request);
        return;
    }

    if (request_matcher(bible_book_url(), bible_book_acl))
    {
        webserver_request.reply = bible_book(webserver_request);
        return;
    }

    if (request_matcher(bible_chapter_url(), bible_chapter_acl))
    {
        webserver_request.reply = bible_chapter(webserver_request);
        return;
    }

    if (request_matcher(bible_import_url(), bible_import_acl))
    {
        webserver_request.reply = bible_import(webserver_request);
        return;
    }

    if (request_matcher(compare_index_url(), compare_index_acl))
    {
        webserver_request.reply = compare_index(webserver_request);
        return;
    }

    if (request_matcher(bible_order_url(), bible_order_acl))
    {
        webserver_request.reply = bible_order(webserver_request);
        return;
    }

    if (request_matcher(bible_css_url(), bible_css_acl))
    {
        webserver_request.reply = bible_css(webserver_request);
        return;
    }

    if (request_matcher(editusfm_index_url(), editusfm_index_acl))
    {
        webserver_request.reply = editusfm_index(webserver_request);
        return;
    }

    if (request_matcher(edit_index_url(), edit_index_acl))
    {
        webserver_request.reply = edit_index(webserver_request);
        return;
    }

    if (request_matcher(edit_position_url(), edit_position_acl))
    {
        webserver_request.reply = edit_position(webserver_request);
        return;
    }

    if (request_matcher(edit_navigate_url(), edit_navigate_acl))
    {
        webserver_request.reply = edit_navigate(webserver_request);
        return;
    }

    if (request_matcher(search_index_url(), search_index_acl))
    {
        webserver_request.reply = search_index(webserver_request);
        return;
    }

    if (request_matcher(workspace_index_url(), workspace_index_acl))
    {
        webserver_request.reply = workspace_index(webserver_request);
        return;
    }

    if (request_matcher(workspace_organize_url(), workspace_organize_acl))
    {
        webserver_request.reply = workspace_organize(webserver_request);
        return;
    }

    if (request_matcher(resource_bible2resource_url(), resource_bible2resource_acl))
    {
        webserver_request.reply = resource_bible2resource(webserver_request);
        return;
    }

    if (request_matcher(checks_index_url(), checks_index_acl))
    {
        webserver_request.reply = checks_index(webserver_request);
        return;
    }

    if (request_matcher(checks_settings_url(), checks_settings_acl))
    {
        webserver_request.reply = checks_settings(webserver_request);
        return;
    }

    if (request_matcher(consistency_index_url(), consistency_index_acl))
    {
        webserver_request.reply = consistency_index(webserver_request);
        return;
    }

    // Notes menu.
    if (request_matcher(notes_index_url(), notes_index_acl))
    {
        webserver_request.reply = notes_index(webserver_request);
        return;
    }

    if (request_matcher(notes_create_url(), notes_create_acl))
    {
        webserver_request.reply = notes_create(webserver_request);
        return;
    }

    if (request_matcher(notes_select_url(), notes_select_acl))
    {
        webserver_request.reply = notes_select(webserver_request);
        return;
    }

    if (request_matcher(notes_note_url(), notes_note_acl))
    {
        webserver_request.reply = notes_note(webserver_request);
        return;
    }

    if (request_matcher(notes_comment_url(), notes_comment_acl))
    {
        webserver_request.reply = notes_comment(webserver_request);
        return;
    }

    if (request_matcher(notes_actions_url(), notes_actions_acl))
    {
        webserver_request.reply = notes_actions(webserver_request);
        return;
    }

    if (request_matcher(notes_assign_1_url(), notes_assign_1_acl))
    {
        webserver_request.reply = notes_assign_1(webserver_request);
        return;
    }

    if (request_matcher(notes_assign_n_url(), notes_assign_n_acl))
    {
        webserver_request.reply = notes_assign_n(webserver_request);
        return;
    }

    if (request_matcher(notes_unassign_n_url(), notes_unassign_n_acl))
    {
        webserver_request.reply = notes_unassign_n(webserver_request);
        return;
    }

    if (request_matcher(notes_status_1_url(), notes_status_1_acl))
    {
        webserver_request.reply = notes_status_1(webserver_request);
        return;
    }

    if (request_matcher(notes_status_n_url(), notes_status_n_acl))
    {
        webserver_request.reply = notes_status_n(webserver_request);
        return;
    }

    if (request_matcher(notes_verses_url(), notes_verses_acl))
    {
        webserver_request.reply = notes_verses(webserver_request);
        return;
    }

    if (request_matcher(notes_severity_1_url(), notes_severity_1_acl))
    {
        webserver_request.reply = notes_severity_1(webserver_request);
        return;
    }

    if (request_matcher(notes_severity_n_url(), notes_severity_n_acl))
    {
        webserver_request.reply = notes_severity_n(webserver_request);
        return;
    }

    if (request_matcher(notes_bible_1_url(), notes_bible_1_acl))
    {
        webserver_request.reply = notes_bible_1(webserver_request);
        return;
    }

    if (request_matcher(notes_bible_n_url(), notes_bible_n_acl))
    {
        webserver_request.reply = notes_bible_n(webserver_request);
        return;
    }

    if (request_matcher(notes_bulk_url(), notes_bulk_acl))
    {
        webserver_request.reply = notes_bulk(webserver_request);
        return;
    }

    if (request_matcher(notes_edit_url(), notes_edit_acl))
    {
        webserver_request.reply = notes_edit(webserver_request);
        return;
    }

    if (request_matcher(notes_summary_url(), notes_summary_acl))
    {
        webserver_request.reply = notes_summary(webserver_request);
        return;
    }

    // Resources menu.
    if (request_matcher(resource_index_url(), resource_index_acl))
    {
        webserver_request.reply = resource_index(webserver_request);
        return;
    }

    if (request_matcher(resource_organize_url(), resource_organize_acl))
    {
        webserver_request.reply = resource_organize(webserver_request);
        return;
    }

    if (request_matcher(resource_manage_url(), resource_manage_acl))
    {
        webserver_request.reply = resource_manage(webserver_request);
        return;
    }

    if (request_matcher(resource_download_url(), resource_download_acl))
    {
        webserver_request.reply = resource_download(webserver_request);
        return;
    }

    if (request_matcher(resource_sword_url(), resource_sword_acl))
    {
        webserver_request.reply = resource_sword(webserver_request);
        return;
    }

    if (request_matcher(resource_select_url(), resource_select_acl))
    {
        webserver_request.reply = resource_select(webserver_request);
        return;
    }

    if (request_matcher(resource_cache_url(), resource_cache_acl))
    {
        webserver_request.reply = resource_cache(webserver_request);
        return;
    }

    if (request_matcher(resource_user9edit_url(), resource_user9edit_acl))
    {
        webserver_request.reply = resource_user9edit(webserver_request);
        return;
    }

    if (request_matcher(resource_user1edit_url(), resource_user1edit_acl))
    {
        webserver_request.reply = resource_user1edit(webserver_request);
        return;
    }

    if (request_matcher(resource_user9view_url(), resource_user9view_acl))
    {
        webserver_request.reply = resource_user9view(webserver_request);
        return;
    }

    if (request_matcher(resource_user1view_url(), resource_user1view_acl))
    {
        webserver_request.reply = resource_user1view(webserver_request);
        return;
    }

    if (request_matcher(resource_biblegateway_url(), resource_biblegateway_acl))
    {
        webserver_request.reply = resource_biblegateway(webserver_request);
        return;
    }

    if (request_matcher(resource_studylight_url(), resource_studylight_acl))
    {
        webserver_request.reply = resource_studylight(webserver_request);
        return;
    }

    if (request_matcher(journal_index_url(), journal_index_acl))
    {
        webserver_request.reply = journal_index(webserver_request);
        return;
    }

    if (request_matcher(changes_changes_url(), changes_changes_acl))
    {
        webserver_request.reply = changes_changes(webserver_request);
        return;
    }

    if (request_matcher(changes_change_url(), changes_change_acl))
    {
        webserver_request.reply = changes_change(webserver_request);
        return;
    }

    if (request_matcher(changes_manage_url(), changes_manage_acl))
    {
        webserver_request.reply = changes_manage(webserver_request);
        return;
    }

    if (request_matcher(changes_statistics_url(), changes_statistics_acl))
    {
        webserver_request.reply = changes_statistics(webserver_request);
        return;
    }

    // Planning menu.
    if (request_matcher(sprint_index_url(), sprint_index_acl))
    {
        webserver_request.reply = sprint_index(webserver_request);
        return;
    }

    // Tools menu.
    if (request_matcher(sendreceive_index_url(), sendreceive_index_acl))
    {
        webserver_request.reply = sendreceive_index(webserver_request);
        return;
    }

    if (request_matcher(manage_exports_url(), manage_exports_acl))
    {
        webserver_request.reply = manage_exports(webserver_request);
        return;
    }

    if (request_matcher(developer_index_url(), developer_index_acl))
    {
        webserver_request.reply = developer_index(webserver_request);
        return;
    }

    if (request_matcher(personalize_index_url(), personalize_index_acl))
    {
        webserver_request.reply = personalize_index(webserver_request);
        return;
    }

    if (request_matcher(manage_users_url(), manage_users_acl))
    {
        webserver_request.reply = manage_users(webserver_request);
        return;
    }

    if (request_matcher(manage_index_url(), manage_index_acl))
    {
        webserver_request.reply = manage_index(webserver_request);
        return;
    }

    if (request_matcher(system_index_url(), system_index_acl))
    {
        webserver_request.reply = system_index(webserver_request);
        return;
    }

    if (request_matcher(system_googletranslate_url(), system_googletranslate_acl))
    {
        webserver_request.reply = system_googletranslate(webserver_request);
        return;
    }

    if (request_matcher(email_index_url(), email_index_acl))
    {
        webserver_request.reply = email_index(webserver_request);
        return;
    }

    if (request_matcher(styles_indexm_url(), styles_indexm_acl))
    {
        webserver_request.reply = styles_indexm(webserver_request);
        return;
    }

    if (request_matcher(styles_new_url(), styles_new_acl))
    {
        webserver_request.reply = styles_new(webserver_request);
        return;
    }

    if (request_matcher(styles_sheetm_url(), styles_sheetm_acl))
    {
        webserver_request.reply = styles_sheetm(webserver_request);
        return;
    }

    if (request_matcher(styles_view_url(), styles_view_acl))
    {
        webserver_request.reply = styles_view(webserver_request);
        return;
    }

    if (request_matcher(versification_index_url(), versification_index_acl))
    {
        webserver_request.reply = versification_index(webserver_request);
        return;
    }

    if (request_matcher(versification_system_url(), versification_system_acl))
    {
        webserver_request.reply = versification_system(webserver_request);
        return;
    }

    if (request_matcher(collaboration_index_url(), collaboration_index_acl))
    {
        webserver_request.reply = collaboration_index(webserver_request);
        return;
    }

    if (request_matcher(client_index_url(), client_index_acl))
    {
        webserver_request.reply = client_index(webserver_request);
        return;
    }

    if (request_matcher(mapping_index_url(), mapping_index_acl))
    {
        webserver_request.reply = mapping_index(webserver_request);
        return;
    }

    if (request_matcher(mapping_map_url(), mapping_map_acl))
    {
        webserver_request.reply = mapping_map(webserver_request);
        return;
    }

    if (request_matcher(paratext_index_url(), paratext_index_acl))
    {
        webserver_request.reply = paratext_index(webserver_request);
        return;
    }

    // Help menu.
    if (help_index_url(url) and browser_request_security_okay(webserver_request) and help_index_acl(webserver_request))
    {
        webserver_request.reply = help_index(webserver_request, url);
        return;
    }

    // User menu.
    if (request_matcher(user_notifications_url(), user_notifications_acl))
    {
        webserver_request.reply = user_notifications(webserver_request);
        return;
    }

    if (request_matcher(user_account_url(), user_account_acl))
    {
        webserver_request.reply = user_account(webserver_request);
        return;
    }

    // Public feedback menu.
    if (request_matcher(public_index_url(), public_index_acl))
    {
        webserver_request.reply = public_index(webserver_request);
        return;
    }

    if (request_matcher(public_login_url(), public_login_acl))
    {
        webserver_request.reply = public_login(webserver_request);
        return;
    }

    if (request_matcher(public_chapter_url(), public_chapter_acl))
    {
        webserver_request.reply = public_chapter(webserver_request);
        return;
    }

    if (request_matcher(public_notes_url(), public_notes_acl))
    {
        webserver_request.reply = public_notes(webserver_request);
        return;
    }

    if (request_matcher(public_new_url(), public_new_acl))
    {
        webserver_request.reply = public_new(webserver_request);
        return;
    }

    if (request_matcher(public_create_url(), public_create_acl))
    {
        webserver_request.reply = public_create(webserver_request);
        return;
    }

    if (request_matcher(public_note_url(), public_note_acl))
    {
        webserver_request.reply = public_note(webserver_request);
        return;
    }

    if (request_matcher(public_comment_url(), public_comment_acl))
    {
        webserver_request.reply = public_comment(webserver_request);
        return;
    }

    if (request_matcher(jobs_index_url(), jobs_index_acl))
    {
        webserver_request.reply = jobs_index(webserver_request);
        return;
    }

    if (request_matcher(search_all_url(), search_all_acl))
    {
        webserver_request.reply = search_all(webserver_request);
        return;
    }

    if (request_matcher(search_replace_url(), search_replace_acl))
    {
        webserver_request.reply = search_replace(webserver_request);
        return;
    }

    if (request_matcher(search_search2_url(), search_search2_acl))
    {
        webserver_request.reply = search_search2(webserver_request);
        return;
    }

    if (request_matcher(search_replace2_url(), search_replace2_acl))
    {
        webserver_request.reply = search_replace2(webserver_request);
        return;
    }

    if (request_matcher(search_similar_url(), search_similar_acl))
    {
        webserver_request.reply = search_similar(webserver_request);
        return;
    }

    if (request_matcher(search_strongs_url(), search_strongs_acl))
    {
        webserver_request.reply = search_strongs(webserver_request);
        return;
    }

    if (request_matcher(search_strong_url(), search_strong_acl))
    {
        webserver_request.reply = search_strong(webserver_request);
        return;
    }

    if (request_matcher(search_originals_url(), search_originals_acl))
    {
        webserver_request.reply = search_originals(webserver_request);
        return;
    }

    if (request_matcher(workspace_settings_url(), workspace_settings_acl))
    {
        webserver_request.reply = workspace_settings(webserver_request);
        return;
    }

    if (request_matcher(collaboration_settings_url(), collaboration_settings_acl))
    {
        webserver_request.reply = collaboration_settings(webserver_request);
        return;
    }

    if (request_matcher(checks_settingspatterns_url(), checks_settingspatterns_acl))
    {
        webserver_request.reply = checks_settingspatterns(webserver_request);
        return;
    }

    if (request_matcher(checks_settingssentences_url(), checks_settingssentences_acl))
    {
        webserver_request.reply = checks_settingssentences(webserver_request);
        return;
    }

    if (request_matcher(checks_settingspairs_url(), checks_settingspairs_acl))
    {
        webserver_request.reply = checks_settingspairs(webserver_request);
        return;
    }

    if (request_matcher(checks_suppress_url(), checks_suppress_acl))
    {
        webserver_request.reply = checks_suppress(webserver_request);
        return;
    }

    if (request_matcher(webbible_search_url(), webbible_search_acl))
    {
        webserver_request.reply = webbible_search(webserver_request);
        return;
    }

    if (request_matcher(manage_write_url(), manage_write_acl))
    {
        webserver_request.reply = manage_write(webserver_request);
        return;
    }

    if (request_matcher(manage_bibles_url(), manage_bibles_acl))
    {
        webserver_request.reply = manage_bibles(webserver_request);
        return;
    }

    if (request_matcher(manage_privileges_url(), manage_privileges_acl))
    {
        webserver_request.reply = manage_privileges(webserver_request);
        return;
    }

    if (request_matcher(editor_select_url(), editor_select_acl))
    {
        webserver_request.reply = editor_select(webserver_request);
        return;
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

    if (request_matcher(navigation_update_url(), navigation_update_acl))
    {
        webserver_request.reply = navigation_update(webserver_request);
        return;
    }

    if (request_matcher(navigation_poll_url(), navigation_poll_acl))
    {
        webserver_request.reply = navigation_poll(webserver_request);
        return;
    }

#ifdef HAVE_WINDOWS
    if (url == navigation_paratext_url())
    {
        webserver_request.reply = navigation_paratext(webserver_request);
        return;
    }
#endif

    if (request_matcher(edit_preview_url(), edit_preview_acl))
    {
        webserver_request.reply = edit_preview(webserver_request);
        return;
    }

    if (request_matcher(editusfm_focus_url(), editusfm_focus_acl))
    {
        webserver_request.reply = editusfm_focus(webserver_request);
        return;
    }

    if (request_matcher(editusfm_load_url(), editusfm_load_acl))
    {
        webserver_request.reply = editusfm_load(webserver_request);
        return;
    }

    if (request_matcher(editusfm_offset_url(), editusfm_offset_acl))
    {
        webserver_request.reply = editusfm_offset(webserver_request);
        return;
    }

    if (request_matcher(editusfm_save_url(), editusfm_save_acl))
    {
        webserver_request.reply = editusfm_save(webserver_request);
        return;
    }

    if (request_matcher(edit_edit_url(), edit_edit_acl))
    {
        webserver_request.reply = edit_edit(webserver_request);
        return;
    }

    if (request_matcher(edit_id_url(), edit_id_acl))
    {
        webserver_request.reply = edit_id(webserver_request);
        return;
    }

    if (request_matcher(edit_load_url(), edit_load_acl))
    {
        webserver_request.reply = edit_load(webserver_request);
        return;
    }

    if (request_matcher(edit_save_url(), edit_save_acl))
    {
        webserver_request.reply = edit_save(webserver_request);
        return;
    }

    if (request_matcher(edit_styles_url(), edit_styles_acl))
    {
        webserver_request.reply = edit_styles(webserver_request);
        return;
    }

    if (request_matcher(search_getids_url(), search_getids_acl))
    {
        webserver_request.reply = search_getids(webserver_request);
        return;
    }

    if (request_matcher(search_replacepre_url(), search_replacepre_acl))
    {
        webserver_request.reply = search_replacepre(webserver_request);
        return;
    }

    if (request_matcher(search_replacego_url(), search_replacego_acl))
    {
        webserver_request.reply = search_replacego(webserver_request);
        return;
    }

    if (request_matcher(search_replacepre2_url(), search_replacepre2_acl))
    {
        webserver_request.reply = search_replacepre2(webserver_request);
        return;
    }

    if (request_matcher(search_getids2_url(), search_getids2_acl))
    {
        webserver_request.reply = search_getids2(webserver_request);
        return;
    }

    if (request_matcher(search_replacego2_url(), search_replacego2_acl))
    {
        webserver_request.reply = search_replacego2(webserver_request);
        return;
    }

    if (request_matcher(resource_get_url(), resource_get_acl))
    {
        webserver_request.reply = resource_get(webserver_request);
        return;
    }

    if (request_matcher(resource_unload_url(), resource_unload_acl))
    {
        webserver_request.reply = resource_unload(webserver_request);
        return;
    }

    if (request_matcher(notes_poll_url(), notes_poll_acl))
    {
        webserver_request.reply = notes_poll(webserver_request);
        return;
    }

    if (request_matcher(notes_notes_url(), notes_notes_acl))
    {
        webserver_request.reply = notes_notes(webserver_request);
        return;
    }

    if (request_matcher(notes_click_url(), notes_click_acl))
    {
        webserver_request.reply = notes_click(webserver_request);
        return;
    }

    if (request_matcher(consistency_poll_url(), consistency_poll_acl))
    {
        webserver_request.reply = consistency_poll(webserver_request);
        return;
    }

    if (request_matcher(consistency_input_url(), consistency_input_acl))
    {
        webserver_request.reply = consistency_input(webserver_request);
        return;
    }

    if (request_matcher(lexicon_definition_url(), lexicon_definition_acl))
    {
        webserver_request.reply = lexicon_definition(webserver_request);
        return;
    }

#ifdef HAVE_CLIENT
    // For security reasons, this is only available in a client configuration.
    if (url == assets_external_url())
    {
        webserver_request.reply = assets_external(webserver_request);
        return;
    }
#endif

    if (request_matcher(rss_feed_url(), rss_feed_acl))
    {
        webserver_request.reply = rss_feed(webserver_request);
        return;
    }

    if (request_matcher(edit_update_url(), edit_update_acl))
    {
        webserver_request.reply = edit_update(webserver_request);
        return;
    }

    if (request_matcher(editor_id_url(), editor_id_acl))
    {
        webserver_request.reply = editor_id(webserver_request);
        return;
    }

    if (request_matcher(editor_style_url(), editor_style_acl))
    {
        webserver_request.reply = editor_style(webserver_request);
        return;
    }

    if (request_matcher(editone_index_url(), editone_index_acl))
    {
        webserver_request.reply = editone_index(webserver_request);
        return;
    }

    if (request_matcher(editone_load_url(), editone_load_acl))
    {
        webserver_request.reply = editone_load(webserver_request);
        return;
    }

    if (request_matcher(editone_save_url(), editone_save_acl))
    {
        webserver_request.reply = editone_save(webserver_request);
        return;
    }

    if (request_matcher(editone_verse_url(), editone_verse_acl))
    {
        webserver_request.reply = editone_verse(webserver_request);
        return;
    }

    if (request_matcher(editone_update_url(), editone_update_acl))
    {
        webserver_request.reply = editone_update(webserver_request);
        return;
    }

    if (request_matcher(read_index_url(), read_index_acl))
    {
        webserver_request.reply = read_index(webserver_request);
        return;
    }

    if (request_matcher(read_load_url(), read_load_acl))
    {
        webserver_request.reply = read_load(webserver_request);
        return;
    }

    if (request_matcher(read_verse_url(), read_verse_acl))
    {
        webserver_request.reply = read_verse(webserver_request);
        return;
    }

    if (request_matcher(resource_divider_url(), resource_divider_acl))
    {
        webserver_request.reply = resource_divider(webserver_request);
        return;
    }

    if (request_matcher(session_confirm_url(), session_confirm_acl))
    {
        webserver_request.reply = session_confirm(webserver_request);
        return;
    }

    if (request_matcher(resource_comparative9edit_url(), resource_comparative9edit_acl))
    {
        webserver_request.reply = resource_comparative9edit(webserver_request);
        return;
    }

    if (request_matcher(resource_comparative1edit_url(), resource_comparative1edit_acl))
    {
        webserver_request.reply = resource_comparative1edit(webserver_request);
        return;
    }

    if (request_matcher(resource_translated9edit_url(), resource_translated9edit_acl))
    {
        webserver_request.reply = resource_translated9edit(webserver_request);
        return;
    }

    if (request_matcher(resource_translated1edit_url(), resource_translated1edit_acl))
    {
        webserver_request.reply = resource_translated1edit(webserver_request);
        return;
    }

    if (url == developer_delay_url() and developer_delay_acl())
    {
        webserver_request.reply = developer_delay();
        return;
    }

    if (request_matcher(images_index_url(), images_index_acl))
    {
        webserver_request.reply = images_index(webserver_request);
        return;
    }

    if (request_matcher(images_view_url(), images_view_acl))
    {
        webserver_request.reply = images_view(webserver_request);
        return;
    }

    if (request_matcher(images_fetch_url(), images_fetch_acl))
    {
        webserver_request.reply = images_fetch(webserver_request);
        return;
    }

    // Forward the browser to the default home page.
    redirect_browser(webserver_request, index_index_url());
}

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
   

#include <bootstrap/bootstrap.h>
#include <webserver/request.h>
#include <webserver/http.h>
#include <filter/url.h>
#include <index/index.h>
#include <session/login.h>
#include <session/logout.h>
#include <session/password.h>
#include <session/signup.h>
#include <session/switch.h>
#include <database/config/general.h>
#include <database/cache.h>
#include <setup/index.h>
#include <journal/index.h>
#include <config/globals.h>
#include <help/index.h>
#include <email/index.h>
#include <user/notifications.h>
#include <user/account.h>
#include <manage/index.h>
#include <manage/users.h>
#include <manage/exports.h>
#include <manage/write.h>
#include <manage/bibles.h>
#include <manage/privileges.h>
#include <system/index.h>
#include <system/googletranslate.h>
#include <collaboration/index.h>
#include <collaboration/settings.h>
#include <styles/indexm.h>
#include <styles/new.h>
#include <styles/sheetm.h>
#include <styles/view.h>
#include <versification/index.h>
#include <versification/system.h>
#include <bb/manage.h>
#include <bb/settings.h>
#include <bb/book.h>
#include <bb/chapter.h>
#include <bb/import.h>
#include <bb/order.h>
#include <bb/css.h>
#include <compare/index.h>
#include <jobs/index.h>
#include <navigation/update.h>
#include <navigation/poll.h>
#include <navigation/paratext.h>
#include <editusfm/index.h>
#include <editusfm/focus.h>
#include <editusfm/load.h>
#include <editusfm/offset.h>
#include <editusfm/save.h>
#include <edit/index.h>
#include <edit/edit.h>
#include <edit/id.h>
#include <edit/load.h>
#include <edit/save.h>
#include <edit/styles.h>
#include <edit/preview.h>
#include <edit/position.h>
#include <edit/navigate.h>
#include <search/all.h>
#include <search/index.h>
#include <search/replace.h>
#include <search/getids.h>
#include <search/replacepre.h>
#include <search/replacego.h>
#include <search/search2.h>
#include <search/replace2.h>
#include <search/replacepre2.h>
#include <search/getids2.h>
#include <search/replacego2.h>
#include <search/similar.h>
#include <search/strongs.h>
#include <search/strong.h>
#include <search/originals.h>
#include <workspace/index.h>
#include <workspace/organize.h>
#include <workspace/settings.h>
#include <sendreceive/index.h>
#include <client/index.h>
#include <sync/setup.h>
#include <sync/settings.h>
#include <sync/bibles.h>
#include <sync/notes.h>
#include <sync/usfmresources.h>
#include <sync/changes.h>
#include <sync/files.h>
#include <sync/resources.h>
#include <sync/mail.h>
#include <resource/index.h>
#include <resource/organize.h>
#include <resource/get.h>
#include <resource/bb2resource.h>
#include <resource/manage.h>
#include <resource/download.h>
#include <resource/select.h>
#include <resource/sword.h>
#include <resource/cache.h>
#include <resource/user9edit.h>
#include <resource/user1edit.h>
#include <resource/user9view.h>
#include <resource/user1view.h>
#include <resource/bbgateway.h>
#include <resource/studylight.h>
#include <resource/unload.h>
#include <locale/translate.h>
#include <mapping/index.h>
#include <mapping/map.h>
#include <notes/index.h>
#include <notes/poll.h>
#include <notes/notes.h>
#include <notes/create.h>
#include <notes/select.h>
#include <notes/note.h>
#include <notes/comment.h>
#include <notes/actions.h>
#include <notes/assign-1.h>
#include <notes/assign-n.h>
#include <notes/unassign-n.h>
#include <notes/status-1.h>
#include <notes/status-n.h>
#include <notes/verses.h>
#include <notes/severity-1.h>
#include <notes/severity-n.h>
#include <notes/bb-1.h>
#include <notes/bb-n.h>
#include <notes/bulk.h>
#include <notes/edit.h>
#include <notes/summary.h>
#include <notes/click.h>
#include <changes/changes.h>
#include <changes/change.h>
#include <changes/manage.h>
#include <changes/statistics.h>
#include <index/listing.h>
#include <sprint/index.h>
#include <checks/index.h>
#include <checks/settings.h>
#include <checks/settingspatterns.h>
#include <checks/settingssentences.h>
#include <checks/settingspairs.h>
#include <consistency/index.h>
#include <checks/suppress.h>
#include <consistency/poll.h>
#include <consistency/input.h>
#include <webbb/search.h>
#include <developer/index.h>
#include <paratext/index.h>
#include <personalize/index.h>
#include <menu/index.h>
#include <fonts/logic.h>
#include <lexicon/definition.h>
#include <database/logs.h>
#include <public/index.h>
#include <public/login.h>
#include <public/chapter.h>
#include <public/notes.h>
#include <public/new.h>
#include <public/create.h>
#include <public/note.h>
#include <public/comment.h>
#include <editor/select.h>
#include <rss/feed.h>
#include <assets/external.h>
#include <system/logic.h>
#include <filter/date.h>
#include <filter/string.h>
#include <journal/logic.h>
#include <editor/id.h>
#include <editor/style.h>
#include <edit/update.h>
#include <editone/index.h>
#include <editone/load.h>
#include <editone/save.h>
#include <editone/verse.h>
#include <editone/update.h>
#include <read/index.h>
#include <read/load.h>
#include <read/verse.h>
#include <resource/divider.h>
#include <session/confirm.h>
#include <resource/comparative9edit.h>
#include <resource/comparative1edit.h>
#include <resource/translated9edit.h>
#include <resource/translated1edit.h>
#include <developer/logic.h>
#include <developer/delay.h>
#include <images/index.h>
#include <images/view.h>
#include <images/fetch.h>


// Check whether a request coming from the browser is considered secure enough.
// It returns true if the security is okay.
bool browser_request_security_okay (Webserver_Request& webserver_request)
{
  // If the request is made via https, the security is OK.
  if (webserver_request.secure) {
    return true;
  }

  // At this stage the request is made via plain http.
  // If https is not enforced for the browser, the security is OK.
  if (!config_globals_enforce_https_browser) {
    return true;
  }
  
  // Not secure enough:
  return false;
}


// This function is the first function to be called after a client requests a page or file.
// Based on the request from the client,
// it decides which functions to call to obtain the response.
void bootstrap_index (Webserver_Request& webserver_request)
{
  std::shared_ptr<Developer_Logic_Tracer> developer_logic_tracer {nullptr};
  if (config_globals_log_network) {
    developer_logic_tracer = std::make_shared<Developer_Logic_Tracer>(webserver_request);
  }
  
  // Record the POST request made to the web server.
  // This can be used for debugging.
  /*
  if (!webserver_request.post.empty () && config_logic_demo_enabled ()) {
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
  
  const std::string extension = filter_url_get_extension (webserver_request.get);
  const std::string url = webserver_request.get.substr (1);
  
  
  // Serve graphics, stylesheets, JavaScript, fonts, and so on.
  // Use direct streaming for low memory usage.
  if (   (extension == "ico")
      || (filter_url_is_image (extension))
      || (extension == "css")
      || (extension == "js")
      || (fonts::logic::is_font (extension))
      || (extension == "sh")
      || (extension == "map")
      || (extension == "webmanifest")
      ) {
    http_stream_file (webserver_request, true);
    return;
  }

  // Serve resource downloads.
  if ((extension == "sqlite") && (webserver_request.get.find (database::cache::sql::fragment ()) != std::string::npos)) {
    http_stream_file (webserver_request, false);
    return;
  }
  
  // Serve initialization notice.
  if (config::logic::version () != database::config::general::getInstalledDatabaseVersion ()) {
    webserver_request.reply = setup_initialization_notice ();
    return;
  }
  
  // Force setup.
  if (config::logic::version () != database::config::general::get_installed_interface_version ()) {
    webserver_request.reply = setup_index (webserver_request);
    return;
  }

  // Home page and menu.
  if ((url == index_index_url ()) && browser_request_security_okay (webserver_request) && index_index_acl (webserver_request)) {
    webserver_request.reply = index_index (webserver_request);
    return;
  }
  
  if ((url == menu_index_url ()) && browser_request_security_okay (webserver_request) && menu_index_acl (webserver_request)) {
    webserver_request.reply = menu_index (webserver_request);
    return;
  }
  
  // Login and logout.
  if ((url == session_login_url ()) && browser_request_security_okay (webserver_request) && session_login_acl (webserver_request)) {
    webserver_request.reply = session_login (webserver_request);
    return;
  }
  
  if ((url == session_logout_url ()) && browser_request_security_okay (webserver_request) && session_logout_acl (webserver_request)) {
    webserver_request.reply = session_logout (webserver_request);
    return;
  }
  
  if ((url == session_password_url ()) && browser_request_security_okay (webserver_request) && session_password_acl (webserver_request)) {
    webserver_request.reply = session_password (webserver_request);
    return;
  }
  
  if ((url == session_signup_url ()) && browser_request_security_okay (webserver_request) && session_signup_acl (webserver_request)) {
    webserver_request.reply = session_signup (webserver_request);
    return;
  }
  
  if ((url == session_switch_url ()) && browser_request_security_okay (webserver_request) && session_switch_acl (webserver_request)) {
    webserver_request.reply = session_switch (webserver_request);
    return;
  }
  
  // Bible menu.
  if ((url == bible_manage_url ()) && browser_request_security_okay (webserver_request) && bible_manage_acl (webserver_request)) {
    webserver_request.reply = bible_manage (webserver_request);
    return;
  }
  
  if ((url == bible_settings_url ()) && browser_request_security_okay (webserver_request) && bible_settings_acl (webserver_request)) {
    webserver_request.reply = bible_settings (webserver_request);
    return;
  }
  
  if ((url == bible_book_url ()) && browser_request_security_okay (webserver_request) && bible_book_acl (webserver_request)) {
    webserver_request.reply = bible_book (webserver_request);
    return;
  }
  
  if ((url == bible_chapter_url ()) && browser_request_security_okay (webserver_request) && bible_chapter_acl (webserver_request)) {
    webserver_request.reply = bible_chapter (webserver_request);
    return;
  }
  
  if ((url == bible_import_url ()) && browser_request_security_okay (webserver_request) && bible_import_acl (webserver_request)) {
    webserver_request.reply = bible_import (webserver_request);
    return;
  }
  
  if ((url == compare_index_url ()) && browser_request_security_okay (webserver_request) && compare_index_acl (webserver_request)) {
    webserver_request.reply = compare_index (webserver_request);
    return;
  }
  
  if ((url == bible_order_url ()) && browser_request_security_okay (webserver_request) && bible_order_acl (webserver_request)) {
    webserver_request.reply = bible_order (webserver_request);
    return;
  }
  
  if ((url == bible_css_url ()) && browser_request_security_okay (webserver_request) && bible_css_acl (webserver_request)) {
    webserver_request.reply = bible_css (webserver_request);
    return;
  }
  
  if ((url == editusfm_index_url ()) && browser_request_security_okay (webserver_request) && editusfm_index_acl (webserver_request)) {
    webserver_request.reply = editusfm_index (webserver_request);
    return;
  }
  
  if ((url == edit_index_url ()) && browser_request_security_okay (webserver_request) && edit_index_acl (webserver_request)) {
    webserver_request.reply = edit_index (webserver_request);
    return;
  }
  
  if ((url == edit_position_url ()) && browser_request_security_okay (webserver_request) && edit_position_acl (webserver_request)) {
    webserver_request.reply = edit_position (webserver_request);
    return;
  }

  if ((url == edit_navigate_url ()) && browser_request_security_okay (webserver_request) && edit_navigate_acl (webserver_request)) {
    webserver_request.reply = edit_navigate (webserver_request);
    return;
  }
  
  if ((url == search_index_url ()) && browser_request_security_okay (webserver_request) && search_index_acl (webserver_request)) {
    webserver_request.reply = search_index (webserver_request);
    return;
  }
  
  if ((url == workspace_index_url ()) && browser_request_security_okay (webserver_request) && workspace_index_acl (webserver_request)) {
    webserver_request.reply = workspace_index (webserver_request);
    return;
  }
  
  if ((url == workspace_organize_url ()) && browser_request_security_okay (webserver_request) && workspace_organize_acl (webserver_request)) {
    webserver_request.reply = workspace_organize (webserver_request);
    return;
  }
  
  if ((url == resource_bible2resource_url ()) && browser_request_security_okay (webserver_request) && resource_bible2resource_acl (webserver_request)) {
    webserver_request.reply = resource_bible2resource (webserver_request);
    return;
  }
  
  if ((url == checks_index_url ()) && browser_request_security_okay (webserver_request) && checks_index_acl (webserver_request)) {
    webserver_request.reply = checks_index (webserver_request);
    return;
  }
  
  if ((url == checks_settings_url ()) && browser_request_security_okay (webserver_request) && checks_settings_acl (webserver_request)) {
    webserver_request.reply = checks_settings (webserver_request);
    return;
  }
  
  if ((url == consistency_index_url ()) && browser_request_security_okay (webserver_request) && consistency_index_acl (webserver_request)) {
    webserver_request.reply = consistency_index (webserver_request);
    return;
  }
  
  // Notes menu.
  if ((url == notes_index_url ()) && browser_request_security_okay (webserver_request) && notes_index_acl (webserver_request)) {
    webserver_request.reply = notes_index (webserver_request);
    return;
  }
  
  if ((url == notes_create_url ()) && browser_request_security_okay (webserver_request) && notes_create_acl (webserver_request)) {
    webserver_request.reply = notes_create (webserver_request);
    return;
  }
  
  if ((url == notes_select_url ()) && browser_request_security_okay (webserver_request) && notes_select_acl (webserver_request)) {
    webserver_request.reply = notes_select (webserver_request);
    return;
  }
  
  if ((url == notes_note_url ()) && browser_request_security_okay (webserver_request) && notes_note_acl (webserver_request)) {
    webserver_request.reply = notes_note (webserver_request);
    return;
  }
  
  if ((url == notes_comment_url ()) && browser_request_security_okay (webserver_request) && notes_comment_acl (webserver_request)) {
    webserver_request.reply = notes_comment (webserver_request);
    return;
  }
  
  if ((url == notes_actions_url ()) && browser_request_security_okay (webserver_request) && notes_actions_acl (webserver_request)) {
    webserver_request.reply = notes_actions (webserver_request);
    return;
  }
  
  if ((url == notes_assign_1_url ()) && browser_request_security_okay (webserver_request) && notes_assign_1_acl (webserver_request)) {
    webserver_request.reply = notes_assign_1 (webserver_request);
    return;
  }
  
  if ((url == notes_assign_n_url ()) && browser_request_security_okay (webserver_request) && notes_assign_n_acl (webserver_request)) {
    webserver_request.reply = notes_assign_n (webserver_request);
    return;
  }
  
  if ((url == notes_unassign_n_url ()) && browser_request_security_okay (webserver_request) && notes_unassign_n_acl (webserver_request)) {
    webserver_request.reply = notes_unassign_n (webserver_request);
    return;
  }
  
  if ((url == notes_status_1_url ()) && browser_request_security_okay (webserver_request) && notes_status_1_acl (webserver_request)) {
    webserver_request.reply = notes_status_1 (webserver_request);
    return;
  }
  
  if ((url == notes_status_n_url ()) && browser_request_security_okay (webserver_request) && notes_status_n_acl (webserver_request)) {
    webserver_request.reply = notes_status_n (webserver_request);
    return;
  }
  
  if ((url == notes_verses_url ()) && browser_request_security_okay (webserver_request) && notes_verses_acl (webserver_request)) {
    webserver_request.reply = notes_verses (webserver_request);
    return;
  }
  
  if ((url == notes_severity_1_url ()) && browser_request_security_okay (webserver_request) && notes_severity_1_acl (webserver_request)) {
    webserver_request.reply = notes_severity_1 (webserver_request);
    return;
  }
  
  if ((url == notes_severity_n_url ()) && browser_request_security_okay (webserver_request) && notes_severity_n_acl (webserver_request)) {
    webserver_request.reply = notes_severity_n (webserver_request);
    return;
  }
  
  if ((url == notes_bible_1_url ()) && browser_request_security_okay (webserver_request) && notes_bible_1_acl (webserver_request)) {
    webserver_request.reply = notes_bible_1 (webserver_request);
    return;
  }
  
  if ((url == notes_bible_n_url ()) && browser_request_security_okay (webserver_request) && notes_bible_n_acl (webserver_request)) {
    webserver_request.reply = notes_bible_n (webserver_request);
    return;
  }
  
  if ((url == notes_bulk_url ()) && browser_request_security_okay (webserver_request) && notes_bulk_acl (webserver_request)) {
    webserver_request.reply = notes_bulk (webserver_request);
    return;
  }
  
  if ((url == notes_edit_url ()) && browser_request_security_okay (webserver_request) && notes_edit_acl (webserver_request)) {
    webserver_request.reply = notes_edit (webserver_request);
    return;
  }
  
  if ((url == notes_summary_url ()) && browser_request_security_okay (webserver_request) && notes_summary_acl (webserver_request)) {
    webserver_request.reply = notes_summary (webserver_request);
    return;
  }

  // Resources menu.
  if ((url == resource_index_url ()) && browser_request_security_okay (webserver_request) && resource_index_acl (webserver_request)) {
    webserver_request.reply = resource_index (webserver_request);
    return;
  }
  
  if ((url == resource_organize_url ()) && browser_request_security_okay (webserver_request) && resource_organize_acl (webserver_request)) {
    webserver_request.reply = resource_organize (webserver_request);
    return;
  }
  
  if ((url == resource_manage_url ()) && browser_request_security_okay (webserver_request) && resource_manage_acl (webserver_request)) {
    webserver_request.reply = resource_manage (webserver_request);
    return;
  }
  
  if ((url == resource_download_url ()) && browser_request_security_okay (webserver_request) && resource_download_acl (webserver_request)) {
    webserver_request.reply = resource_download (webserver_request);
    return;
  }
  
  if ((url == resource_sword_url ()) && browser_request_security_okay (webserver_request) && resource_sword_acl (webserver_request)) {
    webserver_request.reply = resource_sword (webserver_request);
    return;
  }
  
  if ((url == resource_select_url ()) && browser_request_security_okay (webserver_request) && resource_select_acl (webserver_request)) {
    webserver_request.reply = resource_select (webserver_request);
    return;
  }
  
  if ((url == resource_cache_url ()) && browser_request_security_okay (webserver_request) && resource_cache_acl (webserver_request)) {
    webserver_request.reply = resource_cache (webserver_request);
    return;
  }

  if ((url == resource_user9edit_url ()) && browser_request_security_okay (webserver_request) && resource_user9edit_acl (webserver_request)) {
    webserver_request.reply = resource_user9edit (webserver_request);
    return;
  }

  if ((url == resource_user1edit_url ()) && browser_request_security_okay (webserver_request) && resource_user1edit_acl (webserver_request)) {
    webserver_request.reply = resource_user1edit (webserver_request);
    return;
  }

  if ((url == resource_user9view_url ()) && browser_request_security_okay (webserver_request) && resource_user9view_acl (webserver_request)) {
    webserver_request.reply = resource_user9view (webserver_request);
    return;
  }
  
  if ((url == resource_user1view_url ()) && browser_request_security_okay (webserver_request) && resource_user1view_acl (webserver_request)) {
    webserver_request.reply = resource_user1view (webserver_request);
    return;
  }

  if ((url == resource_biblegateway_url ()) && browser_request_security_okay (webserver_request) && resource_biblegateway_acl (webserver_request)) {
    webserver_request.reply = resource_biblegateway (webserver_request);
    return;
  }

  if ((url == resource_studylight_url ()) && browser_request_security_okay (webserver_request) && resource_studylight_acl (webserver_request)) {
    webserver_request.reply = resource_studylight (webserver_request);
    return;
  }
  
  if ((url == journal_index_url ()) && browser_request_security_okay (webserver_request) && journal_index_acl (webserver_request)) {
    webserver_request.reply = journal_index (webserver_request);
    return;
  }
  
  if ((url == changes_changes_url ()) && browser_request_security_okay (webserver_request) && changes_changes_acl (webserver_request)) {
    webserver_request.reply = changes_changes (webserver_request);
    return;
  }
  
  if ((url == changes_change_url ()) && browser_request_security_okay (webserver_request) && changes_change_acl (webserver_request)) {
    webserver_request.reply = changes_change (webserver_request);
    return;
  }
  
  if ((url == changes_manage_url ()) && browser_request_security_okay (webserver_request) && changes_manage_acl (webserver_request)) {
    webserver_request.reply = changes_manage (webserver_request);
    return;
  }
  
  if ((url == changes_statistics_url ()) && browser_request_security_okay (webserver_request) && changes_statistics_acl (webserver_request)) {
    webserver_request.reply = changes_statistics (webserver_request);
    return;
  }

  // Planning menu.
  if ((url == sprint_index_url ()) && browser_request_security_okay (webserver_request) && sprint_index_acl (webserver_request)) {
    webserver_request.reply = sprint_index (webserver_request);
    return;
  }
  
  // Tools menu.
  if ((url == sendreceive_index_url ()) && browser_request_security_okay (webserver_request) && sendreceive_index_acl (webserver_request)) {
    webserver_request.reply = sendreceive_index (webserver_request);
    return;
  }
  
  if ((url == manage_exports_url ()) && browser_request_security_okay (webserver_request) && manage_exports_acl (webserver_request)) {
    webserver_request.reply = manage_exports (webserver_request);
    return;
  }
  
  if ((url == developer_index_url ()) && browser_request_security_okay (webserver_request) && developer_index_acl (webserver_request)) {
    webserver_request.reply = developer_index (webserver_request);
    return;
  }
  
  if ((url == personalize_index_url ()) && browser_request_security_okay (webserver_request) && personalize_index_acl (webserver_request)) {
    webserver_request.reply = personalize_index (webserver_request);
    return;
  }
  
  if ((url == manage_users_url ()) && browser_request_security_okay (webserver_request) && manage_users_acl (webserver_request)) {
    webserver_request.reply = manage_users (webserver_request);
    return;
  }

  if ((url == manage_index_url ()) && browser_request_security_okay (webserver_request) && manage_index_acl (webserver_request)) {
    webserver_request.reply = manage_index (webserver_request);
    return;
  }
  
  if ((url == system_index_url ()) && browser_request_security_okay (webserver_request) && system_index_acl (webserver_request)) {
    webserver_request.reply = system_index (webserver_request);
    return;
  }

  if ((url == system_googletranslate_url ()) && browser_request_security_okay (webserver_request) && system_googletranslate_acl (webserver_request)) {
    webserver_request.reply = system_googletranslate (webserver_request);
    return;
  }

  if ((url == email_index_url ()) && browser_request_security_okay (webserver_request) && email_index_acl (webserver_request)) {
    webserver_request.reply = email_index (webserver_request);
    return;
  }
  
  if ((url == styles_indexm_url ()) && browser_request_security_okay (webserver_request) && styles_indexm_acl (webserver_request)) {
    webserver_request.reply = styles_indexm (webserver_request);
    return;
  }

  if ((url == styles_new_url ()) && browser_request_security_okay (webserver_request) && styles_new_acl (webserver_request)) {
    webserver_request.reply = styles_new (webserver_request);
    return;
  }

  if ((url == styles_sheetm_url ()) && browser_request_security_okay (webserver_request) && styles_sheetm_acl (webserver_request)) {
    webserver_request.reply = styles_sheetm (webserver_request);
    return;
  }
  
  if ((url == styles_view_url ()) && browser_request_security_okay (webserver_request) && styles_view_acl (webserver_request)) {
    webserver_request.reply = styles_view (webserver_request);
    return;
  }
  
  if ((url == versification_index_url ()) && browser_request_security_okay (webserver_request) && versification_index_acl (webserver_request)) {
    webserver_request.reply = versification_index (webserver_request);
    return;
  }
  
  if ((url == versification_system_url ()) && browser_request_security_okay (webserver_request) && versification_system_acl (webserver_request)) {
    webserver_request.reply = versification_system (webserver_request);
    return;
  }
  
  if ((url == collaboration_index_url ()) && browser_request_security_okay (webserver_request) && collaboration_index_acl (webserver_request)) {
    webserver_request.reply = collaboration_index (webserver_request);
    return;
  }
  
  if ((url == client_index_url ()) && browser_request_security_okay (webserver_request) && client_index_acl (webserver_request)) {
    webserver_request.reply = client_index (webserver_request);
    return;
  }
  
  if ((url == mapping_index_url ()) && browser_request_security_okay (webserver_request) && mapping_index_acl (webserver_request)) {
    webserver_request.reply = mapping_index (webserver_request);
    return;
  }
  
  if ((url == mapping_map_url ()) && browser_request_security_okay (webserver_request) && mapping_map_acl (webserver_request)) {
    webserver_request.reply = mapping_map (webserver_request);
    return;
  }
  
  if ((url == paratext_index_url ()) && browser_request_security_okay (webserver_request) && paratext_index_acl (webserver_request)) {
    webserver_request.reply = paratext_index (webserver_request);
    return;
  }
  
  // Help menu.
  if ((help_index_url (url)) && browser_request_security_okay (webserver_request) && help_index_acl (webserver_request)) {
    webserver_request.reply = help_index (webserver_request, url);
    return;
  }

  // User menu.
  if ((url == user_notifications_url ()) && browser_request_security_okay (webserver_request) && user_notifications_acl (webserver_request)) {
    webserver_request.reply = user_notifications (webserver_request);
    return;
  }
  
  if ((url == user_account_url ()) && browser_request_security_okay (webserver_request) && user_account_acl (webserver_request)) {
    webserver_request.reply = user_account (webserver_request);
    return;
  }

  // Public feedback menu.
  if ((url == public_index_url ()) && browser_request_security_okay (webserver_request) && public_index_acl (webserver_request)) {
    webserver_request.reply = public_index (webserver_request);
    return;
  }
  
  if ((url == public_login_url ()) && browser_request_security_okay (webserver_request) && public_login_acl (webserver_request)) {
    webserver_request.reply = public_login (webserver_request);
    return;
  }
  
  if ((url == public_chapter_url ()) && browser_request_security_okay (webserver_request) && public_chapter_acl (webserver_request)) {
    webserver_request.reply = public_chapter (webserver_request);
    return;
  }
  
  if ((url == public_notes_url ()) && browser_request_security_okay (webserver_request) && public_notes_acl (webserver_request)) {
    webserver_request.reply = public_notes (webserver_request);
    return;
  }
  
  if ((url == public_new_url ()) && browser_request_security_okay (webserver_request) && public_new_acl (webserver_request)) {
    webserver_request.reply = public_new (webserver_request);
    return;
  }
  
  if ((url == public_create_url ()) && browser_request_security_okay (webserver_request) && public_create_acl (webserver_request)) {
    webserver_request.reply = public_create (webserver_request);
    return;
  }
  
  if ((url == public_note_url ()) && browser_request_security_okay (webserver_request) && public_note_acl (webserver_request)) {
    webserver_request.reply = public_note (webserver_request);
    return;
  }
  
  if ((url == public_comment_url ()) && browser_request_security_okay (webserver_request) && public_comment_acl (webserver_request)) {
    webserver_request.reply = public_comment (webserver_request);
    return;
  }
  
  if ((url == jobs_index_url ()) && browser_request_security_okay (webserver_request) && jobs_index_acl (webserver_request)) {
    webserver_request.reply = jobs_index (webserver_request);
    return;
  }
  
  if ((url == search_all_url ()) && browser_request_security_okay (webserver_request) && search_all_acl (webserver_request)) {
    webserver_request.reply = search_all (webserver_request);
    return;
  }
  
  if ((url == search_replace_url ()) && browser_request_security_okay (webserver_request) && search_replace_acl (webserver_request)) {
    webserver_request.reply = search_replace (webserver_request);
    return;
  }
  
  if ((url == search_search2_url ()) && browser_request_security_okay (webserver_request) && search_search2_acl (webserver_request)) {
    webserver_request.reply = search_search2 (webserver_request);
    return;
  }
  
  if ((url == search_replace2_url ()) && browser_request_security_okay (webserver_request) && search_replace2_acl (webserver_request)) {
    webserver_request.reply = search_replace2 (webserver_request);
    return;
  }
  
  if ((url == search_similar_url ()) && browser_request_security_okay (webserver_request) && search_similar_acl (webserver_request)) {
    webserver_request.reply = search_similar (webserver_request);
    return;
  }
  
  if ((url == search_strongs_url ()) && browser_request_security_okay (webserver_request) && search_strongs_acl (webserver_request)) {
    webserver_request.reply = search_strongs (webserver_request);
    return;
  }
  
  if ((url == search_strong_url ()) && browser_request_security_okay (webserver_request) && search_strong_acl (webserver_request)) {
    webserver_request.reply = search_strong (webserver_request);
    return;
  }
  
  if ((url == search_originals_url ()) && browser_request_security_okay (webserver_request) && search_originals_acl (webserver_request)) {
    webserver_request.reply = search_originals (webserver_request);
    return;
  }
  
  if ((url == workspace_settings_url ()) && browser_request_security_okay (webserver_request) && workspace_settings_acl (webserver_request)) {
    webserver_request.reply = workspace_settings (webserver_request);
    return;
  }
  
  if ((url == collaboration_settings_url ()) && browser_request_security_okay (webserver_request) && collaboration_settings_acl (webserver_request)) {
    webserver_request.reply = collaboration_settings (webserver_request);
    return;
  }
  
  if ((url == checks_settingspatterns_url ()) && browser_request_security_okay (webserver_request) && checks_settingspatterns_acl (webserver_request)) {
    webserver_request.reply = checks_settingspatterns (webserver_request);
    return;
  }
  
  if ((url == checks_settingssentences_url ()) && browser_request_security_okay (webserver_request) && checks_settingssentences_acl (webserver_request)) {
    webserver_request.reply = checks_settingssentences (webserver_request);
    return;
  }

  if ((url == checks_settingspairs_url ()) && browser_request_security_okay (webserver_request) && checks_settingspairs_acl (webserver_request)) {
    webserver_request.reply = checks_settingspairs (webserver_request);
    return;
  }

  if ((url == checks_suppress_url ()) && browser_request_security_okay (webserver_request) && checks_suppress_acl (webserver_request)) {
    webserver_request.reply = checks_suppress (webserver_request);
    return;
  }
  
  if ((url == webbible_search_url ()) && browser_request_security_okay (webserver_request) && webbible_search_acl (webserver_request)) {
    webserver_request.reply = webbible_search (webserver_request);
    return;
  }
  
  if ((url == manage_write_url ()) && browser_request_security_okay (webserver_request) && manage_write_acl (webserver_request)) {
    webserver_request.reply = manage_write (webserver_request);
    return;
  }

  if ((url == manage_bibles_url ()) && browser_request_security_okay (webserver_request) && manage_bibles_acl (webserver_request)) {
    webserver_request.reply = manage_bibles (webserver_request);
    return;
  }

  if ((url == manage_privileges_url ()) && browser_request_security_okay (webserver_request) && manage_privileges_acl (webserver_request)) {
    webserver_request.reply = manage_privileges (webserver_request);
    return;
  }
  
  if ((url == editor_select_url ()) && browser_request_security_okay (webserver_request) && editor_select_acl (webserver_request)) {
    webserver_request.reply = editor_select (webserver_request);
    return;
  }
  
  // Downloads
  if ((url == index_listing_url (url)) && browser_request_security_okay (webserver_request) && index_listing_acl (webserver_request, url)) {
    webserver_request.reply = index_listing (webserver_request, url);
    return;
  }
  
#ifdef HAVE_CLIENT
  if (extension == "tar") {
    http_stream_file (webserver_request, false);
    return;
  }
#endif
  
  if (url == sync_setup_url ()) {
    webserver_request.reply = sync_setup (webserver_request);
    return;
  }
  if (url == sync_settings_url ()) {
    webserver_request.reply = sync_settings (webserver_request);
    return;
  }
  if (url == sync_bibles_url ()) {
    webserver_request.reply = sync_bibles (webserver_request);
    return;
  }
  if (url == sync_notes_url ()) {
    webserver_request.reply = sync_notes (webserver_request);
    return;
  }
  if (extension == "sqlite") {
    if (filter_url_dirname (url) == filter_url_temp_dir ()) {
      http_stream_file (webserver_request, false);
      return;
    }
  }
  if (url == sync_usfmresources_url ()) {
    webserver_request.reply = sync_usfmresources (webserver_request);
    return;
  }
  if (url == sync_changes_url ()) {
    webserver_request.reply = sync_changes (webserver_request);
    return;
  }
  if (url == sync_files_url ()) {
    webserver_request.reply = sync_files (webserver_request);
    return;
  }
  if (url == sync_resources_url ()) {
    webserver_request.reply = sync_resources (webserver_request);
    return;
  }
  if (url == sync_mail_url ()) {
    webserver_request.reply = sync_mail (webserver_request);
    return;
  }
  
  if ((url == navigation_update_url ()) && browser_request_security_okay (webserver_request) && navigation_update_acl (webserver_request)) {
    webserver_request.reply = navigation_update (webserver_request);
    return;
  }
  
  if ((url == navigation_poll_url ()) && browser_request_security_okay (webserver_request) && navigation_poll_acl (webserver_request)) {
    webserver_request.reply = navigation_poll (webserver_request);
    return;
  }

#ifdef HAVE_WINDOWS
  if (url == navigation_paratext_url ()) {
    webserver_request.reply = navigation_paratext (webserver_request);
    return;
  }
#endif

  if ((url == edit_preview_url ()) && browser_request_security_okay (webserver_request) && edit_preview_acl (webserver_request)) {
    webserver_request.reply = edit_preview (webserver_request);
    return;
  }
  
  if ((url == editusfm_focus_url ()) && browser_request_security_okay (webserver_request) && editusfm_focus_acl (webserver_request)) {
    webserver_request.reply = editusfm_focus (webserver_request);
    return;
  }
  
  if ((url == editusfm_load_url ()) && browser_request_security_okay (webserver_request) && editusfm_load_acl (webserver_request)) {
    webserver_request.reply = editusfm_load (webserver_request);
    return;
  }
  
  if ((url == editusfm_offset_url ()) && browser_request_security_okay (webserver_request) && editusfm_offset_acl (webserver_request)) {
    webserver_request.reply = editusfm_offset (webserver_request);
    return;
  }
  
  if ((url == editusfm_save_url ()) && browser_request_security_okay (webserver_request) && editusfm_save_acl (webserver_request)) {
    webserver_request.reply = editusfm_save (webserver_request);
    return;
  }
  
  if ((url == edit_edit_url ()) && browser_request_security_okay (webserver_request) && edit_edit_acl (webserver_request)) {
    webserver_request.reply = edit_edit (webserver_request);
    return;
  }
  
  if ((url == edit_id_url ()) && browser_request_security_okay (webserver_request) && edit_id_acl (webserver_request)) {
    webserver_request.reply = edit_id (webserver_request);
    return;
  }
  
  if ((url == edit_load_url ()) && browser_request_security_okay (webserver_request) && edit_load_acl (webserver_request)) {
    webserver_request.reply = edit_load (webserver_request);
    return;
  }
  
  if ((url == edit_save_url ()) && browser_request_security_okay (webserver_request) && edit_save_acl (webserver_request)) {
    webserver_request.reply = edit_save (webserver_request);
    return;
  }
  
  if ((url == edit_styles_url ()) && browser_request_security_okay (webserver_request) && edit_styles_acl (webserver_request)) {
    webserver_request.reply = edit_styles (webserver_request);
    return;
  }
  
  if ((url == search_getids_url ()) && browser_request_security_okay (webserver_request) && search_getids_acl (webserver_request)) {
    webserver_request.reply = search_getids (webserver_request);
    return;
  }
  
  if ((url == search_replacepre_url ()) && browser_request_security_okay (webserver_request) && search_replacepre_acl (webserver_request)) {
    webserver_request.reply = search_replacepre (webserver_request);
    return;
  }
  
  if ((url == search_replacego_url ()) && browser_request_security_okay (webserver_request) && search_replacego_acl (webserver_request)) {
    webserver_request.reply = search_replacego (webserver_request);
    return;
  }
  
  if ((url == search_replacepre2_url ()) && browser_request_security_okay (webserver_request) && search_replacepre2_acl (webserver_request)) {
    webserver_request.reply = search_replacepre2 (webserver_request);
    return;
  }
  
  if ((url == search_getids2_url ()) && browser_request_security_okay (webserver_request) && search_getids2_acl (webserver_request)) {
    webserver_request.reply = search_getids2 (webserver_request);
    return;
  }
  
  if ((url == search_replacego2_url ()) && browser_request_security_okay (webserver_request) && search_replacego2_acl (webserver_request)) {
    webserver_request.reply = search_replacego2 (webserver_request);
    return;
  }
  
  if ((url == resource_get_url ()) && browser_request_security_okay (webserver_request) && resource_get_acl (webserver_request)) {
    webserver_request.reply = resource_get (webserver_request);
    return;
  }

  if ((url == resource_unload_url ()) && browser_request_security_okay (webserver_request) && resource_unload_acl (webserver_request)) {
    webserver_request.reply = resource_unload (webserver_request);
    return;
  }

  if ((url == notes_poll_url ()) && browser_request_security_okay (webserver_request) && notes_poll_acl (webserver_request)) {
    webserver_request.reply = notes_poll (webserver_request);
    return;
  }
  
  if ((url == notes_notes_url ()) && browser_request_security_okay (webserver_request) && notes_notes_acl (webserver_request)) {
    webserver_request.reply = notes_notes (webserver_request);
    return;
  }
  
  if ((url == notes_click_url ()) && browser_request_security_okay (webserver_request) && notes_click_acl (webserver_request)) {
    webserver_request.reply = notes_click (webserver_request);
    return;
  }
  
  if ((url == consistency_poll_url ()) && browser_request_security_okay (webserver_request) && consistency_poll_acl (webserver_request)) {
    webserver_request.reply = consistency_poll (webserver_request);
    return;
  }
  
  if ((url == consistency_input_url ()) && browser_request_security_okay (webserver_request) && consistency_input_acl (webserver_request)) {
    webserver_request.reply = consistency_input (webserver_request);
    return;
  }
  
  if ((url == lexicon_definition_url ()) && browser_request_security_okay (webserver_request) && lexicon_definition_acl (webserver_request)) {
    webserver_request.reply = lexicon_definition (webserver_request);
    return;
  }

#ifdef HAVE_CLIENT
  // For security reasons, this is only available in a client configuration.
  if (url == assets_external_url ()) {
    webserver_request.reply = assets_external (webserver_request);
    return;
  }
#endif
  
  if ((url == rss_feed_url ()) && browser_request_security_okay (webserver_request) && rss_feed_acl (webserver_request)) {
    webserver_request.reply = rss_feed (webserver_request);
    return;
  }

  if ((url == edit_update_url ()) && browser_request_security_okay (webserver_request) && edit_update_acl (webserver_request)) {
    webserver_request.reply = edit_update (webserver_request);
    return;
  }

  if ((url == editor_id_url ()) && browser_request_security_okay (webserver_request) && editor_id_acl (webserver_request)) {
    webserver_request.reply = editor_id (webserver_request);
    return;
  }
  
  if ((url == editor_style_url ()) && browser_request_security_okay (webserver_request) && editor_style_acl (webserver_request)) {
    webserver_request.reply = editor_style (webserver_request);
    return;
  }

  if ((url == editone_index_url ()) && browser_request_security_okay (webserver_request) && editone_index_acl (webserver_request)) {
    webserver_request.reply = editone_index (webserver_request);
    return;
  }
  
  if ((url == editone_load_url ()) && browser_request_security_okay (webserver_request) && editone_load_acl (webserver_request)) {
    webserver_request.reply = editone_load (webserver_request);
    return;
  }
  
  if ((url == editone_save_url ()) && browser_request_security_okay (webserver_request) && editone_save_acl (webserver_request)) {
    webserver_request.reply = editone_save (webserver_request);
    return;
  }
  
  if ((url == editone_verse_url ()) && browser_request_security_okay (webserver_request) && editone_verse_acl (webserver_request)) {
    webserver_request.reply = editone_verse (webserver_request);
    return;
  }
  
  if ((url == editone_update_url ()) && browser_request_security_okay (webserver_request) && editone_update_acl (webserver_request)) {
    webserver_request.reply = editone_update (webserver_request);
    return;
  }

  if ((url == read_index_url ()) && browser_request_security_okay (webserver_request) && read_index_acl (webserver_request)) {
    webserver_request.reply = read_index (webserver_request);
    return;
  }

  if ((url == read_load_url ()) && browser_request_security_okay (webserver_request) && read_load_acl (webserver_request)) {
    webserver_request.reply = read_load (webserver_request);
    return;
  }

  if ((url == read_verse_url ()) && browser_request_security_okay (webserver_request) && read_verse_acl (webserver_request)) {
    webserver_request.reply = read_verse (webserver_request);
    return;
  }

  if ((url == resource_divider_url ()) && browser_request_security_okay (webserver_request) && resource_divider_acl (webserver_request)) {
    webserver_request.reply = resource_divider (webserver_request);
    return;
  }

  if ((url == session_confirm_url ()) && browser_request_security_okay (webserver_request) && session_confirm_acl (webserver_request)) {
    webserver_request.reply = session_confirm (webserver_request);
    return;
  }
  
  if ((url == resource_comparative9edit_url ()) && browser_request_security_okay (webserver_request) && resource_comparative9edit_acl (webserver_request)) {
    webserver_request.reply = resource_comparative9edit (webserver_request);
    return;
  }

  if ((url == resource_comparative1edit_url ()) && browser_request_security_okay (webserver_request) && resource_comparative1edit_acl (webserver_request)) {
    webserver_request.reply = resource_comparative1edit (webserver_request);
    return;
  }

  if ((url == resource_translated9edit_url ()) && browser_request_security_okay (webserver_request) && resource_translated9edit_acl (webserver_request)) {
    webserver_request.reply = resource_translated9edit (webserver_request);
    return;
  }
  
  if ((url == resource_translated1edit_url ()) && browser_request_security_okay (webserver_request) && resource_translated1edit_acl (webserver_request)) {
    webserver_request.reply = resource_translated1edit (webserver_request);
    return;
  }

  if ((url == developer_delay_url ()) && developer_delay_acl (webserver_request)) {
    webserver_request.reply = developer_delay (webserver_request);
    return;
  }

  if ((url == images_index_url ()) && browser_request_security_okay (webserver_request) && images_index_acl (webserver_request)) {
    webserver_request.reply = images_index (webserver_request);
    return;
  }

  if ((url == images_view_url ()) && browser_request_security_okay (webserver_request) && images_view_acl (webserver_request)) {
    webserver_request.reply = images_view (webserver_request);
    return;
  }

  if ((url == images_fetch_url ()) && browser_request_security_okay (webserver_request) && images_fetch_acl (webserver_request)) {
    webserver_request.reply = images_fetch (webserver_request);
    return;
  }

  // Forward the browser to the default home page.
  redirect_browser (webserver_request, index_index_url ());
}

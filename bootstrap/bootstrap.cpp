/*
Copyright (©) 2003-2021 Teus Benschop.

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
#include <manage/accounts.h>
#include <manage/exports.h>
#include <manage/hyphenation.h>
#include <manage/write.h>
#include <manage/privileges.h>
#include <system/index.h>
#include <system/indonesianfree.h>
#include <collaboration/index.h>
#include <collaboration/settings.h>
#include <styles/indexm.h>
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
#include <redirect/index.h>
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
#include <resource/print.h>
#include <resource/download.h>
#include <resource/select.h>
#include <resource/images.h>
#include <resource/image.h>
#include <resource/img.h>
#include <resource/imagefetch.h>
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
#include <nmt/index.h>
#include <editor/id.h>
#include <editor/style.h>
#include <edit/update.h>
#include <editone2/index.h>
#include <editone2/load.h>
#include <editone2/save.h>
#include <editone2/verse.h>
#include <editone2/update.h>
#include <read/index.h>
#include <read/load.h>
#include <read/verse.h>
#include <resource/divider.h>
#include <session/confirm.h>
#include <resource/comparative9edit.h>
#include <resource/comparative1edit.h>
#include <developer/logic.h>
#include <developer/delay.h>


// Internal function to check whether a request coming from the browser is considered secure enough.
// It returns true if the security is okay.
bool browser_request_security_okay (Webserver_Request * request)
{
  // If the request is made via https, the security is OK.
  if (request->secure) {
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
void bootstrap_index (void * webserver_request)
{
  shared_ptr<Developer_Logic_Tracer> developer_logic_tracer = nullptr;
  if (config_globals_log_network) {
    developer_logic_tracer = make_shared<Developer_Logic_Tracer>(webserver_request);
  }

  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Record the POST request made to the web server.
  // This can be used for debugging.
  /*
  if (!request->post.empty () && config_logic_demo_enabled ()) {
    string contents;
    int seconds = filter_date_seconds_since_epoch ();
    string rfc822time = filter_date_rfc822 (seconds);
    contents.append (rfc822time + "\n");
    contents.append (request->get + "\n");
    string query;
    for (auto element : request->query) {
      query.append (element.first + "=" + element.second + " and ");
    }
    string post;
    for (auto element : request->post) {
      post.append (element.first + "=" + element.second + " and ");
    }
    contents.append ("query: " + query + "\n");
    contents.append ("post: " + post + "\n");
    string filename;
    filename = filter_url_create_root_path (filter_url_temp_dir (), "http-post-trace.txt");
    filter_url_file_put_contents_append (filename, contents);
  }
  */
  
  string extension = filter_url_get_extension (request->get);
  string url = request->get.substr (1);
  
  // Serve graphics, stylesheets, JavaScript, fonts, with direct streaming for low memory usage.
  if (   (extension == "ico")
      || (extension == "png")
      || (extension == "gif")
      || (extension == "jpg")
      || (extension == "css")
      || (extension == "js")
      || (Fonts_Logic::isFont (extension))
      || (extension == "sh")
      || (extension == "svg")
      || (extension == "map")
      ) {
    http_stream_file (request, true);
    return;
  }

  // check_user_localization_preference (request);

  if ((url == resource_imagefetch_url ()) && resource_imagefetch_acl (request)) {
    request->reply = resource_imagefetch (request);
    return;
  }

  // Serve resource downloads.
  if ((extension == "sqlite") && (request->get.find (Database_Cache::fragment ()) != string::npos)) {
    http_stream_file (request, false);
    return;
  }
  
  // Serve initialization notice.
  if (config_logic_version () != Database_Config_General::getInstalledDatabaseVersion ()) {
    request->reply = setup_initialization_notice ();
    return;
  }
  
  // Force setup.
  if (config_logic_version () != Database_Config_General::getInstalledInterfaceVersion ()) {
    request->reply = setup_index (request);
    return;
  }

  // Home page and menu.
  if ((url == index_index_url ()) && browser_request_security_okay (request) && index_index_acl (request)) {
    request->reply = index_index (request);
    return;
  }
  
  if ((url == menu_index_url ()) && browser_request_security_okay (request) && menu_index_acl (request)) {
    request->reply = menu_index (request);
    return;
  }
  
  // Login and logout.
  if ((url == session_login_url ()) && browser_request_security_okay (request) && session_login_acl (request)) {
    request->reply = session_login (request);
    return;
  }
  
  if ((url == session_logout_url ()) && browser_request_security_okay (request) && session_logout_acl (request)) {
    request->reply = session_logout (request);
    return;
  }
  
  if ((url == session_password_url ()) && browser_request_security_okay (request) && session_password_acl (request)) {
    request->reply = session_password (request);
    return;
  }
  
  if ((url == session_signup_url ()) && browser_request_security_okay (request) && session_signup_acl (request)) {
    request->reply = session_signup (request);
    return;
  }
  
  if ((url == session_switch_url ()) && browser_request_security_okay (request) && session_switch_acl (request)) {
    request->reply = session_switch (request);
    return;
  }
  
  // Bible menu.
  if ((url == bible_manage_url ()) && browser_request_security_okay (request) && bible_manage_acl (request)) {
    request->reply = bible_manage (request);
    return;
  }
  
  if ((url == bible_settings_url ()) && browser_request_security_okay (request) && bible_settings_acl (request)) {
    request->reply = bible_settings (request);
    return;
  }
  
  if ((url == bible_book_url ()) && browser_request_security_okay (request) && bible_book_acl (request)) {
    request->reply = bible_book (request);
    return;
  }
  
  if ((url == bible_chapter_url ()) && browser_request_security_okay (request) && bible_chapter_acl (request)) {
    request->reply = bible_chapter (request);
    return;
  }
  
  if ((url == bible_import_url ()) && browser_request_security_okay (request) && bible_import_acl (request)) {
    request->reply = bible_import (request);
    return;
  }
  
  if ((url == compare_index_url ()) && browser_request_security_okay (request) && compare_index_acl (request)) {
    request->reply = compare_index (request);
    return;
  }
  
  if ((url == bible_order_url ()) && browser_request_security_okay (request) && bible_order_acl (request)) {
    request->reply = bible_order (request);
    return;
  }
  
  if ((url == bible_css_url ()) && browser_request_security_okay (request) && bible_css_acl (request)) {
    request->reply = bible_css (request);
    return;
  }
  
  if ((url == editone_index_url ()) && browser_request_security_okay (request) && editone_index_acl ()) {
    request->reply = editone_index (request);
    return;
  }
  
  if ((url == editusfm_index_url ()) && browser_request_security_okay (request) && editusfm_index_acl (request)) {
    request->reply = editusfm_index (request);
    return;
  }
  
  if ((url == edit_index_url ()) && browser_request_security_okay (request) && edit_index_acl (request)) {
    request->reply = edit_index (request);
    return;
  }
  
  if ((url == edit_position_url ()) && browser_request_security_okay (request) && edit_position_acl (request)) {
    request->reply = edit_position (request);
    return;
  }

  if ((url == edit_navigate_url ()) && browser_request_security_okay (request) && edit_navigate_acl (request)) {
    request->reply = edit_navigate (request);
    return;
  }
  
  if ((url == search_index_url ()) && browser_request_security_okay (request) && search_index_acl (request)) {
    request->reply = search_index (request);
    return;
  }
  
  if ((url == workspace_index_url ()) && browser_request_security_okay (request) && workspace_index_acl (request)) {
    request->reply = workspace_index (request);
    return;
  }
  
  if ((url == workspace_organize_url ()) && browser_request_security_okay (request) && workspace_organize_acl (request)) {
    request->reply = workspace_organize (request);
    return;
  }
  
  if ((url == resource_bible2resource_url ()) && browser_request_security_okay (request) && resource_bible2resource_acl (request)) {
    request->reply = resource_bible2resource (request);
    return;
  }
  
  if ((url == checks_index_url ()) && browser_request_security_okay (request) && checks_index_acl (request)) {
    request->reply = checks_index (request);
    return;
  }
  
  if ((url == checks_settings_url ()) && browser_request_security_okay (request) && checks_settings_acl (request)) {
    request->reply = checks_settings (request);
    return;
  }
  
  if ((url == consistency_index_url ()) && browser_request_security_okay (request) && consistency_index_acl (request)) {
    request->reply = consistency_index (request);
    return;
  }
  
  // Notes menu.
  if ((url == notes_index_url ()) && browser_request_security_okay (request) && notes_index_acl (request)) {
    request->reply = notes_index (request);
    return;
  }
  
  if ((url == notes_create_url ()) && browser_request_security_okay (request) && notes_create_acl (request)) {
    request->reply = notes_create (request);
    return;
  }
  
  if ((url == notes_select_url ()) && browser_request_security_okay (request) && notes_select_acl (request)) {
    request->reply = notes_select (request);
    return;
  }
  
  if ((url == notes_note_url ()) && browser_request_security_okay (request) && notes_note_acl (request)) {
    request->reply = notes_note (request);
    return;
  }
  
  if ((url == notes_comment_url ()) && browser_request_security_okay (request) && notes_comment_acl (request)) {
    request->reply = notes_comment (request);
    return;
  }
  
  if ((url == notes_actions_url ()) && browser_request_security_okay (request) && notes_actions_acl (request)) {
    request->reply = notes_actions (request);
    return;
  }
  
  if ((url == notes_assign_1_url ()) && browser_request_security_okay (request) && notes_assign_1_acl (request)) {
    request->reply = notes_assign_1 (request);
    return;
  }
  
  if ((url == notes_assign_n_url ()) && browser_request_security_okay (request) && notes_assign_n_acl (request)) {
    request->reply = notes_assign_n (request);
    return;
  }
  
  if ((url == notes_unassign_n_url ()) && browser_request_security_okay (request) && notes_unassign_n_acl (request)) {
    request->reply = notes_unassign_n (request);
    return;
  }
  
  if ((url == notes_status_1_url ()) && browser_request_security_okay (request) && notes_status_1_acl (request)) {
    request->reply = notes_status_1 (request);
    return;
  }
  
  if ((url == notes_status_n_url ()) && browser_request_security_okay (request) && notes_status_n_acl (request)) {
    request->reply = notes_status_n (request);
    return;
  }
  
  if ((url == notes_verses_url ()) && browser_request_security_okay (request) && notes_verses_acl (request)) {
    request->reply = notes_verses (request);
    return;
  }
  
  if ((url == notes_severity_1_url ()) && browser_request_security_okay (request) && notes_severity_1_acl (request)) {
    request->reply = notes_severity_1 (request);
    return;
  }
  
  if ((url == notes_severity_n_url ()) && browser_request_security_okay (request) && notes_severity_n_acl (request)) {
    request->reply = notes_severity_n (request);
    return;
  }
  
  if ((url == notes_bible_1_url ()) && browser_request_security_okay (request) && notes_bible_1_acl (request)) {
    request->reply = notes_bible_1 (request);
    return;
  }
  
  if ((url == notes_bible_n_url ()) && browser_request_security_okay (request) && notes_bible_n_acl (request)) {
    request->reply = notes_bible_n (request);
    return;
  }
  
  if ((url == notes_bulk_url ()) && browser_request_security_okay (request) && notes_bulk_acl (request)) {
    request->reply = notes_bulk (request);
    return;
  }
  
  if ((url == notes_edit_url ()) && browser_request_security_okay (request) && notes_edit_acl (request)) {
    request->reply = notes_edit (request);
    return;
  }
  
  if ((url == notes_summary_url ()) && browser_request_security_okay (request) && notes_summary_acl (request)) {
    request->reply = notes_summary (request);
    return;
  }

  // Resources menu.
  if ((url == resource_index_url ()) && browser_request_security_okay (request) && resource_index_acl (request)) {
    request->reply = resource_index (request);
    return;
  }
  
  if ((url == resource_organize_url ()) && browser_request_security_okay (request) && resource_organize_acl (request)) {
    request->reply = resource_organize (request);
    return;
  }
  
  if ((url == resource_print_url ()) && browser_request_security_okay (request) && resource_print_acl (request)) {
    request->reply = resource_print (request);
    return;
  }
  
  if ((url == resource_manage_url ()) && browser_request_security_okay (request) && resource_manage_acl (request)) {
    request->reply = resource_manage (request);
    return;
  }
  
  if ((url == resource_download_url ()) && browser_request_security_okay (request) && resource_download_acl (request)) {
    request->reply = resource_download (request);
    return;
  }
  
  if ((url == resource_images_url ()) && browser_request_security_okay (request) && resource_images_acl (request)) {
    request->reply = resource_images (request);
    return;
  }
  
  if ((url == resource_sword_url ()) && browser_request_security_okay (request) && resource_sword_acl (request)) {
    request->reply = resource_sword (request);
    return;
  }
  
  if ((url == resource_select_url ()) && browser_request_security_okay (request) && resource_select_acl (request)) {
    request->reply = resource_select (request);
    return;
  }
  
  if ((url == resource_cache_url ()) && browser_request_security_okay (request) && resource_cache_acl (request)) {
    request->reply = resource_cache (request);
    return;
  }

  if ((url == resource_user9edit_url ()) && browser_request_security_okay (request) && resource_user9edit_acl (request)) {
    request->reply = resource_user9edit (request);
    return;
  }

  if ((url == resource_user1edit_url ()) && browser_request_security_okay (request) && resource_user1edit_acl (request)) {
    request->reply = resource_user1edit (request);
    return;
  }

  if ((url == resource_user9view_url ()) && browser_request_security_okay (request) && resource_user9view_acl (request)) {
    request->reply = resource_user9view (request);
    return;
  }
  
  if ((url == resource_user1view_url ()) && browser_request_security_okay (request) && resource_user1view_acl (request)) {
    request->reply = resource_user1view (request);
    return;
  }

  if ((url == resource_biblegateway_url ()) && browser_request_security_okay (request) && resource_biblegateway_acl (request)) {
    request->reply = resource_biblegateway (request);
    return;
  }

  if ((url == resource_studylight_url ()) && browser_request_security_okay (request) && resource_studylight_acl (request)) {
    request->reply = resource_studylight (request);
    return;
  }
  
  // Changes menu.
  if ((url == journal_index_url ()) && browser_request_security_okay (request) && journal_index_acl (request)) {
    request->reply = journal_index (request);
    return;
  }
  
  if ((url == changes_changes_url ()) && browser_request_security_okay (request) && changes_changes_acl (request)) {
    request->reply = changes_changes (request);
    return;
  }
  
  if ((url == changes_change_url ()) && browser_request_security_okay (request) && changes_change_acl (request)) {
    request->reply = changes_change (request);
    return;
  }
  
  if ((url == changes_manage_url ()) && browser_request_security_okay (request) && changes_manage_acl (request)) {
    request->reply = changes_manage (request);
    return;
  }
  
  if ((url == changes_statistics_url ()) && browser_request_security_okay (request) && changes_statistics_acl (request)) {
    request->reply = changes_statistics (request);
    return;
  }

  // Planning menu.
  if ((url == sprint_index_url ()) && browser_request_security_okay (request) && sprint_index_acl (request)) {
    request->reply = sprint_index (request);
    return;
  }
  
  // Tools menu.
  if ((url == sendreceive_index_url ()) && browser_request_security_okay (request) && sendreceive_index_acl (request)) {
    request->reply = sendreceive_index (request);
    return;
  }
  
  if ((url == manage_exports_url ()) && browser_request_security_okay (request) && manage_exports_acl (request)) {
    request->reply = manage_exports (request);
    return;
  }
  
  if ((url == manage_hyphenation_url ()) && browser_request_security_okay (request) && manage_hyphenation_acl (request)) {
    request->reply = manage_hyphenation (request);
    return;
  }
  
  if ((url == developer_index_url ()) && browser_request_security_okay (request) && developer_index_acl (request)) {
    request->reply = developer_index (request);
    return;
  }
  
  // Settings menu.
  if ((url == personalize_index_url ()) && browser_request_security_okay (request) && personalize_index_acl (request)) {
    request->reply = personalize_index (request);
    return;
  }
  
  if ((url == manage_users_url ()) && browser_request_security_okay (request) && manage_users_acl (request)) {
    request->reply = manage_users (request);
    return;
  }

  if ((url == manage_accounts_url ()) && browser_request_security_okay (request) && manage_accounts_acl (request)) {
    request->reply = manage_accounts (request);
    return;
  }

  if ((url == manage_index_url ()) && browser_request_security_okay (request) && manage_index_acl (request)) {
    request->reply = manage_index (request);
    return;
  }
  
  if ((url == system_index_url ()) && browser_request_security_okay (request) && system_index_acl (request)) {
    request->reply = system_index (request);
    return;
  }

  if ((url == system_indonesianfree_url ()) && browser_request_security_okay (request) && system_indonesianfree_acl (request)) {
    request->reply = system_indonesianfree (request);
    return;
  }

  if ((url == email_index_url ()) && browser_request_security_okay (request) && email_index_acl (request)) {
    request->reply = email_index (request);
    return;
  }
  
  if ((url == styles_indexm_url ()) && browser_request_security_okay (request) && styles_indexm_acl (request)) {
    request->reply = styles_indexm (request);
    return;
  }
  
  if ((url == styles_sheetm_url ()) && browser_request_security_okay (request) && styles_sheetm_acl (request)) {
    request->reply = styles_sheetm (request);
    return;
  }
  
  if ((url == styles_view_url ()) && browser_request_security_okay (request) && styles_view_acl (request)) {
    request->reply = styles_view (request);
    return;
  }
  
  if ((url == versification_index_url ()) && browser_request_security_okay (request) && versification_index_acl (request)) {
    request->reply = versification_index (request);
    return;
  }
  
  if ((url == versification_system_url ()) && browser_request_security_okay (request) && versification_system_acl (request)) {
    request->reply = versification_system (request);
    return;
  }
  
  if ((url == collaboration_index_url ()) && browser_request_security_okay (request) && collaboration_index_acl (request)) {
    request->reply = collaboration_index (request);
    return;
  }
  
  if ((url == client_index_url ()) && browser_request_security_okay (request) && client_index_acl (request)) {
    request->reply = client_index (request);
    return;
  }
  
  if ((url == mapping_index_url ()) && browser_request_security_okay (request) && mapping_index_acl (request)) {
    request->reply = mapping_index (request);
    return;
  }
  
  if ((url == mapping_map_url ()) && browser_request_security_okay (request) && mapping_map_acl (request)) {
    request->reply = mapping_map (request);
    return;
  }
  
  if ((url == paratext_index_url ()) && browser_request_security_okay (request) && paratext_index_acl (request)) {
    request->reply = paratext_index (request);
    return;
  }
  
  // Help menu.
  if ((help_index_url (url)) && browser_request_security_okay (request) && help_index_acl (request)) {
    request->reply = help_index (request, url);
    return;
  }

  // User menu.
  if ((url == user_notifications_url ()) && browser_request_security_okay (request) && user_notifications_acl (request)) {
    request->reply = user_notifications (request);
    return;
  }
  
  if ((url == user_account_url ()) && browser_request_security_okay (request) && user_account_acl (request)) {
    request->reply = user_account (request);
    return;
  }

  // Public feedback menu.
  if ((url == public_index_url ()) && browser_request_security_okay (request) && public_index_acl (request)) {
    request->reply = public_index (request);
    return;
  }
  
  if ((url == public_login_url ()) && browser_request_security_okay (request) && public_login_acl (request)) {
    request->reply = public_login (request);
    return;
  }
  
  if ((url == public_chapter_url ()) && browser_request_security_okay (request) && public_chapter_acl (request)) {
    request->reply = public_chapter (request);
    return;
  }
  
  if ((url == public_notes_url ()) && browser_request_security_okay (request) && public_notes_acl (request)) {
    request->reply = public_notes (request);
    return;
  }
  
  if ((url == public_new_url ()) && browser_request_security_okay (request) && public_new_acl (request)) {
    request->reply = public_new (request);
    return;
  }
  
  if ((url == public_create_url ()) && browser_request_security_okay (request) && public_create_acl (request)) {
    request->reply = public_create (request);
    return;
  }
  
  if ((url == public_note_url ()) && browser_request_security_okay (request) && public_note_acl (request)) {
    request->reply = public_note (request);
    return;
  }
  
  if ((url == public_comment_url ()) && browser_request_security_okay (request) && public_comment_acl (request)) {
    request->reply = public_comment (request);
    return;
  }
  
  // Pages not in any menu.
  if ((url == jobs_index_url ()) && browser_request_security_okay (request) && jobs_index_acl (request)) {
    request->reply = jobs_index (request);
    return;
  }
  
  if ((url == search_all_url ()) && browser_request_security_okay (request) && search_all_acl (request)) {
    request->reply = search_all (request);
    return;
  }
  
  if ((url == search_replace_url ()) && browser_request_security_okay (request) && search_replace_acl (request)) {
    request->reply = search_replace (request);
    return;
  }
  
  if ((url == search_search2_url ()) && browser_request_security_okay (request) && search_search2_acl (request)) {
    request->reply = search_search2 (request);
    return;
  }
  
  if ((url == search_replace2_url ()) && browser_request_security_okay (request) && search_replace2_acl (request)) {
    request->reply = search_replace2 (request);
    return;
  }
  
  if ((url == search_similar_url ()) && browser_request_security_okay (request) && search_similar_acl (request)) {
    request->reply = search_similar (request);
    return;
  }
  
  if ((url == search_strongs_url ()) && browser_request_security_okay (request) && search_strongs_acl (request)) {
    request->reply = search_strongs (request);
    return;
  }
  
  if ((url == search_strong_url ()) && browser_request_security_okay (request) && search_strong_acl (request)) {
    request->reply = search_strong (request);
    return;
  }
  
  if ((url == search_originals_url ()) && browser_request_security_okay (request) && search_originals_acl (request)) {
    request->reply = search_originals (request);
    return;
  }
  
  if ((url == workspace_settings_url ()) && browser_request_security_okay (request) && workspace_settings_acl (request)) {
    request->reply = workspace_settings (request);
    return;
  }
  
  if ((url == collaboration_settings_url ()) && browser_request_security_okay (request) && collaboration_settings_acl (request)) {
    request->reply = collaboration_settings (request);
    return;
  }
  
  if ((url == checks_settingspatterns_url ()) && browser_request_security_okay (request) && checks_settingspatterns_acl (request)) {
    request->reply = checks_settingspatterns (request);
    return;
  }
  
  if ((url == checks_settingssentences_url ()) && browser_request_security_okay (request) && checks_settingssentences_acl (request)) {
    request->reply = checks_settingssentences (request);
    return;
  }

  if ((url == checks_settingspairs_url ()) && browser_request_security_okay (request) && checks_settingspairs_acl (request)) {
    request->reply = checks_settingspairs (request);
    return;
  }

  if ((url == checks_suppress_url ()) && browser_request_security_okay (request) && checks_suppress_acl (request)) {
    request->reply = checks_suppress (request);
    return;
  }
  
  if ((url == webbible_search_url ()) && browser_request_security_okay (request) && webbible_search_acl (request)) {
    request->reply = webbible_search (request);
    return;
  }
  
  if ((url == manage_write_url ()) && browser_request_security_okay (request) && manage_write_acl (request)) {
    request->reply = manage_write (request);
    return;
  }
  
  if ((url == manage_privileges_url ()) && browser_request_security_okay (request) && manage_privileges_acl (request)) {
    request->reply = manage_privileges (request);
    return;
  }
  
  if ((url == resource_image_url ()) && browser_request_security_okay (request) && resource_image_acl (request)) {
    request->reply = resource_image (request);
    return;
  }
  
  if ((url == resource_img_url ()) && browser_request_security_okay (request) && resource_img_acl (request)) {
    request->reply = resource_img (request);
    return;
  }
  
  if ((url == editor_select_url ()) && browser_request_security_okay (request) && editor_select_acl (request)) {
    request->reply = editor_select (request);
    return;
  }
  
  // Downloads
  if ((url == index_listing_url (url)) && browser_request_security_okay (request) && index_listing_acl (request, url)) {
    request->reply = index_listing (request, url);
    return;
  }
  
#ifdef HAVE_CLIENT
  if (extension == "tar") {
    http_stream_file (request, false);
    return;
  }
#endif
  
  // Client calls.
  if (url == sync_setup_url ()) {
    request->reply = sync_setup (request);
    return;
  }
  if (url == sync_settings_url ()) {
    request->reply = sync_settings (request);
    return;
  }
  if (url == sync_bibles_url ()) {
    request->reply = sync_bibles (request);
    return;
  }
  if (url == sync_notes_url ()) {
    request->reply = sync_notes (request);
    return;
  }
  if (extension == "sqlite") {
    if (filter_url_dirname (url) == filter_url_temp_dir ()) {
      http_stream_file (request, false);
      return;
    }
  }
  if (url == sync_usfmresources_url ()) {
    request->reply = sync_usfmresources (request);
    return;
  }
  if (url == sync_changes_url ()) {
    request->reply = sync_changes (request);
    return;
  }
  if (url == sync_files_url ()) {
    request->reply = sync_files (request);
    return;
  }
  if (url == sync_resources_url ()) {
    request->reply = sync_resources (request);
    return;
  }
  if (url == sync_mail_url ()) {
    request->reply = sync_mail (request);
    return;
  }
  
  // AJAX calls.
  if ((url == navigation_update_url ()) && browser_request_security_okay (request) && navigation_update_acl (request)) {
    request->reply = navigation_update (request);
    return;
  }
  
  if ((url == navigation_poll_url ()) && browser_request_security_okay (request) && navigation_poll_acl (request)) {
    request->reply = navigation_poll (request);
    return;
  }

#ifdef HAVE_WINDOWS
  if (url == navigation_paratext_url ()) {
    request->reply = navigation_paratext (request);
    return;
  }
#endif

  if ((url == edit_preview_url ()) && browser_request_security_okay (request) && edit_preview_acl (request)) {
    request->reply = edit_preview (request);
    return;
  }
  
  if ((url == editusfm_focus_url ()) && browser_request_security_okay (request) && editusfm_focus_acl (request)) {
    request->reply = editusfm_focus (request);
    return;
  }
  
  if ((url == editusfm_load_url ()) && browser_request_security_okay (request) && editusfm_load_acl (request)) {
    request->reply = editusfm_load (request);
    return;
  }
  
  if ((url == editusfm_offset_url ()) && browser_request_security_okay (request) && editusfm_offset_acl (request)) {
    request->reply = editusfm_offset (request);
    return;
  }
  
  if ((url == editusfm_save_url ()) && browser_request_security_okay (request) && editusfm_save_acl (request)) {
    request->reply = editusfm_save (request);
    return;
  }
  
  if ((url == edit_edit_url ()) && browser_request_security_okay (request) && edit_edit_acl (request)) {
    request->reply = edit_edit (request);
    return;
  }
  
  if ((url == edit_id_url ()) && browser_request_security_okay (request) && edit_id_acl (request)) {
    request->reply = edit_id (request);
    return;
  }
  
  if ((url == edit_load_url ()) && browser_request_security_okay (request) && edit_load_acl (request)) {
    request->reply = edit_load (request);
    return;
  }
  
  if ((url == edit_save_url ()) && browser_request_security_okay (request) && edit_save_acl (request)) {
    request->reply = edit_save (request);
    return;
  }
  
  if ((url == edit_styles_url ()) && browser_request_security_okay (request) && edit_styles_acl (request)) {
    request->reply = edit_styles (request);
    return;
  }
  
  if ((url == search_getids_url ()) && browser_request_security_okay (request) && search_getids_acl (request)) {
    request->reply = search_getids (request);
    return;
  }
  
  if ((url == search_replacepre_url ()) && browser_request_security_okay (request) && search_replacepre_acl (request)) {
    request->reply = search_replacepre (request);
    return;
  }
  
  if ((url == search_replacego_url ()) && browser_request_security_okay (request) && search_replacego_acl (request)) {
    request->reply = search_replacego (request);
    return;
  }
  
  if ((url == search_replacepre2_url ()) && browser_request_security_okay (request) && search_replacepre2_acl (request)) {
    request->reply = search_replacepre2 (request);
    return;
  }
  
  if ((url == search_getids2_url ()) && browser_request_security_okay (request) && search_getids2_acl (request)) {
    request->reply = search_getids2 (request);
    return;
  }
  
  if ((url == search_replacego2_url ()) && browser_request_security_okay (request) && search_replacego2_acl (request)) {
    request->reply = search_replacego2 (request);
    return;
  }
  
  if ((url == resource_get_url ()) && browser_request_security_okay (request) && resource_get_acl (request)) {
    request->reply = resource_get (request);
    return;
  }

  if ((url == resource_unload_url ()) && browser_request_security_okay (request) && resource_unload_acl (request)) {
    request->reply = resource_unload (request);
    return;
  }

  if ((url == notes_poll_url ()) && browser_request_security_okay (request) && notes_poll_acl (request)) {
    request->reply = notes_poll (request);
    return;
  }
  
  if ((url == notes_notes_url ()) && browser_request_security_okay (request) && notes_notes_acl (request)) {
    request->reply = notes_notes (request);
    return;
  }
  
  if ((url == notes_click_url ()) && browser_request_security_okay (request) && notes_click_acl (request)) {
    request->reply = notes_click (request);
    return;
  }
  
  if ((url == consistency_poll_url ()) && browser_request_security_okay (request) && consistency_poll_acl (request)) {
    request->reply = consistency_poll (request);
    return;
  }
  
  if ((url == consistency_input_url ()) && browser_request_security_okay (request) && consistency_input_acl (request)) {
    request->reply = consistency_input (request);
    return;
  }
  
  if ((url == lexicon_definition_url ()) && browser_request_security_okay (request) && lexicon_definition_acl (request)) {
    request->reply = lexicon_definition (request);
    return;
  }

#ifdef HAVE_CLIENT
  // For security reasons, this is only available in a client configuration.
  if (url == assets_external_url ()) {
    request->reply = assets_external (request);
    return;
  }
#endif
  
  // RSS feed.
  if ((url == rss_feed_url ()) && browser_request_security_okay (request) && rss_feed_acl (request)) {
    request->reply = rss_feed (request);
    return;
  }

  if ((url == nmt_index_url ()) && browser_request_security_okay (request) && nmt_index_acl (request)) {
    request->reply = nmt_index (request);
    return;
  }

  if ((url == edit_index_url ()) && browser_request_security_okay (request) && edit_index_acl (request)) {
    request->reply = edit_index (request);
    return;
  }
  
  if ((url == edit_position_url ()) && browser_request_security_okay (request) && edit_position_acl (request)) {
    request->reply = edit_position (request);
    return;
  }

  if ((url == edit_navigate_url ()) && browser_request_security_okay (request) && edit_navigate_acl (request)) {
    request->reply = edit_navigate (request);
    return;
  }
  
  if ((url == edit_preview_url ()) && browser_request_security_okay (request) && edit_preview_acl (request)) {
    request->reply = edit_preview (request);
    return;
  }
  
  if ((url == edit_edit_url ()) && browser_request_security_okay (request) && edit_edit_acl (request)) {
    request->reply = edit_edit (request);
    return;
  }

  if ((url == edit_update_url ()) && browser_request_security_okay (request) && edit_update_acl (request)) {
    request->reply = edit_update (request);
    return;
  }

  if ((url == editor_id_url ()) && browser_request_security_okay (request) && editor_id_acl (request)) {
    request->reply = editor_id (request);
    return;
  }
  
  if ((url == edit_load_url ()) && browser_request_security_okay (request) && edit_load_acl (request)) {
    request->reply = edit_load (request);
    return;
  }
  
  if ((url == edit_save_url ()) && browser_request_security_okay (request) && edit_save_acl (request)) {
    request->reply = edit_save (request);
    return;
  }
  
  if ((url == editor_style_url ()) && browser_request_security_okay (request) && editor_style_acl (request)) {
    request->reply = editor_style (request);
    return;
  }

  if ((url == editone2_index_url ()) && browser_request_security_okay (request) && editone2_index_acl (request)) {
    request->reply = editone2_index (request);
    return;
  }
  
  if ((url == editone2_load_url ()) && browser_request_security_okay (request) && editone2_load_acl (request)) {
    request->reply = editone2_load (request);
    return;
  }
  
  if ((url == editone2_save_url ()) && browser_request_security_okay (request) && editone2_save_acl (request)) {
    request->reply = editone2_save (request);
    return;
  }
  
  if ((url == editone2_verse_url ()) && browser_request_security_okay (request) && editone2_verse_acl (request)) {
    request->reply = editone2_verse (request);
    return;
  }
  
  if ((url == editone2_update_url ()) && browser_request_security_okay (request) && editone2_update_acl (request)) {
    request->reply = editone2_update (request);
    return;
  }

  if ((url == read_index_url ()) && browser_request_security_okay (request) && read_index_acl (request)) {
    request->reply = read_index (request);
    return;
  }

  if ((url == read_load_url ()) && browser_request_security_okay (request) && read_load_acl (request)) {
    request->reply = read_load (request);
    return;
  }

  if ((url == read_verse_url ()) && browser_request_security_okay (request) && read_verse_acl (request)) {
    request->reply = read_verse (request);
    return;
  }

  if ((url == resource_divider_url ()) && browser_request_security_okay (request) && resource_divider_acl (request)) {
    request->reply = resource_divider (request);
    return;
  }

  if ((url == session_confirm_url ()) && browser_request_security_okay (request) && session_confirm_acl (request)) {
    request->reply = session_confirm (request);
    return;
  }
  
  if ((url == resource_comparative9edit_url ()) && browser_request_security_okay (request) && resource_comparative9edit_acl (request)) {
    request->reply = resource_comparative9edit (request);
    return;
  }

  if ((url == resource_comparative1edit_url ()) && browser_request_security_okay (request) && resource_comparative1edit_acl (request)) {
    request->reply = resource_comparative1edit (request);
    return;
  }

  if ((url == developer_delay_url ()) && developer_delay_acl (request)) {
    request->reply = developer_delay (request);
    return;
  }

  // Forward the browser to the default home page.
  redirect_browser (request, index_index_url ());
}

/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <system/googletranslate.h>
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
    developer_logic_tracer = std::make_shared<Developer_Logic_Tracer>(std::addressof(webserver_request));
  }
  
  // Record the POST request made to the web server.
  // This can be used for debugging.
  /*
  if (!webserver_request.post.empty () && config_logic_demo_enabled ()) {
    string contents;
    int seconds = filter::date::seconds_since_epoch ();
    string rfc822time = filter::date::rfc822 (seconds);
    contents.append (rfc822time + "\n");
    contents.append (webserver_request.get + "\n");
    string query;
    for (auto element : webserver_request.query) {
      query.append (element.first + "=" + element.second + " and ");
    }
    string post;
    for (auto element : webserver_request.post) {
      post.append (element.first + "=" + element.second + " and ");
    }
    contents.append ("query: " + query + "\n");
    contents.append ("post: " + post + "\n");
    string filename;
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

  // check_user_localization_preference (std::addressof(webserver_request));

  if ((url == resource_imagefetch_url ()) && resource_imagefetch_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_imagefetch (std::addressof(webserver_request));
    return;
  }

  // Serve resource downloads.
  if ((extension == "sqlite") && (webserver_request.get.find (Database_Cache::fragment ()) != std::string::npos)) {
    http_stream_file (webserver_request, false);
    return;
  }
  
  // Serve initialization notice.
  if (config::logic::version () != Database_Config_General::getInstalledDatabaseVersion ()) {
    webserver_request.reply = setup_initialization_notice ();
    return;
  }
  
  // Force setup.
  if (config::logic::version () != Database_Config_General::getInstalledInterfaceVersion ()) {
    webserver_request.reply = setup_index (std::addressof(webserver_request));
    return;
  }

  // Home page and menu.
  if ((url == index_index_url ()) && browser_request_security_okay (webserver_request) && index_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = index_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == menu_index_url ()) && browser_request_security_okay (webserver_request) && menu_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = menu_index (std::addressof(webserver_request));
    return;
  }
  
  // Login and logout.
  if ((url == session_login_url ()) && browser_request_security_okay (webserver_request) && session_login_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_login (std::addressof(webserver_request));
    return;
  }
  
  if ((url == session_logout_url ()) && browser_request_security_okay (webserver_request) && session_logout_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_logout (std::addressof(webserver_request));
    return;
  }
  
  if ((url == session_password_url ()) && browser_request_security_okay (webserver_request) && session_password_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_password (std::addressof(webserver_request));
    return;
  }
  
  if ((url == session_signup_url ()) && browser_request_security_okay (webserver_request) && session_signup_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_signup (std::addressof(webserver_request));
    return;
  }
  
  if ((url == session_switch_url ()) && browser_request_security_okay (webserver_request) && session_switch_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_switch (std::addressof(webserver_request));
    return;
  }
  
  // Bible menu.
  if ((url == bible_manage_url ()) && browser_request_security_okay (webserver_request) && bible_manage_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_manage (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_settings_url ()) && browser_request_security_okay (webserver_request) && bible_settings_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_settings (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_book_url ()) && browser_request_security_okay (webserver_request) && bible_book_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_book (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_chapter_url ()) && browser_request_security_okay (webserver_request) && bible_chapter_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_chapter (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_import_url ()) && browser_request_security_okay (webserver_request) && bible_import_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_import (std::addressof(webserver_request));
    return;
  }
  
  if ((url == compare_index_url ()) && browser_request_security_okay (webserver_request) && compare_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = compare_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_order_url ()) && browser_request_security_okay (webserver_request) && bible_order_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_order (std::addressof(webserver_request));
    return;
  }
  
  if ((url == bible_css_url ()) && browser_request_security_okay (webserver_request) && bible_css_acl (std::addressof(webserver_request))) {
    webserver_request.reply = bible_css (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editone_index_url ()) && browser_request_security_okay (webserver_request) && editone_index_acl ()) {
    webserver_request.reply = editone_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editusfm_index_url ()) && browser_request_security_okay (webserver_request) && editusfm_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editusfm_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_index_url ()) && browser_request_security_okay (webserver_request) && edit_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_position_url ()) && browser_request_security_okay (webserver_request) && edit_position_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_position (std::addressof(webserver_request));
    return;
  }

  if ((url == edit_navigate_url ()) && browser_request_security_okay (webserver_request) && edit_navigate_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_navigate (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_index_url ()) && browser_request_security_okay (webserver_request) && search_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == workspace_index_url ()) && browser_request_security_okay (webserver_request) && workspace_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = workspace_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == workspace_organize_url ()) && browser_request_security_okay (webserver_request) && workspace_organize_acl (std::addressof(webserver_request))) {
    webserver_request.reply = workspace_organize (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_bible2resource_url ()) && browser_request_security_okay (webserver_request) && resource_bible2resource_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_bible2resource (std::addressof(webserver_request));
    return;
  }
  
  if ((url == checks_index_url ()) && browser_request_security_okay (webserver_request) && checks_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == checks_settings_url ()) && browser_request_security_okay (webserver_request) && checks_settings_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_settings (std::addressof(webserver_request));
    return;
  }
  
  if ((url == consistency_index_url ()) && browser_request_security_okay (webserver_request) && consistency_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = consistency_index (std::addressof(webserver_request));
    return;
  }
  
  // Notes menu.
  if ((url == notes_index_url ()) && browser_request_security_okay (webserver_request) && notes_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_create_url ()) && browser_request_security_okay (webserver_request) && notes_create_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_create (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_select_url ()) && browser_request_security_okay (webserver_request) && notes_select_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_select (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_note_url ()) && browser_request_security_okay (webserver_request) && notes_note_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_note (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_comment_url ()) && browser_request_security_okay (webserver_request) && notes_comment_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_comment (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_actions_url ()) && browser_request_security_okay (webserver_request) && notes_actions_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_actions (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_assign_1_url ()) && browser_request_security_okay (webserver_request) && notes_assign_1_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_assign_1 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_assign_n_url ()) && browser_request_security_okay (webserver_request) && notes_assign_n_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_assign_n (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_unassign_n_url ()) && browser_request_security_okay (webserver_request) && notes_unassign_n_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_unassign_n (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_status_1_url ()) && browser_request_security_okay (webserver_request) && notes_status_1_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_status_1 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_status_n_url ()) && browser_request_security_okay (webserver_request) && notes_status_n_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_status_n (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_verses_url ()) && browser_request_security_okay (webserver_request) && notes_verses_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_verses (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_severity_1_url ()) && browser_request_security_okay (webserver_request) && notes_severity_1_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_severity_1 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_severity_n_url ()) && browser_request_security_okay (webserver_request) && notes_severity_n_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_severity_n (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_bible_1_url ()) && browser_request_security_okay (webserver_request) && notes_bible_1_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_bible_1 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_bible_n_url ()) && browser_request_security_okay (webserver_request) && notes_bible_n_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_bible_n (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_bulk_url ()) && browser_request_security_okay (webserver_request) && notes_bulk_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_bulk (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_edit_url ()) && browser_request_security_okay (webserver_request) && notes_edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_edit (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_summary_url ()) && browser_request_security_okay (webserver_request) && notes_summary_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_summary (std::addressof(webserver_request));
    return;
  }

  // Resources menu.
  if ((url == resource_index_url ()) && browser_request_security_okay (webserver_request) && resource_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_organize_url ()) && browser_request_security_okay (webserver_request) && resource_organize_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_organize (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_print_url ()) && browser_request_security_okay (webserver_request) && resource_print_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_print (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_manage_url ()) && browser_request_security_okay (webserver_request) && resource_manage_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_manage (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_download_url ()) && browser_request_security_okay (webserver_request) && resource_download_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_download (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_images_url ()) && browser_request_security_okay (webserver_request) && resource_images_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_images (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_sword_url ()) && browser_request_security_okay (webserver_request) && resource_sword_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_sword (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_select_url ()) && browser_request_security_okay (webserver_request) && resource_select_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_select (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_cache_url ()) && browser_request_security_okay (webserver_request) && resource_cache_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_cache (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_user9edit_url ()) && browser_request_security_okay (webserver_request) && resource_user9edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_user9edit (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_user1edit_url ()) && browser_request_security_okay (webserver_request) && resource_user1edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_user1edit (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_user9view_url ()) && browser_request_security_okay (webserver_request) && resource_user9view_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_user9view (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_user1view_url ()) && browser_request_security_okay (webserver_request) && resource_user1view_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_user1view (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_biblegateway_url ()) && browser_request_security_okay (webserver_request) && resource_biblegateway_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_biblegateway (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_studylight_url ()) && browser_request_security_okay (webserver_request) && resource_studylight_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_studylight (std::addressof(webserver_request));
    return;
  }
  
  // Changes menu.
  if ((url == journal_index_url ()) && browser_request_security_okay (webserver_request) && journal_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = journal_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == changes_changes_url ()) && browser_request_security_okay (webserver_request) && changes_changes_acl (std::addressof(webserver_request))) {
    webserver_request.reply = changes_changes (std::addressof(webserver_request));
    return;
  }
  
  if ((url == changes_change_url ()) && browser_request_security_okay (webserver_request) && changes_change_acl (std::addressof(webserver_request))) {
    webserver_request.reply = changes_change (std::addressof(webserver_request));
    return;
  }
  
  if ((url == changes_manage_url ()) && browser_request_security_okay (webserver_request) && changes_manage_acl (std::addressof(webserver_request))) {
    webserver_request.reply = changes_manage (std::addressof(webserver_request));
    return;
  }
  
  if ((url == changes_statistics_url ()) && browser_request_security_okay (webserver_request) && changes_statistics_acl (std::addressof(webserver_request))) {
    webserver_request.reply = changes_statistics (std::addressof(webserver_request));
    return;
  }

  // Planning menu.
  if ((url == sprint_index_url ()) && browser_request_security_okay (webserver_request) && sprint_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = sprint_index (std::addressof(webserver_request));
    return;
  }
  
  // Tools menu.
  if ((url == sendreceive_index_url ()) && browser_request_security_okay (webserver_request) && sendreceive_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = sendreceive_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == manage_exports_url ()) && browser_request_security_okay (webserver_request) && manage_exports_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_exports (std::addressof(webserver_request));
    return;
  }
  
  if ((url == manage_hyphenation_url ()) && browser_request_security_okay (webserver_request) && manage_hyphenation_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_hyphenation (std::addressof(webserver_request));
    return;
  }
  
  if ((url == developer_index_url ()) && browser_request_security_okay (webserver_request) && developer_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = developer_index (std::addressof(webserver_request));
    return;
  }
  
  // Settings menu.
  if ((url == personalize_index_url ()) && browser_request_security_okay (webserver_request) && personalize_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = personalize_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == manage_users_url ()) && browser_request_security_okay (webserver_request) && manage_users_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_users (std::addressof(webserver_request));
    return;
  }

  if ((url == manage_accounts_url ()) && browser_request_security_okay (webserver_request) && manage_accounts_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_accounts (std::addressof(webserver_request));
    return;
  }

  if ((url == manage_index_url ()) && browser_request_security_okay (webserver_request) && manage_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == system_index_url ()) && browser_request_security_okay (webserver_request) && system_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = system_index (std::addressof(webserver_request));
    return;
  }

  if ((url == system_googletranslate_url ()) && browser_request_security_okay (webserver_request) && system_googletranslate_acl (std::addressof(webserver_request))) {
    webserver_request.reply = system_googletranslate (std::addressof(webserver_request));
    return;
  }

  if ((url == email_index_url ()) && browser_request_security_okay (webserver_request) && email_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = email_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == styles_indexm_url ()) && browser_request_security_okay (webserver_request) && styles_indexm_acl (std::addressof(webserver_request))) {
    webserver_request.reply = styles_indexm (std::addressof(webserver_request));
    return;
  }
  
  if ((url == styles_sheetm_url ()) && browser_request_security_okay (webserver_request) && styles_sheetm_acl (std::addressof(webserver_request))) {
    webserver_request.reply = styles_sheetm (std::addressof(webserver_request));
    return;
  }
  
  if ((url == styles_view_url ()) && browser_request_security_okay (webserver_request) && styles_view_acl (std::addressof(webserver_request))) {
    webserver_request.reply = styles_view (std::addressof(webserver_request));
    return;
  }
  
  if ((url == versification_index_url ()) && browser_request_security_okay (webserver_request) && versification_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = versification_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == versification_system_url ()) && browser_request_security_okay (webserver_request) && versification_system_acl (std::addressof(webserver_request))) {
    webserver_request.reply = versification_system (std::addressof(webserver_request));
    return;
  }
  
  if ((url == collaboration_index_url ()) && browser_request_security_okay (webserver_request) && collaboration_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = collaboration_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == client_index_url ()) && browser_request_security_okay (webserver_request) && client_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = client_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == mapping_index_url ()) && browser_request_security_okay (webserver_request) && mapping_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = mapping_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == mapping_map_url ()) && browser_request_security_okay (webserver_request) && mapping_map_acl (std::addressof(webserver_request))) {
    webserver_request.reply = mapping_map (std::addressof(webserver_request));
    return;
  }
  
  if ((url == paratext_index_url ()) && browser_request_security_okay (webserver_request) && paratext_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = paratext_index (std::addressof(webserver_request));
    return;
  }
  
  // Help menu.
  if ((help_index_url (url)) && browser_request_security_okay (webserver_request) && help_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = help_index (std::addressof(webserver_request), url);
    return;
  }

  // User menu.
  if ((url == user_notifications_url ()) && browser_request_security_okay (webserver_request) && user_notifications_acl (std::addressof(webserver_request))) {
    webserver_request.reply = user_notifications (std::addressof(webserver_request));
    return;
  }
  
  if ((url == user_account_url ()) && browser_request_security_okay (webserver_request) && user_account_acl (std::addressof(webserver_request))) {
    webserver_request.reply = user_account (std::addressof(webserver_request));
    return;
  }

  // Public feedback menu.
  if ((url == public_index_url ()) && browser_request_security_okay (webserver_request) && public_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_login_url ()) && browser_request_security_okay (webserver_request) && public_login_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_login (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_chapter_url ()) && browser_request_security_okay (webserver_request) && public_chapter_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_chapter (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_notes_url ()) && browser_request_security_okay (webserver_request) && public_notes_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_notes (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_new_url ()) && browser_request_security_okay (webserver_request) && public_new_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_new (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_create_url ()) && browser_request_security_okay (webserver_request) && public_create_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_create (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_note_url ()) && browser_request_security_okay (webserver_request) && public_note_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_note (std::addressof(webserver_request));
    return;
  }
  
  if ((url == public_comment_url ()) && browser_request_security_okay (webserver_request) && public_comment_acl (std::addressof(webserver_request))) {
    webserver_request.reply = public_comment (std::addressof(webserver_request));
    return;
  }
  
  // Pages not in any menu.
  if ((url == jobs_index_url ()) && browser_request_security_okay (webserver_request) && jobs_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = jobs_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_all_url ()) && browser_request_security_okay (webserver_request) && search_all_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_all (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replace_url ()) && browser_request_security_okay (webserver_request) && search_replace_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replace (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_search2_url ()) && browser_request_security_okay (webserver_request) && search_search2_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_search2 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replace2_url ()) && browser_request_security_okay (webserver_request) && search_replace2_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replace2 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_similar_url ()) && browser_request_security_okay (webserver_request) && search_similar_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_similar (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_strongs_url ()) && browser_request_security_okay (webserver_request) && search_strongs_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_strongs (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_strong_url ()) && browser_request_security_okay (webserver_request) && search_strong_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_strong (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_originals_url ()) && browser_request_security_okay (webserver_request) && search_originals_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_originals (std::addressof(webserver_request));
    return;
  }
  
  if ((url == workspace_settings_url ()) && browser_request_security_okay (webserver_request) && workspace_settings_acl (std::addressof(webserver_request))) {
    webserver_request.reply = workspace_settings (std::addressof(webserver_request));
    return;
  }
  
  if ((url == collaboration_settings_url ()) && browser_request_security_okay (webserver_request) && collaboration_settings_acl (std::addressof(webserver_request))) {
    webserver_request.reply = collaboration_settings (std::addressof(webserver_request));
    return;
  }
  
  if ((url == checks_settingspatterns_url ()) && browser_request_security_okay (webserver_request) && checks_settingspatterns_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_settingspatterns (std::addressof(webserver_request));
    return;
  }
  
  if ((url == checks_settingssentences_url ()) && browser_request_security_okay (webserver_request) && checks_settingssentences_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_settingssentences (std::addressof(webserver_request));
    return;
  }

  if ((url == checks_settingspairs_url ()) && browser_request_security_okay (webserver_request) && checks_settingspairs_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_settingspairs (std::addressof(webserver_request));
    return;
  }

  if ((url == checks_suppress_url ()) && browser_request_security_okay (webserver_request) && checks_suppress_acl (std::addressof(webserver_request))) {
    webserver_request.reply = checks_suppress (std::addressof(webserver_request));
    return;
  }
  
  if ((url == webbible_search_url ()) && browser_request_security_okay (webserver_request) && webbible_search_acl (std::addressof(webserver_request))) {
    webserver_request.reply = webbible_search (std::addressof(webserver_request));
    return;
  }
  
  if ((url == manage_write_url ()) && browser_request_security_okay (webserver_request) && manage_write_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_write (std::addressof(webserver_request));
    return;
  }
  
  if ((url == manage_privileges_url ()) && browser_request_security_okay (webserver_request) && manage_privileges_acl (std::addressof(webserver_request))) {
    webserver_request.reply = manage_privileges (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_image_url ()) && browser_request_security_okay (webserver_request) && resource_image_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_image (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_img_url ()) && browser_request_security_okay (webserver_request) && resource_img_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_img (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editor_select_url ()) && browser_request_security_okay (webserver_request) && editor_select_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editor_select (std::addressof(webserver_request));
    return;
  }
  
  // Downloads
  if ((url == index_listing_url (url)) && browser_request_security_okay (webserver_request) && index_listing_acl (std::addressof(webserver_request), url)) {
    webserver_request.reply = index_listing (std::addressof(webserver_request), url);
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
    webserver_request.reply = sync_setup (std::addressof(webserver_request));
    return;
  }
  if (url == sync_settings_url ()) {
    webserver_request.reply = sync_settings (std::addressof(webserver_request));
    return;
  }
  if (url == sync_bibles_url ()) {
    webserver_request.reply = sync_bibles (std::addressof(webserver_request));
    return;
  }
  if (url == sync_notes_url ()) {
    webserver_request.reply = sync_notes (std::addressof(webserver_request));
    return;
  }
  if (extension == "sqlite") {
    if (filter_url_dirname (url) == filter_url_temp_dir ()) {
      http_stream_file (webserver_request, false);
      return;
    }
  }
  if (url == sync_usfmresources_url ()) {
    webserver_request.reply = sync_usfmresources (std::addressof(webserver_request));
    return;
  }
  if (url == sync_changes_url ()) {
    webserver_request.reply = sync_changes (std::addressof(webserver_request));
    return;
  }
  if (url == sync_files_url ()) {
    webserver_request.reply = sync_files (std::addressof(webserver_request));
    return;
  }
  if (url == sync_resources_url ()) {
    webserver_request.reply = sync_resources (std::addressof(webserver_request));
    return;
  }
  if (url == sync_mail_url ()) {
    webserver_request.reply = sync_mail (std::addressof(webserver_request));
    return;
  }
  
  // AJAX calls.
  if ((url == navigation_update_url ()) && browser_request_security_okay (webserver_request) && navigation_update_acl (std::addressof(webserver_request))) {
    webserver_request.reply = navigation_update (std::addressof(webserver_request));
    return;
  }
  
  if ((url == navigation_poll_url ()) && browser_request_security_okay (webserver_request) && navigation_poll_acl (std::addressof(webserver_request))) {
    webserver_request.reply = navigation_poll (std::addressof(webserver_request));
    return;
  }

#ifdef HAVE_WINDOWS
  if (url == navigation_paratext_url ()) {
    webserver_request.reply = navigation_paratext (std::addressof(webserver_request));
    return;
  }
#endif

  if ((url == edit_preview_url ()) && browser_request_security_okay (webserver_request) && edit_preview_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_preview (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editusfm_focus_url ()) && browser_request_security_okay (webserver_request) && editusfm_focus_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editusfm_focus (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editusfm_load_url ()) && browser_request_security_okay (webserver_request) && editusfm_load_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editusfm_load (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editusfm_offset_url ()) && browser_request_security_okay (webserver_request) && editusfm_offset_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editusfm_offset (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editusfm_save_url ()) && browser_request_security_okay (webserver_request) && editusfm_save_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editusfm_save (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_edit_url ()) && browser_request_security_okay (webserver_request) && edit_edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_edit (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_id_url ()) && browser_request_security_okay (webserver_request) && edit_id_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_id (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_load_url ()) && browser_request_security_okay (webserver_request) && edit_load_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_load (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_save_url ()) && browser_request_security_okay (webserver_request) && edit_save_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_save (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_styles_url ()) && browser_request_security_okay (webserver_request) && edit_styles_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_styles (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_getids_url ()) && browser_request_security_okay (webserver_request) && search_getids_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_getids (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replacepre_url ()) && browser_request_security_okay (webserver_request) && search_replacepre_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replacepre (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replacego_url ()) && browser_request_security_okay (webserver_request) && search_replacego_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replacego (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replacepre2_url ()) && browser_request_security_okay (webserver_request) && search_replacepre2_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replacepre2 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_getids2_url ()) && browser_request_security_okay (webserver_request) && search_getids2_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_getids2 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == search_replacego2_url ()) && browser_request_security_okay (webserver_request) && search_replacego2_acl (std::addressof(webserver_request))) {
    webserver_request.reply = search_replacego2 (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_get_url ()) && browser_request_security_okay (webserver_request) && resource_get_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_get (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_unload_url ()) && browser_request_security_okay (webserver_request) && resource_unload_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_unload (std::addressof(webserver_request));
    return;
  }

  if ((url == notes_poll_url ()) && browser_request_security_okay (webserver_request) && notes_poll_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_poll (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_notes_url ()) && browser_request_security_okay (webserver_request) && notes_notes_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_notes (std::addressof(webserver_request));
    return;
  }
  
  if ((url == notes_click_url ()) && browser_request_security_okay (webserver_request) && notes_click_acl (std::addressof(webserver_request))) {
    webserver_request.reply = notes_click (std::addressof(webserver_request));
    return;
  }
  
  if ((url == consistency_poll_url ()) && browser_request_security_okay (webserver_request) && consistency_poll_acl (std::addressof(webserver_request))) {
    webserver_request.reply = consistency_poll (std::addressof(webserver_request));
    return;
  }
  
  if ((url == consistency_input_url ()) && browser_request_security_okay (webserver_request) && consistency_input_acl (std::addressof(webserver_request))) {
    webserver_request.reply = consistency_input (std::addressof(webserver_request));
    return;
  }
  
  if ((url == lexicon_definition_url ()) && browser_request_security_okay (webserver_request) && lexicon_definition_acl (std::addressof(webserver_request))) {
    webserver_request.reply = lexicon_definition (std::addressof(webserver_request));
    return;
  }

#ifdef HAVE_CLIENT
  // For security reasons, this is only available in a client configuration.
  if (url == assets_external_url ()) {
    webserver_request.reply = assets_external (std::addressof(webserver_request));
    return;
  }
#endif
  
  // RSS feed.
  if ((url == rss_feed_url ()) && browser_request_security_okay (webserver_request) && rss_feed_acl (std::addressof(webserver_request))) {
    webserver_request.reply = rss_feed (std::addressof(webserver_request));
    return;
  }

  if ((url == nmt_index_url ()) && browser_request_security_okay (webserver_request) && nmt_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = nmt_index (std::addressof(webserver_request));
    return;
  }

  if ((url == edit_index_url ()) && browser_request_security_okay (webserver_request) && edit_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_position_url ()) && browser_request_security_okay (webserver_request) && edit_position_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_position (std::addressof(webserver_request));
    return;
  }

  if ((url == edit_navigate_url ()) && browser_request_security_okay (webserver_request) && edit_navigate_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_navigate (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_preview_url ()) && browser_request_security_okay (webserver_request) && edit_preview_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_preview (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_edit_url ()) && browser_request_security_okay (webserver_request) && edit_edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_edit (std::addressof(webserver_request));
    return;
  }

  if ((url == edit_update_url ()) && browser_request_security_okay (webserver_request) && edit_update_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_update (std::addressof(webserver_request));
    return;
  }

  if ((url == editor_id_url ()) && browser_request_security_okay (webserver_request) && editor_id_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editor_id (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_load_url ()) && browser_request_security_okay (webserver_request) && edit_load_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_load (std::addressof(webserver_request));
    return;
  }
  
  if ((url == edit_save_url ()) && browser_request_security_okay (webserver_request) && edit_save_acl (std::addressof(webserver_request))) {
    webserver_request.reply = edit_save (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editor_style_url ()) && browser_request_security_okay (webserver_request) && editor_style_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editor_style (std::addressof(webserver_request));
    return;
  }

  if ((url == editone2_index_url ()) && browser_request_security_okay (webserver_request) && editone2_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editone2_index (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editone2_load_url ()) && browser_request_security_okay (webserver_request) && editone2_load_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editone2_load (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editone2_save_url ()) && browser_request_security_okay (webserver_request) && editone2_save_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editone2_save (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editone2_verse_url ()) && browser_request_security_okay (webserver_request) && editone2_verse_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editone2_verse (std::addressof(webserver_request));
    return;
  }
  
  if ((url == editone2_update_url ()) && browser_request_security_okay (webserver_request) && editone2_update_acl (std::addressof(webserver_request))) {
    webserver_request.reply = editone2_update (std::addressof(webserver_request));
    return;
  }

  if ((url == read_index_url ()) && browser_request_security_okay (webserver_request) && read_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = read_index (std::addressof(webserver_request));
    return;
  }

  if ((url == read_load_url ()) && browser_request_security_okay (webserver_request) && read_load_acl (std::addressof(webserver_request))) {
    webserver_request.reply = read_load (std::addressof(webserver_request));
    return;
  }

  if ((url == read_verse_url ()) && browser_request_security_okay (webserver_request) && read_verse_acl (std::addressof(webserver_request))) {
    webserver_request.reply = read_verse (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_divider_url ()) && browser_request_security_okay (webserver_request) && resource_divider_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_divider (std::addressof(webserver_request));
    return;
  }

  if ((url == session_confirm_url ()) && browser_request_security_okay (webserver_request) && session_confirm_acl (std::addressof(webserver_request))) {
    webserver_request.reply = session_confirm (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_comparative9edit_url ()) && browser_request_security_okay (webserver_request) && resource_comparative9edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_comparative9edit (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_comparative1edit_url ()) && browser_request_security_okay (webserver_request) && resource_comparative1edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_comparative1edit (std::addressof(webserver_request));
    return;
  }

  if ((url == resource_translated9edit_url ()) && browser_request_security_okay (webserver_request) && resource_translated9edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_translated9edit (std::addressof(webserver_request));
    return;
  }
  
  if ((url == resource_translated1edit_url ()) && browser_request_security_okay (webserver_request) && resource_translated1edit_acl (std::addressof(webserver_request))) {
    webserver_request.reply = resource_translated1edit (std::addressof(webserver_request));
    return;
  }

  if ((url == developer_delay_url ()) && developer_delay_acl (std::addressof(webserver_request))) {
    webserver_request.reply = developer_delay (std::addressof(webserver_request));
    return;
  }

  if ((url == images_index_url ()) && browser_request_security_okay (webserver_request) && images_index_acl (std::addressof(webserver_request))) {
    webserver_request.reply = images_index (std::addressof(webserver_request));
    return;
  }

  if ((url == images_view_url ()) && browser_request_security_okay (webserver_request) && images_view_acl (std::addressof(webserver_request))) {
    webserver_request.reply = images_view (std::addressof(webserver_request));
    return;
  }

  if ((url == images_fetch_url ()) && browser_request_security_okay (webserver_request) && images_fetch_acl (std::addressof(webserver_request))) {
    webserver_request.reply = images_fetch (std::addressof(webserver_request));
    return;
  }

  // Forward the browser to the default home page.
  redirect_browser (std::addressof(webserver_request), index_index_url ());
}

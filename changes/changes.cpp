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


#include <changes/changes.h>
#include <assets/header.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <trash/handler.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <changes/logic.h>
#include <menu/logic.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


std::string changes_changes_url ()
{
  return "changes/changes";
}


bool changes_changes_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


std::string changes_changes (Webserver_Request& webserver_request)
{
  // Handle AJAX call to load the summary of a change notification.
  if (webserver_request.query.count ("load")) {
    const int identifier = filter::string::convert_to_int (webserver_request.query["load"]);
    std::stringstream block {};
    const Passage passage = database::modifications::getNotificationPassage (identifier);
    const std::string link = filter_passage_link_for_opening_editor_at (passage.m_book, passage.m_chapter, passage.m_verse);
    std::string category = database::modifications::getNotificationCategory (identifier);
    if (category == changes_personal_category ())
      category = filter::string::emoji_smiling_face_with_smiling_eyes ();
    if (category == changes_bible_category ()) 
      category = filter::string::emoji_open_book ();
    std::string modification = database::modifications::getNotificationModification (identifier);
    block << "<div id=" << std::quoted("entry" + std::to_string (identifier)) << + ">\n";
    block << "<a href=" << std::quoted ("expand") << ">" << filter::string::emoji_file_folder () << "</a>\n";
    block << "<a href=" << std::quoted("remove") << ">" << filter::string::emoji_wastebasket () << "</a>\n";
    block << link << "\n";
    block << category << "\n";
    block << modification << "\n";
    block << "</div>\n";
    return block.str();
  }
  
  
  // Handle AJAX call to remove a change notification.
  if (webserver_request.post_count("remove")) {
    const int remove = filter::string::convert_to_int (webserver_request.post_get("remove"));
    trash_change_notification (webserver_request, remove);
    database::modifications::deleteNotification (remove);
#ifdef HAVE_CLIENT
    webserver_request.database_config_user ()->add_removed_change (remove);
#endif
    webserver_request.database_config_user ()->set_change_notifications_checksum ("");
    return std::string();
  }
  
  
  // Handle AJAX call to navigate to the passage belonging to the change notification.
  if (webserver_request.post_count("navigate")) {
    const std::string navigate = webserver_request.post_get("navigate");
    const int id = filter::string::convert_to_int (navigate);
    const Passage passage = database::modifications::getNotificationPassage (id);
    if (passage.m_book) {
      ipc_focus::set_passage (webserver_request, passage.m_book, passage.m_chapter, filter::string::convert_to_int (passage.m_verse));
      navigation_passage::record_history (webserver_request, passage.m_book, passage.m_chapter, filter::string::convert_to_int (passage.m_verse));
    }
    // Set the correct default Bible for the user.
    const std::string bible = database::modifications::getNotificationBible (id);
    if (!bible.empty ())
      webserver_request.database_config_user()->set_bible (bible);
    return std::string();
  }
  
  
  // Handle query to update the sorting order.
  const std::string sort = webserver_request.query ["sort"];
  if (sort == "verse") {
    webserver_request.database_config_user ()->set_order_changes_by_author (false);
  }
  if (sort == "author") {
    webserver_request.database_config_user ()->set_order_changes_by_author (true);
  }

  
  const std::string& username = webserver_request.session_logic ()->get_username ();
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();
  
  
  std::string page {};
  Assets_Header header = Assets_Header (translate("Changes"), webserver_request);
  header.set_stylesheet ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page += header.run ();
  Assets_View view {};
  

  // The selected Bible, that is, the Bible for which to show the change notifications.
  std::string selectedbible = webserver_request.query ["selectedbible"];
  if (webserver_request.query.count ("selectbible")) {
    selectedbible = webserver_request.query ["selectbible"];
  }
  view.set_variable ("selectedbible", selectedbible);

  
  // Remove a user's personal changes notifications and their matching change notifications in the Bible.
  const std::string matching = webserver_request.query ["matching"];
  if (!matching.empty ()) {
    std::vector <int> ids = database::modifications::clearNotificationMatches (username, matching, changes_bible_category (), selectedbible);
#ifdef HAVE_CLIENT
    // Client records deletions for sending to the Cloud.
    for (const auto id : ids) {
      webserver_request.database_config_user ()->add_removed_change (id);
    }
#endif
    // Clear checksum cache.
    webserver_request.database_config_user ()->set_change_notifications_checksum ("");
  }
  
  
  // Remove all the personal change notifications.
  if (webserver_request.query.count ("personal")) {
    std::vector <int> ids = database::modifications::getNotificationTeamIdentifiers (username, changes_personal_category (), selectedbible);
    for (const auto id : ids) {
      trash_change_notification (webserver_request, id);
      database::modifications::deleteNotification (id);
#ifdef HAVE_CLIENT
      webserver_request.database_config_user ()->add_removed_change (id);
#endif
      webserver_request.database_config_user ()->set_change_notifications_checksum ("");
    }
  }
  
  
  // Remove all the Bible change notifications.
  if (webserver_request.query.count ("bible")) {
    std::vector <int> ids = database::modifications::getNotificationTeamIdentifiers (username, changes_bible_category (), selectedbible);
    for (const auto id : ids) {
      trash_change_notification (webserver_request, id);
      database::modifications::deleteNotification (id);
#ifdef HAVE_CLIENT
      webserver_request.database_config_user ()->add_removed_change (id);
#endif
      webserver_request.database_config_user ()->set_change_notifications_checksum ("");
    }
  }
  
  
  // Remove all the change notifications made by a certain user.
  if (webserver_request.query.count ("dismiss")) {
    const std::string user = webserver_request.query ["dismiss"];
    std::vector <int> ids = database::modifications::getNotificationTeamIdentifiers (username, user, selectedbible);
    for (auto id : ids) {
      trash_change_notification (webserver_request, id);
      database::modifications::deleteNotification (id);
#ifdef HAVE_CLIENT
      webserver_request.database_config_user ()->add_removed_change (id);
#endif
      webserver_request.database_config_user ()->set_change_notifications_checksum ("");
    }
  }
  
  
  // Read the identifiers, optionally sorted on author (that is, category).
  bool sort_on_author = webserver_request.database_config_user ()->get_order_changes_by_author ();
  const std::vector <int> notification_ids = database::modifications::getNotificationIdentifiers (username, selectedbible, sort_on_author);
  // Send the identifiers to the browser for download there.
  std::string pendingidentifiers {};
  for (const auto id : notification_ids) {
    if (!pendingidentifiers.empty ()) pendingidentifiers.append (" ");
    pendingidentifiers.append (std::to_string (id));
  }
  view.set_variable ("pendingidentifiers", pendingidentifiers);
  
  
  std::stringstream loading {};
  loading << "var loading = " << std::quoted(translate("Loading ...")) << ";";
  std::string script = loading.str();
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);

  
  // Add links to enable the user to show the change notifications for one Bible or for all Bibles.
  std::vector <std::string> distinct_bibles = database::modifications::getNotificationDistinctBibles (username);
  // Show the Bible selector if there's more than one distinct Bible.
  bool show_bible_selector = distinct_bibles.size () > 1;
  // Also show the Bible selector if there's no change notifications to display, yet there's at least one distinct Bible.
  // This situation often occurs after clearing a Bible's notifications, so there's no notifications after clearing them,
  // yet there's notifications for other Bibles.
  // Showing the Bible selector in this situation enables the user to select the other Bible(s).
  if (notification_ids.empty () && !distinct_bibles.empty ()) show_bible_selector = true;
  // Show the Bible selector if needed.
  if (show_bible_selector) {
    // If there's more than one distinct Bible, add the "All Bibles" selector.
    if (distinct_bibles.size () > 1) distinct_bibles.insert (distinct_bibles.begin(), "");
    // Iterate over the Bibles and make them all selectable.
    for (const auto & bible : distinct_bibles) {
      std::string cssclass {};
      if (selectedbible == bible) cssclass = "active";
      std::string name (bible);
      if (name.empty ()) name = translate ("All Bibles");
      view.add_iteration ("bibleselector", { std::pair ("selectbible", bible), std::pair ("biblename", name), std::pair ("class", cssclass) } );
    }
  }

  
  // Enable links to dismiss categories of notifications depending on whether there's anything to dismiss.
  // And give details about the number of changes.
  std::vector <int> personal_ids = database::modifications::getNotificationTeamIdentifiers (username, changes_personal_category (), selectedbible);
  if (!personal_ids.empty ()) {
    view.enable_zone ("personal");
    view.set_variable ("personalcount", std::to_string (personal_ids.size ()));
  }
  const std::vector <int> bible_ids = database::modifications::getNotificationTeamIdentifiers (username, changes_bible_category (), selectedbible);
  if (!bible_ids.empty ()) {
    view.enable_zone ("bible");
    view.set_variable ("teamcount", std::to_string (bible_ids.size ()));
  }
  
  
  // Add links to clear the notifications from the individual contributors.
  const std::vector <std::string> categories = database::modifications::getCategories ();
  for (const auto & category : categories) {
    if (category == changes_personal_category ()) continue;
    if (category == changes_bible_category ()) continue;
    const std::string& user = category;
    const std::vector <int> ids = database::modifications::getNotificationTeamIdentifiers (username, user, selectedbible);
    if (!ids.empty ()) {
      view.add_iteration ("individual", {
        std::pair ("user", user),
        std::pair ("selectedbible", selectedbible),
        std::pair ("count", std::to_string(ids.size()))
      });
    }
  }

  
  // Add links to clear matching notifications of the various users.
  for (const auto& category : categories) {
    if (category == changes_bible_category ()) continue;
    const std::string& user = category;
    std::vector <int> personal_ids2 = database::modifications::getNotificationTeamIdentifiers (username, user, selectedbible);
    std::string user_and_icon = translate ("user") + " " + category;
    if (category == changes_personal_category ()) {
      user_and_icon = translate ("me") + " " + filter::string::emoji_smiling_face_with_smiling_eyes ();
    }
    if (!personal_ids2.empty () && !bible_ids.empty ()) {
      view.add_iteration ("matching", { std::pair ("user", user), std::pair ("icon", user_and_icon) } );
    }
  }
  
  
  view.set_variable ("VERSION", config::logic::version ());

  
  if (touch) view.enable_zone ("touch");

  
  view.set_variable ("interlinks", changes_interlinks (webserver_request, changes_changes_url ()));
  
  
  // Create data for the link for how to sort the change notifications.
  std::string sortquery {};
  std::string sorttext {};
  if (webserver_request.database_config_user ()->get_order_changes_by_author ()) {
    sortquery = "verse";
    sorttext = translate ("Sort on verse" );
  } else {
    sortquery = "author";
    sorttext = translate ("Sort on author");
  }
  view.set_variable ("sortquery", sortquery);
  view.set_variable ("sorttext", sorttext);

  
  // Whether to show the controls for dismissing the changes.
  if (!notification_ids.empty ()) {
    // Whether to put those controls at the bottom of the page, as the default location,
    // or whether to put them at the top of the page.
    if (webserver_request.database_config_user ()->get_dismiss_changes_at_top ()) {
      view.enable_zone ("controlsattop");
    } else {
      view.enable_zone ("controlsatbottom");
    }
  }
  
  
  page += view.render ("changes", "changes");
  
  
  page += assets_page::footer ();
  return page;
}

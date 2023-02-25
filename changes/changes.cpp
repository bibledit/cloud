/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;
using namespace pugi;


string changes_changes_url ()
{
  return "changes/changes";
}


bool changes_changes_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string changes_changes (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Modifications database_modifications;
  
  
  // Handle AJAX call to load the summary of a change notification.
  if (request->query.count ("load")) {
    const int identifier = convert_to_int (request->query["load"]);
    stringstream block {};
    const Passage passage = database_modifications.getNotificationPassage (identifier);
    const string link = filter_passage_link_for_opening_editor_at (passage.m_book, passage.m_chapter, passage.m_verse);
    string category = database_modifications.getNotificationCategory (identifier);
    if (category == changes_personal_category ()) category = emoji_smiling_face_with_smiling_eyes ();
    if (category == changes_bible_category ()) category = emoji_open_book ();
    string modification = database_modifications.getNotificationModification (identifier);
    block << "<div id=" << quoted("entry" + convert_to_string (identifier)) << + ">\n";
    block << "<a href=" << quoted ("expand") << ">" << emoji_file_folder () << "</a>\n";
    block << "<a href=" << quoted("remove") << ">" << emoji_wastebasket () << "</a>\n";
    block << link << "\n";
    block << category << "\n";
    block << modification << "\n";
    block << "</div>\n";
    return block.str();
  }
  
  
  // Handle AJAX call to remove a change notification.
  if (request->post.count ("remove")) {
    const int remove = convert_to_int (request->post["remove"]);
    trash_change_notification (request, remove);
    database_modifications.deleteNotification (remove);
#ifdef HAVE_CLIENT
    request->database_config_user ()->addRemovedChange (remove);
#endif
    request->database_config_user ()->setChangeNotificationsChecksum ("");
    return string();
  }
  
  
  // Handle AJAX call to navigate to the passage belonging to the change notification.
  if (request->post.count ("navigate")) {
    string navigate = request->post["navigate"];
    const int id = convert_to_int (navigate);
    const Passage passage = database_modifications.getNotificationPassage (id);
    if (passage.m_book) {
      Ipc_Focus::set (request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
      Navigation_Passage::record_history (request, passage.m_book, passage.m_chapter, convert_to_int (passage.m_verse));
    }
    // Set the correct default Bible for the user.
    const string bible = database_modifications.getNotificationBible (id);
    if (!bible.empty ()) request->database_config_user()->setBible (bible);
    return string();
  }
  
  
  // Handle query to update the sorting order.
  const string sort = request->query ["sort"];
  if (sort == "verse") {
    request->database_config_user ()->setOrderChangesByAuthor (false);
  }
  if (sort == "author") {
    request->database_config_user ()->setOrderChangesByAuthor (true);
  }

  
  const string username = request->session_logic()->currentUser ();
  const bool touch = request->session_logic ()->touchEnabled ();
  
  
  string page {};
  Assets_Header header = Assets_Header (translate("Changes"), request);
  header.set_stylesheet ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  if (touch) header.jquery_touch_on ();
  page += header.run ();
  Assets_View view {};
  

  // The selected Bible, that is, the Bible for which to show the change notifications.
  string selectedbible = request->query ["selectedbible"];
  if (request->query.count ("selectbible")) {
    selectedbible = request->query ["selectbible"];
  }
  view.set_variable ("selectedbible", selectedbible);

  
  // Remove a user's personal changes notifications and their matching change notifications in the Bible.
  const string matching = request->query ["matching"];
  if (!matching.empty ()) {
    vector <int> ids = database_modifications.clearNotificationMatches (username, matching, changes_bible_category (), selectedbible);
#ifdef HAVE_CLIENT
    // Client records deletions for sending to the Cloud.
    for (const auto id : ids) {
      request->database_config_user ()->addRemovedChange (id);
    }
#endif
    // Clear checksum cache.
    request->database_config_user ()->setChangeNotificationsChecksum ("");
  }
  
  
  // Remove all the personal change notifications.
  if (request->query.count ("personal")) {
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, changes_personal_category (), selectedbible);
    for (const auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
#ifdef HAVE_CLIENT
      request->database_config_user ()->addRemovedChange (id);
#endif
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Remove all the Bible change notifications.
  if (request->query.count ("bible")) {
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, changes_bible_category (), selectedbible);
    for (const auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
#ifdef HAVE_CLIENT
      request->database_config_user ()->addRemovedChange (id);
#endif
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Remove all the change notifications made by a certain user.
  if (request->query.count ("dismiss")) {
    string user = request->query ["dismiss"];
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, user, selectedbible);
    for (auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
#ifdef HAVE_CLIENT
      request->database_config_user ()->addRemovedChange (id);
#endif
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Read the identifiers, optionally sorted on author (that is, category).
  bool sort_on_author = request->database_config_user ()->getOrderChangesByAuthor ();
  vector <int> notification_ids = database_modifications.getNotificationIdentifiers (username, selectedbible, sort_on_author);
  // Send the identifiers to the browser for download there.
  string pendingidentifiers {};
  for (auto id : notification_ids) {
    if (!pendingidentifiers.empty ()) pendingidentifiers.append (" ");
    pendingidentifiers.append (convert_to_string (id));
  }
  view.set_variable ("pendingidentifiers", pendingidentifiers);
  
  
  stringstream loading {};
  loading << quoted(translate("Loading ..."));
  string script = "var loading = " + loading.str() + ";";
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);

  
  // Add links to enable the user to show the change notifications for one Bible or for all Bibles.
  vector <string> distinct_bibles = database_modifications.getNotificationDistinctBibles (username);
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
      string cssclass {};
      if (selectedbible == bible) cssclass = "active";
      string name (bible);
      if (name.empty ()) name = translate ("All Bibles");
      view.add_iteration ("bibleselector", { pair ("selectbible", bible), pair ("biblename", name), pair ("class", cssclass) } );
    }
  }

  
  // Enable links to dismiss categories of notifications depending on whether there's anything to dismiss.
  // And give details about the number of changes.
  vector <int> personal_ids = database_modifications.getNotificationTeamIdentifiers (username, changes_personal_category (), selectedbible);
  if (!personal_ids.empty ()) {
    view.enable_zone ("personal");
    view.set_variable ("personalcount", convert_to_string (personal_ids.size ()));
  }
  vector <int> bible_ids = database_modifications.getNotificationTeamIdentifiers (username, changes_bible_category (), selectedbible);
  if (!bible_ids.empty ()) {
    view.enable_zone ("bible");
    view.set_variable ("teamcount", convert_to_string (bible_ids.size ()));
  }
  
  
  // Add links to clear the notifications from the individual contributors.
  vector <string> categories = database_modifications.getCategories ();
  for (const auto & category : categories) {
    if (category == changes_personal_category ()) continue;
    if (category == changes_bible_category ()) continue;
    const string & user = category;
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, user, selectedbible);
    if (!ids.empty ()) {
      view.add_iteration ("individual", {
        pair ("user", user),
        pair ("selectedbible", selectedbible),
        pair ("count", convert_to_string(ids.size()))
      });
    }
  }

  
  // Add links to clear matching notifications of the various users.
  for (const auto & category : categories) {
    if (category == changes_bible_category ()) continue;
    const string & user = category;
    vector <int> personal_ids2 = database_modifications.getNotificationTeamIdentifiers (username, user, selectedbible);
    string user_and_icon = translate ("user") + " " + category;
    if (category == changes_personal_category ()) {
      user_and_icon = translate ("me") + " " + emoji_smiling_face_with_smiling_eyes ();
    }
    if (!personal_ids2.empty () && !bible_ids.empty ()) {
      view.add_iteration ("matching", { pair ("user", user), pair ("icon", user_and_icon) } );
    }
  }
  
  
  view.set_variable ("VERSION", config::logic::version ());

  
  if (touch) view.enable_zone ("touch");

  
  view.set_variable ("interlinks", changes_interlinks (webserver_request, changes_changes_url ()));
  
  
  // Create data for the link for how to sort the change notifications.
  string sortquery {};
  string sorttext {};
  if (request->database_config_user ()->getOrderChangesByAuthor ()) {
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
    if (request->database_config_user ()->getDismissChangesAtTop ()) {
      view.enable_zone ("controlsattop");
    } else {
      view.enable_zone ("controlsatbottom");
    }
  }
  
  
  page += view.render ("changes", "changes");
  
  
  page += assets_page::footer ();
  return page;
}

/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <menu/logic.h>
#include <menu/index.h>
#include <system/index.h>
#include <bible/manage.h>
#include <changes/changes.h>
#include <changes/manage.h>
#include <checks/index.h>
#include <checks/settings.h>
#include <client/index.h>
#include <client/logic.h>
#include <collaboration/index.h>
#include <config/globals.h>
#include <consistency/index.h>
#include <database/config/general.h>
#include <database/userresources.h>
#include <developer/index.h>
#include <edit/index.h>
#include <editone/index.h>
#include <editusfm/index.h>
#include <editverse/index.h>
#include <email/index.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <index/index.h>
#include <index/listing.h>
#include <journal/index.h>
#include <locale/translate.h>
#include <manage/exports.h>
#include <manage/hyphenation.h>
#include <manage/index.h>
#include <manage/users.h>
#include <mapping/index.h>
#include <notes/index.h>
#include <notes/select.h>
#include <paratext/index.h>
#include <personalize/index.h>
#include <resource/images.h>
#include <resource/index.h>
#include <resource/manage.h>
#include <resource/print.h>
#include <resource/sword.h>
#include <resource/cache.h>
#include <resource/user9edit.h>
#include <resource/user9view.h>
#include <resource/biblegateway.h>
#include <resource/studylight.h>
#include <search/index.h>
#include <search/replace.h>
#include <search/search2.h>
#include <search/replace2.h>
#include <search/all.h>
#include <search/similar.h>
#include <search/strongs.h>
#include <search/strong.h>
#include <search/originals.h>
#include <sendreceive/index.h>
#include <session/login.h>
#include <session/logout.h>
#include <sprint/index.h>
#include <styles/indexm.h>
#include <styles/indext.h>
#include <user/account.h>
#include <user/notifications.h>
#include <versification/index.h>
#include <workspace/index.h>
#include <workspace/logic.h>
#include <workspace/logic.h>
#include <workspace/logic.h>
#include <workspace/organize.h>
#include <workspace/organize.h>
#include <xrefs/index.h>
#include <public/index.h>
#include <public/logic.h>
#include <filter/url.h>
#include <basic/index.h>
#include <bible/logic.h>
#include <ldap/logic.h>


string menu_logic_href (string href)
{
  href = filter_string_str_replace ("?", "__q__", href);
  href = filter_string_str_replace ("&", "__a__", href);
  href = filter_string_str_replace ("=", "__i__", href);
  return href;
}


string menu_logic_click (string item)
{
  item = filter_string_str_replace ("__q__", "?", item);
  item = filter_string_str_replace ("__a__", "&", item);
  item = filter_string_str_replace ("__i__", "=", item);
  Database_Config_General::setLastMenuClick (item);
  return item;
}


string menu_logic_create_item (string href, string text, bool history, string title)
{
  string item;
  item.append ("<span class=\"nowrap\">");
  item.append ("<a href=\"/");
  if (history) {
    item.append (menu_index_url ());
  } else {
    item.append (index_index_url ());
  }
  item.append ("?item=");
  item.append (menu_logic_href (href) + "\" title=\"" + title + "\">" + text + "</a>");
  item.append ("</span>");
  return item;
}


string menu_logic_translate_menu ()
{
  return "translate";
}


string menu_logic_search_menu ()
{
  return "search";
}


string menu_logic_tools_menu ()
{
  return "tools";
}


string menu_logic_settings_menu ()
{
  return "settings";
}


string menu_logic_settings_resources_menu ()
{
  return "settings-resources";
}


string menu_logic_settings_styles_menu ()
{
  return "settings-styles";
}


// Returns the html for the main menu categories.
string menu_logic_main_categories (void * webserver_request, string & tooltip)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  vector <string> html;
  vector <string> tooltipbits;

  if (workspace_index_acl (webserver_request)) {
    string label = translate ("Workspace");
    string tooltip;
    menu_logic_desktop_category (webserver_request, &tooltip);
    html.push_back (menu_logic_create_item (workspace_index_url (), label, true, tooltip));
    tooltipbits.push_back (label);
  }

  string menutooltip;

  if (!menu_logic_translate_category (webserver_request, &menutooltip).empty ()) {
    html.push_back (menu_logic_create_item (menu_logic_translate_menu (), menu_logic_translate_text (), false, menutooltip));
    tooltipbits.push_back (menu_logic_translate_text ());
  }
  
  if (!menu_logic_search_category (webserver_request, &menutooltip).empty ()) {
    html.push_back (menu_logic_create_item (menu_logic_search_menu (), menu_logic_search_text (), false, menutooltip));
    tooltipbits.push_back (menu_logic_search_text ());
  }

  if (!menu_logic_tools_category (webserver_request, &menutooltip).empty ()) {
    html.push_back (menu_logic_create_item (menu_logic_tools_menu (), menu_logic_tools_text (), false, menutooltip));
    tooltipbits.push_back (menu_logic_tools_text ());
  }

  if (!menu_logic_settings_category (webserver_request).empty ()) {
    html.push_back (menu_logic_create_item (menu_logic_settings_menu (), menu_logic_settings_text (), false, menutooltip));
    tooltipbits.push_back (menu_logic_settings_text ());
  }
  
  if (!menu_logic_help_category (webserver_request).empty ()) {
    html.push_back (menu_logic_create_item ("help/index", menu_logic_help_text (), true, menu_logic_help_text ()));
    tooltipbits.push_back (menu_logic_help_text ());
  }

  // When a user is not logged in, or a guest,
  // put the public feedback into the main menu, rather than in a sub menu.
#ifndef HAVE_CLIENT
  if (menu_logic_public_or_guest (webserver_request)) {
    if (!public_logic_bibles (webserver_request).empty ()) {
      html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true));
      tooltipbits.push_back (menu_logic_public_feedback_text ());
    }
  }
#endif

  // When a user is logged in, and is a guest, put the Logout into the main menu, rather than in a sub menu.
  if (request->session_logic ()->loggedIn ()) {
    if (request->session_logic ()->currentLevel () == Filter_Roles::guest ()) {
      if (session_logout_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true));
        tooltipbits.push_back (menu_logic_logout_text ());
      }
    }
  }

  
  // When not logged in, display Login menu item.
  if (request->session_logic ()->currentUser ().empty ()) {
    string label = translate ("Login");
    html.push_back (menu_logic_create_item (session_login_url (), label, true));
    tooltipbits.push_back (label);
  }

  tooltip = filter_string_implode (tooltipbits, " | ");
  return filter_string_implode (html, "\n");
}


/*
 Some of the functions below generate a start menu.
 
 It goes through all possible menu entries.
 It reads the access levels of those entries.
 It takes the menu entries the currently logged-in user has access to.
 
 It originally self-organized the entries such that the ones used clicked often came earlier in the menu.
 But menu entries moving around creates confusion.
 Therefore it was removed again.
 */


string menu_logic_basic_categories (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  vector <string> html;
  
  if (edit_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (editone_index_url (), translate ("Translation"), true));
  }
  
  if (changes_changes_acl (webserver_request)) {
    if (request->database_config_user ()->getMenuChangesInBasicMode ()) {
      html.push_back (menu_logic_create_item (changes_changes_url (), menu_logic_changes_text (), true));
    }
  }

  if (notes_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (notes_index_url (), translate ("Notes"), true));
  }
  
  if (resource_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_index_url (), translate ("Resources"), true));
  }

  if (basic_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (basic_index_url (), "⋮", true));
  }

  // When a user is not logged in, or a guest,
  // put the public feedback into the main menu, rather than in a sub menu.
#ifndef HAVE_CLIENT
  if (menu_logic_public_or_guest (webserver_request)) {
    if (!public_logic_bibles (webserver_request).empty ()) {
      html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true));
    }
  }
#endif

  // When not logged in, display Login menu item.
  if (request->session_logic ()->currentUser ().empty ()) {
    html.push_back (menu_logic_create_item (session_login_url (), translate ("Login"), true));
  }

  // When a user is logged in, and is a guest, put the Logout into the main menu, rather than in a sub menu.
#ifndef HAVE_CLIENT
  if (request->session_logic ()->loggedIn ()) {
    if (request->session_logic ()->currentLevel () == Filter_Roles::guest ()) {
      if (session_logout_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true));
      }
    }
  }
#endif

  return filter_string_implode (html, "\n");
}


string menu_logic_desktop_category (void * webserver_request, string * tooltip)
{
  vector <string> html;
  vector <string> labels;

  // Add the available configured desktops to the menu.
  // The user's role should be sufficiently high.
  if (workspace_organize_acl (webserver_request)) {
    vector <string> workspacees = workspace_get_names (webserver_request);
    for (size_t i = 0; i < workspacees.size(); i++) {
      string item = menu_logic_create_item (workspace_index_url () + "?bench=" + convert_to_string (i), workspacees[i], true);
      html.push_back (item);
      labels.push_back (workspacees [i]);
    }
  }

  if (tooltip) tooltip->assign (filter_string_implode (labels, " | "));
  return filter_string_implode (html, "\n");
}


string menu_logic_translate_category (void * webserver_request, string * tooltip)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  vector <string> html;
  vector <string> labels;
  
  if (edit_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, true)) {
      string label = menu_logic_editor_menu_text (webserver_request, true, true);
      html.push_back (menu_logic_create_item (edit_index_url (), label, true));
      labels.push_back (label);
    }
  }
  
  if (editone_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, false)) {
      string label = menu_logic_editor_menu_text (webserver_request, true, false);
      html.push_back (menu_logic_create_item (editone_index_url (), label, true));
      labels.push_back (label);
    }
  }

  if (editusfm_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, false, true)) {
      string label = menu_logic_editor_menu_text (webserver_request, false, true);
      html.push_back (menu_logic_create_item (editusfm_index_url (), label, true));
      labels.push_back (label);
    }
  }
    
  if (editverse_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, false, false)) {
      string label = menu_logic_editor_menu_text (webserver_request, false, false);
      html.push_back (menu_logic_create_item (editverse_index_url (), label, true));
      labels.push_back (label);
    }
  }
  
  if (notes_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (notes_index_url (), menu_logic_consultation_notes_text (), true));
    labels.push_back (menu_logic_consultation_notes_text ());
  }

  if (resource_index_acl (webserver_request)) {
    string label = translate ("Resources");
    html.push_back (menu_logic_create_item (resource_index_url (), label, true));
    labels.push_back (label);
  }

  if (resource_user9view_acl (webserver_request)) {
    // Only display user-defined resources if they are there.
    if (!Database_UserResources::names ().empty ()) {
      string label = translate ("User resources");
      html.push_back (menu_logic_create_item (resource_user9view_url (), label, true));
      labels.push_back (label);
    }
  }
  
  if (changes_changes_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (changes_changes_url (), menu_logic_changes_text (), true));
    labels.push_back (menu_logic_changes_text ());
  }

  // The exports are available to anyone on the Internet,
  // but the menu item is only displayed when someone is logged in.
  if (request->session_logic ()->currentLevel () > Filter_Roles::guest ()) {
    if (index_listing_acl (webserver_request, "exports")) {
      string label = translate ("Exports");
      html.push_back (menu_logic_create_item (index_listing_url ("exports"), label, true));
      labels.push_back (label);
    }
  }

  // When a user is logged in, but not a guest,
  // put the public feedback into this sub menu, rather than in the main menu.
#ifndef HAVE_CLIENT
  if (!request->session_logic ()->currentUser ().empty ()) {
    if (!menu_logic_public_or_guest (webserver_request)) {
      if (!public_logic_bibles (webserver_request).empty ()) {
        html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true));
        labels.push_back (menu_logic_public_feedback_text ());
      }
    }
  }
#endif
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_translate_text () + ": ");
  }

  if (tooltip) tooltip->assign (filter_string_implode (labels, " | "));
  return filter_string_implode (html, "\n");
}


string menu_logic_search_category (void * webserver_request, string * tooltip)
{
  vector <string> html;
  vector <string> labels;

  if (search_index_acl (webserver_request)) {
    string label = translate ("Search");
    html.push_back (menu_logic_create_item (search_index_url (), label, true));
    labels.push_back (label);
  }
  
  if (search_replace_acl (webserver_request)) {
    string label = translate ("Replace");
    html.push_back (menu_logic_create_item (search_replace_url (), label, true));
    labels.push_back (label);
  }
  
  if (search_search2_acl (webserver_request)) {
    string label = translate ("Advanced search");
    html.push_back (menu_logic_create_item (search_search2_url (), translate ("Advanced search"), true));
    labels.push_back (label);
  }
  
  if (search_replace2_acl (webserver_request)) {
    string label = translate ("Advanced replace");
    html.push_back (menu_logic_create_item (search_replace2_url (), label, true));
    labels.push_back (label);
  }
  
  if (search_all_acl (webserver_request)) {
    string label = translate ("Search all Bibles and notes");
    html.push_back (menu_logic_create_item (search_all_url (), label, true));
    labels.push_back (label);
  }

  if (search_similar_acl (webserver_request)) {
    string label = translate ("Search Bible for similar verses");
    html.push_back (menu_logic_create_item (search_similar_url (), label, true));
    labels.push_back (label);
  }

  if (search_strongs_acl (webserver_request)) {
    string label = translate ("Search Bible for similar Strong's numbers");
    html.push_back (menu_logic_create_item (search_strongs_url (), label, true));
    labels.push_back (label);
  }

  if (search_strong_acl (webserver_request)) {
    string label = translate ("Search Bible for Strong's number");
    html.push_back (menu_logic_create_item (search_strong_url (), label, true));
    labels.push_back (label);
  }

  if (search_originals_acl (webserver_request)) {
    string label = translate ("Search Bible for similar Hebrew or Greek words");
    html.push_back (menu_logic_create_item (search_originals_url (), label, true));
    labels.push_back (label);
  }
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_search_text () + ": ");
  }
  
  if (tooltip) tooltip->assign (filter_string_implode (labels, " | "));
  return filter_string_implode (html, "\n");
}


string menu_logic_tools_category (void * webserver_request, string * tooltip)
{
  // The labels that may end up in the menu.
  string checks = translate ("Checks");
  string consistency = translate ("Consistency");
  string print = translate ("Print");
  string changes = menu_logic_changes_text ();
  string planning = translate ("Planning");
  string send_receive = translate ("Send/receive");
  string hyphenation = translate ("Hyphenate");
  string cross_references = translate ("Transfer cross-references");
  string develop = translate ("Develop");
  string exporting = translate ("Export");
  string journal = translate ("Journal");
  vector <string> labels = {
    checks,
    consistency,
    print,
    changes,
    planning,
    send_receive,
    hyphenation,
    cross_references,
    develop,
    exporting,
    journal
  };
  
  // Sort the labels in alphabetical order for the menu.
  // Using the localized labels means that the sorted order of the menu depends on the localization.
  sort (labels.begin (), labels.end ());
  
  vector <string> html;
  vector <string> tiplabels;

  for (auto & label : labels) {

    if (label == checks) {
      if (checks_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (checks_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }

    if (label == consistency) {
      if (consistency_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (consistency_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }

    if (label == print) {
#ifndef HAVE_CLIENT
      if (resource_print_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (resource_print_url (), label, true));
        tiplabels.push_back (label);
      }
#endif
    }
    
    if (label == changes) {
      // Downloading revisions only on server, not on client.
#ifndef HAVE_CLIENT
      if (index_listing_acl (webserver_request, "revisions")) {
        html.push_back (menu_logic_create_item (index_listing_url ("revisions"), menu_logic_changes_text (), true));
        tiplabels.push_back (menu_logic_changes_text ());
      }
#endif
    }
    
    if (label == planning) {
#ifndef HAVE_CLIENT
      if (sprint_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (sprint_index_url (), label, true));
        tiplabels.push_back (label);
      }
#endif
    }
    
    if (label == send_receive) {
      if (sendreceive_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (sendreceive_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == hyphenation) {
      if (manage_hyphenation_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_hyphenation_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == cross_references) {
      if (xrefs_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (xrefs_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == develop) {
      if (developer_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (developer_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == exporting) {
      if (manage_exports_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_exports_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == journal) {
      if (journal_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (journal_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
  }
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_tools_text () + ": ");
  }
  
  if (tooltip) tooltip->assign (filter_string_implode (tiplabels, " | "));
  return filter_string_implode (html, "\n");
}


string menu_logic_settings_category (void * webserver_request, string * tooltip)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  bool client = client_logic_client_enabled ();
  bool demo = config_logic_demo_enabled ();

  // The labels that may end up in the menu.
  string bibles = menu_logic_bible_manage_text ();
  string desktops = menu_logic_desktop_organize_text ();
  string checks = menu_logic_checks_settings_text ();
  string resources = menu_logic_resources_text ();
  string changes = menu_logic_changes_text ();
  string preferences = translate ("Preferences");
  string users = menu_logic_manage_users_text ();
  string indexes_fonts = translate ("Indexes and Fonts");
  string mail = translate ("Mail");
  string styles = menu_logic_styles_text ();
  string versifications = menu_logic_versification_index_text ();
  string mappings = menu_logic_mapping_index_text ();
  string repository = translate ("Repository");
  string cloud = translate ("Cloud");
  string paratext = translate ("Paratext");
  string logout = menu_logic_logout_text ();
  string notifications = translate ("Notifications");
  string account = translate ("Account");
  string basic_mode = translate ("Basic mode");
  string system = translate ("System");
  vector <string> labels = {
    bibles,
    desktops,
    checks,
    resources,
    changes,
    preferences,
    users,
    indexes_fonts,
    mail,
    styles,
    versifications,
    mappings,
    repository,
    cloud,
    paratext,
    logout,
    notifications,
    account,
    basic_mode,
    system
  };
  
  // Sort the labels in alphabetical order for the menu.
  // Using the localized labels means that the sorted order of the menu depends on the localization.
  sort (labels.begin (), labels.end ());
  
  vector <string> html;
  vector <string> tiplabels;
  
  for (auto & label : labels) {
  
    if (label == bibles) {
      if (bible_manage_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (bible_manage_url (), menu_logic_bible_manage_text (), true));
        tiplabels.push_back (menu_logic_bible_manage_text ());
      }
    }
    
    if (label == desktops) {
      if (workspace_organize_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (workspace_organize_url (), menu_logic_desktop_organize_text (), true));
        tiplabels.push_back (menu_logic_desktop_organize_text ());
      }
    }
    
    if (label == checks) {
      if (checks_settings_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (checks_settings_url (), menu_logic_checks_settings_text (), true));
        tiplabels.push_back (menu_logic_checks_settings_text ());
      }
    }
    
    if (label == resources) {
      if (!menu_logic_settings_resources_category (webserver_request).empty ()) {
        html.push_back (menu_logic_create_item (menu_logic_settings_resources_menu (), menu_logic_resources_text (), false));
        tiplabels.push_back (menu_logic_resources_text ());
      }
#ifdef HAVE_CLIENT
      // Only client can cache resources.
      // The Cloud is always online, with a fast connection, and can easily fetch a resource from the web.
      // Many Cloud instances may run on one server, and if the Cloud were to cache resources,
      /// it was going to use a huge amount of disk space.
      if (resource_cache_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (resource_cache_url (), menu_logic_resources_text (), true));
        tiplabels.push_back (menu_logic_resources_text ());
      }
#endif
    }
    
    if (label == changes) {
#ifndef HAVE_CLIENT
      // Managing change notifications only on server, not on client.
      if (changes_manage_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (changes_manage_url (), menu_logic_changes_text (), true));
        tiplabels.push_back (menu_logic_changes_text ());
      }
#endif
    }
    
    if (label == preferences) {
      if (personalize_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (personalize_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == users) {
#ifndef HAVE_CLIENT
      if (manage_users_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_users_url (), menu_logic_manage_users_text (), true));
        tiplabels.push_back (menu_logic_manage_users_text ());
      }
#endif
    }
    
    if (label == indexes_fonts) {
      if (manage_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == mail) {
#ifndef HAVE_CLIENT
      if (email_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (email_index_url (), label, true));
        tiplabels.push_back (label);
      }
#endif
    }
    
    if (label == styles) {
      if (!menu_logic_settings_styles_category (webserver_request).empty ()) {
        html.push_back (menu_logic_create_item (menu_logic_settings_styles_menu (), label, false));
        tiplabels.push_back (menu_logic_settings_styles_menu ());
      }
    }
    
    if (label == versifications) {
      if (versification_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (versification_index_url (), menu_logic_versification_index_text (), true));
        tiplabels.push_back (menu_logic_versification_index_text ());
      }
    }
    
    if (label == mappings) {
      if (mapping_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (mapping_index_url (), menu_logic_mapping_index_text (), true));
        tiplabels.push_back (menu_logic_mapping_index_text ());
      }
    }
    
#ifndef HAVE_CLIENT
    if (label == repository) {
      if (collaboration_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (collaboration_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }
#endif
    
    if (label == cloud) {
      // If the installation is not prepared for Client mode, disable the Cloud menu item.
      // But keep the menu item in an open installation.
      bool cloud_menu = client_index_acl (webserver_request);
#ifndef HAVE_CLIENT
      cloud_menu = false;
#endif
      if (config_logic_demo_enabled ()) cloud_menu = true;
      if (cloud_menu) {
        if (client_index_acl (webserver_request)) {
          html.push_back (menu_logic_create_item (client_index_url (), label, true));
          tiplabels.push_back (client_index_url ());
        }
      }
    }
    
#ifdef HAVE_PARATEXT
    if (label == paratext) {
      if (paratext_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (paratext_index_url (), label, true));
        tiplabels.push_back (paratext_index_url ());
      }
    }
#endif
    
    if (label == logout) {
      if (!(client || demo)) {
        // If logged in, but not as guest, put the Logout menu here.
        if (request->session_logic ()->loggedIn ()) {
          if (request->session_logic ()->currentLevel () != Filter_Roles::guest ()) {
            if (session_logout_acl (webserver_request)) {
              html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true));
              tiplabels.push_back (menu_logic_logout_text ());
            }
          }
        }
      }
    }
    
    if (label == notifications) {
      if (user_notifications_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (user_notifications_url (), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == account) {
      if (!(client || demo)) {
        if (!ldap_logic_is_on ()) {
          if (user_account_acl (webserver_request)) {
            html.push_back (menu_logic_create_item (user_account_url (), label, true));
            tiplabels.push_back (label);
          }
        }
      }
    }
    
    if (label == basic_mode) {
      if (request->session_logic ()->currentLevel () > Filter_Roles::guest ()) {
        html.push_back (menu_logic_create_item (index_index_url () + convert_to_string ("?mode=basic"), label, true));
        tiplabels.push_back (label);
      }
    }
    
    if (label == system) {
      if (system_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (system_index_url (), label, true));
        tiplabels.push_back (label);
      }
    }

  }
  
  if (!html.empty ()) {
    string user = request->session_logic ()->currentUser ();
    html.insert (html.begin (), menu_logic_settings_text () + " (" + user + "): ");
  }
  
  if (tooltip) tooltip->assign (filter_string_implode (tiplabels, " | "));
  return filter_string_implode (html, "\n");
}


string menu_logic_settings_resources_category (void * webserver_request)
{
  vector <string> html;
  
#ifdef HAVE_CLOUD
  if (resource_manage_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_manage_url (), translate ("USFM"), true));
  }
#endif
  
#ifdef HAVE_CLOUD
  if (resource_images_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_images_url (), translate ("Images"), true));
  }
#endif
  
#ifdef HAVE_CLOUD
  if (resource_sword_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_sword_url (), translate ("SWORD"), true));
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_user9edit_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_user9edit_url (), translate ("User-defined"), true));
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_biblegateway_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_biblegateway_url (), "BibleGateway", true));
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_studylight_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_studylight_url (), "StudyLight", true));
  }
#endif

  (void) webserver_request;
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_resources_text () + ": ");
  }
  
  return filter_string_implode (html, "\n");
}


string menu_logic_settings_styles_category (void * webserver_request)
{
  vector <string> html;
  
  if (styles_indext_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (styles_indext_url (), translate ("Select stylesheet"), true));
  }
  
  if (styles_indexm_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (styles_indexm_url (), menu_logic_styles_indexm_text (), true));
  }
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_styles_text () + ": ");
  }
  
  return filter_string_implode (html, "\n");
}


string menu_logic_help_category (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  vector <string> html;

  if (!request->session_logic ()->currentUser ().empty ()) {
    html.push_back (menu_logic_create_item ("help/index", translate ("Help and About"), true));
  }

  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_help_text () + ": ");
  }
  
  return filter_string_implode (html, "\n");
}


// Returns true in case the user is a public user, that is, not logged-in,
// or when the user has the role of Guest.
bool menu_logic_public_or_guest (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  if (request->session_logic ()->currentUser ().empty ()) return true;
  if (request->session_logic ()->currentLevel () == Filter_Roles::guest ()) return true;
  return false;
}


// Returns the text that belongs to a certain menu item.
string menu_logic_menu_text (string menu_item)
{
  if (menu_item == menu_logic_translate_menu ()) {
    return menu_logic_translate_text ();
  }
  if (menu_item == notes_index_url ()) {
    return menu_logic_consultation_notes_text ();
  }
  if (menu_item == menu_logic_search_menu ()) {
    return menu_logic_search_text ();
  }
  if (menu_item == menu_logic_tools_menu ()) {
    return menu_logic_tools_text ();
  }
  if (menu_item == menu_logic_settings_menu ()) {
    return menu_logic_settings_text ();
  }
  return translate ("Menu");
}


// Returns the URL that belongs to $menu_item.
string menu_logic_menu_url (string menu_item)
{
  if (
      (menu_item == menu_logic_translate_menu ())
      ||
      (menu_item == menu_logic_search_menu ())
      ||
      (menu_item == menu_logic_tools_menu ())
      ||
      (menu_item == menu_logic_settings_menu ())
      ||
      (menu_item == menu_logic_settings_styles_menu ())
    ) {
    return filter_url_build_http_query (index_index_url (), "item", menu_item);
  }

  return menu_item;
}


string menu_logic_translate_text ()
{
  return translate ("Translate");
}


string menu_logic_search_text ()
{
  return translate ("Search");
}


string menu_logic_tools_text ()
{
  return translate ("Tools");
}


string menu_logic_settings_text ()
{
  return translate ("Settings");
}


string menu_logic_help_text ()
{
  return translate ("Help");
}


string menu_logic_public_feedback_text ()
{
  return translate ("Feedback");
}


string menu_logic_logout_text ()
{
  return translate ("Logout");
}


string menu_logic_consultation_notes_text ()
{
  return translate ("Notes");
}


string menu_logic_bible_manage_text ()
{
  return translate ("Bibles");
}


string menu_logic_desktop_organize_text ()
{
  return translate ("Workspaces");
}


string menu_logic_checks_settings_text ()
{
  return translate ("Checks");
}


string menu_logic_resources_text ()
{
  return translate ("Resources");
}


string menu_logic_resource_images_text ()
{
  return translate ("Image resources");
}


string menu_logic_manage_users_text ()
{
  return translate ("Users");
}


string menu_logic_versification_index_text ()
{
  return translate ("Versifications");
}


string menu_logic_mapping_index_text ()
{
  return translate ("Verse mappings");
}


string menu_logic_styles_indext_text ()
{
  return translate ("Select stylesheet");
}


string menu_logic_styles_indexm_text ()
{
  return translate ("Edit stylesheet");
}


string menu_logic_changes_text ()
{
  return translate ("Changes");
}


string menu_logic_styles_text ()
{
  return translate ("Styles");
}


string menu_logic_editor_settings_text (bool visual, int selection)
{
  if (visual) {
    if (selection == 0) return translate ("Both the visual chapter and visual verse editors");
    if (selection == 1) return translate ("Only the visual chapter editor");
    if (selection == 2) return translate ("Only the visual verse editor");
  } else {
    if (selection == 0) return translate ("Both the USFM chapter and USFM verse editors");
    if (selection == 1) return translate ("Only the USFM chapter editor");
    if (selection == 2) return translate ("Only the USFM verse editor");
  }
  return "";
}


bool menu_logic_editor_enabled (void * webserver_request, bool visual, bool chapter)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Get the user's preference for the visual or USFM editors.
  int selection = 0;
  if (visual) selection = request->database_config_user ()->getEnabledVisualEditors ();
  else selection = request->database_config_user ()->getEnabledUsfmEditors ();
  
  // Check whether the visual or USFM chapter/verse editor is active.
  if (selection == 0) return true;
  if ((selection == 1) && chapter) return true;
  if ((selection == 2) && !chapter) return true;
  
  // The requested editor is inactive.
  return false;
}


string menu_logic_editor_menu_text (void * webserver_request, bool visual, bool chapter)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Get the user's preference for the visual or USFM editors.
  int selection = 0;
  if (visual) selection = request->database_config_user ()->getEnabledVisualEditors ();
  else selection = request->database_config_user ()->getEnabledUsfmEditors ();
  
  // Get the correct menu text.
  bool both = (selection == 0);
  if (visual && chapter && both) return translate ("Visual chapter editor");
  if (visual && !chapter && both) return translate ("Visual verse editor");
  if (visual && !both) return translate ("Text");
  
  if (!visual && chapter && both) return translate ("USFM chapter editor");
  if (!visual && !chapter && both) return translate ("USFM verse editor");
  if (!visual && !both) return translate ("USFM");
  
  return translate ("Bible");
}

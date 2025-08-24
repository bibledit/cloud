/*
Copyright (©) 2003-2025 Teus Benschop.

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
#include <bb/manage.h>
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
#include <database/cache.h>
#include <developer/index.h>
#include <edit/index.h>
#include <edit/index.h>
#include <editone/index.h>
#include <editusfm/index.h>
#include <email/index.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <index/index.h>
#include <index/listing.h>
#include <journal/index.h>
#include <locale/translate.h>
#include <manage/exports.h>
#include <manage/users.h>
#include <mapping/index.h>
#include <notes/index.h>
#include <notes/select.h>
#include <paratext/index.h>
#include <personalize/index.h>
#include <resource/index.h>
#include <resource/manage.h>
#include <resource/sword.h>
#include <resource/cache.h>
#include <resource/user9edit.h>
#include <resource/user9view.h>
#include <resource/bbgateway.h>
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
#include <user/account.h>
#include <user/notifications.h>
#include <versification/index.h>
#include <workspace/index.h>
#include <workspace/logic.h>
#include <workspace/logic.h>
#include <workspace/logic.h>
#include <workspace/organize.h>
#include <workspace/organize.h>
#include <public/index.h>
#include <public/logic.h>
#include <filter/url.h>
#include <bb/logic.h>
#include <ldap/logic.h>
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <jsonxx/jsonxx.h>
#pragma GCC diagnostic pop
#include <read/index.h>
#include <filter/css.h>
#include <resource/comparative9edit.h>
#include <resource/translated9edit.h>
#include <images/index.h>


std::string menu_logic_href (std::string href)
{
  href = filter::strings::replace ("?", "__q__", href);
  href = filter::strings::replace ("&", "__a__", href);
  href = filter::strings::replace ("=", "__i__", href);
  return href;
}


std::string menu_logic_click (std::string item)
{
  item = filter::strings::replace ("__q__", "?", item);
  item = filter::strings::replace ("__a__", "&", item);
  item = filter::strings::replace ("__i__", "=", item);
  database::config::general::set_last_menu_click (item);
  return item;
}


std::string menu_logic_create_item (std::string href, std::string text, bool history, std::string title, std::string colour)
{
  std::string item;
  item.append (R"(<span class="nowrap)");
  if (!colour.empty()) item.append (" " + colour);
  item.append (R"("><a href="/)");
  if (history) {
    item.append (menu_index_url ());
  } else {
    item.append (index_index_url ());
  }
  item.append ("?item=");
  item.append (menu_logic_href (href) + R"(" title=")" + title + R"(">)" + text + "</a>");
  item.append ("</span>");
  return item;
}


std::string menu_logic_translate_menu ()
{
  return "translate";
}


std::string menu_logic_search_menu ()
{
  return "search";
}


std::string menu_logic_tools_menu ()
{
  return "tools";
}


std::string menu_logic_settings_menu ()
{
  return "settings";
}


std::string menu_logic_settings_resources_menu ()
{
  return "settings-resources";
}


// Returns the html for the main menu categories.
// Also fills the $tooltip with an appropriate value for this main menu.
// This function is called for the main page, that is, the home page.
std::string menu_logic_main_categories (Webserver_Request& webserver_request, std::string & tooltip)
{
  // The sets of html that is going to form the menu.
  std::vector <std::string> html;
  
  // The sets of tooltips for the main menu item.
  std::vector <std::string> tooltipbits;

  // Deal with a situation the user has access to the workspaces.
  if (workspace_index_acl (webserver_request)) {
    if (config::logic::default_bibledit_configuration ()) {
      std::string label = translate ("Workspace");
      std::string tooltip2;
      menu_logic_workspace_category (webserver_request, &tooltip2);
      html.push_back (menu_logic_create_item (workspace_index_url (), label, true, tooltip2, ""));
      tooltipbits.push_back (label);
    }
  }

  std::string menutooltip;
  int current_theme_index = webserver_request.database_config_user ()->get_current_theme ();
  std::string color = Filter_Css::theme_picker (current_theme_index, 1);

  if (!menu_logic_translate_category (webserver_request, &menutooltip).empty ()) {
    if (config::logic::default_bibledit_configuration ()) {
      html.push_back (menu_logic_create_item (menu_logic_translate_menu (), menu_logic_translate_text (), false, menutooltip, color));
      tooltipbits.push_back (menu_logic_translate_text ());
    }
  }
  
  if (!menu_logic_search_category (webserver_request, &menutooltip).empty ()) {
    if (config::logic::default_bibledit_configuration ()) {
      html.push_back (menu_logic_create_item (menu_logic_search_menu (), menu_logic_search_text (), false, menutooltip, color));
      tooltipbits.push_back (menu_logic_search_text ());
    }
  }

  if (!menu_logic_tools_category (webserver_request, &menutooltip).empty ()) {
    if (config::logic::default_bibledit_configuration ()) {
      html.push_back (menu_logic_create_item (menu_logic_tools_menu (), menu_logic_tools_text (), false, menutooltip, color));
      tooltipbits.push_back (menu_logic_tools_text ());
    }
  }

  if (!menu_logic_settings_category (webserver_request, &menutooltip).empty ()) {
    if (config::logic::default_bibledit_configuration ()) {
      html.push_back (menu_logic_create_item (menu_logic_settings_menu (), menu_logic_settings_text (), false, menutooltip, color));
      tooltipbits.push_back (menu_logic_settings_text ());
    }
  }
  
  if (!menu_logic_help_category (webserver_request).empty ()) {
    if (config::logic::default_bibledit_configuration ()) {
      html.push_back (menu_logic_create_item ("help/index", menu_logic_help_text (), true, menu_logic_help_text (), color));
      tooltipbits.push_back (menu_logic_help_text ());
    }
  }

#ifdef HAVE_CLOUD
  // When a user is not logged in, or if a guest is logged in,
  // put the public feedback into the main menu, rather than in a sub menu.
  if (menu_logic_public_or_guest (webserver_request)) {
    if (!public_logic_bibles ().empty ()) {
      if (!config::logic::create_no_accounts()) {
        html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true, "", ""));
        tooltipbits.push_back (menu_logic_public_feedback_text ());
      }
    }
  }
#endif

  // When a user is logged in, and is a guest, put the Logout into the main menu, rather than in a sub menu.
  if (webserver_request.session_logic ()->get_logged_in ()) {
    if (webserver_request.session_logic ()->get_level () == roles::guest) {
      if (session_logout_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true, "", ""));
        tooltipbits.push_back (menu_logic_logout_text ());
      }
    }
  }

  
  // When not logged in, display Login menu item.
  if (webserver_request.session_logic ()->get_username ().empty ()) {
    std::string label = translate ("Login");
    html.push_back (menu_logic_create_item (session_login_url (), label, true, "", ""));
    tooltipbits.push_back (label);
  }

  // Create one string of tool tips for this menu item, separated by a vertical bar.
  tooltip = filter::strings::implode (tooltipbits, " | ");
  
  // Create one string of html that is going to form the menu.
  return filter::strings::implode (html, "\n");
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


std::string menu_logic_basic_categories (Webserver_Request& webserver_request)
{
  std::vector <std::string> html;

  int current_theme_index = webserver_request.database_config_user ()->get_current_theme ();
  std::string color = Filter_Css::theme_picker (current_theme_index, 1);

  if (read_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (read_index_url (), translate ("Read"), true, "", color));
  }

  if (resource_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_index_url (), menu_logic_resources_text (), true, "", color));
  }

  if (editone_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (editone_index_url (), menu_logic_translate_text (), true, "", color));
  }
  
  if (changes_changes_acl (webserver_request)) {
    if (webserver_request.database_config_user ()->get_menu_changes_in_basic_mode ()) {
      html.push_back (menu_logic_create_item (changes_changes_url (), menu_logic_changes_text (), true, "", color));
    }
  }

  if (notes_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (notes_index_url (), menu_logic_consultation_notes_text (), true, "", color));
  }
  
  if (personalize_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (personalize_index_url (), "⋮", true, "", color));
  }

#ifdef HAVE_CLOUD
  // When a user is not logged in, or a guest,
  // put the public feedback into the main menu, rather than in a sub menu.
  // This is the default configuration.
  bool public_feedback_possible { true };
  if (public_feedback_possible) {
    if (menu_logic_public_or_guest (webserver_request)) {
      if (!public_logic_bibles ().empty ()) {
        if (!config::logic::create_no_accounts()) {
          html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true, "", ""));
        }
      }
    }
  }
#endif

  // When not logged in, display Login menu item.
  if (webserver_request.session_logic ()->get_username ().empty ()) {
    html.push_back (menu_logic_create_item (session_login_url (), translate ("Login"), true, "", ""));
  }

  // When a user is logged in, and is a guest,
  // put the Logout into the main menu,
  // rather than in a sub menu.
#ifdef HAVE_CLOUD
  if (webserver_request.session_logic ()->get_logged_in ()) {
    if (webserver_request.session_logic ()->get_level () == roles::guest) {
      if (session_logout_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true, "", ""));
      }
    }
  }
#endif

  return filter::strings::implode (html, "\n");
}


// Generates html for the workspace main menu.
// Plus the tooltip for it.
std::string menu_logic_workspace_category (Webserver_Request& webserver_request, std::string * tooltip)
{
  std::vector <std::string> html;
  std::vector <std::string> labels;

  // Add the available configured workspaces to the menu.
  // The user's role should be sufficiently high.
  if (workspace_organize_acl (webserver_request)) {
    std::string activeWorkspace = webserver_request.database_config_user()->get_active_workspace ();

    std::vector <std::string> workspaces = workspace_get_names (webserver_request);
    for (size_t i = 0; i < workspaces.size(); i++) {
      std::string item = menu_logic_create_item (workspace_index_url () + "?bench=" + std::to_string (i), workspaces[i], true, "", "");
      // Adds an active class if it is the current workspace.
      if (workspaces[i] == activeWorkspace) {
        size_t startIndex = item.find(R"("><a)");
        if (startIndex != std::string::npos) item.insert (startIndex, " active");
      }
      html.push_back (item);
      labels.push_back (workspaces [i]);
    }
  }

  if (tooltip) tooltip->assign (filter::strings::implode (labels, " | "));
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_translate_category (Webserver_Request& webserver_request, std::string * tooltip)
{
  std::vector <std::string> html;
  std::vector <std::string> labels;
  
  // Visual chapter editor.
  if (edit_index_acl (webserver_request)) {
    std::string label = menu_logic_editor_menu_text (true, true);
    html.push_back (menu_logic_create_item (edit_index_url (), label, true, "", ""));
    labels.push_back (label);
  }

  // Visual verse editor.
  if (editone_index_acl (webserver_request)) {
    std::string label = menu_logic_editor_menu_text (true, false);
    html.push_back (menu_logic_create_item (editone_index_url (), label, true, "", ""));
    labels.push_back (label);
  }

  // USFM (chapter) editor.
  if (editusfm_index_acl (webserver_request)) {
    std::string label = menu_logic_editor_menu_text (false, true);
    html.push_back (menu_logic_create_item (editusfm_index_url (), label, true, "", ""));
    labels.push_back (label);
  }
    
  if (notes_index_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (notes_index_url (), menu_logic_consultation_notes_text (), true, "", ""));
    labels.push_back (menu_logic_consultation_notes_text ());
  }

  if (resource_index_acl (webserver_request)) {
    std::string label = menu_logic_resources_text ();
    html.push_back (menu_logic_create_item (resource_index_url (), label, true, "", ""));
    labels.push_back (label);
  }

  if (resource_user9view_acl (webserver_request)) {
    // Only display user-defined resources if they are there.
    if (!Database_UserResources::names ().empty ()) {
      std::string label = translate ("User resources");
      html.push_back (menu_logic_create_item (resource_user9view_url (), label, true, "", ""));
      labels.push_back (label);
    }
  }
  
  if (changes_changes_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (changes_changes_url (), menu_logic_changes_text (), true, "", ""));
    labels.push_back (menu_logic_changes_text ());
  }

  // When a user is logged in, but not a guest,
  // put the public feedback into this sub menu, rather than in the main menu.
#ifndef HAVE_CLIENT
  if (!webserver_request.session_logic ()->get_username ().empty ()) {
    if (!menu_logic_public_or_guest (webserver_request)) {
      if (!public_logic_bibles ().empty ()) {
        if (!config::logic::create_no_accounts()) {
          html.push_back (menu_logic_create_item (public_index_url (), menu_logic_public_feedback_text (), true, "", ""));
          labels.push_back (menu_logic_public_feedback_text ());
        }
      }
    }
  }
#endif
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_translate_text () + ": ");
  }

  if (tooltip) tooltip->assign (filter::strings::implode (labels, " | "));
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_search_category (Webserver_Request& webserver_request, std::string * tooltip)
{
  std::vector <std::string> html;
  std::vector <std::string> labels;

  if (search_index_acl (webserver_request)) {
    std::string label = translate ("Search");
    html.push_back (menu_logic_create_item (search_index_url (), label, true, "", ""));
    labels.push_back (label);
  }
  
  if (search_replace_acl (webserver_request)) {
    std::string label = translate ("Replace");
    html.push_back (menu_logic_create_item (search_replace_url (), label, true, "", ""));
    labels.push_back (label);
  }
  
  if (search_search2_acl (webserver_request)) {
    std::string label = translate ("Advanced search");
    html.push_back (menu_logic_create_item (search_search2_url (), translate ("Advanced search"), true, "", ""));
    labels.push_back (label);
  }
  
  if (search_replace2_acl (webserver_request)) {
    std::string label = translate ("Advanced replace");
    html.push_back (menu_logic_create_item (search_replace2_url (), label, true, "", ""));
    labels.push_back (label);
  }
  
  if (search_all_acl (webserver_request)) {
    std::string label = translate ("Search all Bibles and notes");
    html.push_back (menu_logic_create_item (search_all_url (), label, true, "", ""));
    labels.push_back (label);
  }

  if (search_similar_acl (webserver_request)) {
    std::string label = translate ("Search Bible for similar verses");
    html.push_back (menu_logic_create_item (search_similar_url (), label, true, "", ""));
    labels.push_back (label);
  }

  if (search_strongs_acl (webserver_request)) {
    std::string label = translate ("Search Bible for similar Strong's numbers");
    html.push_back (menu_logic_create_item (search_strongs_url (), label, true, "", ""));
    labels.push_back (label);
  }

  if (search_strong_acl (webserver_request)) {
    std::string label = translate ("Search Bible for Strong's number");
    html.push_back (menu_logic_create_item (search_strong_url (), label, true, "", ""));
    labels.push_back (label);
  }

  if (search_originals_acl (webserver_request)) {
    std::string label = translate ("Search Bible for similar Hebrew or Greek words");
    html.push_back (menu_logic_create_item (search_originals_url (), label, true, "", ""));
    labels.push_back (label);
  }
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_search_text () + ": ");
  }
  
  if (tooltip) tooltip->assign (filter::strings::implode (labels, " | "));
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_tools_category (Webserver_Request& webserver_request, std::string * tooltip)
{
  // The labels that may end up in the menu.
  const std::string checks = translate ("Checks");
  const std::string consistency = translate ("Consistency");
  const std::string changes = menu_logic_changes_text ();
  const std::string planning = translate ("Planning");
  const std::string send_receive = translate ("Send/receive");
  const std::string develop = translate ("Develop");
  const std::string exporting = translate ("Export");
  const std::string journal = translate ("Journal");
  std::vector <std::string> labels = {
    checks,
    consistency,
    changes,
    planning,
    send_receive,
    develop,
    exporting,
    journal
  };
  
  // Sort the labels in alphabetical order for the menu.
  // Using the localized labels means that the sorted order of the menu depends on the localization.
  sort (labels.begin (), labels.end ());
  
  std::vector <std::string> html;
  std::vector <std::string> tiplabels;

  for (auto & label : labels) {

    if (label == checks) {
      if (checks_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (checks_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }

    if (label == consistency) {
      if (consistency_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (consistency_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }

    if (label == changes) {
      // Downloading revisions only on server, not on client.
#ifndef HAVE_CLIENT
      if (index_listing_acl (webserver_request, "revisions")) {
        html.push_back (menu_logic_create_item (index_listing_url ("revisions"), menu_logic_changes_text (), true, "", ""));
        tiplabels.push_back (menu_logic_changes_text ());
      }
#endif
    }
    
    if (label == planning) {
#ifndef HAVE_CLIENT
      if (sprint_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (sprint_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
#endif
    }
    
    if (label == send_receive) {
      if (sendreceive_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (sendreceive_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
    if (label == develop) {
      if (developer_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (developer_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
    if (label == exporting) {
      if (manage_exports_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_exports_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
    if (label == journal) {
      if (journal_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (journal_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
  }
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_tools_text () + ": ");
  }
  
  if (tooltip) tooltip->assign (filter::strings::implode (tiplabels, " | "));
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_settings_category (Webserver_Request& webserver_request, std::string * tooltip)
{
  [[maybe_unused]] bool demo = config::logic::demo_enabled ();
  
  // The labels that may end up in the menu.
  std::string bibles = menu_logic_bible_manage_text ();
  std::string workspaces = menu_logic_workspace_organize_text ();
  std::string checks = menu_logic_checks_settings_text ();
  std::string resources = menu_logic_resources_text ();
  std::string changes = menu_logic_changes_text ();
  std::string preferences = translate ("Preferences");
  std::string users = menu_logic_manage_users_text ();
  std::string mail = translate ("Mail");
  std::string styles = menu_logic_styles_text ();
  std::string versifications = menu_logic_versification_index_text ();
  std::string mappings = menu_logic_mapping_index_text ();
  std::string repository = translate ("Repository");
  std::string cloud = translate ("Cloud");
  std::string paratext = translate ("Paratext");
  std::string notifications = translate ("Notifications");
  std::string account = translate ("Account");
  std::string basic_mode = translate ("Basic mode");
  std::string system = translate ("System");
  std::string images = menu_logic_images_index_text();
  std::vector <std::string> labels = {
    bibles,
    workspaces,
    checks,
    resources,
    changes,
    preferences,
    users,
    mail,
    styles,
    versifications,
    mappings,
    repository,
    cloud,
    paratext,
    menu_logic_logout_text (),
    notifications,
    account,
    basic_mode,
    system,
    images
  };
  
  // Sort the labels in alphabetical order for the menu.
  // Using the localized labels means that the sorted order of the menu depends on the localization.
  sort (labels.begin (), labels.end ());
  
  std::vector <std::string> html;
  std::vector <std::string> tiplabels;
  
  for (auto & label : labels) {
  
    if (label == bibles) {
      if (bible_manage_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (bible_manage_url (), menu_logic_bible_manage_text (), true, "", ""));
        tiplabels.push_back (menu_logic_bible_manage_text ());
      }
    }
    
    if (label == workspaces) {
      if (workspace_organize_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (workspace_organize_url (), menu_logic_workspace_organize_text (), true, "", ""));
        tiplabels.push_back (menu_logic_workspace_organize_text ());
      }
    }
    
    if (label == checks) {
      if (checks_settings_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (checks_settings_url (), menu_logic_checks_settings_text (), true, "", ""));
        tiplabels.push_back (menu_logic_checks_settings_text ());
      }
    }
    
    if (label == resources) {
      if (!menu_logic_settings_resources_category (webserver_request).empty ()) {
        html.push_back (menu_logic_create_item (menu_logic_settings_resources_menu (), menu_logic_resources_text (), false, "", ""));
        tiplabels.push_back (menu_logic_resources_text ());
      }
#ifdef HAVE_CLIENT
      // Only client can cache resources.
      // The Cloud is always online, with a fast connection, and can easily fetch a resource from the web.
      // Many Cloud instances may run on one server, and if the Cloud were to cache resources,
      // it would be going to use a huge amount of disk space.
      if (resource_cache_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (resource_cache_url (), menu_logic_resources_text (), true, "", ""));
        tiplabels.push_back (menu_logic_resources_text ());
      }
#endif
    }
    
    if (label == changes) {
#ifndef HAVE_CLIENT
      // Managing change notifications only on server, not on client.
      if (changes_manage_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (changes_manage_url (), menu_logic_changes_text (), true, "", ""));
        tiplabels.push_back (menu_logic_changes_text ());
      }
#endif
    }
    
    if (label == preferences) {
      if (personalize_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (personalize_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
    if (label == users) {
#ifndef HAVE_CLIENT
      if (manage_users_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (manage_users_url (), menu_logic_manage_users_text (), true, "", ""));
        tiplabels.push_back (menu_logic_manage_users_text ());
      }
#endif
    }
    
    if (label == mail) {
#ifndef HAVE_CLIENT
      if (email_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (email_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
#endif
    }
    
    if (label == styles) {
      if (styles_indexm_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (styles_indexm_url (), menu_logic_styles_text (), true, "", ""));
        tiplabels.push_back (menu_logic_styles_text ());
      }
    }
    
    if (label == versifications) {
      if (versification_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (versification_index_url (), menu_logic_versification_index_text (), true, "", ""));
        tiplabels.push_back (menu_logic_versification_index_text ());
      }
    }
    
    if (label == mappings) {
      if (mapping_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (mapping_index_url (), menu_logic_mapping_index_text (), true, "", ""));
        tiplabels.push_back (menu_logic_mapping_index_text ());
      }
    }
    
#ifndef HAVE_CLIENT
    if (label == repository) {
      if (collaboration_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (collaboration_index_url (), label, true, "", ""));
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
      if (config::logic::demo_enabled ()) cloud_menu = true;
      if (cloud_menu) {
        if (client_index_acl (webserver_request)) {
          html.push_back (menu_logic_create_item (client_index_url (), label, true, "", ""));
          tiplabels.push_back (client_index_url ());
        }
      }
    }
    
#ifdef HAVE_PARATEXT
    if (label == paratext) {
      if (paratext_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (paratext_index_url (), label, true, "", ""));
        tiplabels.push_back (paratext_index_url ());
      }
    }
#endif
    
#ifdef HAVE_CLOUD
    // Logout menu entry only in the Cloud, never on the client.
    if (label == menu_logic_logout_text ()) {
      // Cannot logout in the demo.
      if (!demo) {
        // If logged in, but not as guest, put the Logout menu here.
        if (webserver_request.session_logic ()->get_logged_in ()) {
          if (webserver_request.session_logic ()->get_level () != roles::guest) {
            if (session_logout_acl (webserver_request)) {
              html.push_back (menu_logic_create_item (session_logout_url (), menu_logic_logout_text (), true, "", ""));
              tiplabels.push_back (menu_logic_logout_text ());
            }
          }
        }
      }
    }
#endif
    
    if (label == notifications) {
      if (user_notifications_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (user_notifications_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
#ifdef HAVE_CLOUD
    if (label == account) {
      if (!demo) {
        if (!ldap_logic_is_on ()) {
          if (user_account_acl (webserver_request)) {
            html.push_back (menu_logic_create_item (user_account_url (), label, true, "", ""));
            tiplabels.push_back (label);
          }
        }
      }
    }
#endif
    
    if (label == basic_mode) {
      if (webserver_request.session_logic ()->get_level () > roles::guest) {
        html.push_back (menu_logic_create_item (index_index_url () + filter::strings::convert_to_string ("?mode=basic"), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }
    
    if (label == system) {
      if (system_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (system_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }

    if (label == images) {
      if (images_index_acl (webserver_request)) {
        html.push_back (menu_logic_create_item (images_index_url (), label, true, "", ""));
        tiplabels.push_back (label);
      }
    }

  }
  
  if (!html.empty ()) {
    const std::string& user = webserver_request.session_logic ()->get_username ();
    html.insert (html.begin (), menu_logic_settings_text () + " (" + user + "): ");
  }
  
  if (tooltip) tooltip->assign (filter::strings::implode (tiplabels, " | "));
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_settings_resources_category ([[maybe_unused]] Webserver_Request& webserver_request)
{
  std::vector <std::string> html;
  
#ifdef HAVE_CLOUD
  if (resource_manage_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_manage_url (), translate ("USFM"), true, "", ""));
  }
#endif
  
#ifdef HAVE_CLOUD
  if (!config_globals_hide_bible_resources) {
    if (resource_sword_acl (webserver_request)) {
      html.push_back (menu_logic_create_item (resource_sword_url (), translate ("SWORD"), true, "", ""));
    }
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_user9edit_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_user9edit_url (), translate ("User-defined"), true, "", ""));
  }
#endif

#ifdef HAVE_CLOUD
  if (!config_globals_hide_bible_resources) {
    if (resource_biblegateway_acl (webserver_request)) {
      html.push_back (menu_logic_create_item (resource_biblegateway_url (), "BibleGateway", true, "", ""));
    }
  }
#endif

#ifdef HAVE_CLOUD
  if (!config_globals_hide_bible_resources) {
    if (resource_studylight_acl (webserver_request)) {
      html.push_back (menu_logic_create_item (resource_studylight_url (), "StudyLight", true, "", ""));
    }
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_comparative9edit_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_comparative9edit_url (), translate ("Comparative"), true, "", ""));
  }
#endif

#ifdef HAVE_CLOUD
  if (resource_translated9edit_acl (webserver_request)) {
    html.push_back (menu_logic_create_item (resource_translated9edit_url (), translate ("Translated"), true, "", ""));
  }
#endif
  
  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_resources_text () + ": ");
  }
  
  return filter::strings::implode (html, "\n");
}


std::string menu_logic_help_category (Webserver_Request& webserver_request)
{
  std::vector <std::string> html;

  if (!webserver_request.session_logic ()->get_username ().empty ()) {
    html.push_back (menu_logic_create_item ("help/index", translate ("Help and About"), true, "", ""));
  }

  if (!html.empty ()) {
    html.insert (html.begin (), menu_logic_help_text () + ": ");
  }
  
  return filter::strings::implode (html, "\n");
}


// Returns true in case the user is a public user, that is, not logged-in,
// or when the user has the role of Guest.
bool menu_logic_public_or_guest (Webserver_Request& webserver_request)
{
  if (webserver_request.session_logic ()->get_username ().empty ()) return true;
  if (webserver_request.session_logic ()->get_level () == roles::guest) return true;
  return false;
}


// Returns the text that belongs to a certain menu item.
std::string menu_logic_menu_text (std::string menu_item)
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
  return menu_logic_menu_text ();
}


// Returns the URL that belongs to $menu_item.
std::string menu_logic_menu_url (std::string menu_item)
{
  if ((menu_item == menu_logic_translate_menu ())
      || (menu_item == menu_logic_search_menu ())
      || (menu_item == menu_logic_tools_menu ())
      || (menu_item == menu_logic_settings_menu ())) {
      return filter_url_build_http_query(index_index_url (), {{"item", menu_item}});
  }

  return menu_item;
}


std::string menu_logic_translate_text ()
{
  return translate ("Translate");
}


std::string menu_logic_search_text ()
{
  return translate ("Search");
}


std::string menu_logic_tools_text ()
{
  return translate ("Tools");
}


std::string menu_logic_settings_text ()
{
  return translate ("Settings");
}


std::string menu_logic_help_text ()
{
  return translate ("Help");
}


std::string menu_logic_public_feedback_text ()
{
  return translate ("Feedback");
}


std::string menu_logic_logout_text ()
{
  return translate ("Logout");
}


std::string menu_logic_consultation_notes_text ()
{
  return translate ("Notes");
}


std::string menu_logic_bible_manage_text ()
{
  return translate ("Bibles");
}


std::string menu_logic_workspace_organize_text ()
{
  return translate ("Workspaces");
}


std::string menu_logic_checks_settings_text ()
{
  return translate ("Checks");
}


std::string menu_logic_resources_text ()
{
  return translate ("Resources");
}


std::string menu_logic_manage_users_text ()
{
  return translate ("Users");
}


std::string menu_logic_versification_index_text ()
{
  return translate ("Versifications");
}


std::string menu_logic_mapping_index_text ()
{
  return translate ("Verse mappings");
}


std::string menu_logic_styles_indext_text ()
{
  return translate ("Select stylesheet");
}


std::string menu_logic_styles_indexm_text ()
{
  return translate ("Edit stylesheet");
}


std::string menu_logic_changes_text ()
{
  return translate ("Changes");
}


std::string menu_logic_styles_text ()
{
  return translate ("Styles");
}


std::string menu_logic_menu_text ()
{
  return translate ("Menu");
}


std::string menu_logic_images_index_text ()
{
  return translate ("Images");
}


std::string menu_logic_editor_settings_text (bool visual, int selection)
{
  if (visual) {
    if (selection == 0) return translate ("Both the visual chapter and visual verse editors");
    if (selection == 1) return translate ("Only the visual chapter editor");
    if (selection == 2) return translate ("Only the visual verse editor");
  } else {
    if (selection <= 0) return translate ("Hide");
    if (selection >= 1) return translate ("Show");
  }
  return std::string();
}


bool menu_logic_editor_enabled (Webserver_Request& webserver_request, bool visual, bool chapter)
{
  // Get the user's preference for the visual or USFM editors.
  int selection = 0;
  if (visual) selection = webserver_request.database_config_user ()->get_fast_switch_visual_editors ();
  else selection = webserver_request.database_config_user ()->get_fast_switch_usfm_editors ();

  if (visual) {
    // Check whether the visual chapter or verse editor is active.
    if (selection == 0) return true;
    if ((selection == 1) && chapter) return true;
    if ((selection == 2) && !chapter) return true;
  } else {
    // Check whether the USFM chapter editor is active.
    if (selection >= 1) return true;
  }

  // The requested editor is inactive.
  return false;
}


std::string menu_logic_editor_menu_text (bool visual, bool chapter)
{
  // Get the correct menu text.
  if (visual && chapter) return translate ("Chapter editor");
  if (visual && !chapter) return translate ("Verse editor");
  if (!visual) return translate ("USFM editor");
  // Fallback.
  return translate ("Bible");
}


// Whether the device can do tabbed mode.
bool menu_logic_can_do_tabbed_mode ()
{
#ifdef HAVE_ANDROID
  return true;
#endif
#ifdef HAVE_IOS
  return true;
#endif
  return false;
}


// For internal repatitive use.
jsonxx::Object menu_logic_tabbed_mode_add_tab (std::string url, std::string label)
{
  jsonxx::Object object;
  object << "url" << url;
  object << "label" << label;
  return object;
}


// This looks at the settings, and then generates JSON, and stores that in the general configuration.
void menu_logic_tabbed_mode_save_json (Webserver_Request& webserver_request)
{
  std::string json;

  // Whether the device can do tabbed mode.
  if (menu_logic_can_do_tabbed_mode ()) {
    
    // If the setting is on, generate the JSON.
    bool generate_json = database::config::general::get_menu_in_tabbed_view_on ();
    
    // Tabbed view not possible in advanced mode.
    if (!webserver_request.database_config_user ()->get_basic_interface_mode ()) {
      generate_json = false;
    }
    
    if (generate_json) {
      // Storage for the tabbed view.
      jsonxx::Array json_array;
      // Adding tabs in the order an average translator uses them most of the time:
      // Add the Bible editor tab.
      json_array << menu_logic_tabbed_mode_add_tab (editone_index_url (), menu_logic_translate_text ());
      // Add the resources tab.
      json_array << menu_logic_tabbed_mode_add_tab (resource_index_url (), menu_logic_resources_text ());
      // Add the consultation notes tab.
      json_array << menu_logic_tabbed_mode_add_tab (notes_index_url (), menu_logic_consultation_notes_text ());
      // Add the change notifications, if enabled.
      if (webserver_request.database_config_user ()->get_menu_changes_in_basic_mode ()) {
        json_array << menu_logic_tabbed_mode_add_tab (changes_changes_url (), menu_logic_changes_text ());
      }
      // Add the preferences tab.
      json_array << menu_logic_tabbed_mode_add_tab (personalize_index_url (), menu_logic_settings_text ());
      // JSON representation of the URLs.
      json = json_array.json ();
    }
  }

  database::config::general::set_menu_in_tabbed_view_json (json);
}


std::string menu_logic_verse_separator (std::string separator)
{
  if (separator == ".") {
    return translate ("dot") + " ( . )";
  }
  if (separator == ":") {
    return translate ("colon") + " ( : )";
  }
  return " ( " + separator + " ) ";
}

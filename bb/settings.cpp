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


#include <bb/settings.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/mappings.h>
#include <database/check.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <dialog/list.h>
#include <dialog/books.h>
#include <access/bible.h>
#include <book/create.h>
#include <bb/logic.h>
#include <client/logic.h>
#include <bb/manage.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <resource/external.h>
#include <sword/logic.h>
#include <tasks/logic.h>
#include <system/index.h>
#include <rss/logic.h>
#include <access/logic.h>
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


std::string bible_settings_url ()
{
  return "bible/settings";
}


bool bible_settings_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string bible_settings (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Bible"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  Assets_View view {};

  
  std::string success_message {};
  std::string error_message {};

  
  // The Bible.
  std::string bible = webserver_request.query["bible"];
  if (bible.empty()) bible = webserver_request.post ["val1"];
  bible = access_bible::clamp (webserver_request, bible);
  view.set_variable ("bible", filter::strings::escape_special_xml_characters (bible));
  view.set_variable ("urlbible", filter_url_urlencode(filter::strings::escape_special_xml_characters (bible)));

  
  // Whether the user has write access to this Bible.
  bool write_access = access_bible::write (webserver_request, bible);
  if (write_access) view.enable_zone ("write_access");
  
  
  // Whether the user has the privilege to change the stylesheet.
  const std::string& current_user = webserver_request.session_logic ()->get_username ();
  bool privilege_stylesheet = access_logic::privilege_set_stylesheets (webserver_request, current_user);
  if (privilege_stylesheet) view.enable_zone ("privilege_stylesheet");

  
  // The state of the checkbox.
  const std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);

  
  // Versification.
  if (webserver_request.query.count ("versification")) {
    const std::string versification = webserver_request.query["versification"];
    if (versification.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the versification system?"), translate ("A versification system determines how many chapters are in each book, and how many verses are in each chapter. Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Versifications database_versifications;
      const std::vector <std::string> versification_names = database_versifications.getSystems ();
      for (const auto& versification_name : versification_names) {
        dialog_list.add_row (versification_name, "versification", versification_name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) database::config::bible::set_versification_system (bible, versification);
    }
  }
  const std::string versification = database::config::bible::get_versification_system (bible);
  view.set_variable ("versification", versification);

  
  // Book creation.
  if (webserver_request.query.count ("createbook")) {
    const std::string createbook = webserver_request.query["createbook"];
    if (createbook.empty()) {
      Dialog_Books dialog_books = Dialog_Books ("settings", translate("Create book"), "", "", "createbook", {}, database::bibles::get_books (bible));
      dialog_books.add_query ("bible", bible);
      page += dialog_books.run ();
      return page;
    } else {
      std::vector <std::string> feedback{};
      if (write_access)
        book_create (bible, static_cast<book_id>(filter::strings::convert_to_int (createbook)), -1, feedback);
    }
    // User creates a book in this Bible: Set it as the default Bible.
    webserver_request.database_config_user()->setBible (bible);
  }
  
  
  // Book deletion.
  const std::string deletebook = webserver_request.query["deletebook"];
  if (!deletebook.empty()) {
    const std::string confirm = webserver_request.query["confirm"];
    if (confirm == "yes") {
      if (write_access) bible_logic::delete_book (bible, filter::strings::convert_to_int (deletebook));
    } 
    else if (confirm == "cancel") {
    } 
    else {
      Dialog_Yes dialog_yes = Dialog_Yes ("settings", translate("Would you like to delete this book?"));
      dialog_yes.add_query ("bible", bible);
      dialog_yes.add_query ("deletebook", deletebook);
      page += dialog_yes.run ();
      return page;
    }
  }
  
  
  // Importing text from a resource.
  if (webserver_request.query.count ("resource")) {
    Dialog_List dialog_list = Dialog_List ("settings", translate("Select a resource to import into the Bible"), translate ("The resource will be imported.") + " " + translate ("It will overwrite the content of the Bible."), "", true);
    dialog_list.add_query ("bible", bible);
    std::vector <std::string> resources = resource_external_names ();
    for (const auto& resource : resources) {
      dialog_list.add_row (resource, "resource", resource);
    }
    resources = sword_logic_get_available ();
    for (const auto& resource : resources) {
      const std::string source = sword_logic_get_source (resource);
      const std::string module = sword_logic_get_remote_module (resource);
      const std::string name = sword_logic_get_resource_name (source, module);
      dialog_list.add_row (resource, "resource", name);
    }
    page += dialog_list.run ();
    return page;
  }
  // The resource should be POSTed.
  // This is for the demo, where a GET request would allow search crawlers to regularly import resources.
  const std::string resource = webserver_request.post["add"];
  if (!resource.empty ()) {
    if (write_access) {
      tasks_logic_queue (task::import_resource, { bible, resource });
      success_message = translate ("The resource will be imported into the Bible.") + " " + translate ("The journal shows the progress.");
    }
  }

  
  const int level = webserver_request.session_logic ()->get_level ();
  const bool manager_level = (level >= Filter_Roles::manager ());
  if (manager_level) view.enable_zone ("manager");

  
  // Available books.
  pugi::xml_document book_document {};
  const std::vector <int> book_ids = filter_passage_get_ordered_books (bible);
  for (const auto book: book_ids) {
    std::string book_name = database::books::get_english_from_id (static_cast<book_id>(book));
    book_name = translate(book_name);
    pugi::xml_node a_or_span_node;
    if (manager_level) {
      a_or_span_node = book_document.append_child("a");
      std::string href = filter_url_build_http_query ("book", "bible", bible);
      href = filter_url_build_http_query (href, "book", std::to_string (book));
      a_or_span_node.append_attribute("href") = href.c_str();
    } else {
      a_or_span_node = book_document.append_child("span");
    }
    a_or_span_node.text().set(book_name.c_str());
    pugi::xml_node space_node = book_document.append_child("span");
    space_node.text().set(" ");
  }
  std::stringstream bookblock2 {};
  book_document.print (bookblock2, "", pugi::format_raw);
  view.set_variable ("bookblock", bookblock2.str());
  view.set_variable ("book_count", std::to_string (static_cast<int>(book_ids.size())));


  // Public feedback.
  if (checkbox == "public") {
    if (write_access) database::config::bible::set_public_feedback_enabled (bible, checked);
  }
  view.set_variable ("public", filter::strings::get_checkbox_status (database::config::bible::get_public_feedback_enabled (bible)));

  
 
  // RSS feed.
#ifdef HAVE_CLOUD
  if (checkbox == "rss") {
    if (write_access) {
      database::config::bible::set_send_changes_to_rss (bible, checked);
      rss_logic_feed_on_off ();
    }
  }
  view.set_variable ("rss", filter::strings::get_checkbox_status (database::config::bible::get_send_changes_to_rss (bible)));
#endif

  
  // Stylesheet for editing.
  if (webserver_request.query.count ("stylesheetediting")) {
    const std::string stylesheet = webserver_request.query["stylesheetediting"];
    if (stylesheet.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the stylesheet for editing?"), translate ("The stylesheet affects how the Bible text in the editor looks.") + " " + translate ("Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Styles database_styles = Database_Styles();
      const std::vector <std::string> sheets = database_styles.getSheets();
      for (const auto& name : sheets) {
        dialog_list.add_row (name, "stylesheetediting", name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) database::config::bible::set_editor_stylesheet (bible, stylesheet);
    }
  }
  std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  view.set_variable ("stylesheetediting", stylesheet);

  
  // Stylesheet for export.
  if (webserver_request.query.count ("stylesheetexport")) {
    const std::string export_stylesheet = webserver_request.query["stylesheetexport"];
    if (export_stylesheet.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the stylesheet for export?"), translate ("The stylesheet affects how the Bible text looks when exported.") + " " + translate ("Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Styles database_styles = Database_Styles();
      const std::vector <std::string> sheets = database_styles.getSheets();
      for (const auto& name : sheets) {
        dialog_list.add_row (name, "stylesheetexport", name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) database::config::bible::set_export_stylesheet (bible, export_stylesheet);
    }
  }
  stylesheet = database::config::bible::get_export_stylesheet (bible);
  view.set_variable ("stylesheetexport", stylesheet);
  
  
  // Automatic daily checks on text.
#ifdef HAVE_CLOUD
  if (checkbox == "checks") {
    if (write_access) {
      database::config::bible::set_daily_checks_enabled (bible, checked);
      if (!checked) {
        // If checking is switched off, also remove any existing checking results for this Bible.
        database::check::truncate_output(bible);
      }
    }
  }
  view.set_variable ("checks", filter::strings::get_checkbox_status (database::config::bible::get_daily_checks_enabled (bible)));
#endif

  
  view.set_variable ("systemindex", system_index_url());
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);

  
  if (client_logic_client_enabled ()) {
    view.enable_zone ("client");
    view.set_variable ("cloudlink", client_logic_link_to_cloud (bible_manage_url (), translate ("More operations in the Cloud")));
  } else {
    view.enable_zone ("server");
  }

  
  page += view.render ("bb", "settings");
  page += assets_page::footer ();
  return page;
}

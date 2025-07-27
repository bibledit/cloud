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
#include <dialog/select.h>
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
  return roles::access_control (webserver_request, roles::translator);
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

  
  // Versification
  {
    constexpr const char* versification {"versification"};
    Database_Versifications database_versifications;
    const std::vector <std::string> systems = database_versifications.getSystems ();
    if (webserver_request.post.count (versification)) {
      const std::string system {webserver_request.post.at(versification)};
      if (write_access)
        database::config::bible::set_versification_system (bible, system);
    }
    dialog::select::Settings settings {
      .identification = versification,
      .values = systems,
      .selected = database::config::bible::get_versification_system (bible),
      .parameters = { {"bible", bible} },
      .disabled = !write_access,
    };
    view.set_variable(versification, dialog::select::ajax(settings));
  }


  
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
    webserver_request.database_config_user()->set_bible (bible);
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
  {
    constexpr const char* selector {"resource"};
    if (webserver_request.post.count (selector)) {
      const std::string resource {webserver_request.post.at(selector)};
      if (!resource.empty ()) {
        const auto bibles = database::bibles::get_books (bible);
        if (bibles.empty()) {
          if (write_access) {
            tasks_logic_queue (task::import_resource, { bible, resource });
            success_message = translate ("The resource will be imported into the Bible.") + " " + translate ("The journal shows the progress.");
          }
        } else {
          error_message = translate ("Cannot import because the Bible still has books.");
        }
      }
    }
    dialog::select::Settings settings {
      .identification = selector,
      .values = {""},
      .displayed = {""},
      .selected = std::string(),
      .parameters = { { "bible", bible } },
      .disabled = !write_access,
    };
    for (const auto& resource : resource_external_names()) {
      settings.values.push_back (resource);
      settings.displayed.push_back (resource);
    }
    for (const auto& resource : sword_logic_get_available ()) {
      const std::string source = sword_logic_get_source (resource);
      const std::string module = sword_logic_get_remote_module (resource);
      const std::string name = sword_logic_get_resource_name (source, module);
      settings.values.push_back (resource);
      settings.displayed.push_back (name);
    }
    dialog::select::Form form { .auto_submit = false };
    view.set_variable(selector, dialog::select::form(settings, form));
  }

  
  const int level = webserver_request.session_logic ()->get_level ();
  const bool manager_level = (level >= roles::manager);
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
  {
    constexpr const char* identification {"stylesheetediting"};
    if (webserver_request.post.count (identification)) {
      const std::string value {webserver_request.post.at(identification)};
      database::config::bible::set_editor_stylesheet (bible, value);
      return std::string();
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = database::styles::get_sheets(),
      .selected = database::config::bible::get_editor_stylesheet (bible),
      .parameters = { {"bible", bible} },
      .disabled = !(write_access and access_logic::privilege_set_stylesheets (webserver_request, current_user)),
    };
    view.set_variable(identification, dialog::select::ajax(settings));
  }
  
  
  // Stylesheet for export.
  {
    constexpr const char* identification {"stylesheetexport"};
    if (webserver_request.post.count (identification)) {
      const std::string value {webserver_request.post.at(identification)};
      database::config::bible::set_export_stylesheet (bible, value);
      return std::string();
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = database::styles::get_sheets(),
      .selected = database::config::bible::get_export_stylesheet (bible),
      .parameters = { {"bible", bible} },
      .disabled = !(write_access and access_logic::privilege_set_stylesheets (webserver_request, current_user)),
    };
    view.set_variable(identification, dialog::select::ajax(settings));
  }
  
  
  // Automatic daily checks on text.
#ifdef HAVE_CLOUD
  if (checkbox == "checks") {
    if (write_access) {
      database::config::bible::set_daily_checks_enabled (bible, checked);
      if (!checked) {
        // If checking is switched off, also remove any existing checking results for this Bible.
        database::check::delete_output(bible);
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

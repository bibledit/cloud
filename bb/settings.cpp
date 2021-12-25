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
#include <pugixml/pugixml.hpp>


using namespace pugi;


string bible_settings_url ()
{
  return "bible/settings";
}


bool bible_settings_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string bible_settings (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page;
  Assets_Header header = Assets_Header (translate("Bible"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  Assets_View view;

  
  string success_message;
  string error_message;

  
  // The Bible.
  string bible = request->query["bible"];
  if (bible.empty()) bible = request->post ["val1"];
  bible = access_bible_clamp (request, bible);
  view.set_variable ("bible", escape_special_xml_characters (bible));

  
  // Whether the user has write access to this Bible.
  bool write_access = access_bible_write (request, bible);
  if (write_access) view.enable_zone ("write_access");

  
  // The state of the checkbox.
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);

  
  // Versification.
  if (request->query.count ("versification")) {
    string versification = request->query["versification"];
    if (versification == "") {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the versification system?"), translate ("A versification system determines how many chapters are in each book, and how many verses are in each chapter. Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Versifications database_versifications;
      vector <string> versification_names = database_versifications.getSystems ();
      for (auto & versification_name : versification_names) {
        dialog_list.add_row (versification_name, "versification", versification_name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) Database_Config_Bible::setVersificationSystem (bible, versification);
    }
  }
  string versification = Database_Config_Bible::getVersificationSystem (bible);
  view.set_variable ("versification", versification);

  
  // Book creation.
  if (request->query.count ("createbook")) {
    string createbook = request->query["createbook"];
    if (createbook == "") {
      Dialog_Books dialog_books = Dialog_Books ("settings", translate("Create book"), "", "", "createbook", {}, request->database_bibles ()->getBooks (bible));
      dialog_books.add_query ("bible", bible);
      page += dialog_books.run ();
      return page;
    } else {
      vector <string> feedback;
      if (write_access) book_create (bible, convert_to_int (createbook), -1, feedback);
    }
    // User creates a book in this Bible: Set it as the default Bible.
    request->database_config_user()->setBible (bible);
  }
  
  
  // Book deletion.
  string deletebook = request->query["deletebook"];
  if (deletebook != "") {
    string confirm = request->query["confirm"];
    if (confirm == "yes") {
      if (write_access) bible_logic_delete_book (bible, convert_to_int (deletebook));
    } else if (confirm == "cancel") {
    } else {
      Dialog_Yes dialog_yes = Dialog_Yes ("settings", translate("Would you like to delete this book?"));
      dialog_yes.add_query ("bible", bible);
      dialog_yes.add_query ("deletebook", deletebook);
      page += dialog_yes.run ();
      return page;
    }
  }
  
  
  // Importing text from a resource.
  if (request->query.count ("resource")) {
    Dialog_List dialog_list = Dialog_List ("settings", translate("Select a resource to import into the Bible"), translate ("The resource will be imported.") + " " + translate ("It will overwrite the content of the Bible."), "", true);
    dialog_list.add_query ("bible", bible);
    vector <string> resources = resource_external_names ();
    for (auto & resource : resources) {
      dialog_list.add_row (resource, "resource", resource);
    }
    resources = sword_logic_get_available ();
    for (auto & resource : resources) {
      string source = sword_logic_get_source (resource);
      string module = sword_logic_get_remote_module (resource);
      string name = sword_logic_get_resource_name (source, module);
      dialog_list.add_row (resource, "resource", name);
    }
    page += dialog_list.run ();
    return page;
  }
  // The resource should be POSTed.
  // This is for the demo, where a GET request would allow search crawlers to regularly import resources.
  string resource = request->post["add"];
  if (!resource.empty ()) {
    if (write_access) {
      tasks_logic_queue (IMPORTRESOURCE, { bible, resource });
      success_message = translate ("The resource will be imported into the Bible.") + " " + translate ("The journal shows the progress.");
    }
  }

  
  int level = request->session_logic ()->currentLevel ();
  bool manager_level = (level >= Filter_Roles::manager ());
  if (manager_level) view.enable_zone ("manager");

  
  // Available books.
  xml_document book_document;
  vector <int> book_ids = filter_passage_get_ordered_books (bible);
  for (auto & book: book_ids) {
    string book_name = Database_Books::getEnglishFromId (book);
    book_name = translate(book_name);
    xml_node a_or_span_node;
    if (manager_level) {
      a_or_span_node = book_document.append_child("a");
      string href = filter_url_build_http_query ("book", "bible", bible);
      href = filter_url_build_http_query (href, "book", convert_to_string (book));
      a_or_span_node.append_attribute("href") = href.c_str();
    } else {
      a_or_span_node = book_document.append_child("span");
    }
    a_or_span_node.text().set(book_name.c_str());
    xml_node space_node = book_document.append_child("span");
    space_node.text().set(" ");
  }
  stringstream bookblock2;
  book_document.print (bookblock2, "", format_raw);
  view.set_variable ("bookblock", bookblock2.str());
  view.set_variable ("book_count", convert_to_string ((int)book_ids.size()));


  // Public feedback.
  if (checkbox == "public") {
    if (write_access) Database_Config_Bible::setPublicFeedbackEnabled (bible, checked);
  }
  view.set_variable ("public", get_checkbox_status (Database_Config_Bible::getPublicFeedbackEnabled (bible)));

  
 
  // RSS feed.
#ifdef HAVE_CLOUD
  if (checkbox == "rss") {
    if (write_access) {
      Database_Config_Bible::setSendChangesToRSS (bible, checked);
      rss_logic_feed_on_off ();
    }
  }
  view.set_variable ("rss", get_checkbox_status (Database_Config_Bible::getSendChangesToRSS (bible)));
#endif

  
  // Stylesheet for editing.
  if (request->query.count ("stylesheetediting")) {
    string stylesheet = request->query["stylesheetediting"];
    if (stylesheet.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the stylesheet for editing?"), translate ("The stylesheet affects how the Bible text in the editor looks.") + " " + translate ("Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Styles database_styles = Database_Styles();
      vector <string> sheets = database_styles.getSheets();
      for (auto & name : sheets) {
        dialog_list.add_row (name, "stylesheetediting", name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) Database_Config_Bible::setEditorStylesheet (bible, stylesheet);
    }
  }
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  view.set_variable ("stylesheetediting", stylesheet);

  
  // Stylesheet for export.
  if (request->query.count ("stylesheetexport")) {
    string stylesheet = request->query["stylesheetexport"];
    if (stylesheet.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Would you like to change the stylesheet for export?"), translate ("The stylesheet affects how the Bible text looks when exported.") + " " + translate ("Please make your choice below."), "");
      dialog_list.add_query ("bible", bible);
      Database_Styles database_styles = Database_Styles();
      vector <string> sheets = database_styles.getSheets();
      for (auto & name : sheets) {
        dialog_list.add_row (name, "stylesheetexport", name);
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write_access) Database_Config_Bible::setExportStylesheet (bible, stylesheet);
    }
  }
  stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  view.set_variable ("stylesheetexport", stylesheet);
  
  
  // Automatic daily checks on text.
#ifdef HAVE_CLOUD
  if (checkbox == "checks") {
    if (write_access) {
      Database_Config_Bible::setDailyChecksEnabled (bible, checked);
      if (!checked) {
        // If checking is switched off, also remove any existing checking results for this Bible.
        Database_Check database_check;
        database_check.truncateOutput(bible);
      }
    }
  }
  view.set_variable ("checks", get_checkbox_status (Database_Config_Bible::getDailyChecksEnabled (bible)));
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
  page += Assets_Page::footer ();
  return page;
}

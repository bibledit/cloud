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


#include <bb/manage.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/git.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <database/privileges.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <client/logic.h>
#include <menu/logic.h>
#include <demo/logic.h>
#include <search/logic.h>
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


std::string bible_manage_url ()
{
  return "bible/manage";
}


bool bible_manage_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string bible_manage (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Bibles"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view {};
  
  std::string success_message {};
  std::string error_message {};
  
  // New Bible handler.
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("manage", translate("Please enter a name for the new empty Bible"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("new")) {
    std::string bible = webserver_request.post ["entry"];
    // No underscrore ( _ ) in the name of a Bible because the underscores are used in the searches to separate data.
    bible = filter::strings::replace ("_", "", bible);
    const std::vector <std::string> bibles = database::bibles::get_bibles ();
    if (find (bibles.begin(), bibles.end(), bible) != bibles.end()) {
      error_message = translate("This Bible already exists");
    } else {
      database::bibles::create_bible (bible);
      // Check / grant access.
      if (!access_bible::write (webserver_request, bible)) {
        const std::string& me = webserver_request.session_logic ()->get_username ();
        DatabasePrivileges::set_bible (me, bible, true);
      }
      success_message = translate("The Bible was created");
      // Creating a Bible removes any Sample Bible that might have been there.
      if (!config::logic::demo_enabled ()) {
        database::bibles::delete_bible (demo_sample_bible_name ());
        search_logic_delete_bible (demo_sample_bible_name ());
      }
    }
  }
  
  // Copy Bible handler.
  if (webserver_request.query.count ("copy")) {
    const std::string copy = webserver_request.query["copy"];
    Dialog_Entry dialog_entry = Dialog_Entry ("manage", translate("Please enter a name for where to copy the Bible to"), "", "", "A new Bible will be created with the given name, and the current Bible copied to it");
    dialog_entry.add_query ("origin", copy);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.query.count ("origin")) {
    const std::string origin = webserver_request.query["origin"];
    if (webserver_request.post.count ("entry")) {
      std::string destination = webserver_request.post["entry"];
      destination = filter::strings::replace ("_", "", destination); // No underscores in the name.
      const std::vector <std::string> bibles = database::bibles::get_bibles ();
      if (find (bibles.begin(), bibles.end(), destination) != bibles.end()) {
        error_message = translate("Cannot copy the Bible because the destination Bible already exists.");
      } else {
        // User needs read access to the original.
        if (access_bible::read (webserver_request, origin)) {
          // Copy the Bible data.
          const std::string origin_folder = database::bibles::bible_folder (origin);
          const std::string destination_folder = database::bibles::bible_folder (destination);
          filter_url_dir_cp (origin_folder, destination_folder);
          // Copy the Bible search index.
          search_logic_copy_bible (origin, destination);
          // Feedback.
          success_message = translate("The Bible was copied.");
          // Check / grant access to destination Bible.
          if (!access_bible::write (webserver_request, destination)) {
            const std::string& me = webserver_request.session_logic ()->get_username ();
            DatabasePrivileges::set_bible (me, destination, true);
          }
          // Creating a Bible removes any Sample Bible that might have been there.
          if (!config::logic::demo_enabled ()) {
            database::bibles::delete_bible (demo_sample_bible_name ());
            search_logic_delete_bible (demo_sample_bible_name ());
          }
        }
      }
    }
  }

  // Delete Bible handler.
  if (webserver_request.query.count ("delete")) {
    const std::string bible = webserver_request.query ["delete"];
    const std::string confirm = webserver_request.query ["confirm"];
    if (confirm == "yes") {
      // User needs write access for delete operation.
      if (access_bible::write (webserver_request, bible)) {
        bible_logic::delete_bible (bible);
      } else {
        page += assets_page::error ("Insufficient privileges to complete action");
      }
    }
    if (confirm.empty()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("manage", translate("Would you like to delete this Bible?") + " (" + bible + ")");
      dialog_yes.add_query ("delete", bible);
      page += dialog_yes.run ();
      return page;
    }
  }

  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  const std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  pugi::xml_document document{};
  for (const auto& bible : bibles) {
    pugi::xml_node li_node = document.append_child ("li");
    pugi::xml_node a_node = li_node.append_child("a");
    const std::string href = filter_url_build_http_query ("settings", "bible", filter_url_urlencode(bible));
    a_node.append_attribute("href") = href.c_str();
    a_node.text().set(bible.c_str());
  }
  std::stringstream bibleblock {};
  document.print(bibleblock, "", pugi::format_raw);
  view.set_variable ("bibleblock", bibleblock.str());

  if (!client_logic_client_enabled ()) 
    view.enable_zone ("server");

  page += view.render ("bb", "manage");
  
  page += assets_page::footer ();
  
  return page;
}

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


#include <sendreceive/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <access/bible.h>
#include <dialog/list.h>
#include <sendreceive/logic.h>
#include <demo/logic.h>
#include <client/logic.h>
#include <paratext/logic.h>
#include <menu/logic.h>
#include <config/globals.h>
#include <tasks/enums.h>


std::string sendreceive_index_url ()
{
  return "sendreceive/index";
}


bool sendreceive_index_acl (Webserver_Request& webserver_request)
{
  // In Client mode, also a Consultant can send/receive.
  if (client_logic_client_enabled ()) {
    return true;
  }
  // The role of Translator or higher enables send/receive.
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) {
    return true;
  }
  // No access.
  return false;
}


std::string sendreceive_index (Webserver_Request& webserver_request)
{
  if (webserver_request.query.count ("status")) {
    std::vector <std::string> bits;
    if (config_globals_syncing_bibles)    bits.push_back (translate ("Bibles"));
    if (config_globals_syncing_notes)     bits.push_back (translate ("Notes"));
    if (config_globals_syncing_settings)  bits.push_back (translate ("Settings"));
    if (config_globals_syncing_changes)   bits.push_back (translate ("Changes"));
    if (config_globals_syncing_files)     bits.push_back (translate ("Files"));
    if (config_globals_syncing_resources) bits.push_back (translate ("Resources"));
    if (bits.empty ()) return std::string();
    std::string status = translate ("Sending and receiving:") + " " + filter::strings::implode (bits, ", ") + " ...";
    return status;
  }
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Send/Receive"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  std::string bible;
  if (webserver_request.query.count ("bible")) {
    bible = webserver_request.query["bible"];
    if (bible.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select a Bible"), "", "");
      std::vector <std::string> bibles = access_bible::bibles (webserver_request);
      for (auto & selectable_bible : bibles) {
        // Select Bibles the user has write access to.
        if (access_bible::write (webserver_request, selectable_bible)) {
          dialog_list.add_row (selectable_bible, "bible", selectable_bible);
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (bible);
    }
  }
  
  
  bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  view.set_variable ("bible", bible);


  std::string starting_to_sync;
  if (database::config::bible::get_read_from_git (bible)) {
    starting_to_sync = translate ("Starting to send and receive now.");
  } else {
    starting_to_sync = translate ("Starting to send now.");
  }
  
  
  if (webserver_request.query.count ("runbible")) {
    sendreceive_queue_bible (bible);
    view.set_variable ("successbible", starting_to_sync);
  }
  
  
  std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);
  if (checkbox == "repeatbible") {
    database::config::bible::set_repeat_send_receive (bible, checked);
    return std::string();
  }
  view.set_variable ("repeatbible", filter::strings::get_checkbox_status (database::config::bible::get_repeat_send_receive (bible)));
  
    
  if (sendreceive_git_repository_linked (bible)) {
    view.enable_zone ("collab_on");
    std::string url = database::config::bible::get_remote_repository_url (bible);
    view.set_variable ("url", filter_url_remove_username_password (url));
  } else {
    view.enable_zone ("collab_off");
  }
  
  
  if (webserver_request.query.count ("runsync")) {
    if (sendreceive_sync_queued ()) {
      view.set_variable ("error", translate("Still sending and receiving from the last time."));
    }
    sendreceive_queue_sync (-1, 0);
    view.set_variable ("success", starting_to_sync);
  }
  

  {
    auto sync_method = tasks::enums::paratext_sync::none;
    if (webserver_request.query.count ("syncparatext")) {
      sync_method = tasks::enums::paratext_sync::bi_directional;
    }
    if (webserver_request.query.count ("bibledit2paratext")) {
      sync_method = tasks::enums::paratext_sync::bibledit_to_paratext;
    }
    if (webserver_request.query.count ("paratext2bibledit")) {
      sync_method = tasks::enums::paratext_sync::paratext_to_bibledit;
    }
    if (sync_method != tasks::enums::paratext_sync::none) {
      if (sendreceive_paratext_queued ()) {
        view.set_variable ("error", translate("Still synchronizing with Paratext."));
      } else {
        sendreceive_queue_paratext (sync_method);
        view.set_variable ("success", starting_to_sync);
      }
    }
   }
  
  
#ifdef HAVE_CLIENT
  if (client_logic_client_enabled ()) {
    view.enable_zone ("clienton");
  } else {
    view.enable_zone ("clientoff");
  }
#else
  view.enable_zone ("server");
#endif

  
  if (webserver_request.query.count ("repeatsync")) {
    int repeatsync = filter::strings::convert_to_int (webserver_request.query["repeatsync"]);
    // Clamp the values.
    if (repeatsync < 0) repeatsync = 0;
    if (repeatsync > 2) repeatsync = 2;
    database::config::general::set_repeat_send_receive (repeatsync);
  }
  int repeatsync = database::config::general::get_repeat_send_receive ();
  // After removing value 3, if the setting had "3", make it "2".
  if (repeatsync > 2) repeatsync = 2;
  std::string repeatsynczone = "repeatsync" + std::to_string (repeatsync);
  view.enable_zone (repeatsynczone);
  
  
  view.set_variable ("demo", demo_client_warning ());

  
#ifdef HAVE_PARATEXT
  std::vector <std::string> bibles = Paratext_Logic::enabledBibles ();
  if (!bibles.empty ()) {
    view.enable_zone ("paratexton");
    view.set_variable ("paratextbibles", filter::strings::implode (bibles, ", "));
  } else {
    view.enable_zone ("paratextoff");
  }
#endif

  
  if (database::config::bible::get_read_from_git (bible)) {
    view.enable_zone ("gitreadwrite");
  } else {
    view.enable_zone ("gitwrite");
  }

  
  bool basic_mode = config::logic::basic_mode (webserver_request);
  if (basic_mode) view.enable_zone("basicmode");
  
  page += view.render ("sendreceive", "index");
  page += assets_page::footer ();
  return page;
}

/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


string sendreceive_index_url ()
{
  return "sendreceive/index";
}


bool sendreceive_index_acl (void * webserver_request)
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


string sendreceive_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  if (request->query.count ("status")) {
    vector <string> bits;
    if (config_globals_syncing_bibles)    bits.push_back (translate ("Bibles"));
    if (config_globals_syncing_notes)     bits.push_back (translate ("Notes"));
    if (config_globals_syncing_settings)  bits.push_back (translate ("Settings"));
    if (config_globals_syncing_changes)   bits.push_back (translate ("Changes"));
    if (config_globals_syncing_files)     bits.push_back (translate ("Files"));
    if (config_globals_syncing_resources) bits.push_back (translate ("Resources"));
    if (bits.empty ()) return "";
    string status = translate ("Sending and receiving:") + " " + filter_string_implode (bits, ", ") + " ...";
    return status;
  }
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Send/Receive"), request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  string bible;
  if (request->query.count ("bible")) {
    bible = request->query["bible"];
    if (bible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select a Bible"), "", "");
      vector <string> bibles = access_bible_bibles (request);
      for (auto & bible : bibles) {
        // Select Bibles the user has write access to.
        if (access_bible_write (request, bible)) {
          dialog_list.add_row (bible, "bible", bible);
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (bible);
    }
  }
  
  
  bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);


  string starting_to_sync;
  if (Database_Config_Bible::getReadFromGit (bible)) {
    starting_to_sync = translate ("Starting to send and receive now.");
  } else {
    starting_to_sync = translate ("Starting to send now.");
  }
  
  
  if (request->query.count ("runbible")) {
    sendreceive_queue_bible (bible);
    view.set_variable ("successbible", starting_to_sync);
  }
  
  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);
  if (checkbox == "repeatbible") {
    Database_Config_Bible::setRepeatSendReceive (bible, checked);
    return "";
  }
  view.set_variable ("repeatbible", get_checkbox_status (Database_Config_Bible::getRepeatSendReceive (bible)));
  
    
  if (sendreceive_git_repository_linked (bible)) {
    view.enable_zone ("collab_on");
    string url = Database_Config_Bible::getRemoteRepositoryUrl (bible);
    view.set_variable ("url", filter_url_remove_username_password (url));
  } else {
    view.enable_zone ("collab_off");
  }
  
  
  if (request->query.count ("runsync")) {
    if (sendreceive_sync_queued ()) {
      view.set_variable ("error", translate("Still sending and receiving from the last time."));
    }
    sendreceive_queue_sync (-1, 0);
    view.set_variable ("success", starting_to_sync);
  }
  
  
  if (request->query.count ("syncparatext")) {
    if (sendreceive_paratext_queued ()) {
      view.set_variable ("error", translate("Still synchronizing with Paratext."));
    }
    sendreceive_queue_paratext ();
    view.set_variable ("success", starting_to_sync);
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

  
  if (request->query.count ("repeatsync")) {
    int repeatsync = convert_to_int (request->query["repeatsync"]);
    if (repeatsync < 0) repeatsync = 0;
    if (repeatsync > 3) repeatsync = 3;
    Database_Config_General::setRepeatSendReceive (repeatsync);
  }
  int repeatsync = Database_Config_General::getRepeatSendReceive ();
  string repeatsynczone = "repeatsync" + convert_to_string (repeatsync);
  view.enable_zone (repeatsynczone);
  
  
  view.set_variable ("demo", demo_client_warning ());

  
#ifdef HAVE_PARATEXT
  vector <string> bibles = Paratext_Logic::enabledBibles ();
  if (!bibles.empty ()) {
    view.enable_zone ("paratexton");
    view.set_variable ("paratextbibles", filter_string_implode (bibles, ", "));
  } else {
    view.enable_zone ("paratextoff");
  }
#endif

  
  if (Database_Config_Bible::getReadFromGit (bible)) {
    view.enable_zone ("gitreadwrite");
  } else {
    view.enable_zone ("gitwrite");
  }
  
  page += view.render ("sendreceive", "index");
  page += Assets_Page::footer ();
  return page;
}

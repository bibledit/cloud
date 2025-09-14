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


#include <statistics/statistics.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <database/notes.h>
#include <database/logs.h>
#include <database/modifications.h>
#include <database/config/general.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <notes/index.h>
#include <client/logic.h>
#include <changes/changes.h>
#include <email/send.h>


void statistics_statistics ()
{
  Webserver_Request webserver_request;
  Database_Notes database_notes (webserver_request);
  
  
  Database_Logs::log (translate("Sending statistics"), roles::manager);

  
  std::string siteUrl = config::logic::site_url (webserver_request);
  
  
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  
  
  std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  for (auto & user : users) {
    
    
    std::string subject = "Bibledit " + translate("statistics");
    std::stringstream body;
    
  
    size_t change_notificatons_count = 0;
    if (webserver_request.database_config_user()->get_user_pending_changes_notification (user)) {
      std::string any_bible = std::string();
      std::vector <int> ids = database::modifications::getNotificationIdentifiers (user, any_bible);
      change_notificatons_count = ids.size();
      body << "<p><a href=" << std::quoted (siteUrl + changes_changes_url ()) << ">" << translate("Number of change notifications") << "</a>: " << ids.size() << "</p>" << std::endl;
    }
    

    size_t assigned_notes_count = 0;
    if (webserver_request.database_config_user()->get_user_assigned_notes_statistics_notification (user)) {
      Database_Notes::Selector selector {
        .bibles = bibles,
        .assignment_selector = user,
      };
      const std::vector<int> ids = database_notes.select_notes(selector);

      assigned_notes_count = ids.size();
      body << "<p><a href=" << std::quoted (siteUrl + notes_index_url () + "?presetselection=assigned") << ">" << translate("Number of consultation notes assigned to you awaiting your response") << "</a>: " << ids.size() << "</p>" << std::endl;
    }
    

    size_t subscribed_notes_count = 0;
    if (webserver_request.database_config_user()->get_user_subscribed_notes_statistics_notification (user)) {
      body << "<p>" << translate("Number of consultation notes you are subscribed to") << ":</p>" << std::endl;
      body << "<ul>" << std::endl;
      webserver_request.session_logic ()->set_username (user);
      
      std::vector<int> ids;
      {
        Database_Notes::Selector selector {
          .bibles = bibles,
          .subscription_selector = true,
        };
        ids = database_notes.select_notes(selector);
      }
      subscribed_notes_count = ids.size();
      body << "<li><a href=" << std::quoted (siteUrl + notes_index_url () + "?presetselection=subscribed") << ">" << translate("Total") << "</a>: " << ids.size () << "</li>" << std::endl;
      {
        Database_Notes::Selector selector {
          .bibles = bibles,
          .non_edit_selector = Database_Notes::NonEditSelector::a_day,
          .subscription_selector = true,
        };
        ids = database_notes.select_notes(selector);
      }
      body << "<li><a href=" << std::quoted (siteUrl + notes_index_url () + "?presetselection=subscribeddayidle") << ">" << translate("Inactive for a day") << "</a>: " << ids.size() << "</li>" << std::endl;
      {
        Database_Notes::Selector selector {
          .bibles = bibles,
          .non_edit_selector = Database_Notes::NonEditSelector::a_week,
          .subscription_selector = true,
        };
        ids = database_notes.select_notes(selector);
      }
      body << "<li><a href=" << std::quoted (siteUrl + notes_index_url () + "?presetselection=subscribedweekidle") << ">" << translate("Inactive for a week") << "</a>: " << ids.size() << "</li>" << std::endl;
      body << "</ul>" << std::endl;
      webserver_request.session_logic ()->set_username ("");
    }

    
    if (change_notificatons_count || assigned_notes_count || subscribed_notes_count) {
      email::schedule (user, subject, body.str());
    }
  }
}

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
using namespace std;


void statistics_statistics ()
{
  Webserver_Request request;
  Database_Modifications database_modifications;
  Database_Notes database_notes (&request);
  
  
  Database_Logs::log (translate("Sending statistics"), Filter_Roles::manager ());

  
  string siteUrl = config::logic::site_url (nullptr);
  
  
  vector <string> bibles = request.database_bibles()->getBibles ();
  
  
  vector <string> users = request.database_users ()->get_users ();
  for (auto & user : users) {
    
    
    string subject = "Bibledit " + translate("statistics");
    stringstream body;
    
  
    size_t change_notificatons_count = 0;
    if (request.database_config_user()->getUserPendingChangesNotification (user)) {
      string any_bible = string();
      vector <int> ids = database_modifications.getNotificationIdentifiers (user, any_bible);
      change_notificatons_count = ids.size();
      body << "<p><a href=" << quoted (siteUrl + changes_changes_url ()) << ">" << translate("Number of change notifications") << "</a>: " << ids.size() << "</p>" << endl;
    }
    

    size_t assigned_notes_count = 0;
    if (request.database_config_user()->getUserAssignedNotesStatisticsNotification (user)) {
      vector <int> ids = database_notes.select_notes (
                                                     bibles, // Bibles.
                                                     0,      // Book
                                                     0,      // Chapter
                                                     0,      // Verse
                                                     3,      // Passage selector.
                                                     0,      // Edit selector.
                                                     0,      // Non-edit selector.
                                                     "",     // Status selector.
                                                     "",     // Bible selector.
                                                     user,   // Assignment selector.
                                                     0,      // Subscription selector.
                                                     -1,     // Severity selector.
                                                     0,      // Text selector.
                                                     "",     // Search text.
                                                     -1);     // Limit.
      assigned_notes_count = ids.size();
      body << "<p><a href=" << quoted (siteUrl + notes_index_url () + "?presetselection=assigned") << ">" << translate("Number of consultation notes assigned to you awaiting your response") << "</a>: " << ids.size() << "</p>" << endl;
    }
    

    size_t subscribed_notes_count = 0;
    if (request.database_config_user()->getUserSubscribedNotesStatisticsNotification (user)) {
      body << "<p>" << translate("Number of consultation notes you are subscribed to") << ":</p>" << endl;
      body << "<ul>" << endl;
      request.session_logic ()->set_username (user);
      
      vector <int> ids = database_notes.select_notes (
                                                     bibles, // Bible.
                                                     0,      // Book
                                                     0,      // Chapter
                                                     0,      // Verse
                                                     3,      // Passage selector.
                                                     0,      // Edit selector.
                                                     0,      // Non-edit selector.
                                                     "",     // Status selector.
                                                     "",     // Bible selector.
                                                     "",     // Assignment selector.
                                                     1,      // Subscription selector.
                                                     -1,     // Severity selector.
                                                     0,      // Text selector.
                                                     "",     // Search text.
                                                     -1);     // Limit.
      subscribed_notes_count = ids.size();
      body << "<li><a href=" << quoted (siteUrl + notes_index_url () + "?presetselection=subscribed") << ">" << translate("Total") << "</a>: " << ids.size () << "</li>" << endl;
      ids = database_notes.select_notes (
                                                     bibles, // Bible.
                                                     0,      // Book
                                                     0,      // Chapter
                                                     0,      // Verse
                                                     3,      // Passage selector.
                                                     0,      // Edit selector.
                                                     1,      // Non-edit selector.
                                                     "",     // Status selector.
                                                     "",     // Bible selector.
                                                     "",     // Assignment selector.
                                                     1,      // Subscription selector.
                                                     -1,     // Severity selector.
                                                     0,      // Text selector.
                                                     "",     // Search text.
                                                     -1);     // Limit.
      body << "<li><a href=" << quoted (siteUrl + notes_index_url () + "?presetselection=subscribeddayidle") << ">" << translate("Inactive for a day") << "</a>: " << ids.size() << "</li>" << endl;
      ids = database_notes.select_notes (
                                                     bibles, // Bible.
                                                     0,      // Book
                                                     0,      // Chapter
                                                     0,      // Verse
                                                     3,      // Passage selector.
                                                     0,      // Edit selector.
                                                     3,      // Non-edit selector.
                                                     "",     // Status selector.
                                                     "",     // Bible selector.
                                                     "",     // Assignment selector.
                                                     1,      // Subscription selector.
                                                     -1,     // Severity selector.
                                                     0,      // Text selector.
                                                     "",     // Search text.
                                                     -1);     // Limit.
      body << "<li><a href=" << quoted (siteUrl + notes_index_url () + "?presetselection=subscribedweekidle") << ">" << translate("Inactive for a week") << "</a>: " << ids.size() << "</li>" << endl;
      body << "</ul>" << endl;
      request.session_logic ()->set_username ("");
    }

    
    if (change_notificatons_count || assigned_notes_count || subscribed_notes_count) {
      email_schedule (user, subject, body.str());
    }
  }
}

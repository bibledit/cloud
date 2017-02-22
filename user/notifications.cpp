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


#include <user/notifications.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/config/user.h>
#include <database/config/general.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <access/bible.h>


string user_notifications_url ()
{
  return "user/notifications";
}


bool user_notifications_acl (void * webserver_request)
{
  // Consultant has access.
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  // Whoever can view notes has access.
  if (access_logic_privilege_view_notes (webserver_request)) return true;
  // Whoever has access to a Bible has access to this page.
  bool read, write;
  access_a_bible (webserver_request, read, write);
  if (read) return true;
  // No access.
  return false;
}


string user_notifications (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Config_User database_config_user = Database_Config_User (webserver_request);
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Notifications"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);

  if (checkbox == "editednotessubscription") {
    database_config_user.setSubscribeToConsultationNotesEditedByMe (checked);
    return "";
  }
  view.set_variable ("editednotessubscription", get_checkbox_status (database_config_user.getSubscribeToConsultationNotesEditedByMe ()));

  if (checkbox == "anynotessubscription") {
    database_config_user.setNotifyMeOfAnyConsultationNotesEdits (checked);
    return "";
  }
  view.set_variable ("anynotessubscription", get_checkbox_status (database_config_user.getNotifyMeOfAnyConsultationNotesEdits ()));

  if (checkbox == "emailconfirmationyourposts") {
    database_config_user.setNotifyMeOfMyPosts (checked);
    return "";
  }
  view.set_variable ("emailconfirmationyourposts", get_checkbox_status (database_config_user.getNotifyMeOfMyPosts ()));

  if (checkbox == "subscribednotenotification") {
    database_config_user.setSubscribedConsultationNoteNotification (checked);
    return "";
  }
  view.set_variable ("subscribednotenotification", get_checkbox_status (database_config_user.getSubscribedConsultationNoteNotification ()));

  if (checkbox == "notesassignment") {
    database_config_user.setAssignedToConsultationNotesChanges (checked);
    return "";
  }
  view.set_variable ("notesassignment", get_checkbox_status (database_config_user.getAssignedToConsultationNotesChanges ()));
  
  if (checkbox == "assignednotenotification") {
    database_config_user.setAssignedConsultationNoteNotification (checked);
    return "";
  }
  view.set_variable ("assignednotenotification", get_checkbox_status (database_config_user.getAssignedConsultationNoteNotification ()));
  
  if (checkbox == "suppressemailsfromnotesyouupdated") {
    database_config_user.setSuppressMailFromYourUpdatesNotes (checked);
    return "";
  }
  view.set_variable ("suppressemailsfromnotesyouupdated", get_checkbox_status (database_config_user.getSuppressMailFromYourUpdatesNotes ()));

  if (checkbox == "assignednotesnotification") {
    database_config_user.setAssignedNotesStatisticsNotification (checked);
    return "";
  }
  view.set_variable ("assignednotesnotification", get_checkbox_status (database_config_user.getAssignedNotesStatisticsNotification ()));

  if (checkbox == "subscribednotesnotification") {
    database_config_user.setSubscribedNotesStatisticsNotification (checked);
    return "";
  }
  view.set_variable ("subscribednotesnotification", get_checkbox_status (database_config_user.getSubscribedNotesStatisticsNotification ()));

  if (checkbox == "deletednotenotification") {
    database_config_user.setDeletedConsultationNoteNotification (checked);
    return "";
  }
  view.set_variable ("deletednotenotification", get_checkbox_status (database_config_user.getDeletedConsultationNoteNotification ()));

  if (checkbox == "postponenewnotesmails") {
    database_config_user.setPostponeNewNotesMails (checked);
    return "";
  }
  view.set_variable ("postponenewnotesmails", get_checkbox_status (database_config_user.getPostponeNewNotesMails ()));

  if (checkbox == "anyonechangesemailnotification") {
    database_config_user.setBibleChangesNotification (checked);
    return "";
  }
  view.set_variable ("anyonechangesemailnotification", get_checkbox_status (database_config_user.getBibleChangesNotification ()));

  if (checkbox == "anyonechangesonlinenotifications") {
    database_config_user.setGenerateChangeNotifications (checked);
    return "";
  }
  view.set_variable ("anyonechangesonlinenotifications", get_checkbox_status (database_config_user.getGenerateChangeNotifications ()));

  if (checkbox == "pendingchangenotifications") {
    database_config_user.setPendingChangesNotification (checked);
    return "";
  }
  view.set_variable ("pendingchangenotifications", get_checkbox_status (database_config_user.getPendingChangesNotification ()));
  
  if (checkbox == "mychangesemailnotifications") {
    database_config_user.setUserChangesNotification (checked);
    return "";
  }
  view.set_variable ("mychangesemailnotifications", get_checkbox_status (database_config_user.getUserChangesNotification ()));
  
  if (checkbox == "mychangesonlinenotifications") {
    database_config_user.setUserChangesNotificationsOnline (checked);
    return "";
  }
  view.set_variable ("mychangesonlinenotifications", get_checkbox_status (database_config_user.getUserChangesNotificationsOnline ()));

  if (checkbox == "contributorschangesonlinenotifications") {
    database_config_user.setContributorChangesNotificationsOnline (checked);
    return "";
  }
  view.set_variable ("contributorschangesonlinenotifications", get_checkbox_status (database_config_user.getContributorChangesNotificationsOnline ()));

  if (checkbox == "biblechecksnotification") {
    database_config_user.setBibleChecksNotification (checked);
    return "";
  }
  view.set_variable ("biblechecksnotification", get_checkbox_status (database_config_user.getBibleChecksNotification ()));

  if (checkbox == "sprintprogressnotification") {
    database_config_user.setSprintProgressNotification (checked);
    return "";
  }
  view.set_variable ("sprintprogressnotification", get_checkbox_status (database_config_user.getSprintProgressNotification ()));

  if (checkbox == "yoursoftware") {
    database_config_user.setYourSoftwareUpdatesNotification (checked);
    return "";
  }
  view.set_variable ("yoursoftware", get_checkbox_status (database_config_user.getYourSoftwareUpdatesNotification ()));
  
  if (checkbox == "allsoftware") {
    database_config_user.setAllSoftwareUpdatesNotification (checked);
    return "";
  }
  view.set_variable ("allsoftware", get_checkbox_status (database_config_user.getAllSoftwareUpdatesNotification ()));
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#else
  view.enable_zone ("server");
#endif

  view.set_variable ("url", client_logic_link_to_cloud (user_notifications_url (), translate("You can set the notifications in Bibledit Cloud.")));

  // The bits accessible to the user depends on the user's privileges.
  bool read_bible, write_bible;
  access_a_bible (webserver_request, read_bible, write_bible);
  if (read_bible) view.enable_zone ("readbible");
  if (write_bible) view.enable_zone ("writebible");
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    view.enable_zone ("consultant");
  
  page += view.render ("user", "notifications");

  page += Assets_Page::footer ();

  return page;
}

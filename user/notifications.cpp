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


#include <user/notifications.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/config/user.h>
#include <database/config/general.h>
#include <database/noteassignment.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <access/bible.h>
#include <email/send.h>


std::string user_notifications_url ()
{
  return "user/notifications";
}


bool user_notifications_acl (Webserver_Request& webserver_request)
{
  // Consultant has access.
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    return true;
  // Whoever can view notes has access.
  if (access_logic::privilege_view_notes (webserver_request))
    return true;
  // Whoever has access to a Bible has access to this page.
  auto [ read, write ] = access_bible::any (webserver_request);
  if (read)
    return true;
  // No access.
  return false;
}


std::string user_notifications (Webserver_Request& webserver_request)
{
  Database_Config_User database_config_user (webserver_request);
  Database_NoteAssignment database_noteassignment;

  std::string page;
  
  Assets_Header header = Assets_Header (translate("Notifications"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);

  if (checkbox == "editednotessubscription") {
    database_config_user.setSubscribeToConsultationNotesEditedByMe (checked);
    return std::string();
  }
  view.set_variable ("editednotessubscription", filter::strings::get_checkbox_status (database_config_user.getSubscribeToConsultationNotesEditedByMe ()));

  if (checkbox == "anynotessubscription") {
    database_config_user.setNotifyMeOfAnyConsultationNotesEdits (checked);
    return std::string();
  }
  view.set_variable ("anynotessubscription", filter::strings::get_checkbox_status (database_config_user.getNotifyMeOfAnyConsultationNotesEdits ()));

  if (checkbox == "emailconfirmationyourposts") {
    database_config_user.setNotifyMeOfMyPosts (checked);
    return std::string();
  }
  view.set_variable ("emailconfirmationyourposts", filter::strings::get_checkbox_status (database_config_user.getNotifyMeOfMyPosts ()));

  if (checkbox == "subscribednotenotification") {
    database_config_user.setSubscribedConsultationNoteNotification (checked);
    return std::string();
  }
  view.set_variable ("subscribednotenotification", filter::strings::get_checkbox_status (database_config_user.getSubscribedConsultationNoteNotification ()));

  if (checkbox == "notesassignment") {
    database_config_user.setAssignedToConsultationNotesChanges (checked);
    return std::string();
  }
  view.set_variable ("notesassignment", filter::strings::get_checkbox_status (database_config_user.getAssignedToConsultationNotesChanges ()));
  
  if (checkbox == "assignednotenotification") {
    database_config_user.setAssignedConsultationNoteNotification (checked);
    return std::string();
  }
  view.set_variable ("assignednotenotification", filter::strings::get_checkbox_status (database_config_user.getAssignedConsultationNoteNotification ()));
  
  if (checkbox == "suppressemailsfromnotesyouupdated") {
    database_config_user.setSuppressMailFromYourUpdatesNotes (checked);
    return std::string();
  }
  view.set_variable ("suppressemailsfromnotesyouupdated", filter::strings::get_checkbox_status (database_config_user.getSuppressMailFromYourUpdatesNotes ()));

  if (checkbox == "assignednotesnotification") {
    database_config_user.setAssignedNotesStatisticsNotification (checked);
    return std::string();
  }
  view.set_variable ("assignednotesnotification", filter::strings::get_checkbox_status (database_config_user.getAssignedNotesStatisticsNotification ()));

  if (checkbox == "subscribednotesnotification") {
    database_config_user.setSubscribedNotesStatisticsNotification (checked);
    return std::string();
  }
  view.set_variable ("subscribednotesnotification", filter::strings::get_checkbox_status (database_config_user.getSubscribedNotesStatisticsNotification ()));

  if (checkbox == "deletednotenotification") {
    database_config_user.setDeletedConsultationNoteNotification (checked);
    return std::string();
  }
  view.set_variable ("deletednotenotification", filter::strings::get_checkbox_status (database_config_user.getDeletedConsultationNoteNotification ()));

  if (checkbox == "postponenewnotesmails") {
    database_config_user.setPostponeNewNotesMails (checked);
    return std::string();
  }
  view.set_variable ("postponenewnotesmails", filter::strings::get_checkbox_status (database_config_user.getPostponeNewNotesMails ()));

  const std::string& user = webserver_request.session_logic ()->get_username ();
  const std::vector <std::string> all_assignees = database_noteassignment.assignees (user);
  std::vector <std::string> current_assignees = database_config_user.getAutomaticNoteAssignment();
  for (const auto& assignee : all_assignees) {
    if (checkbox == "autoassign" + assignee) {
      if (checked) {
        current_assignees.push_back (assignee);
      } else {
        current_assignees = filter::strings::array_diff (current_assignees, {assignee});
      }
      database_config_user.setAutomaticNoteAssignment (current_assignees);
    }
  }
  for (const auto& assignee : all_assignees) {
    std::map <std::string, std::string> values;
    values ["user"] = assignee;
    values ["assign"] = filter::strings::get_checkbox_status (in_array (assignee, current_assignees));
    view.add_iteration ("autoassign", values);
  }
  
  if (checkbox == "anyonechangesemailnotification") {
    database_config_user.setBibleChangesNotification (checked);
    return std::string();
  }
  view.set_variable ("anyonechangesemailnotification", filter::strings::get_checkbox_status (database_config_user.getBibleChangesNotification ()));

  if (checkbox == "anyonechangesonlinenotifications") {
    database_config_user.setGenerateChangeNotifications (checked);
    return std::string();
  }
  view.set_variable ("anyonechangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.getGenerateChangeNotifications ()));

  if (checkbox == "pendingchangenotifications") {
    database_config_user.setPendingChangesNotification (checked);
    return std::string();
  }
  view.set_variable ("pendingchangenotifications", filter::strings::get_checkbox_status (database_config_user.getPendingChangesNotification ()));
  
  if (checkbox == "mychangesemailnotifications") {
    database_config_user.setUserChangesNotification (checked);
    return std::string();
  }
  view.set_variable ("mychangesemailnotifications", filter::strings::get_checkbox_status (database_config_user.getUserChangesNotification ()));
  
  if (checkbox == "mychangesonlinenotifications") {
    database_config_user.setUserChangesNotificationsOnline (checked);
    return std::string();
  }
  view.set_variable ("mychangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.getUserChangesNotificationsOnline ()));

  if (checkbox == "contributorschangesonlinenotifications") {
    database_config_user.setContributorChangesNotificationsOnline (checked);
    return std::string();
  }
  view.set_variable ("contributorschangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.getContributorChangesNotificationsOnline ()));

  // Setting per user for which Bibles the user will receive change notifications.
  // The set of Bibles the user can choose
  // is limited to those Bibles the user has read access to.
  {
    std::vector <std::string> bibles = access_bible::bibles (webserver_request);
    for (const auto& bible : bibles) {
      if (checkbox == "changenotificationbible" + bible) {
        std::vector <std::string> currentbibles = database_config_user.getChangeNotificationsBibles();
        if (checked) {
          currentbibles.push_back(bible);
        } else {
          currentbibles = filter::strings::array_diff (currentbibles, {bible});
        }
        database_config_user.setChangeNotificationsBibles(currentbibles);
      }
    }
    const std::vector <std::string> currentbibles = database_config_user.getChangeNotificationsBibles();
    for (const auto& bible : bibles) {
      std::map <std::string, std::string> values;
      values ["bible"] = bible;
      values ["checked"] = filter::strings::get_checkbox_status (in_array (bible, currentbibles));
      view.add_iteration ("changenotificationbible", values);
    }
  }
  
  if (checkbox == "biblechecksnotification") {
    database_config_user.setBibleChecksNotification (checked);
    return std::string();
  }
  view.set_variable ("biblechecksnotification", filter::strings::get_checkbox_status (database_config_user.getBibleChecksNotification ()));

  if (checkbox == "sprintprogressnotification") {
    database_config_user.setSprintProgressNotification (checked);
    return std::string();
  }
  view.set_variable ("sprintprogressnotification", filter::strings::get_checkbox_status (database_config_user.getSprintProgressNotification ()));
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#else
  view.enable_zone ("server");
#endif

  view.set_variable ("url", client_logic_link_to_cloud (user_notifications_url (), translate("You can set the notifications in Bibledit Cloud.")));

  // The bits accessible to the user depends on the user's privileges.
  auto [ read_bible, write_bible ] = access_bible::any (webserver_request);
  if (read_bible)
    view.enable_zone ("readbible");
  if (write_bible) 
    view.enable_zone ("writebible");
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    view.enable_zone ("consultant");
  
  view.set_variable ("error", email_setup_information (true, false));
  
  page += view.render ("user", "notifications");

  page += assets_page::footer ();

  return page;
}

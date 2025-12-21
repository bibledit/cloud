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
  if (roles::access_control (webserver_request, roles::consultant))
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

  std::string checkbox = webserver_request.post_get("checkbox");
  bool checked = filter::strings::convert_to_bool (webserver_request.post_get("checked"));

  if (checkbox == "editednotessubscription") {
    database_config_user.set_subscribe_to_consultation_notes_edited_by_me (checked);
    return std::string();
  }
  view.set_variable ("editednotessubscription", filter::strings::get_checkbox_status (database_config_user.get_subscribe_to_consultation_notes_edited_by_me ()));

  if (checkbox == "anynotessubscription") {
    database_config_user.set_notify_me_of_any_consultation_notes_edits (checked);
    return std::string();
  }
  view.set_variable ("anynotessubscription", filter::strings::get_checkbox_status (database_config_user.get_notify_me_of_any_consultation_notes_edits ()));

  if (checkbox == "emailconfirmationyourposts") {
    database_config_user.set_notify_me_of_my_posts (checked);
    return std::string();
  }
  view.set_variable ("emailconfirmationyourposts", filter::strings::get_checkbox_status (database_config_user.get_notify_me_of_my_posts ()));

  if (checkbox == "subscribednotenotification") {
    database_config_user.set_subscribed_consultation_note_notification (checked);
    return std::string();
  }
  view.set_variable ("subscribednotenotification", filter::strings::get_checkbox_status (database_config_user.get_subscribed_consultation_note_notification ()));

  if (checkbox == "notesassignment") {
    database_config_user.set_assigned_to_consultation_notes_changes (checked);
    return std::string();
  }
  view.set_variable ("notesassignment", filter::strings::get_checkbox_status (database_config_user.get_assigned_to_consultation_notes_changes ()));
  
  if (checkbox == "assignednotenotification") {
    database_config_user.set_assigned_consultation_note_notification (checked);
    return std::string();
  }
  view.set_variable ("assignednotenotification", filter::strings::get_checkbox_status (database_config_user.get_assigned_consultation_note_notification ()));
  
  if (checkbox == "suppressemailsfromnotesyouupdated") {
    database_config_user.set_suppress_mail_from_your_updates_notes (checked);
    return std::string();
  }
  view.set_variable ("suppressemailsfromnotesyouupdated", filter::strings::get_checkbox_status (database_config_user.get_suppress_mail_from_your_updates_notes ()));

  if (checkbox == "assignednotesnotification") {
    database_config_user.set_assigned_notes_statistics_notification (checked);
    return std::string();
  }
  view.set_variable ("assignednotesnotification", filter::strings::get_checkbox_status (database_config_user.get_assigned_notes_statistics_notification ()));

  if (checkbox == "subscribednotesnotification") {
    database_config_user.set_subscribed_notes_statistics_notification (checked);
    return std::string();
  }
  view.set_variable ("subscribednotesnotification", filter::strings::get_checkbox_status (database_config_user.get_subscribed_notes_statistics_notification ()));

  if (checkbox == "deletednotenotification") {
    database_config_user.set_deleted_consultation_note_notification (checked);
    return std::string();
  }
  view.set_variable ("deletednotenotification", filter::strings::get_checkbox_status (database_config_user.get_deleted_consultation_note_notification ()));

  if (checkbox == "postponenewnotesmails") {
    database_config_user.set_postpone_new_notes_mails (checked);
    return std::string();
  }
  view.set_variable ("postponenewnotesmails", filter::strings::get_checkbox_status (database_config_user.get_postpone_new_notes_mails ()));

  const std::string& user = webserver_request.session_logic ()->get_username ();
  const std::vector <std::string> all_assignees = database_noteassignment.assignees (user);
  std::vector <std::string> current_assignees = database_config_user.get_automatic_note_assignment();
  for (const auto& assignee : all_assignees) {
    if (checkbox == "autoassign" + assignee) {
      if (checked) {
        current_assignees.push_back (assignee);
      } else {
        current_assignees = filter::strings::array_diff (current_assignees, {assignee});
      }
      database_config_user.set_automatic_note_assignment (current_assignees);
    }
  }
  for (const auto& assignee : all_assignees) {
    std::map <std::string, std::string> values;
    values ["user"] = assignee;
    values ["assign"] = filter::strings::get_checkbox_status (in_array (assignee, current_assignees));
    view.add_iteration ("autoassign", values);
  }
  
  if (checkbox == "anyonechangesemailnotification") {
    database_config_user.set_bible_changes_notification (checked);
    return std::string();
  }
  view.set_variable ("anyonechangesemailnotification", filter::strings::get_checkbox_status (database_config_user.get_bible_changes_notification ()));

  if (checkbox == "anyonechangesonlinenotifications") {
    database_config_user.set_generate_change_notifications (checked);
    return std::string();
  }
  view.set_variable ("anyonechangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.get_generate_change_notifications ()));

  if (checkbox == "pendingchangenotifications") {
    database_config_user.set_pending_changes_notification (checked);
    return std::string();
  }
  view.set_variable ("pendingchangenotifications", filter::strings::get_checkbox_status (database_config_user.get_pending_changes_notification ()));
  
  if (checkbox == "mychangesemailnotifications") {
    database_config_user.set_user_changes_notification (checked);
    return std::string();
  }
  view.set_variable ("mychangesemailnotifications", filter::strings::get_checkbox_status (database_config_user.get_user_changes_notification ()));
  
  if (checkbox == "mychangesonlinenotifications") {
    database_config_user.set_user_changes_notifications_online (checked);
    return std::string();
  }
  view.set_variable ("mychangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.get_user_changes_notifications_online ()));

  if (checkbox == "contributorschangesonlinenotifications") {
    database_config_user.set_contributor_changes_notifications_online (checked);
    return std::string();
  }
  view.set_variable ("contributorschangesonlinenotifications", filter::strings::get_checkbox_status (database_config_user.get_contributor_changes_notifications_online ()));

  // Setting per user for which Bibles the user will receive change notifications.
  // The set of Bibles the user can choose
  // is limited to those Bibles the user has read access to.
  {
    std::vector <std::string> bibles = access_bible::bibles (webserver_request);
    for (const auto& bible : bibles) {
      if (checkbox == "changenotificationbible" + bible) {
        std::vector <std::string> currentbibles = database_config_user.get_change_notifications_bibles();
        if (checked) {
          currentbibles.push_back(bible);
        } else {
          currentbibles = filter::strings::array_diff (currentbibles, {bible});
        }
        database_config_user.set_change_notifications_bibles(currentbibles);
      }
    }
    const std::vector <std::string> currentbibles = database_config_user.get_change_notifications_bibles();
    for (const auto& bible : bibles) {
      std::map <std::string, std::string> values;
      values ["bible"] = bible;
      values ["checked"] = filter::strings::get_checkbox_status (in_array (bible, currentbibles));
      view.add_iteration ("changenotificationbible", values);
    }
  }
  
  if (checkbox == "biblechecksnotification") {
    database_config_user.set_bible_checks_notification (checked);
    return std::string();
  }
  view.set_variable ("biblechecksnotification", filter::strings::get_checkbox_status (database_config_user.get_bible_checks_notification ()));

  if (checkbox == "sprintprogressnotification") {
    database_config_user.set_sprint_progress_notification (checked);
    return std::string();
  }
  view.set_variable ("sprintprogressnotification", filter::strings::get_checkbox_status (database_config_user.get_sprint_progress_notification ()));
  
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
  if (roles::access_control (webserver_request, roles::consultant))
    view.enable_zone ("consultant");
  
  view.set_variable ("error", email::setup_information (true, false));
  
  page += view.render ("user", "notifications");

  page += assets_page::footer ();

  return page;
}

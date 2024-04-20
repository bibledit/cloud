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


#include <notes/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <database/notes.h>
#include <database/noteactions.h>
#include <database/noteassignment.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/books.h>
#include <trash/handler.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <sync/logic.h>
#include <notes/note.h>
#include <workspace/index.h>
#include <access/bible.h>
#include <email/send.h>


Notes_Logic::Notes_Logic (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
}


// Create a consultation note.
// $bible: The notes's Bible.
// $book, $chapter, $verse: The note's passage.
// $summary: The note's summary.
// $contents: The note's contents.
// $raw: Import $contents as it is.
// It returns the $identifier of this new note.
int Notes_Logic::createNote (std::string bible, int book, int chapter, int verse, std::string summary, std::string contents, bool raw)
{
  summary = filter::strings::replace ("\n", "", summary);
  Database_Notes database_notes (m_webserver_request);
  int note_id = database_notes.store_new_note (bible, book, chapter, verse, summary, contents, raw);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    Database_NoteActions database_noteactions;
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_create_initiate, "");
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_summary, "");
    // The contents to submit to the server, take it from the database, as it was updated in the logic above.
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_contents, database_notes.get_contents (note_id));
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_bible, "");
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_passages, "");
    database_noteactions.record (m_webserver_request.session_logic()->currentUser (), note_id, Sync_Logic::notes_put_create_complete, "");
  } else {
    // Server: do the notifications.
    handlerNewNote (note_id);
  }
  return note_id;
}


// Set the $content of an existing note $identifier.
void Notes_Logic::setContent (int identifier, const std::string& content)
{
  // Do nothing if there was no content.
  if (content.empty ()) return;
  
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_contents (identifier, content);

  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_contents, content);
  } else {
    // Server: do the notifications.
    handlerAddComment (identifier);
  }
}


// Add a comment to an exiting note identified by identifier.
void Notes_Logic::addComment (int identifier, const std::string& comment)
{
  // Do nothing if there was no content.
  if (comment == "") return;

  Database_Notes database_notes (m_webserver_request);
  database_notes.add_comment (identifier, comment);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_comment, comment);
  } else {
    // Server: do the notifications.
    handlerAddComment (identifier);
  }
}


void Notes_Logic::set_summary (int identifier, const std::string& summary)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_summary (identifier, summary);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_summary, "");
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::subscribe (int identifier)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.subscribe (identifier);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_subscribe, "");
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::unsubscribe (int identifier)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.unsubscribe (identifier);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_unsubscribe, "");
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::assignUser (int identifier, const std::string& user)
{
  Database_Notes database_notes (m_webserver_request);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string myuser = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions;
    database_noteactions.record (myuser, identifier, Sync_Logic::notes_put_assign, user);
  } else {
    // Server: do the notifications.
    // Assign logic comes before the database action in this particular case.
    handlerAssignNote (identifier, user);
  }
  database_notes.assign_user (identifier, user);
}


void Notes_Logic::unassignUser (int identifier, const std::string& user)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.unassign_user (identifier, user);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string myuser = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (myuser, identifier, Sync_Logic::notes_put_unassign, user);
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::setStatus (int identifier, const std::string& status)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_status (identifier, status);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_status, "");
  } else {
    // Server: do nothing extra.
  }
}


// Set the passages for note identifier.
void Notes_Logic::setPassages (int identifier, const std::vector <Passage> & passages)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_passages (identifier, passages);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_passages, "");
  } else {
    // Server: do nothing extra.
  }
}


// Sets the severity as a number for note identifier.
void Notes_Logic::setRawSeverity (int identifier, int severity)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_raw_severity (identifier, severity);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_severity, "");
  } else {
    // Server: do nothing extra.
  }
}


// Sets the bible for note identifier.
void Notes_Logic::setBible (int identifier, const std::string& bible)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.set_bible (identifier, bible);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_bible, "");
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::markForDeletion (int identifier)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.mark_for_deletion (identifier);
  trash_consultation_note (m_webserver_request, identifier);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_mark_delete, "");
  } else {
    // Server: notifications.
    handlerMarkNoteForDeletion (identifier);
  }
}


void Notes_Logic::unmarkForDeletion (int identifier)
{
  Database_Notes database_notes (m_webserver_request);
  database_notes.unmark_for_deletion (identifier);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_unmark_delete, "");
  } else {
    // Server: do nothing extra.
  }
}


void Notes_Logic::erase (int identifier)
{
  Database_Notes database_notes (m_webserver_request);
  if (client_logic_client_enabled ()) {
    // Client: record the action in the database.
    std::string user = m_webserver_request.session_logic ()->currentUser ();
    Database_NoteActions database_noteactions = Database_NoteActions ();
    database_noteactions.record (user, identifier, Sync_Logic::notes_put_delete, "");
  } else {
    // Server: notification.
    handlerDeleteNote (identifier);
  }
  trash_consultation_note (m_webserver_request, identifier);
  database_notes.erase (identifier);
}


void Notes_Logic::handlerNewNote (int identifier)
{
  notifyUsers (identifier, notifyNoteNew);
}


void Notes_Logic::handlerAddComment (int identifier)
{
  notifyUsers (identifier, notifyNoteComment);
  // If the note status was Done, and a comment is added, mark it Reopened.
  Database_Notes database_notes (m_webserver_request);
  std::string status = database_notes.get_raw_status (identifier);
  if (status == "Done") {
    database_notes.set_status (identifier, "Reopened");
  }
}


void Notes_Logic::handlerAssignNote (int identifier, const std::string& user)
{
  // Take no action in client mode.
  if (client_logic_client_enabled ()) return;
  
  Database_Config_User database_config_user (m_webserver_request);
  if (database_config_user.getUserAssignedConsultationNoteNotification (user)) {
    // Only email the user if the user was not yet assigned this note.
    Database_Notes database_notes (m_webserver_request);
    std::vector <std::string> assignees = database_notes.get_assignees (identifier);
    if (find (assignees.begin(), assignees.end(), user) == assignees.end()) {
      emailUsers (identifier, translate("Assigned"), "", {user}, false);
    }
  }
}


void Notes_Logic::handlerMarkNoteForDeletion (int identifier)
{
  notifyUsers (identifier, notifyMarkNoteForDeletion);
}


void Notes_Logic::handlerDeleteNote (int identifier)
{
  notifyUsers (identifier, notifyNoteDelete);
}


// This handles notifications for the users
// identifier: the note that is being handled.
// notification: the type of action on the consultation note.
void Notes_Logic::notifyUsers (int identifier, int notification)
{
  // Take no action in client mode.
  if (client_logic_client_enabled ()) return;

  // Data objects.
  Database_Notes database_notes (m_webserver_request);
  
  // This note's Bible.
  std::string bible = database_notes.get_bible (identifier);

  // Subscription and assignment not to be used for notes marked for deletion,
  // because marking notes for deletion is nearly the same as deleting them straightaway.
  if (notification != notifyMarkNoteForDeletion) {

    // Whether current user gets subscribed to the note.
    if (m_webserver_request.database_config_user ()->getSubscribeToConsultationNotesEditedByMe ()) {
      database_notes.subscribe (identifier);
    }

    // Users to get subscribed to the note, or to whom the note is to be assigned.
    std::vector <std::string> users = m_webserver_request.database_users ()->get_users ();
    for (const std::string& user : users) {
      if (access_bible::read (m_webserver_request, bible, user)) {
        if (m_webserver_request.database_config_user ()->getNotifyUserOfAnyConsultationNotesEdits (user)) {
          database_notes.subscribe_user (identifier, user);
        }
        if (m_webserver_request.database_config_user ()->getUserAssignedToConsultationNotesChanges (user)) {
          database_notes.assign_user (identifier, user);
        }
      }
    }
    std::vector <std::string> auto_assignees = m_webserver_request.database_config_user ()->getAutomaticNoteAssignment ();
    for (auto assignee : auto_assignees) {
      database_notes.assign_user (identifier, assignee);
    }
  }

  // The recipients who receive a notification by email.
  std::vector <std::string> recipients;

  // Subscribers who receive email.
  std::vector <std::string> subscribers = database_notes.get_subscribers (identifier);
  for (const std::string& subscriber : subscribers) {
    if (m_webserver_request.database_config_user ()->getUserSubscribedConsultationNoteNotification (subscriber)) {
      recipients.push_back (subscriber);
    }
  }

  // Assignees who receive email.
  std::vector <std::string> assignees = database_notes.get_assignees (identifier);
  for (const std::string& assignee : assignees) {
    if (m_webserver_request.database_config_user ()->getUserAssignedConsultationNoteNotification (assignee)) {
      recipients.push_back (assignee);
    }
  }

  // In case the consultation note is deleted or marked for deletion,
  // notify only the users with this specific notification set.
  if ((notification == notifyNoteDelete) || (notification == notifyMarkNoteForDeletion)) {
    recipients.clear ();
    std::vector <std::string> users = m_webserver_request.database_users ()->get_users ();
    for (const auto & user : users) {
      if (m_webserver_request.database_config_user ()->getUserDeletedConsultationNoteNotification (user)) {
        if (access_bible::read (m_webserver_request, bible, user)) {
          recipients.push_back (user);
        }
      }
    }
  }

  // Remove duplicates from the recipients.
  std::set <std::string> unique (recipients.begin (), recipients.end ());
  recipients.assign (unique.begin (), unique.end());

  // Deal with suppressing mail to the user when he made the update himself.
  std::string username = m_webserver_request.session_logic ()->currentUser ();
  if (m_webserver_request.database_config_user ()->getUserSuppressMailFromYourUpdatesNotes (username)) {
    recipients.erase (remove (recipients.begin(), recipients.end(), username), recipients.end());
  }

  // Generate the label prefixed to the subject line of the email.
  std::string label = translate("General");
  switch (notification) {
    case notifyNoteNew             : label = translate("New");                 break;
    case notifyNoteComment         : label = translate("Comment");             break;
    case notifyNoteDelete          : label = translate("Deleted");             break;
    case notifyMarkNoteForDeletion : label = translate("Marked for deletion"); break;
    default: break;
  }

  // Optional postponing sending email.
  bool postpone = false;
  if (notification == notifyNoteNew) {
    if (m_webserver_request.database_config_user ()->getPostponeNewNotesMails ()) {
      postpone = true;
    }
  }

  // Send mail to all recipients.
  emailUsers (identifier, label, bible, recipients, postpone);
}


// This handles email to users.
// identifier: the note that is being handled.
// label: prefix to the subject line of the email.
// bible: If given, to include in the subject line of the email.
// users: array of users to be mailed.
// postpone: whether to postpone sending the email till the evening.
void Notes_Logic::emailUsers (int identifier, const std::string& label, std::string bible, const std::vector <std::string> & users, bool postpone)
{
  // Databases.
  Database_Notes database_notes (m_webserver_request);

  // Send mail to all users.
  std::string summary = database_notes.get_summary (identifier);
  std::string passages = filter_passage_display_inline (database_notes.get_passages (identifier));
  std::stringstream contents;
  contents << database_notes.get_contents (identifier);

  // Include links to the Cloud: One to the note, and one to the active workspace.
  contents << "<br>" << std::endl;
  contents << "<p>";
  contents << "<a href=";
  std::string notelink = config::logic::site_url (m_webserver_request) + notes_note_url () + "?id=" + filter::strings::convert_to_string (identifier);
  contents << std::quoted (notelink);
  contents << ">";
  contents << translate ("View or respond online");
  contents << "</a>";
  contents << " " << translate ("or") << " ";

  contents << "<a href=";
  std::string workspacelink = config::logic::site_url (m_webserver_request) + workspace_index_url () + "?note=" + filter::strings::convert_to_string (identifier);
  contents << std::quoted (workspacelink);
  contents << ">";
  contents << translate ("open the workspace online");
  contents << "</a>";

  contents << "</p>" << std::endl;
  std::string mailto = "mailto:" + Database_Config_General::getSiteMailAddress () + "?subject=(CNID" + filter::strings::convert_to_string (identifier) + ")";
  contents << "<p><a href=";
  contents << std::quoted (mailto);
  contents << ">Respond by email</a></p>" << std::endl;

  // Deal with possible postponing email till 9 PM.
  int timestamp = filter::date::seconds_since_epoch ();
  if (postpone) {
    int localseconds = filter::date::local_seconds (timestamp);
    float localhour = static_cast<float>(filter::date::numerical_hour (localseconds)) + static_cast<float>(filter::date::numerical_minute (localseconds)) / 60;
    if (localhour < 21) {
      float difference = 21 - localhour;
      timestamp += static_cast<int>(3600 * difference) - 10;
    }
  }

  // Send (but not in client mode).
  for (auto & user : users) {
    if (!client_logic_client_enabled ()) {
      std::string subject = label;
      subject.append (" | ");
      if (!bible.empty ()) {
        subject.append (bible);
        subject.append (" ");
      }
      subject.append (passages);
      subject.append (" | ");
      subject.append (summary);
      subject.append (" | (CNID");
      subject.append (filter::strings::convert_to_string (identifier));
      subject.append (")");
      email_schedule (user, subject, contents.str(), timestamp);
    }
  }
}


// handleEmailComment - handles an email received from from with subject subject and body body.
// Returns true if the mail was processed, else false.
// The email is considered to have been processed if it comments on an existing Consultation Note.
bool Notes_Logic::handleEmailComment (std::string from, std::string subject, std::string body)
{
  // Check whether the Consultation Notes Identifier in the subject exists in the notes database.
  // The CNID looks like: (CNID123456789)
  size_t pos = subject.find ("(CNID");
  if (pos == std::string::npos) return false;
  subject = subject.substr (pos + 5);
  pos = subject.find (")");
  if (pos == std::string::npos) return false;
  subject = subject.substr (0, pos);
  // Webserver request.
  // At this stage, the subject contains an identifier.
  // Check that the identifier is an existing Consultation Note.
  int identifier = filter::strings::convert_to_int (subject);
  Database_Notes database_notes (m_webserver_request);
  if (!database_notes.identifier_exists (identifier)) return false;
  // Check that the from address of the email belongs to an existing user.
  // Or else use the obfuscated email address as the user name.
  std::string username;
  from = filter::strings::extract_email (from);
  if (m_webserver_request.database_users()->emailExists (from)) {
    username = m_webserver_request.database_users()->getEmailToUser (from);
  } else {
    username = from;
    username = filter::strings::replace ("@", " ", username);
    username = filter::strings::replace (".", " ", username);
  }
  // Clean the email's body.
  std::string year = filter::strings::convert_to_string (filter::date::numerical_year (filter::date::seconds_since_epoch ()));
  std::string sender = Database_Config_General::getSiteMailName();
  body = filter::strings::extract_body (body, year, sender);
  // Remove any new lines from the body. This cleans up the email considerably,
  // because some emails that get posted would otherwise look untidy,
  // when the many new lines take up a lot of space.
  body = filter::strings::replace ("\n", " ", body);
  // Make comment on the consultation note.
  std::string sessionuser = m_webserver_request.session_logic ()->currentUser ();
  m_webserver_request.session_logic ()->set_username (username);
  addComment (identifier, body);
  m_webserver_request.session_logic ()->set_username (sessionuser);
  // Mail confirmation to the username.
  if (m_webserver_request.database_config_user()->getUserNotifyMeOfMyPosts (username)) {
    std::string confirm_subject = translate("Your comment was posted");
    confirm_subject.append (" [CNID");
    confirm_subject.append (filter::strings::convert_to_string (identifier));
    confirm_subject.append ("]");
    email_schedule (username, confirm_subject, body);
  }
  // Log operation.
  Database_Logs::log ("Comment posted: " + body);
  // Job done.
  return true;
}


// handleEmailNew - handles an email received from $from with subject $subject and body $body.
// Returns true if the mail was processed, else false.
// The email is considered to have been processed if it created a new Consultation Note.
bool Notes_Logic::handleEmailNew (std::string from, std::string subject, std::string body)
{
  // Store the original subject.
  std::string originalSubject = subject;
  // Check that the subject indicates that a new consultation note is to be created.
  size_t pos = filter::strings::unicode_string_casefold (subject).find ("new note");
  if (pos == std::string::npos) return false;
  // There is a new note. Remove that bit from the subject.
  if (pos > 0) subject.erase (0, pos + 8);
  // Clean the subject line.
  subject = filter::strings::trim (subject);
  subject = filter::strings::replace (".", " ", subject);
  subject = filter::strings::replace (":", " ", subject);
  subject = filter::strings::collapse_whitespace (subject);
  // Check that the from address of the email belongs to an existing user.
  from = filter::strings::extract_email (from);
  if (!m_webserver_request.database_users()->emailExists (from)) return false;
  std::string username = m_webserver_request.database_users()->getEmailToUser (from);
  // Extract book, chapter, verse, and note summary from the subject
  book_id book {book_id::_unknown};
  int chapter {-1};
  int verse {-1};
  std::string summary {};
  std::vector <std::string> subjectlines = filter::strings::explode (subject, ' ');
  if (!subjectlines.empty()) {
    book = filter_passage_interpret_book_v2 (subjectlines[0]);
    subjectlines.erase (subjectlines.begin());
  }
  if (!subjectlines.empty()) {
    chapter = filter::strings::convert_to_int (subjectlines[0]);
    subjectlines.erase (subjectlines.begin());
  }
  if (!subjectlines.empty()) {
    verse = filter::strings::convert_to_int (subjectlines[0]);
    subjectlines.erase (subjectlines.begin());
  }
  summary = filter::strings::implode (subjectlines, " ");
  // Check book, chapter, verse, and summary. Give feedback if there's anything wrong.
  std::string noteCheck;
  if (book == book_id::_unknown) noteCheck.append (translate("Unknown book"));
  if (chapter < 0) {
    noteCheck.append (" ");
    noteCheck.append (translate("Unknown chapter"));
  }
  if (verse < 0) {
    noteCheck.append (" ");
    noteCheck.append (translate("Unknown verse"));
  }
  if (summary.empty ()) {
    noteCheck.append (" ");
    noteCheck.append (translate("Unknown summary"));
  }
  // Mail user if the note could not be posted.
  if (noteCheck != "") {
    subject = translate("Your new note could not be posted");
    email_schedule (username, subject  + ": " + originalSubject, noteCheck);
    return false;
  }
  // Clean the email's body.
  body = filter::strings::extract_body (body);
  // Post the note.
  std::string sessionuser = m_webserver_request.session_logic()->currentUser ();
  m_webserver_request.session_logic()->set_username (username);
  Database_Notes database_notes (m_webserver_request);
  std::string bible = m_webserver_request.database_config_user()->getBible ();
  int identifier = database_notes.store_new_note (bible, static_cast<int>(book), chapter, verse, summary, body, false);
  handlerNewNote (identifier);
  m_webserver_request.session_logic()->set_username (sessionuser);
  // Mail confirmation to the username.
  if (m_webserver_request.database_config_user()->getUserNotifyMeOfMyPosts (username)) {
    subject = translate("Your new note was posted");
    email_schedule (username, subject + ": " + originalSubject, body);
  }
  // Log operation.
  Database_Logs::log ("New note posted : " + body);
  // Job done.
  return true;
}


std::string Notes_Logic::generalBibleName ()
{
  return "[" + translate("no Bible") + "]";
}


// Maintains the database with note assignees.
// $force: Force maintenance.
void notes_logic_maintain_note_assignees (bool force)
{
  Database_NoteAssignment database_noteassignment;
  Webserver_Request webserver_request;
  
  Database_Users database_users;
  std::vector <std::string> users = database_users.get_users ();

  // If even one user's assignees are absent, force rebuilding them for all users.
  for (auto & user : users) {
    if (!database_noteassignment.exists (user)) force = true;
  }
  if (!force) return;

  Database_Bibles database_bibles;
  std::vector <std::string> bibles = database_bibles.get_bibles ();
  
  // A user can assign notes to other users
  // who have access to the Bibles the user has access to.
  for (auto & user : users) {

    std::vector <std::string> assignees;
    
    for (auto & bible : bibles) {
      
      // Continue with this Bible if the user has access to it.
      if (access_bible::read (webserver_request, bible, user)) {

        for (auto & assignee : users) {
          if (access_bible::read (webserver_request, bible, assignee)) {
            assignees.push_back (assignee);
          }
        }
      }
    }

    assignees = filter::strings::array_unique (assignees);
    database_noteassignment.assignees (user, assignees);
  }
}

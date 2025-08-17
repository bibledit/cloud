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


#include <confirm/worker.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <database/confirm.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <email/send.h>
#include <session/confirm.h>
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


#ifdef HAVE_CLOUD


namespace confirm::worker {


// Inform the managers about an account change.
static void inform_managers (const std::string& email, const std::string& body)
{
  Database_Users database_users {};
  const std::vector <std::string>& users = database_users.get_users ();
  for (const auto& user : users) {
    const int level = database_users.get_level (user);
    if (level >= roles::manager) {
      const std::string mailto = database_users.get_email (user);
      const std::string subject = translate ("User account change");
      std::string newbody = translate ("A user account was changed.");
      newbody.append (" ");
      newbody.append (translate ("Email address:"));
      newbody.append (" ");
      newbody.append (email);
      newbody.append (". ");
      newbody.append (translate ("The following email was sent to this user:"));
      newbody.append (" ");
      newbody.append (body);
      email::schedule (mailto, subject, newbody);
    }
  }
}


// Sets up a confirmation cycle in order to change something in the database.
// If for example a user requests the email address to be changed,
// an initial email will be sent, which the user should confirm.
// mailto            : Email address for the initial email and the response.
// initial_subject   : The subject of the initial email message.
// initial_body      : The body of the initial email message.
// query             : The query to be executed on the database if the user confirms the email successfully.
// subsequent_subject: The subject of the email to send upon user confirmation.
// subsequent_body   : The body of the email to send upon user confirmation.
void setup (Webserver_Request& webserver_request,
            const std::string& mailto, const std::string& username,
            const std::string& initial_subject, std::string initial_body,
            const std::string& query,
            const std::string& subsequent_subject, const std::string& subsequent_body)
{
  const unsigned int confirmation_id = database::confirm::get_new_id ();
  pugi::xml_document document;
  pugi::xml_node node = document.append_child ("p");
  std::string information;
  if (config::logic::default_bibledit_configuration ()) {
    information = translate ("Please confirm this request by clicking this following link:");
  }
  node.text ().set (information.c_str());
  node = document.append_child ("p");
  const std::string site_url = config::logic::site_url (webserver_request);
  std::string confirmation_url = filter_url_build_http_query(site_url + session_confirm_url (), {{"id", std::to_string(confirmation_id)}});
  node.text ().set (confirmation_url.c_str());
  std::stringstream output;
  document.print (output, "", pugi::format_raw);
  initial_body.append (output.str ());
  email::schedule (mailto, initial_subject, initial_body);
  database::confirm::store (confirmation_id, query, mailto, subsequent_subject, subsequent_body, username);
}


// Handles a confirmation link clicked with a confirmation ID.
// Returns true if link was valid, else false.
bool handle_link (Webserver_Request& webserver_request, std::string& email)
{
  // Get the confirmation identifier from the link that was clicked.
  const std::string web_id = webserver_request.query["id"];
  
  // If the identifier was not given, the link was not handled successfully.
  if (web_id.empty()) 
    return false;
  
  // Find out from the confirmation database whether the subject line contains an active ID.
  // If not, bail out.
  const unsigned int id = database::confirm::search_id (web_id);
  if (id == 0) {
    return false;
  }
  
  // An active ID was found: Execute the associated database query.
  const std::string query = database::confirm::get_query (id);
  webserver_request.database_users()->execute (query);
  
  // Send confirmation mail.
  const std::string mailto = database::confirm::get_mail_to (id);
  const std::string subject = database::confirm::get_subject (id);
  const std::string body = database::confirm::get_body (id);
  email::schedule (mailto, subject, body);
  
  // Delete the confirmation record.
  database::confirm::erase (id);
  
  // Notify the manager(s).
  confirm::worker::inform_managers (mailto, body);
  
  // Pass the email address to the caller.
  email = mailto;
  
  // Job done.
  return true;
}


} // namespace.


#endif

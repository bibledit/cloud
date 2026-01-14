/*
Copyright (©) 2003-2026 Teus Benschop.

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


#include <email/send.h>
#include <email/index.h>
#include <webserver/request.h>
#include <database/logs.h>
#include <database/mail.h>
#include <database/users.h>
#include <database/config/general.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/date.h>
#ifdef HAVE_CLOUD
#include <curl/curl.h>
#endif
#include <config/globals.h>
#include <tasks/logic.h>
#include <sync/mail.h>
#include <sync/logic.h>
#include <client/logic.h>
#include <locale/translate.h>


namespace email {


void send ()
{
  // No more than one email send process to run simultaneously.
  if (config_globals_mail_send_running) return;
  config_globals_mail_send_running = true;
  
  // The databases involved.
  Webserver_Request webserver_request;
  Database_Mail database_mail (webserver_request);
  Database_Users database_users;
  
  std::vector <int> mails = database_mail.getMailsToSend ();
  for (auto id : mails) {
    
    // Get all details of the mail.
    Database_Mail_Item details = database_mail.get (id);
    std::string username = details.username;
    std::string email = database_users.get_email (username);
    std::string subject = details.subject;
    std::string body = details.body;
    
#ifdef HAVE_CLIENT
    
    // On a client, set the email to the username before sending the email to the Cloud for further distribution.
    email = username;
    
#else
    
    // In the Cloud, if this username was not found, it could be that the email was addressed to a non-user,
    // and that the To: address was actually contained in the username.
    if (email.empty()) {
      email = username;
      username.clear();
    }
    
    // In the Cloud, if the email address validates, ok, else remove this mail from the queue and log the action.
    if (!filter_url_email_is_valid (email)) {
      database_mail.erase (id);
      Database_Logs::log ("Email to " + email + " was deleted because of an invalid email address");
      continue;
    }
    
#endif
    
    // If there had been a crash while sending an email,
    // log this email and erase it.
    // Do not attempt to send it again as it would cause the crash again in an endless loop.
    if (config_globals_has_crashed_while_mailing) {
      database_mail.erase (id);
      Database_Logs::log ("Email to " + email + " with subject \"" + subject + "\" was deleted because it has caused a crash");
      config_globals_has_crashed_while_mailing = false;
      continue;
    }
    
    // Send the email.
    std::string result = email::send (email, username, subject, body);
    if (result.empty ()) {
      database_mail.erase (id);
      std::stringstream ss;
      ss << "Email to " << email << " with subject " << std::quoted(subject) << " was ";
      result = ss.str();
#ifdef HAVE_CLOUD
      result.append ("sent successfully");
#endif
#ifdef HAVE_CLIENT
      result.append ("queued for sending through the Cloud");
#endif
      Database_Logs::log (result, roles::manager);
    } else {
      // Special handling of cases that the smart host denied login.
      bool login_denied = result == "Login denied";
      // Write result to logbook.
      result.insert (0, "Email to " + email + " could not be sent - reason: ");
      Database_Logs::log (result, roles::manager);
      // If the login was denied, then postpone all emails queued for sending,
      // rather than trying to send them all, and have them all cause a 'login denied' error.
      if (login_denied) {
        std::vector <int> ids = database_mail.getAllMails ();
        for (auto id2 : ids) {
          database_mail.postpone (id2);
        }
        Database_Logs::log ("Postponing sending " + std::to_string (ids.size()) + " emails", roles::manager);
        break;
      } else {
        database_mail.postpone (id);
      }
    }
  }
  
  config_globals_mail_send_running = false;
}


static std::vector <std::string> payload_text;


struct upload_status {
  size_t lines_read;
};


#ifdef HAVE_CLIENT
#else
static size_t payload_source (void *ptr, size_t size, size_t nmemb, void *userp)
{
  upload_status *upload_ctx = static_cast <upload_status *> (userp);
  const char *data;
  
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
  
  if (upload_ctx->lines_read >= payload_text.size()) {
    return 0;
  }
  
  data = payload_text[upload_ctx->lines_read].c_str();
  
  size_t len = strlen(data);
  memcpy(ptr, data, len);
  upload_ctx->lines_read++;
  
  return len;
}
#endif


// Sends the email as specified by the parameters.
// If all went well, it returns an empty string.
// In case of failure, it returns the error message.
std::string send ([[maybe_unused]] std::string to_mail,
                        std::string to_name,
                        std::string subject,
                        std::string body,
                        [[maybe_unused]] bool verbose)
{
  // Truncate huge emails because libcurl crashes on it.
  const size_t length = body.length();
  if (length > max_email_size)
    body = "This email was " + std::to_string (length) + " bytes long. It was too long, and could not be sent.";
  
  // Deal with empty subject.
  if (subject.empty ())
    subject = translate ("Bibledit");
  
#ifdef HAVE_CLIENT
  
  if (!client_logic_client_enabled ())
    return std::string();
  
  Webserver_Request webserver_request;
  Sync_Logic sync_logic (webserver_request);
  
  std::map <std::string, std::string> post;
  post ["n"] = filter::string::bin2hex (to_name);
  post ["s"] = subject;
  post ["b"] = body;
  
  std::string address = database::config::general::get_server_address ();
  int port = database::config::general::get_server_port ();
  std::string url = client_logic_url (address, port, sync_mail_url ());
  
  std::string error;
  std::string response = sync_logic.post (post, url, error);
  
  if (!error.empty ()) {
    Database_Logs::log ("Failure sending email: " + error, roles::guest);
  }
  
  return error;
  
#else
  
  std::string from_mail = database::config::general::get_site_mail_address ();
  std::string from_name = database::config::general::get_site_mail_name ();
  
  CURL *curl;
  CURLcode res = CURLE_OK;
  curl_slist * recipients {nullptr};
  upload_status upload_ctx;
  
  upload_ctx.lines_read = 0;
  
  int seconds = filter::date::seconds_since_epoch ();
  payload_text.clear();
  std::string payload;
  payload = "Date: " + std::to_string (filter::date::numerical_year (seconds)) + "/" + std::to_string (filter::date::numerical_month (seconds)) + "/" + std::to_string (filter::date::numerical_month_day (seconds)) + " " + std::to_string (filter::date::numerical_hour (seconds)) + ":" + std::to_string (filter::date::numerical_minute (seconds)) + "\n";
  payload_text.push_back (payload);
  payload = "To: <" + to_mail + "> " + to_name + "\n";
  payload_text.push_back (payload);
  payload = "From: <" + from_mail + "> " + from_name + "\n";
  payload_text.push_back (payload);
  std::string site = from_mail;
  size_t pos = site.find ("@");
  if (pos != std::string::npos) site = site.substr (pos);
  payload = "Message-ID: <" + md5 (std::to_string (filter::string::rand (0, 1000000))) + site + ">\n";
  payload_text.push_back (payload);
  payload = "Subject: " + subject + "\n";
  payload_text.push_back (payload);
  payload_text.push_back ("Mime-version: 1.0\n");
  payload_text.push_back (R"(Content-Type: multipart/alternative; boundary="------------010001060501040600060905")");
  // Empty line to divide headers from body, see RFC5322.
  payload_text.push_back ("\n");
  // Plain text part.
  payload_text.push_back ("--------------010001060501040600060905\n");
  payload_text.push_back ("Content-Type: text/plain; charset=utf-8\n");
  payload_text.push_back ("Content-Transfer-Encoding: 7bit\n");
  payload_text.push_back ("\n");
  payload_text.push_back ("Plain text message.\n");
  payload_text.push_back ("--------------010001060501040600060905\n");
  payload_text.push_back ("Content-Type: text/html; charset=\"utf-8\"\n");
  payload_text.push_back ("Content-Transfer-Encoding: 8bit\n");
  payload_text.push_back ("\n");
  payload_text.push_back ("<!DOCTYPE html>\n");
  payload_text.push_back ("<html>\n");
  payload_text.push_back ("<head>\n");
  payload_text.push_back ("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"></meta>\n");
  payload_text.push_back ("<meta charset=\"utf-8\" />\n");
  payload_text.push_back ("</head>\n");
  payload_text.push_back ("<body>\n");
  std::vector <std::string> bodylines = filter::string::explode (body, '\n');
  for (auto & line : bodylines) {
    if (filter::string::trim (line).empty ()) payload_text.push_back (" ");
    else payload_text.push_back (line);
    payload_text.push_back ("\n");
  }
  payload_text.push_back ("</body>");
  payload_text.push_back ("</html>\n");
  // Empty line.
  payload_text.push_back ("\n");
  
  curl = curl_easy_init();
  /* Set username and password */
  std::string username = database::config::general::get_mail_send_username();
  std::string password = database::config::general::get_mail_send_password();
  curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
  
  /* This is the URL for your mailserver. Note the use of port 587 here,
   * instead of the normal SMTP port (25). Port 587 is commonly used for
   * secure mail submission (see RFC4403), but you should use whatever
   * matches your server configuration. */
  std::string smtp = "smtp://";
  smtp.append (database::config::general::get_mail_send_host());
  smtp.append (":");
  std::string port = database::config::general::get_mail_send_port();
  smtp.append (port);
  curl_easy_setopt(curl, CURLOPT_URL, smtp.c_str());
  
  /* In this example, we'll start with a plain text connection, and upgrade
   * to Transport Layer Security (TLS) using the STARTTLS command. Be careful
   * of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
   * will continue anyway - see the security discussion in the libcurl
   * tutorial for more details. */
  if (port != "25") curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
  
  /* If your server doesn't have a valid certificate, then you can disable
   * part of the Transport Layer Security protection by setting the
   * CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).
   */
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  /*
   * That is, in general, a bad idea. It is still better than sending your
   * authentication details in plain text though.
   * Instead, you should get the issuer certificate (or the host certificate
   * if the certificate is self-signed) and add it to the set of certificates
   * that are known to libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See
   * docs/SSLCERTS for more information. */
  //curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
  
  /* Note that this option isn't strictly required, omitting it will result in
   * libcurl sending the MAIL FROM command with empty sender data. All
   * autoresponses should have an empty reverse-path, and should be directed
   * to the address in the reverse-path which triggered them. Otherwise, they
   * could cause an endless loop. See RFC 5321 Section 4.5.5 for more details.
   */
  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_mail.c_str());
  
  /* Add the recipients, in this particular case they correspond to the
   * To: addressee in the header, but they could be any kind of recipient. */
  recipients = curl_slist_append(recipients, to_mail.c_str());
  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
  
  /* We're using a callback function to specify the payload (the headers and
   * body of the message). You could just use the CURLOPT_READDATA option to
   * specify a FILE pointer to read from. */
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
  curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
  
  // Since the traffic will be encrypted, it is very useful to turn on debug
  // information within libcurl to see what is happening during the transfer.
  if (verbose) {
    curl_easy_setopt (curl, CURLOPT_DEBUGFUNCTION, filter_url_curl_debug_callback);
    curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
  }
  
  // Timeout values.
  filter_url_curl_set_timeout (curl);
  
  /* Send the message */
  res = curl_easy_perform(curl);
  
  /* Check for errors */
  std::string result;
  if (res != CURLE_OK) result = curl_easy_strerror (res);
  
  /* Free the list of recipients */
  curl_slist_free_all(recipients);
  
  /* Always cleanup */
  curl_easy_cleanup(curl);
  
  return result;
#endif
}


void schedule (std::string to, std::string subject, std::string body, int time)
{
  // Schedule the mail for sending.
  Webserver_Request webserver_request;
  Database_Mail database_mail (webserver_request);
  database_mail.send (to, subject, body, time);
  // Schedule a task to send the mail right away.
  // If the task is already scheduled, don't schedule one more, to avoid plenty of similar tasks.
  if (!tasks_logic_queued (task::send_email))
    tasks_logic_queue (task::send_email);
}


// If the email sending and receiving has not yet been (completely) set up,
// it returns information about that.
// If everything's OK, it returns nothing.
std::string setup_information (bool require_send, bool require_receive)
{
#ifdef HAVE_CLIENT
  (void) require_send;
  (void) require_receive;
#endif
#ifdef HAVE_CLOUD
  bool incomplete = false;
  if (database::config::general::get_site_mail_name ().empty ()) incomplete = true;
  if (database::config::general::get_site_mail_address ().empty ()) incomplete = true;
  if (require_receive) {
    if (database::config::general::get_mail_storage_host ().empty ()) incomplete = true;
    if (database::config::general::get_mail_storage_username ().empty ()) incomplete = true;
    if (database::config::general::get_mail_storage_password ().empty ()) incomplete = true;
    if (database::config::general::get_mail_storage_protocol ().empty ()) incomplete = true;
    if (database::config::general::get_mail_storage_port ().empty ()) incomplete = true;
  }
  if (require_send) {
    if (database::config::general::get_mail_send_host ().empty ()) incomplete = true;
    if (database::config::general::get_mail_send_username ().empty ()) incomplete = true;
    if (database::config::general::get_mail_send_password ().empty ()) incomplete = true;
    if (database::config::general::get_mail_send_port ().empty ()) incomplete = true;
  }
  if (incomplete) {
    std::string msg1 = translate ("Cannot send email yet.");
    std::string msg2 = translate ("The emailer is not yet set up.");
    return msg1 + R"( <a href="../)" + email_index_url () + + R"(">)" + msg2 + "</a>";
  }
#endif
  return std::string();
}


} // End namespace.

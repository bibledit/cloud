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
#ifndef HAVE_CLIENT
#include <curl/curl.h>
#endif
#include <config/globals.h>
#include <tasks/logic.h>
#include <sync/mail.h>
#include <sync/logic.h>
#include <client/logic.h>
#include <locale/translate.h>


void email_send ()
{
  // No more than one email send process to run simultaneously.
  if (config_globals_mail_send_running) return;
  config_globals_mail_send_running = true;

  // The databases involved.
  Webserver_Request request;
  Database_Mail database_mail = Database_Mail (&request);
  Database_Users database_users;

  vector <int> mails = database_mail.getMailsToSend ();
  for (auto id : mails) {

    // Get all details of the mail.
    Database_Mail_Item details = database_mail.get (id);
    string username = details.username;
    string email = database_users.get_email (username);
    string subject = details.subject;
    string body = details.body;
    
#ifdef HAVE_CLIENT
    
    // On a client, set the email to the username before sending the email to the Cloud for further distribution.
    email = username;
    
#else

    // In the Cloud, if this username was not found, it could be that the email was addressed to a non-user,
    // and that the To: address was actually contained in the username.
    if (email == "") {
      email = username;
      username = "";
    }
    
    // In the Cloud, if the email address validates, ok, else remove this mail from the queue and log the action.
    if (!filter_url_email_is_valid (email)) {
      database_mail.erase (id);
      Database_Logs::log ("Email to " + email + " was deleted because of an invalid email address");
      continue;
    }

#endif
  
    // Send the email.
    string result = email_send (email, username, subject, body);
    if (result.empty ()) {
      database_mail.erase (id);
      result = "Email to " + email + " with subject " + subject + " was sent successfully";
    } else {
      database_mail.postpone (id);
      result.insert (0, "Email to " + email + " could not be sent - reason: ");
    }
    Database_Logs::log (result, Filter_Roles::manager ());
  }
  
  config_globals_mail_send_running = false;
}


static vector <string> payload_text;


struct upload_status {
  int lines_read;
};


#ifdef HAVE_CLIENT
#else
static size_t payload_source (void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
  
  if (upload_ctx->lines_read >= (int)payload_text.size()) {
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
string email_send (string to_mail, string to_name, string subject, string body, bool verbose)
{
  // Truncate huge emails because libcurl crashes on it.
  int length = body.length ();
  if (length > 100000) {
    body = "This email was " + convert_to_string (length) + " bytes long. It was too long, and could not be sent.";
  }
  
  // Deal with empty subject.
  if (subject.empty ()) subject = translate ("Bibledit");
  
#ifdef HAVE_CLIENT

  if (!client_logic_client_enabled ()) {
    return "";
  }
  
  (void) verbose;
  (void) to_mail;

  Webserver_Request request;
  Sync_Logic sync_logic = Sync_Logic (&request);

  map <string, string> post;
  post ["n"] = bin2hex (to_name);
  post ["s"] = subject;
  post ["b"] = body;

  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_mail_url ());

  string error;
  string response = sync_logic.post (post, url, error);

  if (!error.empty ()) {
    Database_Logs::log ("Failure sending email: " + error, Filter_Roles::guest ());
  }

  return error;
  
#else
  
  string from_mail = Database_Config_General::getSiteMailAddress ();
  string from_name = Database_Config_General::getSiteMailName ();
  
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;

  upload_ctx.lines_read = 0;

  int seconds = filter_date_seconds_since_epoch ();
  payload_text.clear();
  string payload;
  payload = "Date: " + convert_to_string (filter_date_numerical_year (seconds)) + "/" + convert_to_string (filter_date_numerical_month (seconds)) + "/" + convert_to_string (filter_date_numerical_month_day (seconds)) + " " + convert_to_string (filter_date_numerical_hour (seconds)) + ":" + convert_to_string (filter_date_numerical_minute (seconds)) + "\n";
  payload_text.push_back (payload);
  payload = "To: <" + to_mail + "> " + to_name + "\n";
  payload_text.push_back (payload);
  payload = "From: <" + from_mail + "> " + from_name + "\n";
  payload_text.push_back (payload);
  string site = from_mail;
  size_t pos = site.find ("@");
  if (pos != string::npos) site = site.substr (pos);
  payload = "Message-ID: <" + md5 (convert_to_string (filter_string_rand (0, 1000000))) + site + ">\n";
  payload_text.push_back (payload);
  payload = "Subject: " + subject + "\n";
  payload_text.push_back (payload);
  payload_text.push_back ("Mime-version: 1.0\n");
  payload_text.push_back ("Content-Type: multipart/alternative; boundary=\"------------010001060501040600060905\"");
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
  vector <string> bodylines = filter_string_explode (body, '\n');
  for (auto & line : bodylines) {
    if (filter_string_trim (line).empty ()) payload_text.push_back (" ");
    else payload_text.push_back (line);
    payload_text.push_back ("\n");
  }
  payload_text.push_back ("</body>");
  payload_text.push_back ("</html>\n");
  // Empty line.
  payload_text.push_back ("\n");

  curl = curl_easy_init();
  /* Set username and password */
  curl_easy_setopt(curl, CURLOPT_USERNAME, Database_Config_General::getMailSendUsername().c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, Database_Config_General::getMailSendPassword().c_str());

  /* This is the URL for your mailserver. Note the use of port 587 here,
   * instead of the normal SMTP port (25). Port 587 is commonly used for
   * secure mail submission (see RFC4403), but you should use whatever
   * matches your server configuration. */
  string smtp = "smtp://";
  smtp.append (Database_Config_General::getMailSendHost());
  smtp.append (":");
  smtp.append (Database_Config_General::getMailSendPort());
  curl_easy_setopt(curl, CURLOPT_URL, smtp.c_str());

  /* In this example, we'll start with a plain text connection, and upgrade
   * to Transport Layer Security (TLS) using the STARTTLS command. Be careful
   * of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
   * will continue anyway - see the security discussion in the libcurl
   * tutorial for more details. */
  curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

  /* If your server doesn't have a valid certificate, then you can disable
   * part of the Transport Layer Security protection by setting the
   * CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).
   *   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
   *   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
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
  string result;
  if (res != CURLE_OK) result = curl_easy_strerror (res);

  /* Free the list of recipients */
  curl_slist_free_all(recipients);

  /* Always cleanup */
  curl_easy_cleanup(curl);

  return result;
#endif
}


void email_schedule (string to, string subject, string body, int time)
{
  // Schedule the mail for sending.
  Database_Mail database_mail (NULL);
  database_mail.send (to, subject, body, time);
  // Schedule a task to send the scheduled mail right away.
  tasks_logic_queue (SENDEMAIL);
}


// If the email sending and receiving has not yet been (completely) set up,
// it returns information about that.
// If everything's OK, it returns nothing.
string email_setup_information ()
{
#ifdef HAVE_CLOUD
  int missing_items = 0;
  if (Database_Config_General::getSiteMailName ().empty ()) missing_items++;
  if (Database_Config_General::getSiteMailAddress ().empty ()) missing_items++;
  if (Database_Config_General::getMailStorageHost ().empty ()) missing_items++;
  if (Database_Config_General::getMailStorageUsername ().empty ()) missing_items++;
  if (Database_Config_General::getMailStoragePassword ().empty ()) missing_items++;
  if (Database_Config_General::getMailStorageProtocol ().empty ()) missing_items++;
  if (Database_Config_General::getMailStoragePort ().empty ()) missing_items++;
  if (Database_Config_General::getMailSendHost ().empty ()) missing_items++;
  if (Database_Config_General::getMailSendUsername ().empty ()) missing_items++;
  if (Database_Config_General::getMailSendPassword ().empty ()) missing_items++;
  if (Database_Config_General::getMailSendPort ().empty ()) missing_items++;
  if (missing_items) {
    string msg1 = translate ("Cannot send email yet.");
    string msg2 = translate ("The emailer is not yet set up.");
    return msg1 + " <a href=\"../" + email_index_url () + + "\">" + msg2 + "</a>";
  }
#endif
  return "";
}

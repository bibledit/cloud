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


#include <email/receive.h>
#include <database/logs.h>
#ifndef HAVE_CLIENT
#include <curl/curl.h>
#endif
#include <database/config/general.h>
#include <filter/string.h>
#include <filter/mail.h>
#include <config/globals.h>
#include <notes/logic.h>
#include <filter/url.h>


namespace email {


void receive ()
{
#ifdef HAVE_CLOUD
  // Bail out when the mail storage host has not been defined, rather than giving an error message.
  if (database::config::general::get_mail_storage_host () == "") return;
  
  // One email receiver runs at a time.
  if (config_globals_mail_receive_running) return;
  config_globals_mail_receive_running = true;
  // Once this flag is set, the coder should be careful not to exit from the function
  // without clearin this flag.
  
  // Email count.
  std::string error;
  int emailcount = email::receive_count (error);
  // Messages start at number 1 instead of 0.
  for (int i = 1; i <= emailcount; i++) {
    
    Webserver_Request webserver_request;
    Notes_Logic notes_logic (webserver_request);
    
    error.clear ();
    std::string message = email::receive_message (error);
    if (error.empty ()) {
      
      // Extract "from" and subject, and clean body.
      std::string from;
      std::string subject;
      std::string body;
      filter_mail_dissect (message, from, subject, body);
      
      Database_Logs::log ("Processing email from " + from + " with subject " + subject);
      
      if (notes_logic.handleEmailComment (from, subject, body)) {
      }
      else if (notes_logic.handleEmailNew (from, subject, body)) {
      }
      else {
        Database_Logs::log ("Could not allocate email from " + from + ", subject " + subject);
        Database_Logs::log (body);
      }
      
    } else {
      Database_Logs::log ("Error retrieving mail: " + error);
    }
    
  }
  
  config_globals_mail_receive_running = false;
#endif
}


struct cstring {
  char *ptr;
  size_t len;
};


void init_string (cstring *s) {
  s->len = 0;
  s->ptr = static_cast<char *>(malloc(s->len+1));
  s->ptr[0] = '\0';
}


size_t writefunc(void *ptr, size_t size, size_t nmemb, cstring *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = static_cast<char *>(realloc (s->ptr, new_len+1));
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}


std::string url ()
{
  std::string url;
  const char * pop3s = "POP3S";
  if (database::config::general::get_mail_storage_protocol() == pop3s) url.append (pop3s);
  else url.append ("pop3");
  url.append ("://");
  url.append (database::config::general::get_mail_storage_host ());
  url.append (":");
  url.append (database::config::general::get_mail_storage_port ());
  return url;
}


// Returns how many emails are waiting in the mail storage host's POP3 email inbox.
int receive_count (std::string& error, bool verbose)
{
#ifdef HAVE_CLIENT
  error = "Not implemented with embedded http library";
  if (verbose) {}
  return 0;
#endif
  
#ifdef HAVE_CLOUD
  CURL *curl;
  CURLcode res = CURLE_OK;
  
  cstring s;
  init_string (&s);
  
  curl = curl_easy_init ();
  
  curl_easy_setopt (curl, CURLOPT_USERNAME, database::config::general::get_mail_storage_username ().c_str());
  curl_easy_setopt (curl, CURLOPT_PASSWORD, database::config::general::get_mail_storage_password ().c_str());
  
  curl_easy_setopt (curl, CURLOPT_URL, url ().c_str());
  
  curl_easy_setopt (curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
  
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writefunc);
  
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &s);
  
  if (verbose) {
    curl_easy_setopt (curl, CURLOPT_DEBUGFUNCTION, filter_url_curl_debug_callback);
    curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
  }
  
  // Some servers need this validation.
  curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  
  filter_url_curl_set_timeout (curl);
  
  res = curl_easy_perform (curl);
  
  int mailcount = 0;
  
  if (res == CURLE_OK) {
    if (s.ptr) {
      std::string response = s.ptr;
      response = filter::strings::trim (response);
      mailcount = static_cast<int>(filter::strings::explode (response, '\n').size());
    }
  } else {
    error = curl_easy_strerror (res);
  }
  
  if (s.ptr) free (s.ptr);
  
  curl_easy_cleanup (curl);
  
  return mailcount;
#endif
}


std::string receive_message (std::string& error)
{
#ifdef HAVE_CLIENT
  error = "Not implemented with embedded http library";
  return std::string();
#endif
  
#ifdef HAVE_CLOUD
  CURL *curl;
  CURLcode res = CURLE_OK;
  
  cstring s;
  init_string (&s);
  
  curl = curl_easy_init ();
  
  curl_easy_setopt (curl, CURLOPT_USERNAME, database::config::general::get_mail_storage_username ().c_str());
  curl_easy_setopt (curl, CURLOPT_PASSWORD, database::config::general::get_mail_storage_password ().c_str());
  
  std::string message_url = url () + "/1";
  curl_easy_setopt (curl, CURLOPT_URL, message_url.c_str());
  
  curl_easy_setopt (curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
  
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writefunc);
  
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &s);
  
  // Some servers need this validation.
  curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  
  filter_url_curl_set_timeout (curl);
  
  res = curl_easy_perform (curl);
  
  std::string body {};
  
  if (res == CURLE_OK) {
    if (s.ptr) body = s.ptr;
  } else {
    error = curl_easy_strerror (res);
  }
  
  // Set the DELE command.
  curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "DELE");
  
  // Do not perform a transfer as DELE returns no data.
  curl_easy_setopt (curl, CURLOPT_NOBODY, 1L);
  
  // Perform the custom request.
  res = curl_easy_perform(curl);
  
  if (s.ptr) free (s.ptr);
  
  curl_easy_cleanup (curl);
  
  return body;
#endif
}


} // End namespace.

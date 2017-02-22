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


#include <email/receive.h>
#include <database/logs.h>
#ifndef HAVE_CLIENT
#include <curl/curl.h>
#endif
#include <database/config/general.h>
#include <filter/string.h>
#include <config/globals.h>
#include <confirm/worker.h>
#include <notes/logic.h>
#include <filter/url.h>


// Dissects a raw email.
void email_dissect (string & body, string & from, string & subject)
{
  // Remove \r, remaining with \n only.
  body = filter_string_str_replace ("\r", "", body);

  vector <string> lines = filter_string_explode (body, '\n');
  
  // Extraction of the plain text from the email.
  // It works like this:
  // 1. Look for a line that contains: Content-Type: text/plain.
  // 2. Next look for an empty line.
  // 3. Now the relevant content starts.
  // The relevant content runs till the line that starts with "--" or to the end of the mail body.
  int plaintext_trigger = 0;
  body.clear ();
  
  for (auto & line : lines) {
    // Find the email address of the sender from a line like this:
    // From: "Name of Sender" <aanbieding@e-mail.marktplaats.nl>
    if (from.empty()) {
      if (line.find ("From: ") != string::npos) {
        from = filter_string_extract_email (line);
      }
    }
    // Find the subject from a line like this:
    // Subject: This is is a message to Bibledit.
    if (subject.empty()) {
      if (line.find ("Subject: ") != string::npos) {
        subject = line.substr (9);
        subject = filter_string_trim (subject);
      }
    }
    // Find the relevant text: The contents within the text/plain portion of the email body.
    if (plaintext_trigger == 2) {
      if (line.substr (0, 2) == "--") {
        plaintext_trigger++;
      }
    }
    if (plaintext_trigger == 2) {
      // If the line starts with ">", it indicates quoted text: Skip it.
      if (!line.empty ()) if (line.substr (0, 1) == ">") continue;
      // Handle the "=" at the end of a line so it properly connects to the previous line.
      if (line.empty ()) {
        body.append ("\n");
      } else {
        if (line.back () == '=') {
          line.pop_back ();
        } else {
          body.append ("\n");
        }
      }
      body.append (line);
    }
    if (plaintext_trigger == 0) {
      if ((line.find ("Content-Type:") != string::npos) && (line.find ("text/plain") != string::npos)) {
        plaintext_trigger++;
      }
    }
    if (plaintext_trigger == 1) {
      if (filter_string_trim (line).empty()) {
        plaintext_trigger++;
      }
    }
  }
}


void email_receive ()
{
#ifdef HAVE_CLOUD
  // Bail out when the mail storage host has not been defined, rather than giving an error message.
  if (Database_Config_General::getMailStorageHost () == "") return;
  
  // One email receiver runs at a time.
  if (config_globals_mail_receive_running) return;
  config_globals_mail_receive_running = true;
  // Once this flag is set, the coder should be careful not to exit from the function
  // without clearin this flag.
  
  // Email count.
  string error;
  int emailcount = email_receive_count (error);
  // Messages start at number 1 instead of 0.
  for (int i = 1; i <= emailcount; i++) {

    Webserver_Request request;
    Confirm_Worker confirm_worker = Confirm_Worker (&request);
    Notes_Logic notes_logic = Notes_Logic (&request);
    
    error.clear ();
    string body = email_receive_message (error);
    if (error.empty ()) {
  
      // Extract "from" and subject, and clean body.
      string from;
      string subject;
      email_dissect (body, from, subject);
  
      Database_Logs::log ("Processing email from " + from + " with subject " + subject);

      if (confirm_worker.handleEmail (from, subject, body)) {
      }
      else if (notes_logic.handleEmailComment (from, subject, body)) {
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


void init_string (struct cstring *s) {
  s->len = 0;
  s->ptr = (char *) malloc(s->len+1);
  s->ptr[0] = '\0';
}


size_t writefunc(void *ptr, size_t size, size_t nmemb, struct cstring *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char *) realloc (s->ptr, new_len+1);
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}


string url ()
{
  string url;
  const char * pop3s = "POP3S";
  if (Database_Config_General::getMailStorageProtocol() == pop3s) url.append (pop3s);
  else url.append ("pop3");
  url.append ("://");
  url.append (Database_Config_General::getMailStorageHost ());
  url.append (":");
  url.append (Database_Config_General::getMailStoragePort ());
  return url;
}


// Returns how many emails are waiting in the mail storage host's POP3 email inbox.
int email_receive_count (string& error, bool verbose)
{
#ifdef HAVE_CLIENT
  error = "Not implemented with embedded http library";
  if (verbose) {}
  return 0;
#else
  CURL *curl;
  CURLcode res = CURLE_OK;

  struct cstring s;
  init_string (&s);

  curl = curl_easy_init ();

  curl_easy_setopt (curl, CURLOPT_USERNAME, Database_Config_General::getMailStorageUsername ().c_str());
  curl_easy_setopt (curl, CURLOPT_PASSWORD, Database_Config_General::getMailStoragePassword ().c_str());

  curl_easy_setopt (curl, CURLOPT_URL, url ().c_str());

  curl_easy_setopt (curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
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
    string response = (char *) s.ptr;
    response = filter_string_trim (response);
    mailcount = filter_string_explode (response, '\n').size();
  } else {
    error = curl_easy_strerror (res);
  }

  if (s.ptr) free (s.ptr);

  curl_easy_cleanup (curl);

  return mailcount;
#endif
}


string email_receive_message (string& error)
{
#ifdef HAVE_CLIENT
  error = "Not implemented with embedded http library";
  return "";
#else
  CURL *curl;
  CURLcode res = CURLE_OK;

  struct cstring s;
  init_string (&s);

  curl = curl_easy_init ();

  curl_easy_setopt (curl, CURLOPT_USERNAME, Database_Config_General::getMailStorageUsername ().c_str());
  curl_easy_setopt (curl, CURLOPT_PASSWORD, Database_Config_General::getMailStoragePassword ().c_str());

  string message_url = url () + "/1";
  curl_easy_setopt (curl, CURLOPT_URL, message_url.c_str());

  curl_easy_setopt (curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0); 
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0); 

  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writefunc);

  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &s);

  // Some servers need this validation.
  curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  filter_url_curl_set_timeout (curl);
  
  res = curl_easy_perform (curl);

  string body;
  
  if (res == CURLE_OK) {
    body = (char *) s.ptr;
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

/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <webserver/http.h>
#include <vector>
#include <sstream>
#include <filter/url.h>
#include <config/globals.h>
#include <filter/string.h>
#include <parsewebdata/ParseWebData.h>
#include <webserver/request.h>



// The http headers from a browser could look as follows:
//
// GET /index/page HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// User-Agent: Mozilla/n.0 (X11; Linux x86_64) AppleWebKit/nnn.nn (KHTML, like Gecko) Chrome/3nn.n.nnnn.nnn Safari/nnn.nn
// Accept-Language: sn,en-US;q=0.8,en;q=0.6
// Cookie: Session=abcdefghijklmnopqrstuvwxyz; foo=bar; extra=clutter
//
// The function extracts the relevant information from the headers.
//
// It returns true if a header was (or could have been) parsed.
bool http_parse_header (string header, void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  // Clean the header line.
  header = filter_string_trim (header);

  // Deal with a header like this: GET /css/stylesheet.css?1.0.1 HTTP/1.1
  // Or like this: POST /session/login?request= HTTP/1.1
  bool get = false;
  if (header.substr (0, 3) == "GET") get = true;
  if (header.substr (0, 4) == "POST") {
    get = true;
    request->is_post = true;
  }
  if (get) {
    string query_data;
    vector <string> get = filter_string_explode (header, ' ');
    if (get.size () >= 2) {
      request->get = get [1];
      // The GET or POST value may be, for example: stylesheet.css?1.0.1.
      // Split it up into two parts: The part before the ?, and the part after the ?.
      istringstream issquery (request->get);
      int counter = 0;
      string s;
      while (getline (issquery, s, '?')) {
        if (counter == 0) request->get = s;
        if (counter == 1) query_data = s;
        counter++;
      }
    }
    // Read and parse the GET data.
    try {
      if (!query_data.empty ()) {
        ParseWebData::WebDataMap dataMap;
        ParseWebData::parse_get_data (query_data, dataMap);
        for (ParseWebData::WebDataMap::const_iterator iter = dataMap.begin(); iter != dataMap.end(); ++iter) {
          request->query [(*iter).first] = filter_url_urldecode ((*iter).second.value);
        }
      }
    } catch (...) {
    }
  }
  
  // Extract the User-Agent from a header like this:
  // User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.104 Safari/537.36
  if (header.substr (0, 10) == "User-Agent") {
    request->user_agent = header.substr (12);
  }
  
  // Extract the Accept-Language from a header like this:
  // Accept-Language: sn,en-US;q=0.8,en;q=0.6
  if (header.substr (0, 15) == "Accept-Language") {
    request->accept_language = header.substr (17);
  }
  
  // Extract the host from headers like this:
  // Host: 192.168.1.139:8080
  // Host: [::1]:8080
  // Host: localhost:8080
  // Host: <empty>
  if (header.substr (0, 4) == "Host") {
    if (header.size () >= 6) {
      request->host = http_parse_host (header.substr (6));
    }
  }
  
  // Extract the Content-Type from a header like this:
  // Content-Type: application/x-www-form-urlencoded
  if (header.substr (0, 12) == "Content-Type") {
    request->content_type = header.substr (14);
  }

  // Extract the Content-Length from a header.
  if (header.substr (0, 14) == "Content-Length") {
    request->content_length = convert_to_int (header.substr (16));
  }
  
  // Extract the ETag from a header.
  if (header.substr (0, 13) == "If-None-Match") {
    request->if_none_match = header.substr (15);
  }

  // Extract the relevant cookie.
  // When the browser has more than one cookies, it sends them all, e.g.:
  // Cookie: Session=abcdefghijklmnopqrstuvwxyz; foo=bar; extra=clutter
  // Bibledit is only interested in the "Session" one.
  if (header.substr (0, 6) == "Cookie") {
    string cookie_data = header.substr (8);
    size_t pos = cookie_data.find ("Session=");
    if (pos != string::npos) {
      cookie_data.erase (0, pos + 8);
      pos = cookie_data.find (";");
      if (pos != string::npos) {
        cookie_data.erase (pos);
      }
    }
    request->session_identifier = cookie_data;
  }
  
  // Something was or could have been parsed if the header contained something.
  return !header.empty ();
}


// Takes data POSTed from the browser, and parses it.
void http_parse_post (string content, void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // Read and parse the POST data.
  try {
    if (!content.empty ()) {
      // Standard parse.
      bool urlencoded = request->content_type.find ("urlencoded") != string::npos;
      ParseWebData::WebDataMap dataMap;
      ParseWebData::parse_post_data (content, request->content_type, dataMap);
      for (ParseWebData::WebDataMap::const_iterator iter = dataMap.begin(); iter != dataMap.end(); ++iter) {
        string value;
        if (urlencoded) value = filter_url_urldecode ((*iter).second.value);
        else value = (*iter).second.value;
        request->post [(*iter).first] = value;
      }
      // Special case: Extract the filename in case of a file upload.
      if (content.length () > 1000) content.resize (1000);
      if (content.find ("filename=") != string::npos) {
        vector <string> lines = filter_string_explode (content, '\n');
        for (auto & line : lines) {
          if (line.find ("Content-Disposition") == string::npos) continue;
          size_t pos = line.find ("filename=");
          if (pos == string::npos) continue;
          line = line.substr (pos + 10);
          line = filter_string_trim (line);
          line.pop_back ();
          request->post ["filename"] = line;
        }
      }
    }
  } catch (...) {
  }
}


/*
The function assembles the response to be given to the browser.
code: an integer response code, normally this is 200.
header: An extra header to be sent with the response. May be empty.
contents: the response body to be sent.
The function inserts the correct headers,
and creates the entire result to be sent back to the browser.
*/
void http_assemble_response (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  ostringstream length;
  if (request->stream_file.empty()) {
    // Serving data: Take the length from the size of the reply.
    length << request->reply.size ();
  } else {
    // Streaming a file: Take the file size as the length.
    length << filter_url_filesize (request->stream_file);
  }

  // Assemble the HTTP response code fragment.
  string http_response_code_fragment = filter_url_http_response_code_text (request->response_code);
  
  // Assemble the Content-Type.
  string extension = filter_url_get_extension (request->get);
  extension = unicode_string_casefold (extension);
  string content_type = filter_url_get_mime_type (extension);
  if (extension == "usfm") content_type = "text/plain";
  if (extension.empty()) content_type = "text/html";
  if (extension == "sh") content_type = "application/octet-stream";
  if (extension == "sqlite") content_type = "application/octet-stream";
  if (extension == "download") content_type = "application/octet-stream";
  // If still empty, take the default binary content type.
  if (content_type.empty()) content_type = "application/octet-stream";
  // If already defined, take that.
  if (!request->response_content_type.empty ()) content_type = request->response_content_type;

  // Assemble the complete response for the browser.
  vector <string> response;
  response.push_back ("HTTP/1.1 " + http_response_code_fragment);
  response.push_back ("Accept-Ranges: bytes");
  response.push_back ("Content-Length: " + length.str());
  response.push_back ("Content-Type: " + content_type);
  if (!request->etag.empty ()) {
    response.push_back ("Cache-Control: max-age=120");
    response.push_back ("ETag: " + request->etag);
  }
  if (request->session_identifier.empty () || request->resend_cookie) {
    // If the browser did not send a cookie to the server, the server sends a new one to the browser.
    // Once a day, it resends the existing one, to refresh it in the browser.
    
    // The cookie consists of the following components:
    // * Name
    // * Value
    // * Zero or more attributes
    
    // Create a new identifier, and stores it in the cookie as name "Session".
    // The identifier is a long string of random letters and numbers.
    
    // The Path is given as / because typically the client connects to e.g. bibledit.org:8080/.
    // It would be good if the port number can be used with cookies also,
    // but it can't, see https://tools.ietf.org/html/rfc6265.

    // The Max-Age attribute is used to set the cookie’s expiration
    // as an interval of seconds in the future,
    // relative to the time the browser received the cookie.
    // If the expiry information were omitted from the cookie, closing the browser would remove the cookie.
    
    // The Secure attribute could be used, but it is not currently used.
    // It is meant to keep cookie communication limited to encrypted transmission,
    // directing browsers to use cookies only via secure/encrypted connections.
    // For maximum security, cookies with the Secure attribute should only be set over a secure connection.

    // The HttpOnly attribute means that the cookie can be accessed by the HTTP API only,
    // and not by for example Javascript running in the browser.
    // This provides extra security.

    string identifier = request->session_identifier;
    if (identifier.empty ()) identifier = get_new_random_string ();
    string cookie = "Session=" + identifier + "; Path=/; Max-Age=2678400; HttpOnly";
    response.push_back ("Set-Cookie: " + cookie);
  }
  if (!request->header.empty ()) response.push_back (request->header);
  response.push_back ("");
  if (request->stream_file.empty()) {
    // Serving data: Add the data from the "reply" property.
    response.push_back (request->reply);
  } else {
    // Streaming a file: Add empty string.
    // This triggers adding the new line between headers and body.
    response.push_back ("");
  }
  
  string assembly;
  for (unsigned int i = 0; i < response.size (); i++) {
    if (i > 0) assembly += "\n";
    assembly += response [i];
  }
  request->reply = assembly;
}


// This function serves a file and enables caching by the browser.
// It enables streaming the file straight from disk to the network connection,
// without loading it in memory first.
// By doing so, it uses little memory, independent from the size of the file it serves.
// $enable_cache: Whether to enable caching by the browser.
void http_stream_file (void * webserver_request, bool enable_cache)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  // Full path to the file.
  string url = filter_url_urldecode (request->get);
  string filename = filter_url_create_root_path (url);
  
  // File size for browser caching.
  if (enable_cache) {
    int size = filter_url_filesize (filename);
    request->etag = "\"" + convert_to_string (size) + "\"";
  }
  
  // Deal with situation that the file in the browser's cache is up to date.
  // https://developers.google.com/web/fundamentals/performance/optimizing-content-efficiency/http-caching
  if (enable_cache) {
    if (request->etag == request->if_none_match) {
      request->response_code = 304;
      return;
    }
  }
  
  // Store the file name as a flag for processing the streaming.
  request->stream_file = filename;
}


// Obtain the host name from lines like this:
// 192.168.1.139:8080
// localhost:8080
// [::1]:8080
// [fe80::601:25ff:fe07:6801]:8080
string http_parse_host (const string & line)
{
  string host;

  size_t ipv6_opener = line.find ("[");
  size_t ipv6_closer = line.find ("]");
  if ((ipv6_opener != string::npos) && (ipv6_closer != string::npos)) {
    // Square brackets? That's IPv6.
    host = line.substr (0, ++ipv6_closer);
  } else {
    // IPv4.
    host = line;
    size_t pos = line.find (":");
    if (pos != string::npos) {
      host = host.substr (0, pos);
    }
  }

  return host;
}

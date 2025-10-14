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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <webserver/http.h>
#include <webserver/request.h>
#include <filter/url.h>


TEST (http, parse_host)
{
  std::string host;
  std::string line;
  
  line = "192.168.1.139:8080";
  host = http_parse_host (line);
  EXPECT_EQ ("192.168.1.139", host);

  line = "localhost:8080";
  host = http_parse_host (line);
  EXPECT_EQ ("localhost", host);

  line = "[::1]:8080";
  host = http_parse_host (line);
  EXPECT_EQ ("[::1]", host);

  line = "[fe80::601:25ff:fe07:6801]:8080";
  host = http_parse_host (line);
  EXPECT_EQ ("[fe80::601:25ff:fe07:6801]", host);
}


TEST (http, parse_post)
{
  using container = std::vector<std::pair<std::string,std::string>>;
  
  // Test POST data of type application/x-www-form-urlencoded.
  for (const std::string& content_type : {
    std::string(application_x_www_form_urlencoded),
    std::string(application_x_www_form_urlencoded) + "; charset=UTF-8",
  }) {
    Webserver_Request webserver_request{};
    webserver_request.content_type = content_type;
    {
      const std::string posted {"key=value"};
      container standard = {
        {"key", "value"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    {
      const std::string posted {"key1=value1&key2=value2"};
      container standard = {
        {"key1", "value1"},
        {"key2", "value2"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    {
      const std::string posted {"key1&key2"};
      container standard = {
        {"key1", ""},
        {"key2", ""},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    {
      const std::string posted {"key=Hello+World"};
      container standard = {
        {"key", "Hello World"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    {
      const std::string posted {"key=H%C3%ABllo+W%C3%B6rld"};
      container standard = {
        {"key", "Hëllo Wörld"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
  }
  
  // Test POSTed content type of text/plain.
  {
    Webserver_Request webserver_request{};
    webserver_request.content_type = text_plain;
    
    {
      const std::string posted {"key=value\r\n"};
      container standard = {
        {"key", "value"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    
    {
      const std::string posted {
        "key1=value1\r\n"
        "key2=value2\r\n"
      };
      container standard = {
        {"key1", "value1"},
        {"key2", "value2"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    
    {
      const std::string posted {
        "key1\r\n"
        "key2\r\n"
      };
      container standard = {
        {"key1", ""},
        {"key2", ""},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    
    {
      const std::string posted {
        "key=Hello World\r\n"
      };
      container standard = {
        {"key", "Hello World"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
    
    {
      const std::string posted {
        "key=Hëllo Wörld\r\n"
      };
      container standard = {
        {"key", "Hëllo Wörld"},
      };
      http_parse_post (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post);
    }
  }
  
  // Test POSTed content of type multipart/form-data.
  {
    const std::string test_path {"unittests/tests/"};
    const std::string boundary {"----WebKitFormBoundary1234abcd"};
    const std::string content_type = std::string(multipart_form_data) + "; boundary=" + boundary;
    {
      // Test a single-file upload.
      {
        const std::string content = filter_url_file_get_contents(test_path + "http-post-1.txt");
        Webserver_Request webserver_request{};
        webserver_request.content_type = content_type;
        http_parse_post (content, webserver_request);
        const container standard_new {
          {"filename", "00_test1.txt"},
          {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
          {"upload", "Upload"}
        };
        EXPECT_EQ (webserver_request.post, standard_new);
      }
    }
    {
      // Test a multiple-file upload.
      const std::string content = filter_url_file_get_contents(test_path + "http-post-2.txt");
      Webserver_Request webserver_request{};
      webserver_request.content_type = content_type;
      http_parse_post (content, webserver_request);
      const container standard {
        {"filename", "00_test1.txt"},
        {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
        {"filename", "00_test2.txt"},
        {"data", "Contents for test2.\nLine one 2.\nLine two 2.\nLine three 2.\n"},
        {"filename", "00_test3.txt"},
        {"data", "Contents for test3.\nLine one 3.\nLine two 3.\nLine three 3.\n"},
        {"upload", "Upload"}
      };
      EXPECT_EQ (webserver_request.post, standard);
    }
  }
}


TEST (http, parse_header)
{
  // Test pstandard GET request.
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.get, ("page"));
    EXPECT_TRUE(request.query.empty());
  }
  // Test that a non GET/POST request gives a default value for the URL.
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("XXX page HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.get, ("/index"));
    EXPECT_TRUE(request.query.empty());
  }
  // Test an incorrect URL (page&get instead of page?get).
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page&get HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.get, ("page&get"));
    EXPECT_TRUE(request.query.empty());
  }
  // Test basic GET request with URL.
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?get HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.get, ("page"));
    EXPECT_EQ(request.query.at("get"), "");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?key1=value1 HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.get, ("page"));
    EXPECT_EQ(request.query.at("key1"), "value1");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?key1=value1&key2 HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.query.at("key1"), "value1");
    EXPECT_EQ(request.query.at("key2"), "");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?key1=value1&key2=value2 HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.query.at("key1"), "value1");
    EXPECT_EQ(request.query.at("key2"), "value2");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?key1=value1&key2=value2&key3=value3 HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.query.at("key1"), "value1");
    EXPECT_EQ(request.query.at("key2"), "value2");
    EXPECT_EQ(request.query.at("key3"), "value3");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("GET page?key1=Hello%20World HTTP/1.1", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.query.at("key1"), "Hello World");
  }
  // Test a few more header lines.
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("Host: host", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.host, "host");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("User-Agent: agent", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.user_agent, "agent");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("Accept-Language: language", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.accept_language, "language");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("Content-Type: content-type", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.content_type, "content-type");
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("Content-Length: 123", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.content_length, 123);
  }
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("If-None-Match: etag", request);
    EXPECT_TRUE(parsed);
    EXPECT_EQ(request.if_none_match, "etag");
  }
  // Test cookie session.
  {
    {
      Webserver_Request request{};
      const bool parsed = http_parse_header ("Cookie: Session=session; foo=bar; extra=clutter", request);
      EXPECT_TRUE(parsed);
      EXPECT_EQ(request.session_identifier, "session");
    }
    {
      Webserver_Request request{};
      const bool parsed = http_parse_header ("Cookie: Session=1d15e82d0cf17d8b2b675ee9a7b8bb2f", request);
      EXPECT_TRUE(parsed);
      EXPECT_EQ(request.session_identifier, "1d15e82d0cf17d8b2b675ee9a7b8bb2f");
    }
    {
      Webserver_Request request{};
      const bool parsed = http_parse_header ("Cookie: Session=a809d7bdf691e86a409db85c9cfe69f2; Path=/; Max-Age=2678400; HttpOnly; SameSite=None; Secure", request);
      EXPECT_TRUE(parsed);
      EXPECT_EQ(request.session_identifier, "a809d7bdf691e86a409db85c9cfe69f2");
    }
  }
  // Test empty line as end of header.
  {
    Webserver_Request request{};
    const bool parsed = http_parse_header ("", request);
    EXPECT_FALSE(parsed);
  }
}


TEST (http, dev)
{
}


#endif

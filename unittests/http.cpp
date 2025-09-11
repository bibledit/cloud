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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <parsewebdata/ParseWebData.h>
#pragma GCC diagnostic pop


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
  {
    Webserver_Request webserver_request{};
    webserver_request.content_type = application_x_www_form_urlencoded;
    {
      const std::string posted {"key=value"};
      container standard = {
        {"key", "value"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, application_x_www_form_urlencoded, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    {
      const std::string posted {"key1=value1&key2=value2"};
      container standard = {
        {"key1", "value1"},
        {"key2", "value2"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, application_x_www_form_urlencoded, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    {
      const std::string posted {"key1&key2"};
      container standard = {
        {"key1", ""},
        {"key2", ""},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, application_x_www_form_urlencoded, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    {
      const std::string posted {"key=Hello+World"};
      container standard = {
        {"key", "Hello World"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, application_x_www_form_urlencoded, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, filter_url_urldecode(element.second.value));
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    {
      const std::string posted {"key=H%C3%ABllo+W%C3%B6rld"};
      container standard = {
        {"key", "Hëllo Wörld"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, application_x_www_form_urlencoded, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, filter_url_urldecode(element.second.value));
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
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
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, text_plain, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
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
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, text_plain, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
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
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, text_plain, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    
    {
      const std::string posted {
        "key=Hello World\r\n"
      };
      container standard = {
        {"key", "Hello World"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, text_plain, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
    }
    
    {
      const std::string posted {
        "key=Hëllo Wörld\r\n"
      };
      container standard = {
        {"key", "Hëllo Wörld"},
      };
      container post;
      ParseWebData::WebDataMap data_map;
      ParseWebData::parse_post_data (posted, text_plain, data_map);
      for (const auto& element : data_map) {
        post.emplace_back(element.first, element.second.value);
      }
      EXPECT_EQ(standard, post);
      http_parse_post_v2 (posted, webserver_request);
      EXPECT_EQ(standard, webserver_request.post_v2);
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
        const std::map <std::string, std::string> standard_old {
          {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
          {"filename", "00_test1.txt"},
          {"upload", "Upload"}
        };
        EXPECT_EQ (webserver_request.post, standard_old);
        http_parse_post_v2 (content, webserver_request);
        const container standard_new {
          {"filename", "00_test1.txt"},
          {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
          {"upload", "Upload"}
        };
        EXPECT_EQ (webserver_request.post_v2, standard_new);
      }
    }
    {
      // Test a multiple-file upload.
      const std::string content = filter_url_file_get_contents(test_path + "http-post-2.txt");
      Webserver_Request webserver_request{};
      webserver_request.content_type = content_type;
      http_parse_post (content, webserver_request);
      const std::map <std::string, std::string> standard1 {
        {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
        {"filename", "00_test1.txt"},
        {"upload", "Upload"}
      };
      EXPECT_EQ (webserver_request.post, standard1);
      std::map <std::string, std::vector<std::string>> standard2 {
        {"data",
          { "Contents for test2.\nLine one 2.\nLine two 2.\nLine three 2.\n",
            "Contents for test3.\nLine one 3.\nLine two 3.\nLine three 3.\n"
          }
        },
        {"filename",
          { "00_test2.txt", "00_test3.txt" }
        }
      };
      http_parse_post_v2 (content, webserver_request);
      const container standard_new {
        {"filename", "00_test1.txt"},
        {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
        {"filename", "00_test2.txt"},
        {"data", "Contents for test2.\nLine one 2.\nLine two 2.\nLine three 2.\n"},
        {"filename", "00_test3.txt"},
        {"data", "Contents for test3.\nLine one 3.\nLine two 3.\nLine three 3.\n"},
        {"upload", "Upload"}
      };
      EXPECT_EQ (webserver_request.post_v2, standard_new);
    }
  }
}


TEST (http, dev)
{
}


#endif

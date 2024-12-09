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


void http_parse_multi_file_upload (std::string content, Webserver_Request& webserver_request) // Todo write and test and use.
{
  // Get the boundary string from the content type.
  // Example content type:
  // multipart/form-data; boundary=----WebKitFormBoundarye9wsWKTf5zcLAGUn
  // The boundary, as used in the posted body, starts with two extra hyphen.
  const std::string content_type {webserver_request.content_type};
  constexpr std::string_view boundary_is {"boundary="};
  size_t pos = content_type.find(boundary_is);
  if (pos == std::string::npos)
    return;
  const std::string boundary = "--" + content_type.substr(pos + boundary_is.size());
  // Iterate over the posted body as long as it's longer than two bytes.
  while (content.size() > boundary.size()) {
    // The first boundary is expected to be located at position 0.
    // If it's not there, stop parsing right away.
    size_t pos = content.find(boundary);
    if (pos != 0)
      return;
    // Remove the first boundary plus any following carriage return or new line after that boundary.
    content.erase(0, boundary.size());
    const auto remove_cr_or_nl = [&content]() {
      if (content.empty())
        return;
      if (content.find_first_of("\r\n") == 0)
        content.erase(0, 1);
    };
    remove_cr_or_nl();
    remove_cr_or_nl();
    // Find the boundary following the posted data fragment.
    // The last boundary in the posted body ends with two extra hyphens appended.
    // The code does not check on that.
    pos = content.find(boundary);
    if (pos == std::string::npos)
      return;
    // Parse this chunk of data and then remove it.
    http_parse_post (content.substr(0, pos), webserver_request);
    content.erase(0, pos);
  }
}


TEST (http, parse_post) // Todo
{
  const std::string test_path {"unittests/tests/"};
  {
    // Test a single-file upload.
    Webserver_Request webserver_request{};
    const std::string type_multipart = filter_url_file_get_contents(test_path + "http-post-type-1.txt");
    const std::string content = filter_url_file_get_contents(test_path + "http-post-content-1.txt");
    webserver_request.content_type = type_multipart;
    http_parse_multi_file_upload (content, webserver_request);
    const std::map <std::string, std::string> standard {
      {"data", "Contents for test1.\nLine one 1.\nLine two 1.\nLine three 1.\n"},
      {"filename", "00_test1.txt"},
      {"upload", "Upload"}
    };
    EXPECT_EQ (webserver_request.post, standard);
    EXPECT_TRUE (webserver_request.post_multiple.empty());
  }
  {
    // Test a multiple-file upload.
    // Standard upload data goes into the "post" container.
    // Extra keys, duplicate keys, go into the "post_multiple" container.
    Webserver_Request webserver_request{};
    const std::string type_multipart = filter_url_file_get_contents(test_path + "http-post-type-2.txt");
    const std::string content = filter_url_file_get_contents(test_path + "http-post-content-2.txt");
    webserver_request.content_type = type_multipart;
    http_parse_multi_file_upload (content, webserver_request);
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
    EXPECT_EQ (webserver_request.post_multiple, standard2);
  }
}


#endif

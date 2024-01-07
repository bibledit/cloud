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
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <workspace/logic.h>


void test_workspaces_setup (Webserver_Request & request)
{
  request.database_users ()->create ();
  request.session_logic ()->set_username ("phpunit");
}


TEST (workspaces, basic)
{
  // Initial setup for the tests.
  refresh_sandbox (false);
  {
    Webserver_Request webserver_request;
    test_workspaces_setup (webserver_request);

    EXPECT_EQ ("100", workspace_process_units ("100"));
    EXPECT_EQ ("1", workspace_process_units ("100 %"));
    EXPECT_EQ ("1", workspace_process_units ("100 px"));

    EXPECT_EQ ("Default", workspace_get_active_name (webserver_request));
    webserver_request.database_config_user()->setActiveWorkspace ("unittest");
    EXPECT_EQ ("unittest", workspace_get_active_name (webserver_request));

    std::map <int, std::string> standard = {
      std::pair (0, "editone2/index"),
      std::pair (5, "resource/index")
    };
    EXPECT_EQ (standard, workspace_get_default_urls (1));

    {
      std::map <int, std::string> urls = {
        std::pair (10, "url1"),
        std::pair (2, "url2")
      };
      workspace_set_urls (webserver_request, urls);
      std::map <int, std::string> result = workspace_get_urls (webserver_request, false);
      EXPECT_EQ (urls, result);
    }
  
    {
      std::map <int, std::string> widths = {
        std::pair (0, "1"),
        std::pair (1, "1"),
        std::pair (2, "1"),
        std::pair (3, "1")
      };
      std::map <int, std::string> result = workspace_get_widths (webserver_request);
      EXPECT_EQ (widths, result);
    }

    {
      std::vector <std::string> workspaces = workspace_get_names (webserver_request);
      EXPECT_EQ (std::vector<std::string>{"unittest"}, workspaces);
    }
  }

  refresh_sandbox (true);
  {
    Webserver_Request webserver_request;
    test_workspaces_setup (webserver_request);
    webserver_request.database_config_user()->setActiveWorkspace ("unittest");
    workspace_set_urls (webserver_request, {std::pair (10, "url10")});
    webserver_request.database_config_user()->setActiveWorkspace ("unittest2");
    std::map <int, std::string> standard = { std::pair (0, "url0"), std::pair (5, "url5")};
    workspace_set_urls (webserver_request, standard);
    std::vector <std::string> workspaces = workspace_get_names (webserver_request);
    EXPECT_EQ ((std::vector <std::string>{"unittest", "unittest2"}), workspaces);
    workspace_delete (webserver_request, "unittest3");
    workspaces = workspace_get_names (webserver_request);
    EXPECT_EQ ((std::vector <std::string>{"unittest", "unittest2"}), workspaces);
    workspace_delete (webserver_request, "unittest2");
    workspaces = workspace_get_names (webserver_request);
    EXPECT_EQ (std::vector <std::string>{"unittest"}, workspaces);
  }

  refresh_sandbox (true);
  {
    Webserver_Request webserver_request;
    test_workspaces_setup (webserver_request);
    webserver_request.database_config_user()->setActiveWorkspace ("unittest2");
    workspace_set_urls (webserver_request, {std::pair (10, "url10")});
    webserver_request.database_config_user()->setActiveWorkspace ("abc32");
    workspace_set_urls (webserver_request, {std::pair (10, "url10"), std::pair (11, "url11")});
    webserver_request.database_config_user()->setActiveWorkspace ("zzz");
    workspace_set_urls (webserver_request, {std::pair (120, "url120"), std::pair (121, "url121")});
    workspace_reorder (webserver_request, {"zzz", "yyy", "unittest2", "abc32"});
    std::vector <std::string> workspaces = workspace_get_names (webserver_request);
    EXPECT_EQ ((std::vector <std::string>{"zzz", "unittest2", "abc32"}), workspaces);
  }
}

#endif


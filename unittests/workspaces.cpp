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


#include <unittests/workspaces.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <workspace/logic.h>


void test_workspaces_setup (Webserver_Request & request)
{
  request.database_users ()->create ();
  request.session_logic ()->setUsername ("phpunit");
}


void test_workspaces ()
{
  trace_unit_tests (__func__);


  // Initial setup for the tests.
  refresh_sandbox (true);
  {
    Webserver_Request request;
    test_workspaces_setup (request);

    evaluate (__LINE__, __func__, "100", workspace_process_units ("100"));
    evaluate (__LINE__, __func__, "1", workspace_process_units ("100 %"));
    evaluate (__LINE__, __func__, "1", workspace_process_units ("100 px"));

    evaluate (__LINE__, __func__, "Default", workspace_get_active_name (&request));
    request.database_config_user()->setActiveWorkspace ("unittest");
    evaluate (__LINE__, __func__, "unittest", workspace_get_active_name (&request));

    map <int, string> standard = { make_pair (0, "editone/index"), make_pair (5, "resource/index")};
    evaluate (__LINE__, __func__, standard, workspace_get_default_urls (1));

    {
      map <int, string> urls = { make_pair (10, "url1"), make_pair (2, "url2")};
      workspace_set_urls (&request, urls);
      map <int, string> result = workspace_get_urls (&request, false);
      evaluate (__LINE__, __func__, urls, result);
    }
  
    {
      map <int, string> widths = { make_pair (0, "1"), make_pair (1, "1"), make_pair (2, "1"), make_pair (3, "1")};
      map <int, string> result = workspace_get_widths (&request);
      evaluate (__LINE__, __func__, widths, result);
    }

    {
      vector <string> workspaces = workspace_get_names (&request);
      evaluate (__LINE__, __func__, {"unittest"}, workspaces);
    }
  }

  refresh_sandbox (true);
  {
    Webserver_Request request;
    test_workspaces_setup (request);
    request.database_config_user()->setActiveWorkspace ("unittest");
    workspace_set_urls (&request, {make_pair (10, "url10")});
    request.database_config_user()->setActiveWorkspace ("unittest2");
    map <int, string> standard = { make_pair (0, "url0"), make_pair (5, "url5")};
    workspace_set_urls (&request, standard);
    vector <string> workspaces = workspace_get_names (&request);
    evaluate (__LINE__, __func__, {"unittest", "unittest2"}, workspaces);
    workspace_delete (&request, "unittest3");
    workspaces = workspace_get_names (&request);
    evaluate (__LINE__, __func__, {"unittest", "unittest2"}, workspaces);
    workspace_delete (&request, "unittest2");
    workspaces = workspace_get_names (&request);
    evaluate (__LINE__, __func__, {"unittest"}, workspaces);
  }

  refresh_sandbox (true);
  {
    Webserver_Request request;
    test_workspaces_setup (request);
    request.database_config_user()->setActiveWorkspace ("unittest2");
    workspace_set_urls (&request, {make_pair (10, "url10")});
    request.database_config_user()->setActiveWorkspace ("abc32");
    workspace_set_urls (&request, {make_pair (10, "url10"), make_pair (11, "url11")});
    request.database_config_user()->setActiveWorkspace ("zzz");
    workspace_set_urls (&request, {make_pair (120, "url120"), make_pair (121, "url121")});
    workspace_reorder (&request, {"zzz", "yyy", "unittest2", "abc32"});
    vector <string> workspaces = workspace_get_names (&request);
    evaluate (__LINE__, __func__, {"zzz", "unittest2", "abc32"}, workspaces);
  }
}

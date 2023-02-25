/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <session/confirm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <session/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <confirm/worker.h>
#include <email/send.h>
#include <index/index.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <user/logic.h>
using namespace std;


class Verification
{
public:
  string question;
  string answer;
  string passage;
};


const char * session_confirm_url ()
{
  return "session/confirm";
}


bool session_confirm_acl (void * webserver_request)
{
  // Find the level of the user.
  // This confirmation page only allows access if the user is not yet logged in.
  // Such a situation produces level 1, that is the guest level.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int level = request->session_logic ()->currentLevel ();
  return (level == Filter_Roles::guest());
}


string session_confirm ([[maybe_unused]] void * webserver_request)
{
  string page;

#ifdef HAVE_CLOUD

  Confirm_Worker confirm_worker = Confirm_Worker (webserver_request);
  string email;
  bool is_valid_confirmation = confirm_worker.handleLink (email);

  // Handle a valid confirmation.
  if (is_valid_confirmation) {

    Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

    // Authenticate against local database, but skipping some checks.
    if (request->session_logic()->attempt_login (email, "", true, true)) {
      // Log the login.
      Database_Logs::log (request->session_logic()->currentUser () + " confirmed account and logged in");
      // Store web site's base URL.
      string siteUrl = get_base_url (request);
      Database_Config_General::setSiteURL (siteUrl);
      // Store account creation time.
      user_logic_store_account_creation (request->session_logic()->currentUser ());
    }

  }
  
  // In all cases, go to the home page.
  redirect_browser (webserver_request, index_index_url());

#endif

  return page;
}

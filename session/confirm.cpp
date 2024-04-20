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


class Verification
{
public:
  std::string question;
  std::string answer;
  std::string passage;
};


const char * session_confirm_url ()
{
  return "session/confirm";
}


bool session_confirm_acl (Webserver_Request& webserver_request)
{
  // Find the level of the user.
  // This confirmation page only allows access if the user is not yet logged in.
  // Such a situation produces level 1, that is the guest level.
  int level = webserver_request.session_logic ()->currentLevel ();
  return (level == Filter_Roles::guest());
}


std::string session_confirm ([[maybe_unused]] Webserver_Request& webserver_request)
{
  std::string page;

#ifdef HAVE_CLOUD

  Confirm_Worker confirm_worker = (webserver_request);
  std::string email;
  bool is_valid_confirmation = confirm_worker.handleLink (email);

  // Handle a valid confirmation.
  if (is_valid_confirmation) {

    // Authenticate against local database, but skipping some checks.
    if (webserver_request.session_logic()->attempt_login (email, "", true, true)) {
      // Log the login.
      Database_Logs::log (webserver_request.session_logic()->currentUser () + " confirmed account and logged in");
      // Store web site's base URL.
      std::string siteUrl = get_base_url (webserver_request);
      Database_Config_General::setSiteURL (siteUrl);
      // Store account creation time.
      user_logic_store_account_creation (webserver_request.session_logic()->currentUser ());
    }

  }
  
  // In all cases, go to the home page.
  redirect_browser (webserver_request, index_index_url());

#endif

  return page;
}

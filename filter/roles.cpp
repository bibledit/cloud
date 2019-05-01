/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <filter/roles.h>
#include <locale/translate.h>


int Filter_Roles::guest ()
{
  return 1;
}


int Filter_Roles::member ()
{
  return 2;
}


int Filter_Roles::consultant ()
{
  return 3;
}


int Filter_Roles::translator ()
{
  return 4;
}


int Filter_Roles::manager ()
{
  return 5;
}


int Filter_Roles::admin ()
{
  return 6;
}


int Filter_Roles::lowest ()
{
  return guest ();
}


int Filter_Roles::highest ()
{
  return admin ();
}


string Filter_Roles::english (int role)
{
  if (role == admin ()) return "Administrator";
  if (role == manager ()) return "Manager";
  if (role == translator ()) return "Translator";
  if (role == consultant ())return "Consultant";
  if (role == member ()) return "Member";
  return "Guest";
}


string Filter_Roles::text (int role)
{
  if (role == admin ()) return translate ("Administrator");
  if (role == manager ()) return translate ("Manager");
  if (role == translator ()) return translate ("Translator");
  if (role == consultant ())return translate ("Consultant");
  if (role == member ()) return translate ("Member");
  return translate ("Guest");
}


// This is for access control.
// The "role" is the role required for the user to have access.
bool Filter_Roles::access_control (void * webserver_request, int role)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int level = request->session_logic ()->currentLevel ();
  return level >= role;
}


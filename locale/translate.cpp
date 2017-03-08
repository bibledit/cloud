/*
Copyright (©) 2003-2017 Teus Benschop.

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
#include <database/config/general.h>
#include <database/localization.h>


// Translates $english to its localized string.
string translate (string english)
{
  string localization = Database_Config_General::getSiteLanguage ();
  if (localization.empty ()) return english;
  Database_Localization database_localization = Database_Localization (localization);
  return database_localization.translate (english);
}


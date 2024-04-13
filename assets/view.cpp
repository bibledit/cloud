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


#include <assets/view.h>
#include <cstdlib>
#include <iostream>
#include <filter/url.h>
#include <config/globals.h>
#include <database/logs.h>
#include <flate/flate.h>


Assets_View::Assets_View ()
{
  // On some installations like on iOS / Android / Mac, the browser has no controls.
#ifdef HAVE_BARE_BROWSER
  enable_zone ("bare_browser");
#endif
  set_variable("VERSION", config::logic::version ());
}


// Sets a variable (key and value) for the html template.
void Assets_View::set_variable (const std::string& key, const std::string& value)
{
  m_variables[key] = value;
}


// Enable displaying a zone in the html template.
void Assets_View::enable_zone (const std::string& zone)
{
  m_zones [zone] = true;
}


void Assets_View::disable_zone (const std::string& zone)
{
  m_zones.erase (zone);
}


void Assets_View::add_iteration (const std::string& key, const std::map <std::string, std::string>& value)
{
  m_iterations[key].push_back (value);
}


// Renders the "tpl" template through the flate template engine.
// The "tpl" consists of two bits: 
// 1: Relative folder
// 2: Basename of the html template without the .html extension.
// Setting the session variables in the template is postponed to the very last moment,
// since these could change during the course of the calling page.
std::string Assets_View::render (const std::string& tpl1, const std::string& tpl2)
{
  // Variable tpl is a relative path. Make it a full one.
  const std::string tpl = filter_url_create_root_path ({tpl1, tpl2 + ".html"});

  // The flate engine crashes if the template does not exist, so be sure it exists.  
  if (!file_or_dir_exists (tpl)) {
    Database_Logs::log ("Cannot find template file " + tpl);
    return std::string();
  }

  // Instantiate and fill the template engine. 
  Flate flate;

  // Copy the variables and zones and iterations to the engine.
  std::map <std::string, std::string>::iterator iter1 {};
  for (iter1 = m_variables.begin (); iter1 != m_variables.end(); ++iter1) {
    flate.set_variable (iter1->first, iter1->second);
  }
  std::map <std::string, bool>::iterator iter2{};
  for (iter2 = m_zones.begin (); iter2 != m_zones.end(); ++iter2) {
    flate.enable_zone (iter2->first);
  }
  flate.iterations = m_iterations;

  // Get and return the page contents.
  return flate.render (tpl);
}

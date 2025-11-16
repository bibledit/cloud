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


function debugLog (message)
{
  var url = new URL("/developer/index", document.location);
  url.searchParams.append("log", message)
  fetch(url, {cache: "no-cache"});
}


function showAlertWithTimeout ()
{
  notifySuccess ("The keyboard is blocked for a short while")
  debugLog("disable text input");
  document.getElementById('textinput').setAttribute('readonly', true);
  setTimeout (enableTextInput, 2000);
}


function enableTextInput ()
{
  document.getElementById('textinput').removeAttribute('readonly');
  debugLog("enable text input again");
}


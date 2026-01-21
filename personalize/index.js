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

const identifiers = [
  "fontsizegeneral",
  "fontsizemenu",
  "fontsizeeditors",
  "fontsizeresources",
  "fontsizehebrew",
  "fontsizegreek",
  "caretposition",
  "workspacefadeoutdelay",
  "chapterpercentage",
  "versepercentage",
];


document.addEventListener("DOMContentLoaded", function()
{
  identifiers.forEach((identifier) => {
    var element = document.querySelector("#" + identifier);
    if (element) {
       element.addEventListener('change', handleChange);
    }
  });
});


function handleChange (event) {
  const identifier = event.target.id;
  const value = event.target.value;
  fetch("", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["identifier", identifier], ["value", value]]).toString(),
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((text) => {
    if (text == "reload") {
      window.location.reload()
    }
  })
  .catch((error) => {
    console.log(error);
  });
}

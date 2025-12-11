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
function noteClickSetup ()
{
  document.querySelectorAll("a.opennote").forEach((element) => {
    element.removeEventListener("click", aOpenNoteClick, false);
    element.addEventListener("click", aOpenNoteClick);
  });
  document.querySelectorAll("a.newnote").forEach((element) => {
    element.removeEventListener("click", aNewNoteClick, false);
    element.addEventListener("click", aNewNoteClick);
  });
}

function aOpenNoteClick (event) {
  event.preventDefault ();
  const url = "/notes/click?open=" + event.target.href;
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .catch((error) => {
    console.log(error);
  })
}

function aNewNoteClick (event) {
  event.preventDefault ();
  const url = "/notes/click?new=" + event.target.href;
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .catch((error) => {
    console.log(error);
  })
}

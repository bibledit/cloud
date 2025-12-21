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


document.addEventListener("DOMContentLoaded", function(e) {
  passageConnectToAll ();
});


function passageConnectToAll () {
  document.querySelectorAll('.starteditor').forEach((element) => {
    element.removeEventListener("click", passageStartEditor, false);
    element.addEventListener("click", passageStartEditor);
  });
}

function passageConnectToLast () {
  const nodes = document.querySelectorAll('.starteditor');
  if (nodes.length) {
    const node = nodes[nodes.length - 1];
    node.removeEventListener("click", passageStartEditor, false);
    node.addEventListener("click", passageStartEditor);
  }
}

function passageStartEditor (event) {
  event.preventDefault ();
  const a = event.target;
  var passage = a.getAttribute ("passage");
  const url = "/edit/edit?passage=" + passage;
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var next = a.nextSibling;
    next.innerHTML = " " + response;
    setTimeout(() => {
      next.innerHTML = " ";
    }, "3000");
  })
  .catch((error) => {
    console.log(error);
  })
}

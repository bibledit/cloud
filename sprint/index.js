/*
Copyright (©) 2003-2026 Teus Benschop.

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
  document.querySelectorAll('input[type="checkbox"]').forEach((element) => {
    element.addEventListener("change", function(event) {
      var box = event.target;
      var id = box.id;
      var checked = box.checked;
      fetch("index", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: new URLSearchParams([ ["id", id], ["checked", checked] ]).toString(),
      })
      .catch((error) => {
        console.log(error);
      })
      var identifier = sprint_get_identifier (id);
      var box = sprint_get_box (id);
      var fragment = "task" + identifier.toString () + "box";
      var i;
      if (checked) {
        for (i = 0; i < box; i++) {
          const id = fragment + i.toString ();
          document.querySelector ("#" + id).checked = true;
        }
      } else {
        for (i = box + 1; i < 100; i++) {
          const id = fragment + i.toString ();
          const boxn = document.querySelector ("#" + id)
          if (boxn) boxn.checked = false;
        }
      }
    });
  });
});


function sprint_get_identifier (fragment)
{
  // Remove "task" and convert to integer identifier.
  var id = fragment.substring (4);
  var identifier = parseInt (id);
  return identifier;
}


function sprint_get_box (fragment)
{
  // Find the fragment "box".
  var pos = fragment.indexOf ("box");
  // Remove the fragment "box".
  var id = fragment.substring (pos + 3);
  // Convert the box to an integer.
  var box = parseInt (id);
  return box;
}

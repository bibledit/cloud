/*
Copyright (©) 2003-2016 Teus Benschop.

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


$(document).ready (function () {
  $ ("table :checkbox").change (function () {
    var box = $ (this);
    var id = box.attr ("id");
    var checked = box.is (':checked')
    $.post ("index", { id:id, checked:checked });
    var identifier = sprint_get_identifier (id);
    var box = sprint_get_box (id);
    var fragment = "task" + identifier.toString () + "box";
    var i;
    if (checked) {
      for (i = 0; i < box; i++) {
        var id = fragment + i.toString ();
        $ ("#" + id).prop ("checked", true);
      }
    } else {
      for (i = box + 1; i < 100; i++) {
        var id = fragment + i.toString ();
        $ ("#" + id).prop ("checked", false);
      }
    }
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
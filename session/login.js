/*
Copyright (©) 2003-2021 Teus Benschop.

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

var touchCounter = 1;

$(document).ready (function () {
  window.addEventListener('mousemove', function mouseMoveDetector() {
    touchCounter++;
    // Just touching the Login button already gives a few mouse movements.
    // Skip the first few movements.
    if (touchCounter > 10) {
      window.removeEventListener('mousemove', mouseMoveDetector);
      var touch = $ ("#touch");
      touch.attr ("value", "0");
    }
  });
  $("#show").change(function() {
    var input = $("#pass");
      if (input.attr("type") === "password") {
        input.attr("type", "text");
      } else {
        input.attr("type", "password");
      }
  });
});

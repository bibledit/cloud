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


// Reconciles the inter-dependent checkboxes.
document.addEventListener("DOMContentLoaded", function(e) {
  var checkboxes = document.querySelectorAll('input[type="checkbox"]');
  checkboxes.forEach((checkbox) => {
    checkbox.addEventListener("change", function(event) {
      var name = checkbox.id;
      var checked = checkbox.checked;
      if ((name == "view") && !checked) {
        box = document.querySelector ("#create");
        if (box.checked) box.click();
      }
      if ((name == "create") && checked) {
        box = document.querySelector ("#view");
        if (!box.checked) box.click();
      }
    });
  });
});

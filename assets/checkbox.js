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


var checkbox_last_input;

function checkbox_v2 (input, val1, val2, val3 ) {
  // Store this input.
  checkbox_last_input = input;
  // The URL where to POST to.
  url = window.location.href.split("?")[0];
  url = url.split('/').reverse()[0];
  // Post the checkbox state.
  fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["checkbox", input.name], ["checked", input.checked], ["val1", val1], ["val2", val2], ["val3", val3]]).toString(),
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((text) => {
    if (text == "reload") {
      location.reload();
    }
  })
  .catch((error) => {
    // Could not save: Revert the checkbox for consistency.
    checkbox_last_input.checked = !checkbox_last_input.checked;
  });
}

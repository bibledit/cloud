/*
Copyright (©) 2003-2019 Teus Benschop.

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


var checkboxLastInput;

function checkbox (input, url) {
  // Store this input.
  checkboxLastInput = input;
  // The URL where to POST to.
  if (url === undefined) {
    url = window.location.href.split("?")[0];
    url = url.split('/').reverse()[0];
  }
  // Post the checkbox state.
  $.ajax ({
    url: url,
    type: "POST",
    data: { checkbox: input.name, checked: input.checked },
    error: function (jqXHR, textStatus, errorThrown) {
      // Could not save: Revert the checkbox.
      checkboxLastInput.checked = !checkboxLastInput.checked;
    }
  });
}

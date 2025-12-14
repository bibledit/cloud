/*
 * Copyright (©) 2003-2025 Teus Benschop.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

document.addEventListener("DOMContentLoaded", function(e) {
  function setupSlip(list) {
    list.addEventListener("slip:beforereorder", function (e) {}, false);

    list.addEventListener("slip:beforeswipe", function (e) {}, false);

    list.addEventListener(
      "slip:beforewait",
      function (e) {
        if (e.target.classList.contains("instant")) {
          e.preventDefault();
        }
      },
      false
    );

    list.addEventListener(
      "slip:afterswipe",
      function (e) {
        e.target.parentNode.appendChild(e.target);
      },
      false
    );

    list.addEventListener(
      "slip:reorder",
      function (e) {
        fetch("organize", {
          method: "POST",
          headers: { "Content-Type": "application/x-www-form-urlencoded" },
          body: new URLSearchParams([ ["movefrom", e.detail.originalIndex], ["moveto", e.detail.spliceIndex], ["type", (e.target.parentNode.id == "list-def" ? "def" : "no")] ]).toString(),
        })
        .then((response) => {
          if (!response.ok) {
            throw new Error(response.status);
          }
          return response.text();
        })
        .catch((error) => {
          console.log(error);
        })
        e.target.parentNode.insertBefore(e.target, e.detail.insertBefore);
        return false;
      },
      false
    );

    return new Slip(list);
  }

  setupSlip(document.getElementById("list"));
  setupSlip(document.getElementById("list-def"));
});

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

var scrolledTimeout = null
var workspacewrapper = null

document.addEventListener("DOMContentLoaded", (event) => {
  workspacewrapper = document.querySelector("#workspacewrapper")
  workspacewrapper.addEventListener('scroll', (event) => {
    // The scroll events may occur in very quick succession.
    // The timeout greatly reduces the number of actions on the fired events.
    if (scrolledTimeout)
      clearTimeout(scrolledTimeout);
    scrolledTimeout = setTimeout (handleScrolled, 100)
  })
  // Restore the horizontal Workspace position.
  var scrollLeft = window.sessionStorage.workspaceScrollLeft
  if (scrollLeft)
    workspacewrapper.scrollLeft = scrollLeft
})

// Save the horizontal Workspace position.
function handleScrolled()
{
  window.sessionStorage.workspaceScrollLeft = workspacewrapper.scrollLeft
}

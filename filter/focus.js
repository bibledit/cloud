/*
Copyright (©) 2003-2022 Teus Benschop.

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


// Return the active element of a page, regardless of shadow root or iframe window.
// @returns {HTMLElement}
function getActiveElement (element = document.activeElement)
{
  const shadowRoot = element.shadowRoot
  const contentDocument = element.contentDocument
  
  if (shadowRoot && shadowRoot.activeElement) {
    return getActiveElement(shadowRoot.activeElement)
  }
  
  if (contentDocument && contentDocument.activeElement) {
    return getActiveElement(contentDocument.activeElement)
  }
  
  return element
}

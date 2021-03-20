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


// Returns true if the keypress can be ignored for triggering a content change.
function editKeysIgnoreForContentChange (event)
{
  if (event) {
    // Escape.
    if (event.keyCode == 27) return true;
    // Modifiers.
    if (editKeysModifierKeysPressed (event)) return true;
    // Arrows.
    if (event.keyCode == 37) return true;
    if (event.keyCode == 38) return true;
    if (event.keyCode == 39) return true;
    if (event.keyCode == 40) return true;
  }
  return false;
}


// Returns true if the keypress can be ignored for triggering a caret change.
function editKeysIgnoreForCaretChange (event)
{
  if (event) {
    // Ctrl-G.
    if ((event.ctrlKey == true) && (event.keyCode == 71)) {
      return true;
    }
    // Modifiers.
    if (editKeysModifierKeysPressed (event)) return true;
    // Work around the phenomenon that in some browsers it gives an extra key code 229.
    if (event.keyCode == 229) return true;
    // Alt-Up / Alt-Down.
    if (event.altKey) {
      if (event.keyCode == 38) return true;
      if (event.keyCode == 40) return true;
    }
  }
  return false;
}


// Returns true if any of the modifier keys are pressed.
function editKeysModifierKeysPressed (event)
{
  // Shift / Ctrl / Alt / Alt Gr / Win keys.
  if (event.keyCode == 16) return true;
  if (event.keyCode == 17) return true;
  if (event.keyCode == 18) return true;
  if (event.keyCode == 225) return true;
  if (event.keyCode == 91) return true;
  // Mac right cmd key.
  if (event.keyCode == 93) return true;
  // Caps lock.
  if (event.keyCode == 20) return true;
  return false;
}

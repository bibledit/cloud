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

var touchStartX = 0;

document.addEventListener("DOMContentLoaded", function(e) {
  navigationNewPassage ();

  if (swipe_operations) {
    // The minimum distance to swipe is 10% of the screen width.
    // This is to eliminate small unintended swipes.
    let minSwipeDistance = parseInt(window.screen.width / 10);
    
    document.body.addEventListener('touchstart', event => {
      touchStartX = event.changedTouches[0].screenX;
    });
    
    document.body.addEventListener('touchend', event => {
      let touchEndX = event.changedTouches[0].screenX
      if (touchEndX < touchStartX - minSwipeDistance) {
        userResourceSwipeLeft (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        userResourceSwipeRight (event);
      }
    })
  }
});


var userResourceBook;
var userResourceChapter;
var userResourceVerse;


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
    userResourceBook = navigationBook;
    userResourceChapter = navigationChapter;
    userResourceVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    userResourceBook = parent.window.navigationBook;
    userResourceChapter = parent.window.navigationChapter;
    userResourceVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  if (userResourceBook == undefined) return;
  userResourceSetUrl ();
}


function userResourceSetUrl ()
{
  var url = userResourceUrl;
  url = url.replace ("[book]", userResourceBooks [userResourceBook]);
  url = url.replace ("[chapter]", userResourceChapter);
  url = url.replace ("[verse]", userResourceVerse);
  var iframe = document.querySelector("iframe");
  if (iframe) {
    iframe.setAttribute("src", url);
  }
}


function userResourceSwipeLeft (event)
{
  if (is_outside_workspace()) {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function userResourceSwipeRight (event)
{
  if (is_outside_workspace()) {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}

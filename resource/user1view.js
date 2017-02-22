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
  navigationNewPassage ();
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        userResourceSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        userResourceSwipeRight (event);
      }
    });
  }
});


var userResourceBook;
var userResourceChapter;
var userResourceVerse;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
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
  $ ("iframe").attr ("src", url);
}


function userResourceSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function userResourceSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}

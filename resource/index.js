/*
Copyright (©) 2003-2017 Teus Benschop.

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
        resourceSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        resourceSwipeRight (event);
      }
    });
  }
  $ (window).on ("unload", resourceUnload);
});


var resourceBook;
var resourceChapter;
var resourceVerse;
var resourceAjaxRequest;
var resourceDoing;
var resourceAborting = false;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    resourceBook = navigationBook;
    resourceChapter = navigationChapter;
    resourceVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    resourceBook = parent.window.navigationBook;
    resourceChapter = parent.window.navigationChapter;
    resourceVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  if (resourceBook == undefined) return;
  resourceAborting = true;
  try {
    resourceAjaxRequest.abort ();
  } catch (err) {
  }
  resourceAborting = false;
  resourceDoing = 0;
  resourceGetOne ();
}


function resourceGetOne ()
{
  if (resourceAborting) return;
  resourceDoing++;
  if (resourceDoing > resourceCount) {
    // No longer position window.
    resourceWindowPosition = 0;
    // Done.
    return;
  }
  resourceAjaxRequest = $.ajax ({
    url: "get",
    type: "GET",
    data: { resource: resourceDoing, book: resourceBook, chapter: resourceChapter, verse: resourceVerse },
    success: function (response) {
      if (response == "") {
        $ ("#line" + resourceDoing).hide ();
        $ ("#name" + resourceDoing).hide ();
      } else {
        $ ("#line" + resourceDoing).show ();
        $ ("#name" + resourceDoing).show ();
        if (response.charAt (0) == "$") {
          $ ("#name" + resourceDoing).hide ();
          response = response.substring (1);
        }
        var current_content = String ($ ("#content" + resourceDoing).html ());
        $ ("#reload").html (response);
        if (current_content != String ($ ("#reload").html ())) {
          $ ("#content" + resourceDoing).html (response);
        }
      }
      navigationSetup ();
      resourcePosition ();
    },
    error: function (jqXHR, textStatus, errorThrown) {
      resourceDoing--;
    },
    complete: function (jqXHR) {
      setTimeout (resourceGetOne, 10);
    }
  });
}


function resourceSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function resourceSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}


function resourceUnload ()
{
  var position = $("#workspacewrapper").scrollTop();
  $.ajax ({
    url: "unload",
    type: "POST",
    async: false,
    data: { position: position },
  });
}


// Scroll the window to the same position it had the previous time it displayed.
function resourcePosition ()
{
  if (resourceWindowPosition != 0) {
    var position = $("#workspacewrapper").scrollTop();
    if (position != resourceWindowPosition) {
      $("#workspacewrapper").scrollTop (resourceWindowPosition);
    } else {
      resourceWindowPosition = 0;
    }
  }
}

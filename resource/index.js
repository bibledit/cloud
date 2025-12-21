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
var resourceAbortController = new AbortController();

document.addEventListener("DOMContentLoaded", function(event) {
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
        resourceSwipeLeft (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        resourceSwipeRight (event);
      }
    })
  }
  
  window.addEventListener ("unload", resourceUnload);
});


var resourceBook;
var resourceChapter;
var resourceVerse;
var resourceDoing;
var resourceAborting = false;


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
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
  resourceAbortController.abort();
  resourceAbortController = new AbortController();
  resourceDoing = 0;
  resourceAborting = false;
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
  const url = "get?" + new URLSearchParams([ ["resource", resourceDoing], ["book", resourceBook], ["chapter", resourceChapter], ["verse", resourceVerse] ]).toString()
  fetch(url, {
    method: "GET",
    resourceDoing: resourceDoing,
    signal: resourceAbortController.signal
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var line = document.querySelector("#line" + this.resourceDoing);
    var name = document.querySelector("#name" + this.resourceDoing);
    if (response == "") {
      line.hidden = true;
      name.hidden = true;
    } else {
      line.hidden = false;
      name.hidden = false;
      if (response.charAt (0) == "$") {
        name.hidden = true;
        response = response.substring (1);
      }
      var content = document.querySelector("#content" + this.resourceDoing);
      var reload = document.querySelector("#reload");
      reload.innerHTML = response;
      if (content.innerHTML != reload.innerHTML) {
        content.innerHTML = response;
      }
    }
    navigationSetup ();
    resourcePosition ();
    runScript(response);
  })
  .catch((error) => {
    console.log(error);
    if (!resourceAborting) resourceDoing--;
  })
  .finally(() => {
    if (!resourceAborting) setTimeout (resourceGetOne, 10);
  });
}


function resourceSwipeLeft (event)
{
  if (is_outside_workspace()) {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function resourceSwipeRight (event)
{
  if (is_outside_workspace()) {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}


function resourceUnload ()
{
  var position = document.querySelector("#workspacewrapper").scrollTop;
  fetch("unload", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["position", position]]).toString(),
    keepalive: true, // Synchronous call.
  })
}


// Scroll the window to the same position it had the previous time it displayed.
// Later this was disabled.
// The reason is explained in https://github.com/bibledit/cloud/issues/660.
function resourcePosition ()
{
}


// If the response contains Javascript, extract it and evaluate (run) it.
function runScript (response)
{
  const startTag = "<script>";
  const scriptStartPos = response.indexOf(startTag);
  if (scriptStartPos < 0) return;
  const scriptEndPos = response.indexOf("</script>");
  if (scriptEndPos < 0) return;
  const script = response.substring(scriptStartPos + startTag.length, scriptEndPos);
  eval(script);
}

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


var quill = undefined;
var Delta = Quill.import ("delta");
var verseReaderUniqueID = Math.floor (Math.random() * 100000000);
var touchStartX = 0;
var abortController = new AbortController();


document.addEventListener("DOMContentLoaded", function(e) {
  // Make the editor's menu to never scroll out of view.
  var bar = document.querySelector ("#editorheader");
  if (bar) {
    document.querySelector ("#workspacemenu").insertAdjacentHTML('beforeend', bar.outerHTML);
    bar.remove()
  }

  visualVerseReaderInitializeOnce ();
  visualVerseReaderInitializeLoad ();

  navigationNewPassage ();
  
  if (swipe_operations) {
    // The minimum distance to swipe is 10% of the screen width.
    // This is to eliminate small unintended swipes.
    let minSwipeDistance = parseInt(window.screen.width / 10);
    
    let body = document.querySelector("body");
    
    body.addEventListener('touchstart', event => {
      touchStartX = event.changedTouches[0].screenX;
      });
    
    body.addEventListener('touchend', event => {
      let touchEndX = event.changedTouches[0].screenX
      if (touchEndX < touchStartX - minSwipeDistance) {
        readchooseSwipeLeft (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        readchooseSwipeRight (event);
      }
    })
  }

  setTimeout (readchooseEditorPollId, 1000);
});


function visualVerseReaderInitializeOnce ()
{
  var Parchment = Quill.import ('parchment');
  
  // Register block formatting class.
  var ParagraphClass = new Parchment.Attributor.Class ('paragraph', 'b', { scope: Parchment.Scope.BLOCK });
  Quill.register (ParagraphClass, true);
  
  // Register inline formatting class.
  var CharacterClass = new Parchment.Attributor.Class ('character', 'i', { scope: Parchment.Scope.INLINE });
  Quill.register (CharacterClass, true);
}


function visualVerseReaderInitializeLoad ()
{
  // Work around https://github.com/quilljs/quill/issues/1116
  // It sets the margins to 0 by adding an overriding class.
  // The Quill editor will remove that class again.
  document.querySelectorAll("#oneeditor > p").forEach((element) => {
    element.classList.add("nomargins");
  });

  // Instantiate editor.
  quill = new Quill ('#oneeditor', { });
}


var readchooseBible;
var readchooseBook;
var readchooseNavigationBook;
var readchooseChapter;
var readchooseNavigationChapter;
var readchooseVerse;
var readchooseNavigationVerse;
var readchooseChapterId = 0;
var readchooseChapterChanged = false;


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
    readchooseNavigationBook = navigationBook;
    readchooseNavigationChapter = navigationChapter;
    readchooseNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    readchooseNavigationBook = parent.window.navigationBook;
    readchooseNavigationChapter = parent.window.navigationChapter;
    readchooseNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  readchooseEditorLoadChapter ();
}


function readchooseEditorLoadChapter ()
{
  if ((readchooseNavigationBook != readchooseBook) || (readchooseNavigationChapter != readchooseChapter) || (readchooseNavigationVerse != readchooseVerse) || readchooseChapterChanged ) {
    readchooseBible = navigationBible;
    readchooseBook = readchooseNavigationBook;
    readchooseChapter = readchooseNavigationChapter;
    readchooseVerse = readchooseNavigationVerse;
    readchooseChapterId = 0;
    readchooseChapterChanged = false;
    abortController.abort();
    abortController = new AbortController();
    const url = "load?" + new URLSearchParams([ ["bible", readchooseBible], ["book", readchooseBook], ["chapter", readchooseChapter], ["verse", readchooseVerse], ["id", verseReaderUniqueID] ]).toString();
    fetch(url, {
      method: "GET",
      signal: abortController.signal
    })
    .then((response) => {
      if (!response.ok) {
        throw new Error(response.status);
      }
      return response.text();
    })
    .then((response) => {
      // Remove the flag for editor read-write or read-only.
      checksum_readwrite (response);
      // Checksumming.
      response = checksum_receive (response);
      // Splitting.
      var bits;
      if (response !== false) {
        bits = response.split ("#_be_#");
        if (bits.length != 3) response == false;
      }
      if (response !== false) {
        var oneprefix = document.querySelector("#oneprefix");
        oneprefix.innerHTML = bits [0];
        oneprefix.removeEventListener("click", readchooseHtmlClicked, false);
        oneprefix.addEventListener("click", readchooseHtmlClicked);
      }
      if (response !== false) {
        // Destroy existing editor.
        if (quill) delete quill;
        // Load the html in the DOM.
        document.querySelector("#oneeditor").innerHTML = bits [1];
        readchooseEditorStatus (readchooseEditorVerseLoaded);
        // Create the editor based on the DOM's content.
        visualVerseReaderInitializeLoad ();
        quill.enable (false);
        // Create CSS for embedded styles.
        css4embeddedstyles ();
      }
      if (response !== false) {
        var onesuffix = document.querySelector("#onesuffix");
        onesuffix.innerHTML = bits [2];
        onesuffix.removeEventListener("click", readchooseHtmlClicked, false);
        onesuffix.addEventListener("click", readchooseHtmlClicked);
      }
      if (response !== false) {
        readchooseScrollVerseIntoView ();
      }
      if (response === false) {
        // Checksum or other error: Reload.
        readchooseEditorLoadChapter ();
      }
    })
    .catch((error) => {
      console.log(error);
    })
    .finally(() => {
    });
  }
}


function readchooseEditorStatus (text)
{
  if (document.body.contains(document.getElementById("onestatus"))) {
    document.querySelector ("#onestatus").innerHTML = text;
  }
}


function readchooseEditorPollId ()
{
  const url ="../editor/id?" + new URLSearchParams([ ["bible", readchooseBible], ["book", readchooseBook], ["chapter", readchooseChapter] ]).toString();
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    if (readchooseChapterId != 0) {
      if (response != readchooseChapterId) {
        // The chapter identifier changed.
        // That means that likely there's updated text on the server.
        // Reload the chapter.
        readchooseChapterChanged = true;
        readchooseEditorLoadChapter();
      }
    }
    readchooseChapterId = response;
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    setTimeout (readchooseEditorPollId, 1000);
  });
}


function readchooseScrollVerseIntoView ()
{
  var workspacewrapper = document.querySelector("#workspacewrapper");
  var oneeditor = document.querySelector("#oneeditor");
  var verseTop = oneeditor.getBoundingClientRect().top;
  var workspaceHeight = workspacewrapper.clientHeight;
  var currentScrollTop = workspacewrapper.scrollTop;
  var scrollTo = verseTop - (workspaceHeight * verticalCaretPosition / 100) + currentScrollTop;
  var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
  var upperBoundary = currentScrollTop + (workspaceHeight / 10);
  if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
    workspacewrapper.scroll({
      top: scrollTo,
      behavior: "smooth",
    });
  }
}


function readchooseHtmlClicked (event)
{
  // If the user selects text, do nothing.
  var text = "";
  if (window.getSelection) {
    text = window.getSelection().toString();
  } else if (document.selection && document.selection.type != "Control") {
    text = document.selection.createRange().text;
  }
  if (text.length) return;
  
  var verse = "";
  
  var iterations = 0;
  var target = event.target;
  var tagName = target.tagName;
  if (tagName == "P") {
    target = target.lastChild;
  }
  while ((iterations < 10) && (!target.classList.contains ("i-v"))) {
    var previous = target.previousSibling;
    if (!previous) {
      target = target.parentElement.previousSibling;
      target = target.lastChild;
    } else {
      target = previous;
    }
    iterations++;
  }
                                          
  // Too many iterations: Undefined location.
  if (iterations >= 10) return
  
  if (!target) verse = "0";
  
  if (target.classList.contains ("i-v")) {
    verse = target.innerText;
  }

  const url = "verse?verse=" + verse;
  fetch(url, {
    method: "GET",
    cache: "no-cache"
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
}


function readchooseSwipeLeft (event)
{
  if (is_outside_workspace()) {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function readchooseSwipeRight (event)
{
  if (is_outside_workspace()) {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}

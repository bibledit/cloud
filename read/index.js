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
        readSwipeLeft (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        readSwipeRight (event);
      }
    })
  }

  setTimeout (readEditorPollId, 1000);
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


var readBible;
var readBook;
var readNavigationBook;
var readChapter;
var readNavigationChapter;
var readVerse;
var readNavigationVerse;
var readChapterId = 0;
var readChapterChanged = false;


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
    readNavigationBook = navigationBook;
    readNavigationChapter = navigationChapter;
    readNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    readNavigationBook = parent.window.navigationBook;
    readNavigationChapter = parent.window.navigationChapter;
    readNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  readEditorLoadChapter ();
}


function readEditorLoadChapter ()
{
  if ((readNavigationBook != readBook) || (readNavigationChapter != readChapter) || (readNavigationVerse != readVerse) || readChapterChanged ) {
    readBible = navigationBible;
    readBook = readNavigationBook;
    readChapter = readNavigationChapter;
    readVerse = readNavigationVerse;
    readChapterId = 0;
    readChapterChanged = false;
    abortController.abort("");
    abortController = new AbortController();
    const url = "load?" + new URLSearchParams([ ["bible", readBible], ["book", readBook], ["chapter", readChapter], ["verse", readVerse], ["id", verseReaderUniqueID] ]).toString();
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
        oneprefix.removeEventListener("click", readHtmlClicked, false);
        oneprefix.addEventListener("click", readHtmlClicked);
      }
      if (response !== false) {
        // Destroy existing editor.
        if (quill) delete quill;
        // Load the html in the DOM.
        document.querySelector("#oneeditor").innerHTML = bits [1];
        readEditorStatus (readEditorVerseLoaded);
        // Create the editor based on the DOM's content.
        visualVerseReaderInitializeLoad ();
        quill.enable (false);
        // Create CSS for embedded styles.
        css4embeddedstyles ();
      }
      if (response !== false) {
        var onesuffix = document.querySelector("#onesuffix");
        onesuffix.innerHTML = bits [2];
        onesuffix.removeEventListener("click", readHtmlClicked, false);
        onesuffix.addEventListener("click", readHtmlClicked);
      }
      if (response !== false) {
        readScrollVerseIntoView ();
      }
      if (response === false) {
        // Checksum or other error: Reload.
        readEditorLoadChapter ();
      }
    })
    .catch((error) => {
      console.log(error);
    })
    .finally(() => {
    });
  }
}


function readEditorStatus (text)
{
  if (document.body.contains(document.getElementById("onestatus"))) {
    document.querySelector ("#onestatus").innerHTML = text;
  }
}


function readEditorPollId ()
{
  const url ="../editor/id?" + new URLSearchParams([ ["bible", readBible], ["book", readBook], ["chapter", readChapter] ]).toString();
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
    if (readChapterId != 0) {
      if (response != readChapterId) {
        // The chapter identifier changed.
        // That means that likely there's updated text on the server.
        // Reload the chapter.
        readChapterChanged = true;
        readEditorLoadChapter();
      }
    }
    readChapterId = response;
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    setTimeout (readEditorPollId, 1000);
  });
}


function readScrollVerseIntoView ()
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


function readHtmlClicked (event)
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


function readSwipeLeft (event)
{
  if (is_outside_workspace()) {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function readSwipeRight (event)
{
  if (is_outside_workspace()) {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}

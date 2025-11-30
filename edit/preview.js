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


document.addEventListener("DOMContentLoaded", function(e) {
  // Make the menu to never scroll out of view.
  var bar = document.querySelector ("#editorheader");
  if (bar) {
    document.querySelector ("#workspacemenu").insertAdjacentHTML('beforeend', bar.outerHTML);
    bar.remove()
  }
  
  previewIdPollerTimeoutStart ();
});


var previewNavigationBook;
var previewNavigationChapter;
var previewNavigationVerse;


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
    previewNavigationBook = navigationBook;
    previewNavigationChapter = navigationChapter;
    previewNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    previewNavigationBook = parent.window.navigationBook;
    previewNavigationChapter = parent.window.navigationChapter;
    previewNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }

  if ((previewNavigationBook != previewLoadedBook) || (previewNavigationChapter != previewLoadedChapter)) {
    previewLoadChapter ();
  }

  previewScrollVerseIntoView ();
}


var previewLoadedBible;
var previewLoadedBook;
var previewLoadedChapter;
var previewChapterInitialized = false;


function previewLoadChapter ()
{
  previewLoadedBible = navigationBible;
  previewLoadedBook = previewNavigationBook;
  previewLoadedChapter = previewNavigationChapter;
  previewChapterIdOnServer = 0;
  if (previewChapterInitialized) location.reload ();
  else previewChapterInitialized = true;
}


var previewChapterIdOnServer = 0;
var previewChapterIdPollerTimeoutId;


function previewIdPollerTimeoutStart ()
{
  if (previewChapterIdPollerTimeoutId) clearTimeout (previewChapterIdPollerTimeoutId);
  previewChapterIdPollerTimeoutId = setTimeout (editorEditorPollId, 1000);
}


function editorEditorPollId ()
{
  const url = "../editor/id?" + new URLSearchParams([ ["bible", previewLoadedBible], ["book", previewLoadedBook], ["chapter", previewLoadedChapter] ]).toString()
  fetch(url, {
    method: "GET",
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    if (previewChapterIdOnServer != 0) {
      if (response != previewChapterIdOnServer) {
        previewLoadChapter ();
        previewChapterIdOnServer = 0;
      }
    }
    previewChapterIdOnServer = response;
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    previewIdPollerTimeoutStart ();
  });
}


function previewScrollVerseIntoView ()
{
  var verses = [0];
  var navigated = false;
  var ivs = document.querySelectorAll(".i-v");
  ivs.forEach((element) => {
    verses = element.innerText;
    if (verses.indexOf (parseInt (previewNavigationVerse)) >= 0) {
      if (navigated == false) {
        element.scrollIntoView({
          behavior: 'smooth',
          block: 'center',
          inline: 'center'
        });
        navigated = true;
      }
    }
  });
}

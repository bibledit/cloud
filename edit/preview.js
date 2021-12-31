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


$ (document).ready (function ()
{
  // Make the menu to never scroll out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);
  
  previewIdPollerTimeoutStart ();
});


var previewNavigationBook;
var previewNavigationChapter;
var previewNavigationVerse;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
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
  $.ajax ({
    url: "../editor/id",
    type: "GET",
    data: { bible: previewLoadedBible, book: previewLoadedBook, chapter: previewLoadedChapter },
    success: function (response) {
      if (previewChapterIdOnServer != 0) {
        if (response != previewChapterIdOnServer) {
          previewLoadChapter ();
          previewChapterIdOnServer = 0;
        }
      }
      previewChapterIdOnServer = response;
    },
    complete: function (xhr, status) {
      previewIdPollerTimeoutStart ();
    }
  });
}


function previewScrollVerseIntoView ()
{
  $ ("#workspacewrapper").stop ();
  var verses = [0];
  var navigated = false;
  $ (".v").each (function (index) {
    var element = $(this);
    verses = usfm_get_verse_numbers (element[0].textContent);
    if (verses.indexOf (parseInt (previewNavigationVerse)) >= 0) {
      if (navigated == false) {
        var verseTop = element.offset().top;
        var workspaceHeight = $("#workspacewrapper").height();
        var currentScrollTop = $("#workspacewrapper").scrollTop();
        var scrollTo = verseTop - (workspaceHeight / 2) + currentScrollTop;
        var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
        var upperBoundary = currentScrollTop + (workspaceHeight / 10);
        if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
          $("#workspacewrapper").animate({ scrollTop: scrollTo }, 500);
        }
        navigated = true;
      }
    }
  });
  if (editorNavigationVerse == 0) {
    $ ("#workspacewrapper").animate ({ scrollTop: 0 }, 500);
  }
}
